#include"SymmeticChiper.h"
#include<string.h>
#include<vector>
#include<openssl/aes.h>
#include<openssl/evp.h>
#include <openssl/err.h>
#include <openssl/bn.h>
namespace jxtd{
    namespace misc{
        namespace crypto{
                SymmeticChiper:: SymmeticChiper(RandomGenerator* generator):Chiper(Symmetic_type),gen(generator){
                    usrkey = "";
                    for(int i = 0;i<4;i++){
                        usrkey += gen->rand();
                    }
                    init = gen->rand();
                }
                SymmeticChiper:: ~SymmeticChiper(){
                    usrkey = "";
                    gen = nullptr;
                }
                //key不存在时主动生成密钥并返回
                std::string SymmeticChiper::get_key(){
                    if(usrkey==""){
                        for(int i = 0;i<4;i++){
                            usrkey+=gen->rand();
                        }
                    }
                    return usrkey;
                }
                void SymmeticChiper::set_key(const std::string& key){
                    usrkey = key;
                    while(usrkey.length()<64){
                        usrkey+='0';
                    }
                }
                void SymmeticChiper::reset_key(){
                    usrkey = "";
                }
                //清空现有初始化向量，并重新设置用于加密
                std::string SymmeticChiper::encrypt(const std::string& text, uint32_t mode){
                    while(usrkey.length()<64){
                        usrkey+='0';
                    }
                    init = "";
                    init = gen->rand();
                    while(init.length()<16){
                        init+='0';
                    }
                    return do_encrypt(text,mode,init);
                }
                //调用encrypt后有效，返回初始化向量（若存在），否则返回空字符串
                std::string SymmeticChiper::init_vector(){
                    return init;
                }
                void SymmeticChiper::set_init(const std::string& init){
                    this->init = init;
                    if(this->init.length()<16){
                        this->init += '0';
                    }
                }
                //使用现有初始化向量解密
                std::string SymmeticChiper::decrypt(const std::string& text,uint32_t mode){
                   return do_decrypt(text,mode,init);
                }


                //AES128
                AES128_Chiper::AES128_Chiper(RandomGenerator* generator):SymmeticChiper(generator){}
                AES128_Chiper::~AES128_Chiper(){
                    usrkey = "";
                }
                std::string AES128_Chiper::do_encrypt(const std::string& text,uint32_t mode,const std::string& init) {
                    const std::string& key = usrkey;
                    const EVP_CIPHER* cipher;
                    if (mode == 0) {
                        cipher = EVP_aes_128_ecb();
                    }
                    else if (mode == 1) {
                        cipher = EVP_aes_128_cbc();
                    }
                    else if (mode == 2) {
                        cipher = EVP_aes_128_cfb();
                    }
                    else if (mode == 3) {
                        cipher = EVP_aes_128_ofb();
                    }
                    else if (mode == 4) {
                        cipher = EVP_aes_128_ctr();
                    }
                    else {
                        return "";
                    }

                    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
                    if (ctx == nullptr) {
                        return "";
                    }

                    std::string ciphertext;
                    int len;

                    // 初始化加密操作
                    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, (const unsigned char*)key.c_str(), (const unsigned char*)init.c_str()) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }

                    // 执行加密操作
                    ciphertext.resize(text.size() + AES_BLOCK_SIZE);
                    if (EVP_EncryptUpdate(ctx, (unsigned char*)ciphertext.data(), &len, (const unsigned char*)text.c_str(), text.size()) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }
                    int ciphertext_len = len;

                    // 完成加密操作
                    if (EVP_EncryptFinal_ex(ctx, (unsigned char*)ciphertext.data() + len, &len) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }
                    ciphertext_len += len;

                    EVP_CIPHER_CTX_free(ctx);

                    ciphertext.resize(ciphertext_len);

                    return ciphertext;
                }
            
