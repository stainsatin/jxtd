#pragma once
#include<gflags/gflags.h>

//通用配置
DECLARE_string(mana_cert);
DECLARE_string(mana_key);
DECLARE_string(mana_ca);
//应用网关(预留)
DECLARE_string(mana_api_address);
DECLARE_int32(mana_api_port);
DECLARE_string(mana_api_cert);
DECLARE_string(mana_api_key);
DECLARE_string(mana_api_ca);
DECLARE_string(mana_comp_token);//必填项，长度128
DECLARE_string(mana_mana_token);//默认值为空，值为空时随机生成128长度的token
DECLARE_string(mana_stor_token);//必填项，长度128

//数据源管理
DECLARE_string(mana_datasource_addr);
DECLARE_int32(mana_datasource_port);
DECLARE_string(mana_datasource_cert);
DECLARE_string(mana_datasource_key);
DECLARE_string(mana_datasource_ca);
//任务部署
DECLARE_string(mana_taskdep_addr);
DECLARE_string(mana_taskdep_comp_addr);//必填项
DECLARE_int32(mana_taskdep_comp_port);
DECLARE_string(mana_taskdep_stor_addr);//必填项
DECLARE_int32(mana_taskdep_stor_port);

/*
 * 资源管理器(预留)
 */


/*
 * 边缘设备(预留)
 */

