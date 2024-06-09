#include<stdlib.h>
#include<string.h>
#include"action.h"
#include"C_ATMessage.h"

static struct C_ATMessage result;

void C_ATMessage_init(){
    result.param = NULL;
    result.data = NULL;
    result.cmd = NULL;
    result.type = NON;
    result.param_num = 0;
    result.data_leng = 0;
}

void ATMessage_set_type(enum ATMessage_Type type){
    result.type = type;
}

void ATMessage_set_cmd(const char* cmd){
    result.cmd = (char*)malloc((strlen(cmd) + 1) * sizeof(char));
    strcpy(result.cmd ,cmd);
}

void ATMessage_add_param(const char* param){
    if(result.param_num > 0)
        result.param = (char**)realloc(result.param ,(result.param_num + 1) * sizeof(char*));
    else
        result.param = (char**)malloc((result.param_num + 1) * sizeof(char*));
    result.param[result.param_num] = (char*)malloc((strlen(param) + 1) * sizeof(char));
    strcpy(result.param[result.param_num] ,param);
    ++result.param_num;
}

void ATMessage_set_data(const char* data ,int data_leng){
    result.data = (char*)malloc((data_leng) * sizeof(char));
    memcpy(result.data ,data ,data_leng);
    result.data_leng = data_leng;
}

void ATMessage_stc_error(){
    if(result.param){
        for(int i = 0 ;i < result.param_num ;i++)
            free(result.param[i]);
        free(result.param);
    }
    if(result.data)
        free(result.data);
    if(result.cmd)
	free(result.cmd);
}

struct C_ATMessage* ATMessage_stc_end(){
    return &result;
}