                std::string AES128_Chiper:: do_decrypt(const std::string& pass,uint32_t mode,const std::string& init){
                    const std::string& key = usrkey;
                    const EVP_CIPHER* cipher;
                    if (mode == 0) {
                        cipher = EVP_aes_128_ecb();
                    }
                    else if (mode == 1) {
                        cipher = EVP_aes_128_cbc();
                    }
                    else if (mode ==2) {
                        cipher = EVP_aes_128_cfb();
                    }
                    else if (mode == 3) {
                        cipher = EVP_aes_128_ofb();
                    }
                    else if (mode == 4) {
                        cipher = EVP_aes_128_ctr();
                    }
                    else {
                        return "";
                    }

                    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
                    if (ctx == nullptr) {
                        return "";
                    }

                    std::string plaintext;
                    int len;

                    // 初始化解密操作
                    if (EVP_DecryptInit_ex(ctx, cipher, nullptr, (const unsigned char*)key.c_str(), (const unsigned char*)init.c_str()) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }

                    // 执行解密操作
                    plaintext.resize(pass.size());
                    if (EVP_DecryptUpdate(ctx, (unsigned char*)plaintext.data(), &len, (const unsigned char*)pass.c_str(), pass.size()) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }
                    int plaintext_len = len;

                    // 完成解密操作
                    if (EVP_DecryptFinal_ex(ctx, (unsigned char*)plaintext.data() + len, &len) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }
                    plaintext_len += len;

                    EVP_CIPHER_CTX_free(ctx);

                    plaintext.resize(plaintext_len);

                    return plaintext;
                }


                //AES256
                AES256_Chiper::AES256_Chiper(RandomGenerator* generator):SymmeticChiper(generator){
                    std::string s1 = generator->rand();
                    std::string s2 = generator->rand();
                    usrkey = s1 + s2;
                }

                AES256_Chiper::~AES256_Chiper(){
                    usrkey = "";
                    init = "";
                }

