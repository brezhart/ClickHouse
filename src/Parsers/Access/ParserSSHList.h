#pragma once

#include <Parsers/IParserBase.h>
#include <Parsers/Access/ASTExternalSSHList.h>


namespace DB
{

class ParserSSHList
{
public:
    static std::vector<std::pair<String, String>> parse(const ASTExternalSSHList & external_list);
};

}
