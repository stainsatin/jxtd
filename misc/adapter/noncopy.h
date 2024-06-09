#pragma once
namespace jxtd{
    namespace misc{
        namespace adapter{
            class NonCopy{
                public:
                    NonCopy() =default;
                    virtual ~NonCopy() =default;
                private:
                    NonCopy(const NonCopy&) =delete;
                    NonCopy& operator=(const NonCopy&) =delete;
            };
        }
    }
}
