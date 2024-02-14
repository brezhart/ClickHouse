#include <Parsers/Access/ParserSSHList.h>
#include <Parsers/Access/ASTPublicSSHKey.h>

#include <Parsers/CommonParsers.h>
#include <Parsers/parseIdentifierOrStringLiteral.h>


namespace DB
{

namespace
{
bool parseSSHList(IParserBase::Pos & pos, Expected & expected, std::shared_ptr<ASTPublicSSHKey> & ast)
{
    return IParserBase::wrapParseImpl(pos, [&]
      {
          String key_base64;
          String type = "ssh-ed25519";
          if (!ParserKeyword{"ssh"}.ignore(pos, expected)){
              return false;
          }
          if (pos->type == TokenType::Minus){
              ++pos;
          } else {
              return false;
          }
          if (!ParserKeyword{"rsa"}.ignore(pos, expected)){
              return false;
          }

          if (!parseIdentifierOrStringLiteral(pos, expected, key_base64)){
              return false;
          }
          ast = std::make_shared<ASTPublicSSHKey>();
          ast->key_base64 = std::move(key_base64);
          ast->type = std::move(type);
          return true;
      });
}
}


bool ParserSSHList::parseImpl(Pos & pos, ASTPtr & node, Expected & expected)
{
    std::shared_ptr<ASTPublicSSHKey> res;
    if (!parseSSHList(pos, expected, res))
        return false;

    node = res;
    return true;
}

}
