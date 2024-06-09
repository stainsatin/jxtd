#pragma once
#include <stdint.h>
namespace jxtd{
    namespace misc{
        namespace crypto{
            class Chiper{
                public:
                    enum{
                        Digest_type = 0,
                        Symmetic_type,
                        Asymmetic_type
                    };
                    explicit Chiper(uint32_t type);
                    virtual ~Chiper()=default;
                    bool is_digest() const;
                    bool is_symmetic() const;
                    bool is_asymmetic() const;
                    uint32_t get_type() const;
                private:
                    const uint32_t type;
            };
        }
    }
}