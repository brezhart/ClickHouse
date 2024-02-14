#pragma once

#include <Parsers/IAST.h>


namespace DB
{

class ASTExternalSSHList : public IAST
{
public:
    String login;
    String service;

    ASTExternalSSHList() = default;
    ASTExternalSSHList(String login_, String service_)
        : login(login_)
        , service(service_)
    {}
    String getID(char) const override { return "PublicSSHKey"; }
    ASTPtr clone() const override { return std::make_shared<ASTExternalSSHList>(*this); }
    void formatImpl(const FormatSettings & settings, FormatState &, FormatStateStacked) const override;
};

}
