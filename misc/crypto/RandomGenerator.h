#pragma once
#include <stdint.h>
#include<string>
#include<random>
#include"misc/adapter/noncopy.h"
namespace jxtd{
    namespace misc{
        namespace crypto{
            class RandomGenerator:public jxtd::misc::adapter::NonCopy{
                public:
                    RandomGenerator();
                    virtual ~RandomGenerator();
                    RandomGenerator(RandomGenerator&& other)noexcept;
                    RandomGenerator& operator=(RandomGenerator&& RandomGenerator)noexcept;
                    //生成器状态重置
                    virtual void reset()=0;
                    //熵不足时仅返回可用部分
                    virtual std::string rand()=0;
                    //重设种子
                    virtual void srand(const std::string& seed)=0;
                    //生成器熵补充
                    virtual void entropy(const std::string& extra)=0;
                protected:
                    std::string seed;
            };

             class LCG_Random:public RandomGenerator{
                public:
                    LCG_Random();
                    ~LCG_Random();
                    LCG_Random(LCG_Random&& other)noexcept;
                    LCG_Random& operator=(LCG_Random&& other)noexcept;
                    //生成器状态重置
                    void reset()override;
                    //熵不足时仅返回可用部分
                    std::string rand()override;
                    //重设种子
                    void srand(const std::string& seed)override;
                    //生成器熵补充
                    void entropy(const std::string& extra)override;
                private:
                    int num;
            };

            class DRBG_Random:public RandomGenerator{
                public:
                    DRBG_Random();
                    ~DRBG_Random();
                    DRBG_Random(DRBG_Random&& other)noexcept;
                    DRBG_Random& operator=(DRBG_Random&& other)noexcept;
                    //生成器状态重置
                    void reset()override;
                    //熵不足时仅返回可用部分
                    std::string rand()override;
                    //重设种子
                    void srand(const std::string& seed)override;
                    //生成器熵补充
                    void entropy(const std::string& extra)override;
            };
        }
    }
}