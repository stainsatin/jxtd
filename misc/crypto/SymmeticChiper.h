#pragma once
#include <stdint.h>
#include<string>
#include"Chiper.h"
#include"RandomGenerator.h"
namespace jxtd{
    namespace misc{
        namespace crypto{
            class SymmeticChiper:public Chiper{
                public:
                    enum{
                        ECB_Mode = 0,
                        CBC_Mode,
                        CFB_Mode,
                        OFB_Mode,
                        CTR_Mode
                    };
                    explicit SymmeticChiper(RandomGenerator* generator);
                    virtual ~SymmeticChiper();
                    //key不存在时主动生成密钥并返回
                    std::string get_key();
                    void set_key(const std::string& key);
                    void reset_key();

                    //清空现有初始化向量，并重新设置用于加密
                    std::string encrypt(const std::string& text, uint32_t mode);
                    //调用encrypt后有效，返回初始化向量（若存在），否则返回空字符串
                    std::string init_vector();
                    void set_init(const std::string& init);
                    //使用现有初始化向量解密
                    std::string decrypt(const std::string& text,uint32_t mode);
                
                protected:
                    std::string usrkey;
                    std::string init;
                private:
                    //NVI模式，实际加解密接口
                    virtual std::string do_encrypt(const std::string& text,uint32_t mode,const std::string& init)=0;
                    virtual std::string do_decrypt(const std::string& pass,uint32_t mode,const std::string& init)=0;
                    RandomGenerator* gen;
            };

            class AES128_Chiper:public SymmeticChiper{
                public:
                    explicit AES128_Chiper(RandomGenerator* generator);
                    ~AES128_Chiper();
                    //key不存在时主动生成密钥并返回
                    std::string get_key();
                private:
                    std::string do_encrypt(const std::string& text,uint32_t mode,const std::string& init);
                    std::string do_decrypt(const std::string& pass,uint32_t mode,const std::string& init);
            };

            class AES256_Chiper:public SymmeticChiper{
                public:
                    explicit AES256_Chiper(RandomGenerator* generator);
                    ~AES256_Chiper();
                    //key不存在时主动生成密钥并返回
                    std::string get_key();
                private:
                    std::string do_encrypt(const std::string& text,uint32_t mode,const std::string& init);
                    std::string do_decrypt(const std::string& pass,uint32_t mode,const std::string& init);
            };

            class SM4_Chiper:public SymmeticChiper{
                public:
                    explicit SM4_Chiper(RandomGenerator* generator);
                    ~SM4_Chiper();
                    //key不存在时主动生成密钥并返回
                    std::string get_key();
                private:
                    std::string do_encrypt(const std::string& text,uint32_t mode,const std::string& init);
                    std::string do_decrypt(const std::string& pass,uint32_t mode,const std::string& init);
            };
        }
    }
}