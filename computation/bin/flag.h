#pragma once
#include<gflags/gflags.h>
//应用网关（预留）
DECLARE_int32(comp_api_port);
DECLARE_string(comp_api_cert);
DECLARE_string(comp_api_key);
DECLARE_string(comp_api_ca);
DECLARE_string(comp_comp_token);
DECLARE_string(comp_mana_token);
DECLARE_string(comp_stor_token);
//计算核心
DECLARE_string(comp_comp_name);
DECLARE_string(comp_comp_addr);
DECLARE_int32(comp_comp_port);
DECLARE_bool(comp_comp_boot);//自集群启动模式
DECLARE_string(comp_comp_peer_addr);//${comp_comp_boot}为false时为必填项，否则本项无效
DECLARE_int32(comp_comp_peer_port);//${comp_comp_boot}为false时为必填项，否则本项无效
DECLARE_string(comp_comp_cert);
DECLARE_string(comp_comp_key);
DECLARE_string(comp_comp_ca);

//数据传输
DECLARE_string(comp_datatrans_remote_addr);
DECLARE_int32(comp_datatrans_remote_port);//必填项
DECLARE_string(comp_datatrans_remote_proto);
DECLARE_string(comp_datatrans_bind_addr);
DECLARE_bool(comp_datatrans_ssl);
DECLARE_string(comp_datatrans_ca);//不开启ssl时本选项失效

//任务部署
DECLARE_string(comp_taskdep_addr);
DECLARE_int32(comp_taskdep_port);
DECLARE_string(comp_taskdep_cert);
DECLARE_string(comp_taskdep_key);
DECLARE_string(comp_taskdep_ca);
DECLARE_string(comp_taskdep_stor_addr);//必填项
DECLARE_int32(comp_taskdep_stor_port);
DECLARE_string(comp_taskdep_cli_addr);


