#pragma once

#include <Parsers/IParserBase.h>


namespace DB
{

class ParserExternalSSHList : public IParserBase
{
protected:
    const char * getName() const override { return "ExternalSSHList"; }
    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override;
};

}
