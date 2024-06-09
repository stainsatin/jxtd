#include"DigestChiper.h"
#include<cstring>
#include<openssl/md5.h>
#include<openssl/sha.h>
#include<openssl/aes.h>
#include<openssl/evp.h>
namespace jxtd{
    namespace misc{
        namespace crypto{
            DigestChiper::DigestChiper():Chiper(Digest_type){
            }

            Md5_Digest::Md5_Digest():DigestChiper(){
            }
            std::string Md5_Digest::digest(const std::string& text){
                unsigned int len = 0;
                EVP_MD_CTX* ctx = EVP_MD_CTX_new();
                EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);
                // hash计算
                EVP_DigestUpdate(ctx, text.c_str(),
                    text.length());
                unsigned char result[MD5_DIGEST_LENGTH] = {};
                EVP_DigestFinal_ex(ctx, result, &len);
                EVP_MD_CTX_free(ctx);
                std::string res = (char*)result;
                res = res.substr(0, MD5_DIGEST_LENGTH);
                return res;
            }

            SHA1_Digest::SHA1_Digest():DigestChiper(){
            }
            std::string SHA1_Digest::digest(const std::string& text){
                unsigned int len = 0;
                EVP_MD_CTX* ctx = EVP_MD_CTX_new();
                EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr);
                // hash计算
                EVP_DigestUpdate(ctx, text.c_str(),
                    text.length());
                unsigned char result[SHA_DIGEST_LENGTH] = {};
                EVP_DigestFinal_ex(ctx, result, &len);
                EVP_MD_CTX_free(ctx);
                std::string res = (char*)result;
                res = res.substr(0, SHA_DIGEST_LENGTH);
                return res;
            }

            SHA256_Digest::SHA256_Digest():DigestChiper(){
            }
            std::string SHA256_Digest::digest(const std::string& text){
                unsigned int len = 0;
                EVP_MD_CTX* ctx = EVP_MD_CTX_new();
                EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
                // hash计算
                EVP_DigestUpdate(ctx, text.c_str(),
                    text.length());
                unsigned char result[SHA256_DIGEST_LENGTH] = {};
                EVP_DigestFinal_ex(ctx, result, &len);
                EVP_MD_CTX_free(ctx);
                std::string res = (char*)result;
                res = res.substr(0, SHA256_DIGEST_LENGTH);
                return res;
            }
            
            SHA512_Digest::SHA512_Digest():DigestChiper(){
            }
            std::string SHA512_Digest::digest(const std::string& text){
                unsigned int len = 0;
                EVP_MD_CTX* ctx = EVP_MD_CTX_new();
                EVP_DigestInit_ex(ctx, EVP_sha512(), nullptr);
                // hash计算
                EVP_DigestUpdate(ctx, text.c_str(),
                    text.length());
                unsigned char result[SHA512_DIGEST_LENGTH] = {};
                EVP_DigestFinal_ex(ctx, result, &len);
                EVP_MD_CTX_free(ctx);
                std::string res = (char*)result;
                res = res.substr(0, SHA512_DIGEST_LENGTH);
                return res;
            }

            SM3_Digest::SM3_Digest():DigestChiper(){
            }
            std::string SM3_Digest::digest(const std::string& text){
                unsigned int len = 0;
                EVP_MD_CTX* ctx = EVP_MD_CTX_new();
                EVP_DigestInit_ex(ctx, EVP_sm3(), nullptr);
                // hash计算
                EVP_DigestUpdate(ctx, text.c_str(),
                    text.length());
                unsigned char result[32] = {};
                EVP_DigestFinal_ex(ctx, result, &len);
                EVP_MD_CTX_free(ctx);
                std::string res = (char*)result;
                res = res.substr(0,32);
                return res;
            }
        }
    }
}