#include <Parsers/Access/ParserSSHList.h>
#include <Parsers/Access/ASTPublicSSHKey.h>
#include <Parsers/Access/ASTExternalSSHList.h>
#include <IO/ReadWriteBufferFromHTTP.h>

#include <Parsers/CommonParsers.h>
#include <Parsers/parseIdentifierOrStringLiteral.h>

namespace DB
{
std::vector<std::pair<String, String>> ParserSSHList::parse(const ASTExternalSSHList & external_list)
{
    auto login = external_list.login;
    auto service = external_list.service;
    Poco::URI uri("http://" + service + ".com/" + login + ".keys");
    Poco::Net::HTTPBasicCredentials creds{};

    std::unique_ptr<ReadWriteBufferFromHTTP> in = std::make_unique<ReadWriteBufferFromHTTP>(
        uri, Poco::Net::HTTPRequest::HTTP_GET, nullptr, ConnectionTimeouts{}, creds, DBMS_DEFAULT_BUFFER_SIZE, 2);
    size_t file_size = in->getFileInfo().file_size.value();
    char * buff = static_cast<char *>(malloc(file_size));

    [[maybe_unused]] size_t readed = in->read(buff, file_size);
    std::vector<std::pair<String, String>> sshKeyTypeValues;
    std::string curr;
    bool itr = false;
    for (size_t j = 0; j <= file_size; j++)
    {
        if (j == file_size || isspace(buff[j]))
        {
            if (curr.size() > 0)
            {
                if (!itr)
                {
                    sshKeyTypeValues.push_back({"", ""});
                    sshKeyTypeValues.back().first = curr;
                }
                else
                {
                    sshKeyTypeValues.back().second = curr;
                }
                itr ^= true;
                curr.clear();
            }
        }
        else
        {
            curr += buff[j];
        }
    }
    free(buff);
    return sshKeyTypeValues;
}
}
