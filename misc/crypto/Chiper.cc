#include"Chiper.h"
#include <stdint.h>
namespace jxtd{
    namespace misc{
        namespace crypto{
            Chiper::Chiper(uint32_t type):type(type){

            }
            bool Chiper::is_digest() const{
                return this->type == Digest_type;
            }
            bool Chiper::is_symmetic() const{
                return this->type == Symmetic_type;
            }
            bool Chiper::is_asymmetic() const{
                return this->type == Asymmetic_type;
            }
            uint32_t Chiper::get_type() const{
                return this->type;
            }
        }
    }
}