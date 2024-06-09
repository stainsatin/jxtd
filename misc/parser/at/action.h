#pragma once
#include"C_ATMessage.h"

void C_ATMessage_init();
void ATMessage_set_type(enum ATMessage_Type type);
void ATMessage_set_cmd(const char* cmd);
void ATMessage_add_param(const char* param);
void ATMessage_set_data(const char* data ,int data_leng);
void ATMessage_stc_error();
struct C_ATMessage* ATMessage_stc_end();
