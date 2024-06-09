#pragma once
#include <stdint.h>
#include<string>
#include"Chiper.h"
#include"RandomGenerator.h"
#include<openssl/evp.h>
#include<openssl/rsa.h>
namespace jxtd{
    namespace misc{
        namespace crypto{
            class AsymmeticChiper:public Chiper{
                public:
                   explicit AsymmeticChiper(RandomGenerator* generator);
                   virtual ~AsymmeticChiper();

                   //pub和prv一旦获取立刻同时生成，pub可以从prv中计算得到。
                   //pub
                   std::string get_pubkey();
                   //prv
                   std::string get_prvkey();
                   bool set_prvkey(const std::string& key);
                   void reset_prvkey();

                   //cert:签发模式，点亮为私钥加密，否则为公钥
                   std::string encrypt(const std::string& text,bool cert=true);
                   //reply:回复模式，点亮为私钥解密，否则为公钥
                   std::string decrypt(const std::string& pass,bool reply=false);
                
                protected:
                    std::string pbk;
                    std::string prk;
                private:
                   virtual std::string do_encrypt_pubkey(const std::string& text, const std::string& pubkey)=0;
                   virtual std::string do_encrypt_prvkey(const std::string& text, const std::string& prvkey)=0;
                   virtual std::string do_decrypt_pubkey(const std::string& text, const std::string& pubkey)=0;
                   virtual std::string do_decrypt_prvkey(const std::string& text, const std::string& prvkey)=0;

            };

            class RSA_Chiper:public AsymmeticChiper{
                public:
                   explicit RSA_Chiper(RandomGenerator* generator);
                   ~RSA_Chiper();

                private:
                   std::string do_encrypt_pubkey(const std::string& text, const std::string& pubkey)override;
                   std::string do_encrypt_prvkey(const std::string& text, const std::string& prvkey)override;
                   std::string do_decrypt_pubkey(const std::string& text, const std::string& pubkey)override;
                   std::string do_decrypt_prvkey(const std::string& text, const std::string& prvkey)override;
                   RSA* rsa;
                   BIGNUM* bn;
            };
        }   
    }
}