#include <Parsers/Access/ParserExternalSSHList.h>
#include <Parsers/Access/ASTExternalSSHList.h>

#include <Parsers/CommonParsers.h>
#include <Parsers/parseIdentifierOrStringLiteral.h>


namespace DB
{

namespace
{
bool parseExternalSSHList(IParserBase::Pos & pos, Expected & expected, std::shared_ptr<ASTExternalSSHList> & ast)
{
    return IParserBase::wrapParseImpl(pos, [&]
      {
          String login;
          if (!ParserKeyword{"account"}.ignore(pos, expected) || !parseIdentifierOrStringLiteral(pos, expected, login))
              return false;

          String service;
          if (!ParserKeyword{"from"}.ignore(pos, expected) || !parseIdentifierOrStringLiteral(pos, expected, service))
              return false;

          ast = std::make_shared<ASTExternalSSHList>();
          ast->login = std::move(login);
          ast->service = std::move(service);
          return true;
      });
}
}


bool ParserExternalSSHList::parseImpl(Pos & pos, ASTPtr & node, Expected & expected)
{
    std::shared_ptr<ASTExternalSSHList> res;
    if (!parseExternalSSHList(pos, expected, res))
        return false;

    node = res;
    return true;
}

}
