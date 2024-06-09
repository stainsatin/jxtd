//这里我是将at指令分为指令类型type,指令内容cmd,指令参数param以及指令数据data
#pragma once
enum ATMessage_Type{
    NON,//还没有进行解析时的类型
    CMD,
    SEARCH,
    TEST,
    LONG//对应长数据类型
};

struct C_ATMessage {
    enum ATMessage_Type type;
    char* cmd;
    char** param;//数据按照顺序依次往下存,如果没有的话就是NULL
    char* data;//没有的话就是NULL
    int param_num;//这里标识参数数目
    int data_leng;
};
