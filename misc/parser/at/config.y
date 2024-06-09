%{
#include"misc/parser/at/action.h"
#include<string.h>
#include<stdbool.h>
#include"misc/parser/at/C_ATMessage.h"
#include<stdio.h>
#include<stdlib.h>
int yylex(void);
int* yy_scan_bytes(const char* bytes ,int len);
void yy_delete_buffer(int* buffer);
extern char* yytext;
void yyerror(const char* msg);
extern int yyleng;
static struct C_ATMessage* parsed_result;
static bool error_state;
static int data_len;
%}
%union{
     char strval[8192];
}
%token<strval> AT COMMAND EQUAL QST PARAM DATAEND END DATA STARTPARAM LASTPARAM SINGLEPARAM ERROR;
%type<strval> next param data
%start begin

%%
begin
     : AT {C_ATMessage_init();} COMMAND {ATMessage_set_cmd(yytext + 1);} next
     ;

next
    : data
    | EQUAL QST {ATMessage_set_type(SEARCH);} END {parsed_result = ATMessage_stc_end();}
    | QST {ATMessage_set_type(TEST);} END {parsed_result = ATMessage_stc_end();}
    | SINGLEPARAM {ATMessage_add_param(yytext + 1);} data
    | STARTPARAM {ATMessage_add_param(yytext + 1);} param data
    ;

param
     : PARAM {ATMessage_add_param(yytext + 1);} param
     | LASTPARAM {ATMessage_add_param(yytext +1);}
     ;

data
    : END {ATMessage_set_type(CMD); parsed_result = ATMessage_stc_end();}
    | DATA {ATMessage_set_data(yytext + 2 ,yyleng - 2);} DATAEND {ATMessage_set_type(LONG); parsed_result = ATMessage_stc_end();}
    ;
%%

void yyerror(const char* msg){
    error_state = false;
    ATMessage_stc_error();
    C_ATMessage_init();
    parsed_result = ATMessage_stc_end();
}

bool ATParser_get_error(){
    return error_state;
}

struct C_ATMessage* ATParser_parse_entrance(const char* text ,int length){
    error_state = true;
    int* YY_BUFFER_STATE = yy_scan_bytes(text ,length);
    yyparse();
    yy_delete_buffer(YY_BUFFER_STATE);
    return parsed_result;
}
