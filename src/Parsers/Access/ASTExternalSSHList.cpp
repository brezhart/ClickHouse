#include <Parsers/Access/ASTExternalSSHList.h>
#include <Common/quoteString.h>
#include <IO/Operators.h>


namespace DB
{

void ASTExternalSSHList::formatImpl(const FormatSettings & settings, FormatState &, FormatStateStacked) const
{
    settings.ostr << "LOGIN ";
    settings.ostr << backQuoteIfNeed(login) << ' ';
    settings.ostr << "SERVICE ";
    settings.ostr << backQuoteIfNeed(service);
}

}
