#pragma once
#include<gflags/gflags.h>
DECLARE_string(stor_api_address);
DECLARE_int32(stor_api_port);
DECLARE_string(stor_api_cert);
DECLARE_string(stor_api_key);
DECLARE_string(stor_api_ca);
DECLARE_string(stor_comp_token);//必填项
DECLARE_string(stor_mana_token);//必填项
DECLARE_string(stor_stor_token);//默认值为空，值为空时随机生成128长度token

//数据传输
DECLARE_string(stor_datatrans_addr);
DECLARE_int32(stor_datatrans_port);
DECLARE_string(stor_datatrans_cert);
DECLARE_string(stor_datatrans_key);
DECLARE_string(stor_datatrans_ca);

//存储配置
DECLARE_string(stor_backend);//任务书上面写的是local，我怀疑是localhost
DECLARE_string(stor_path);//默认值为空，但是是必填项
DECLARE_bool(stor_creat);
DECLARE_string(stor_remote_addr);//预留，远程文件系统地址，默认值为空
DECLARE_int32(stor_remote_port);//预留，远程文件系统端口，默认值为0，当${stor_remote_addr}不为空时为必填项，否则本项忽略

//任务部署
DECLARE_string(stor_taskdep_addr);
DECLARE_int32(stor_taskdep_port);
DECLARE_string(stor_taskdep_cert);
DECLARE_string(stor_taskdep_key);
DECLARE_string(stor_taskdep_ca);


