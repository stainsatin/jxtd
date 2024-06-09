#pragma once
#include<string>
#include<vector>
#include<misc/parser/at/C_ATMessage.h>
#include<misc/parser/Parser.h>
extern "C"{
extern struct C_ATMessage* ATParser_parse_entrance(const char* text ,int length);
extern bool ATParser_get_error();
}
namespace jxtd{
    namespace misc{
        namespace parser{
            namespace at{
                struct ATMessage{
                    ATMessage_Type type;
                    std::string cmd;
                    std::vector<std::string> param;
                    std::string data; 
                };
                class ATParser:public ::jxtd::misc::parser::Parser<ATMessage>{
                    public:
                        explicit ATParser();
                        explicit ATParser(const std::string& str);
                        explicit ATParser(FILE* file);
                        ~ATParser();
                        bool is_correct() const override;
		        bool is_parsed() const override;
		        ATMessage* parse() override;
                    private:
                        ATMessage* result;
		        };
	    }
	}
    }
}
