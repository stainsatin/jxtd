#include"RandomGenerator.h"
#include<openssl/rand.h>
#include<ctime>
namespace jxtd{
    namespace misc{
        namespace crypto{
            RandomGenerator::RandomGenerator():seed(""){
            }
            RandomGenerator::~RandomGenerator(){
                seed = "";
            }
            RandomGenerator::RandomGenerator(RandomGenerator&& other)noexcept{
                this->seed = other.seed;
                other.seed = "";
            }
            RandomGenerator& RandomGenerator:: operator=(RandomGenerator&& other)noexcept{
                this->seed = other.seed;
                other.seed = "";
                return *this;
            }

            //LCG
            LCG_Random::LCG_Random():RandomGenerator(),num(time(0)){
                std::srand(num);
            }
            LCG_Random::~LCG_Random(){
                num = 0;
                seed = "";
            }
            LCG_Random::LCG_Random(LCG_Random&& other)noexcept:RandomGenerator(std::move(other)){
                this->num = other.num;
                other.reset();
                std::srand(num);
            }
            LCG_Random& LCG_Random::operator=(LCG_Random&& other)noexcept{
                this->seed = other.seed;
                this->num = other.num;
                std::srand(num);
                other.reset();
                return *this;
            }
            void LCG_Random::reset(){
                num = 0;
                seed = "";
                std::srand(num);
            }
            std::string LCG_Random::rand(){
                std:: string rand_seq="";
                for(int i=0;i<16;i++){
                    int tmp = std::rand()%26+97;
                    rand_seq = rand_seq+(char)tmp;
                }
                return rand_seq;
            }

            void LCG_Random::srand(const std::string& seed){
                this->seed = seed;
                num = 0;
                for(char a:seed){
                    num+= static_cast<int>(a);
                }
                std::srand(num);
            }
            //生成器熵补充
            void LCG_Random::entropy(const std::string& extra){
                srand(extra+this->seed);
            }

                //DRBG
                DRBG_Random::DRBG_Random():RandomGenerator(){
                    RAND_seed(static_cast<const void*>(seed.c_str()),seed.length());
                }
                DRBG_Random::~DRBG_Random(){
                    this->seed = "";
                    RAND_seed(static_cast<const void*>(seed.c_str()),seed.length());
                }
                DRBG_Random::DRBG_Random(DRBG_Random&& other)noexcept:RandomGenerator(std::move(other)){
                    RAND_seed(static_cast<const void*>(seed.c_str()),seed.length());
                }
                DRBG_Random& DRBG_Random::operator=(DRBG_Random&& other)noexcept{
                    this->seed = other.seed;
                    other.seed = "";
                    RAND_seed(static_cast<const void*>(seed.c_str()),seed.length());
                    return *this;
                }
                //生成器状态重置
                void DRBG_Random:: reset(){
                    this->seed = "";
                    RAND_seed(static_cast<const void*>(seed.c_str()),seed.length());
                }
                //熵不足时仅返回可用部分
                std::string DRBG_Random::rand(){
                    int ret=RAND_status();
                    unsigned char* buf = new unsigned char[16];
                    if(ret==1){
                        RAND_bytes(buf,16);
                    }
                    else {
                        RAND_poll();
                        RAND_bytes(buf,16);
                    }
                    char rand_seq[16];
                    for(int i =0;i<16;i++){
                        rand_seq[i] = buf[i];
                    }
                    std::string rand_str(rand_seq);
                    delete[]buf;
                    return rand_str;
                }
                //重设种子
                void DRBG_Random::srand(const std::string& seed){
                    this->seed = seed;   
                    RAND_seed(static_cast<const void*>(seed.c_str()),seed.size());
                }
                //生成器熵补充
                void DRBG_Random::entropy(const std::string& extra){
                    RAND_add(static_cast<const void*>(extra.c_str()),extra.size(),extra.size());
                }
        }
    }
}