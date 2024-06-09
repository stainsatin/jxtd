#include"AsymmeticChiper.h"
#include<openssl/pem.h>
#include<openssl/err.h>
#include<openssl/rsa.h>
#define KEY_LENGTH  4096             
namespace jxtd{
    namespace misc{
        namespace crypto{
            AsymmeticChiper::AsymmeticChiper(RandomGenerator* generator):Chiper(Asymmetic_type){
                prk = generator->rand();
                pbk = generator->rand();
            }
            AsymmeticChiper:: ~AsymmeticChiper(){
                prk = "";
                pbk = "";
            }
            
            //pub和prv一旦获取立刻同时生成，pub可以从prv中计算得到。
            //pub
            std::string AsymmeticChiper::get_pubkey(){
                return pbk;
            }
            //prv
            std::string AsymmeticChiper::get_prvkey(){
                return prk;
            }
            bool AsymmeticChiper::set_prvkey(const std::string& key){
                this->prk = key;
                return true;
            }
            void AsymmeticChiper::reset_prvkey(){
                prk = "";
                pbk = "";
            }

            //cert:签发模式，点亮为私钥加密，否则为公钥
            std::string AsymmeticChiper::encrypt(const std::string& text,bool cert){
                if(cert==true)
                    return do_encrypt_prvkey(text,this->prk);
                else
                    return do_decrypt_pubkey(text, this->pbk);
            }
            //reply:回复模式，点亮为私钥解密，否则为公钥
            std::string AsymmeticChiper::decrypt(const std::string& pass,bool reply){
                if(reply==true)
                    return do_decrypt_prvkey(pass,this->prk);
                else
                    return do_decrypt_pubkey(pass,this->pbk);
            }

            RSA_Chiper::RSA_Chiper(RandomGenerator* generator):AsymmeticChiper(generator){
                this->rsa =  RSA_new();
                this->bn = BN_new();
                int key = 0;
                for(int i=0;i<pbk.size();i++){
                    key+=(int)pbk[i];
                    key+=(int)prk[i];
                }
                int ret = BN_set_word(bn,key);
                ret = RSA_generate_key_ex(rsa, 512, bn, nullptr);
                while(ret!=1){
                    ret=BN_set_word(bn,RSA_F4);
                    ret = RSA_generate_key_ex(rsa,512,bn,NULL);
                }   
            }

            RSA_Chiper::~RSA_Chiper(){
                pbk = "";
                prk = "";
                RSA_free(rsa);
                BN_free(bn);
            }

            std::string RSA_Chiper::do_encrypt_prvkey(const std::string& data, const std::string& prvkey){
                //keysize=64字节512bit
                //输入数据大小=64-11=53
                int blocksize = RSA_size(rsa) - RSA_PKCS1_PADDING_SIZE;
                int outsize = 0;
                int datasize = data.size();
                unsigned char outdata[1024];
                for (int i = 0; i < data.size(); i += blocksize)
                {
                    int ensize = blocksize;
                    if (datasize - i < blocksize)
                    {
                        ensize = datasize - i;
                    }
                    int outoff = i + RSA_PKCS1_PADDING_SIZE * (i / blocksize);
                    int ret = RSA_private_encrypt(ensize, (unsigned char*)data.c_str() + i,
                        outdata + outoff, rsa, RSA_PKCS1_PADDING);
                    if (ret < 0)
                    {
                        return "";
                    }
                    outsize = outoff + RSA_size(rsa);
                }
                std::string res((char*)outdata, outsize);
                return res;
            }

            std::string RSA_Chiper::do_encrypt_pubkey(const std::string& data, const std::string& pubkey){
                int blocksize = RSA_size(rsa) - RSA_PKCS1_PADDING_SIZE;
                int outsize = 0;
                int datasize = data.size();
                unsigned char outdata[1024];
                for (int i = 0; i < data.size(); i += blocksize)
                {
                    int ensize = blocksize;
                    if (datasize - i < blocksize)
                    {
                        ensize = datasize - i;
                    }
                    int outoff = i + RSA_PKCS1_PADDING_SIZE * (i / blocksize);
                    int ret = RSA_public_encrypt(ensize, (unsigned char*)data.c_str() + i,
                        outdata + outoff, rsa, RSA_PKCS1_PADDING);
                    if (ret < 0)
                    {
                        return "";
                    }
                    outsize = outoff + RSA_size(rsa);
                }
                std::string res((char*)outdata, outsize);
                return res;
            }

            std::string RSA_Chiper::do_decrypt_pubkey(const std::string& data, const std::string& pubkey){
                int ensize = RSA_size(rsa);
                int outoff = 0;
                int datasize = data.size();
                unsigned char outdata[1024];
                for (int i = 0; i < datasize; i += ensize)
                {
                    int len = RSA_public_decrypt(ensize, (unsigned char*)data.c_str() + i,
                        outdata + outoff, rsa, RSA_PKCS1_PADDING);
                    outoff += len;
                }
                std::string res((char*)outdata, outoff);
                return res;
            }

            std::string RSA_Chiper::do_decrypt_prvkey(const std::string& data, const std::string& prvkey){
                int ensize = RSA_size(rsa);
                int outoff = 0;
                int datasize = data.size();
                unsigned char outdata[1024];
                for (int i = 0; i < datasize; i += ensize)
                {
                    int len = RSA_private_decrypt(ensize, (unsigned char*)data.c_str() + i,
                        outdata + outoff, rsa, RSA_PKCS1_PADDING);
                    outoff += len;
                }
                std::string res((char*)outdata, outoff);
                return res;
            }

            
        }
    }
}