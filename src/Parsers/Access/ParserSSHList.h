#pragma once

#include <Parsers/IParserBase.h>


namespace DB
{

class ParserSSHList : public IParserBase
{
protected:
    const char * getName() const override { return "SSHList"; }
    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override;
};

}
