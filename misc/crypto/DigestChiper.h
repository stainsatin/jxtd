#pragma once
#include <stdint.h>
#include<string>
#include"Chiper.h"
namespace jxtd{
    namespace misc{
        namespace crypto{
            class DigestChiper:public Chiper{
                public:
                    DigestChiper();
                    virtual ~DigestChiper() = default;
                    virtual std::string digest(const std::string& text)=0;
            };

            class Md5_Digest:public DigestChiper{
                public:
                    Md5_Digest();
                    std::string digest(const std::string& text)override;
            };

            class SHA1_Digest:public DigestChiper{
                public:
                    SHA1_Digest();
                    std::string digest(const std::string& text)override;
            };

            class SHA256_Digest:public DigestChiper{
                public:
                    SHA256_Digest();
                    std::string digest(const std::string& text)override;
            };

             class SHA512_Digest:public DigestChiper{
                public:
                    SHA512_Digest();
                    std::string digest(const std::string& text)override;
            };

            class SM3_Digest:public DigestChiper{
                public:
                    SM3_Digest();
                    std::string digest(const std::string& text)override;
            };
        }
    }
}