                std::string AES256_Chiper:: do_encrypt(const std::string& text,uint32_t mode,const std::string& init){
                    const std::string& key = usrkey;
                    const EVP_CIPHER* cipher;
                    if (mode == 0) {
                        cipher = EVP_aes_256_ecb();
                    }
                    else if (mode == 1) {
                        cipher = EVP_aes_256_cbc();
                    }
                    else if (mode == 2) {
                        cipher = EVP_aes_256_cfb();
                    }
                    else if (mode == 3) {
                        cipher = EVP_aes_256_ctr();
                    }
                    else {
                        return "";
                    }

                    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
                    if (ctx == nullptr) {
                        return "";
                    }

                    std::string ciphertext;
                    int len;

                    // 初始化加密操作
                    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, (const unsigned char*)key.c_str(), (const unsigned char*)init.c_str()) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }

                    // 执行加密操作
                    ciphertext.resize(text.size() + AES_BLOCK_SIZE);
                    if (EVP_EncryptUpdate(ctx, (unsigned char*)ciphertext.data(), &len, (const unsigned char*)text.c_str(), text.size()) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }
                    int ciphertext_len = len;

                    // 完成加密操作
                    if (EVP_EncryptFinal_ex(ctx, (unsigned char*)ciphertext.data() + len, &len) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }
                    ciphertext_len += len;

                    EVP_CIPHER_CTX_free(ctx);

                    ciphertext.resize(ciphertext_len);

                    return ciphertext;
                }

                std::string AES256_Chiper:: do_decrypt(const std::string& pass,uint32_t mode,const std::string& init){
                    const std::string& key = usrkey;
                    const EVP_CIPHER* cipher;
                    if (mode == 0) {
                        cipher = EVP_aes_256_ecb();
                    }
                    else if (mode == 1) {
                        cipher = EVP_aes_256_cbc();
                    }
                    else if (mode == 2) {
                        cipher = EVP_aes_256_cfb();
                    }
                    else if (mode == 3) {
                        cipher = EVP_aes_256_ctr();
                    }
                    else {
                        return "";
                    }

                    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
                    if (ctx == nullptr) {
                        return "";
                    }

                    std::string plaintext;
                    int len;

                    // 初始化解密操作
                    if (EVP_DecryptInit_ex(ctx, cipher, nullptr, (const unsigned char*)key.c_str(), (const unsigned char*)init.c_str()) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }

                    // 执行解密操作
                    plaintext.resize(pass.size());
                    if (EVP_DecryptUpdate(ctx, (unsigned char*)plaintext.data(), &len, (const unsigned char*)pass.c_str(), pass.size()) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }
                    int plaintext_len = len;

                    // 完成解密操作
                    if (EVP_DecryptFinal_ex(ctx, (unsigned char*)plaintext.data() + len, &len) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }
                    plaintext_len += len;

                    EVP_CIPHER_CTX_free(ctx);

                    plaintext.resize(plaintext_len);

                    return plaintext;
                }

                //SM4
                SM4_Chiper::SM4_Chiper(RandomGenerator* generator):SymmeticChiper(generator){
                    init = "";
                }

                SM4_Chiper::~SM4_Chiper(){
                    usrkey = "";
                    init = "";
                }

                
                std::string SM4_Chiper:: do_encrypt(const std::string& text,uint32_t mode,const std::string& init){
                    const std::string& key = usrkey;
                    const EVP_CIPHER* cipher;
                    if (mode == 0) {
                        cipher = EVP_sm4_ecb();
                    }
                    else if (mode == 1) {
                        cipher = EVP_sm4_cbc();
                    }
                    else if (mode == 2) {
                        cipher = EVP_sm4_cfb();
                    }
                    else if (mode == 3) {
                        cipher = EVP_sm4_ctr();
                    }
                    else {
                        return "";
                    }

                    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
                    if (ctx == nullptr) {
                        return "";
                    }

                    std::string ciphertext;
                    int len;

                    // 初始化加密操作
                    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, (const unsigned char*)key.c_str(), (const unsigned char*)init.c_str()) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }

                    // 执行加密操作
                    ciphertext.resize(text.size() + AES_BLOCK_SIZE);
                    if (EVP_EncryptUpdate(ctx, (unsigned char*)ciphertext.data(), &len, (const unsigned char*)text.c_str(), text.size()) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }
                    int ciphertext_len = len;

                    // 完成加密操作
                    if (EVP_EncryptFinal_ex(ctx, (unsigned char*)ciphertext.data() + len, &len) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }
                    ciphertext_len += len;

                    EVP_CIPHER_CTX_free(ctx);

                    ciphertext.resize(ciphertext_len);

                    return ciphertext;
                }
                
                std::string SM4_Chiper:: do_decrypt(const std::string& pass,uint32_t mode,const std::string& init){
                    const std::string& key = usrkey;
                    const EVP_CIPHER* cipher;
                    if (mode == 0) {
                        cipher = EVP_sm4_ecb();
                    }
                    else if (mode == 1) {
                        cipher = EVP_sm4_cbc();
                    }
                    else if (mode == 2) {
                        cipher = EVP_sm4_cfb();
                    }
                    else if (mode == 3) {
                        cipher = EVP_sm4_ctr();
                    }
                    else {
                        return "";
                    }

                    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
                    if (ctx == nullptr) {
                        return "";
                    }

                    std::string plaintext;
                    int len;

                    // 初始化解密操作
                    if (EVP_DecryptInit_ex(ctx, cipher, nullptr, (const unsigned char*)key.c_str(), (const unsigned char*)init.c_str()) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }

                    // 执行解密操作
                    plaintext.resize(pass.size());
                    if (EVP_DecryptUpdate(ctx, (unsigned char*)plaintext.data(), &len, (const unsigned char*)pass.c_str(), pass.size()) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }
                    int plaintext_len = len;

                    // 完成解密操作
                    if (EVP_DecryptFinal_ex(ctx, (unsigned char*)plaintext.data() + len, &len) != 1) {
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }
                    plaintext_len += len;

                    EVP_CIPHER_CTX_free(ctx);

                    plaintext.resize(plaintext_len);

                    return plaintext;
                }    
            }
        }
    }
