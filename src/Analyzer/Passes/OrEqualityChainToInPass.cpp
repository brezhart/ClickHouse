#include <Analyzer/Passes/OrEqualityChainToInPass.h>

#include <Functions/FunctionFactory.h>

#include <Analyzer/InDepthQueryTreeVisitor.h>
#include <Analyzer/FunctionNode.h>
#include <Analyzer/ConstantNode.h>
#include <Analyzer/HashUtils.h>

namespace DB
{

class OrEqualityChainToInVisitor : public InDepthQueryTreeVisitorWithContext<OrEqualityChainToInVisitor>
{
public:
    using Base = InDepthQueryTreeVisitorWithContext<OrEqualityChainToInVisitor>;

    explicit OrEqualityChainToInVisitor(ContextPtr context)
        : Base(std::move(context))
    {}


    void visitImpl(QueryTreeNodePtr & node)
    {
        auto * function_node = node->as<FunctionNode>();

        if (!function_node || function_node->getFunctionName() != "or")
            return;

        QueryTreeNodes or_operands;

        QueryTreeNodePtrWithHashMap<QueryTreeNodes> node_to_equals_functions;

        for (const auto & argument : function_node->getArguments())
        {
            auto * argument_function = argument->as<FunctionNode>();
            if (!argument_function)
            {
                or_operands.push_back(argument);
                continue;
            }

            /// collect all equality checks (x = value)
            if (argument_function->getFunctionName() != "equals")
            {
                or_operands.push_back(argument);
                continue;
            }

            const auto & equals_arguments = argument_function->getArguments().getNodes();
            const auto & lhs = equals_arguments[0];

            const auto * rhs_literal = equals_arguments[1]->as<ConstantNode>();
            if (!rhs_literal)
            {
                or_operands.push_back(argument);
                continue;
            }

            node_to_equals_functions[lhs].push_back(argument);
        }

        auto in_function_resolver = FunctionFactory::instance().get("in", getContext());

        for (auto & [lhs, equals_functions] : node_to_equals_functions)
        {
            const auto & settings = getSettings();
            if (equals_functions.size() < settings.optimize_min_equality_disjunction_chain_length && !lhs.node->getResultType()->lowCardinality())
            {
                std::move(equals_functions.begin(), equals_functions.end(), std::back_inserter(or_operands));
                continue;
            }

            Tuple args;
            args.reserve(equals_functions.size());
            /// first we create tuple from RHS of equals functions
            for (const auto & equals : equals_functions)
            {
                const auto * equals_function = equals->as<FunctionNode>();
                assert(equals_function && equals_function->getFunctionName() == "equals");
                const auto * rhs_literal = equals_function->getArguments().getNodes()[1]->as<ConstantNode>();
                assert(rhs_literal);
                args.push_back(rhs_literal->getValue());
            }

            auto rhs_node = std::make_shared<ConstantNode>(std::move(args));

            auto in_function = std::make_shared<FunctionNode>("in");

            QueryTreeNodes in_arguments;
            in_arguments.reserve(2);
            in_arguments.push_back(lhs.node);
            in_arguments.push_back(std::move(rhs_node));

            in_function->getArguments().getNodes() = std::move(in_arguments);
            in_function->resolveAsFunction(in_function_resolver);

            or_operands.push_back(std::move(in_function));
        }

        if (or_operands.size() == 1)
            or_operands.push_back(std::make_shared<ConstantNode>(static_cast<UInt8>(0)));

        auto or_function_resolver = FunctionFactory::instance().get("or", getContext());
        function_node->getArguments().getNodes() = std::move(or_operands);
        function_node->resolveAsFunction(or_function_resolver);
    }
};


void OrEqualityChainToInPass::run(QueryTreeNodePtr query_tree_node, ContextPtr context)
{
    OrEqualityChainToInVisitor visitor(std::move(context));
    visitor.visit(query_tree_node);
}

}
