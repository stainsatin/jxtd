#include"ATParser.h"
#include<string>
#include<vector>
#include"C_ATMessage.h"
#include<misc/parser/Parser.h>
#include<malloc.h>
#include<cstring>

jxtd::misc::parser::at::ATParser::ATParser():Parser(){
    this -> result = new ATMessage;
    this -> result -> type = NON;
}

jxtd::misc::parser::at::ATParser::ATParser(FILE* file):Parser(file){
    this -> result = new ATMessage;
    this -> result -> type = NON;
}

jxtd::misc::parser::at::ATParser::ATParser(const std::string& str):Parser(str){
    this -> result = new ATMessage;
    result -> type = NON;
}

jxtd::misc::parser::at::ATParser::~ATParser(){
    delete result;
}

bool jxtd::misc::parser::at::ATParser::is_correct() const{
    return correct_sign;
}

bool jxtd::misc::parser::at::ATParser::is_parsed() const{
        return parsed_sign; 
}

jxtd::misc::parser::at::ATMessage* jxtd::misc::parser::at::ATParser::parse(){
    if(parsed_sign == true)
        return result;
    else{
	const char* words_const_char = words.c_str();
	struct C_ATMessage* temporary_result = ATParser_parse_entrance(words_const_char ,words.size());
        correct_sign = ATParser_get_error();
        if(correct_sign){
            result -> type = temporary_result -> type;
            if(temporary_result -> cmd){
                this -> result -> cmd = temporary_result -> cmd;
                free(temporary_result -> cmd);
	        temporary_result -> cmd = NULL;
	    }
            if(temporary_result -> param){
                for(int i = 0 ;i < temporary_result -> param_num ;i++){
		    std::string temporary_param = temporary_result -> param[i];
		    this -> result -> param.push_back(temporary_param);
		    free(temporary_result -> param[i]);
                }
                free(temporary_result -> param);
	        temporary_result -> param = NULL;
            }
            if(temporary_result -> data){
		const char* temporary_data = const_cast<char*>(temporary_result -> data);
		this -> result -> data.insert(0 ,temporary_data ,temporary_result -> data_leng);
		free(temporary_result -> data);
	        temporary_result -> data = NULL;
	    }
        }
        parsed_sign = true;
        return this -> result;
    }
}
