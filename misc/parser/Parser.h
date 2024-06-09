#pragma once
#include<string>

namespace jxtd{
    namespace misc{
        namespace parser{
			template <typename ParserResult>
	    class Parser{
                public:
                    Parser(): parsed_sign(false) ,correct_sign(false){
	       	    };
	    	    virtual ~Parser() =default;
		    explicit Parser(FILE* file): parsed_sign(false) ,correct_sign(false){
                        if(file){
                            fseek(file ,0 ,SEEK_END);
                            long fileSize = ftell(file);
                            rewind(file);
                            if(fileSize > 0){
                                words.resize(fileSize);
                                fread(&words[0] ,1 ,fileSize ,file);
                            }
                        }
    	            };
	            explicit Parser(const std::string& str): parsed_sign(false) ,correct_sign(false) ,words(str){
		    };
	            virtual bool is_correct() const{
		        return false;
		    };
	            virtual bool is_parsed() const{
		        return true;
		    };
	            virtual ParserResult* parse(){
		        return nullptr;
		    };
		protected:
		    bool correct_sign;
		    bool parsed_sign;
	            std::string words;
	    };
	}
    }
}
