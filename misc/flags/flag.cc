#include<gflags/gflags.h>
#include<misc/flags/flag.h>
#include<string>
#include<ctime>
#include<random>
#include<sstream>
#include<regex>
/*
 *计算单元
 */
//通用配置
DEFINE_string(comp_cert ,"" ,"");
DEFINE_string(comp_key ,"" ,"");
DEFINE_string(comp_ca ,"" ,"");

//应用网关（预留）
DEFINE_string(comp_api_address ,"localhost" ,"");
DEFINE_int32(comp_api_port ,5983 ,"");
DEFINE_string(comp_api_cert ,"" ,"");
DEFINE_string(comp_api_key ,"" ,"");
DEFINE_string(comp_api_ca ,"" ,"");
DEFINE_string(comp_comp_token ,"" ,"");
DEFINE_string(comp_mana_token ,"" ,"");//必填项
DEFINE_string(comp_stor_token ,"" ,"");//必填项

//计算核心
DEFINE_string(comp_comp_name ,"" ,"");//必填项
DEFINE_string(comp_comp_addr ,"localhost" ,"");
DEFINE_int32(comp_comp_port ,2667 ,"");
DEFINE_bool(comp_comp_boot ,true ,"");//自集群启动模式
DEFINE_string(comp_comp_peer_addr ,"" ,"");//${comp_comp_boot}为false时为必填项，否则本项无效
DEFINE_int32(comp_comp_peer_port ,0 ,"");//${comp_comp_boot}为false时为必填项，否则本项无效
DEFINE_string(comp_comp_cert ,"" ,"");
DEFINE_string(comp_comp_key ,"" ,"");
DEFINE_string(comp_comp_ca ,"" ,"");

//数据传输
DEFINE_string(comp_datatrans_remote_addr ,"" ,"");
DEFINE_int32(comp_datatrans_remote_port ,0 ,"");//必填项
DEFINE_string(comp_datatrans_remote_proto ,"builtin" ,"");
DEFINE_string(comp_datatrans_bind_addr ,"localhost" ,"");
DEFINE_bool(comp_datatrans_ssl ,false ,"");
DEFINE_string(comp_datatrans_ca ,"" ,"");//不开启ssl时本选项失效

//任务部署
DEFINE_string(comp_taskdep_addr ,"localhost" ,"");
DEFINE_int32(comp_taskdep_port ,8537 ,"");
DEFINE_string(comp_taskdep_cert ,"" ,"");
DEFINE_string(comp_taskdep_key ,"" ,"");
DEFINE_string(comp_taskdep_ca ,"" ,"");
DEFINE_string(comp_taskdep_stor_addr ,"" ,"");//必填项 DEFINE_int32(comp_taskdep_stor_port ,8537 ,"");
DEFINE_int32(comp_taskdep_stor_port ,8537 ,"");
DEFINE_string(comp_taskdep_cli_addr ,"localhost" ,"");

/*
 * 存储单元
 */

//通用配置
DEFINE_string(stor_cert ,"" ,"");
DEFINE_string(stor_key ,"" ,"");
DEFINE_string(stor_ca ,"" ,"");

//应用网关（预留）
DEFINE_string(stor_api_address ,"localhost" ,"");
DEFINE_int32(stor_api_port ,5983 ,"");
DEFINE_string(stor_api_cert ,"" ,"");
DEFINE_string(stor_api_key ,"" ,"");
DEFINE_string(stor_api_ca ,"" ,"");
DEFINE_string(stor_comp_token ,"" ,"");//必填项
DEFINE_string(stor_mana_token ,"" ,"");//必填项
DEFINE_string(stor_stor_token ,"" ,"");//默认值为空，值为空时随机生成128长度token

//数据传输
DEFINE_string(stor_datatrans_addr ,"localhost" ,"");
DEFINE_int32(stor_datatrans_port ,3887 ,"");
DEFINE_string(stor_datatrans_cert ,"" ,"");
DEFINE_string(stor_datatrans_key ,"" ,"");
DEFINE_string(stor_datatrans_ca ,"" ,"");

//存储配置
DEFINE_string(stor_backend ,"local" ,"");
DEFINE_string(stor_path ,"" ,"");//默认值为空，但是是必填项
DEFINE_bool(stor_creat ,false ,"");
DEFINE_string(stor_remote_addr ,"" ,"");//预留，远程文件系统地址，默认值为空
DEFINE_int32(stor_remote_port ,0 ,"");//预留，远程文件系统端口，默认值为0，当${stor_remote_addr}不为空时为必填项，否则本项忽略

//任务部署
DEFINE_string(stor_taskdep_addr ,"localhost" ,"");
DEFINE_int32(stor_taskdep_port ,8537 ,"");
DEFINE_string(stor_taskdep_cert ,"" ,"");
DEFINE_string(stor_taskdep_key ,"" ,"");
DEFINE_string(stor_taskdep_ca ,"" ,"");

/*
 * 管理单元
 */

//通用配置
DEFINE_string(mana_cert ,"" ,"");
DEFINE_string(mana_key ,"" ,"");
DEFINE_string(mana_ca ,"" ,"");
//应用网关(预留)
DEFINE_string(mana_api_address ,FLAGS_stor_taskdep_addr ,"");
DEFINE_int32(mana_api_port ,5983 ,"");
DEFINE_string(mana_api_cert ,"" ,"");
DEFINE_string(mana_api_key ,"" ,"");
DEFINE_string(mana_api_ca ,"" ,"");
DEFINE_string(mana_comp_token ,"" ,"");//必填项，长度128
DEFINE_string(mana_mana_token ,"" ,"");//默认值为空，值为空时随机生成128长度的token
DEFINE_string(mana_stor_token ,"" ,"");//必填项，长度128

//数据源管理
DEFINE_string(mana_datasource_addr ,"localhost" ,"");
DEFINE_int32(mana_datasource_port ,3877 ,"");
DEFINE_string(mana_datasource_cert ,"" ,"");
DEFINE_string(mana_datasource_key ,"" ,"");
DEFINE_string(mana_datasource_ca ,"" ,"");

//任务部署
DEFINE_string(mana_taskdep_addr ,"localhost" ,"");
DEFINE_string(mana_taskdep_comp_addr ,"" ,"");//必填项
DEFINE_int32(mana_taskdep_comp_port ,8537 ,"");
DEFINE_string(mana_taskdep_stor_addr ,"" ,"");//必填项
DEFINE_int32(mana_taskdep_stor_port ,8537 ,"");

/*
 * 资源管理器(预留)
 */


/*
 * 边缘设备(预留)
 */

//用于检查ipv6的辅助函数
bool check_IPV6(const std::string& str){
    //先对非省略的情况做检查
    std::regex r("^\\[([0-9A-F]{1,4}:){7}[0-9A-F]{1,4}\\]" ,std::regex_constants::icase);
    if(std::regex_match(str ,r))
        return true;
    //对零位压缩形式的检查
    std::regex r_omit("^\\[(([0-9A-F]([0-9a-f]{0,3}:[0-9a-f])*[0-9a-f]{0,3}:)|(:)):([0-9a-f]([0-9a-f]{0,3}:[0-9a-f])*[0-9a-f]{0,3})?\\]" ,std::regex_constants::icase);
    if(std::regex_match(str ,r_omit)){
	//这是防止长度过长的问题
        int counter = count(str.begin() ,str.end() ,':');
        return counter <= 7;
    }
    else
        return false;
}

bool check_IPV4(const std::string& str){
    std::regex r("(^localhost)|(^(([0-9]){1,3}\\.){3}([0-9]){1,3})");
    return std::regex_match(str ,r);
}

//下面就是对输入必填项和输入格式的一些检查

bool required_string_field(const char* flag_name ,const std::string& value){
    if(value.empty())
        throw std::logic_error("missing required field");
    else
	    return true;
}

bool required_int_field(const char* flag_name ,int32_t value){
    if(value == 0)
        throw std::logic_error("missing required field");
    else
	    return true;
}

bool required_token(const char* flag_name ,const std::string& value){
    if(value.empty())
        throw std::logic_error("missing required field");
    else if(value.length() != 128)
	    throw std::logic_error("the length of token should be 128");
    else
	    return true;
}

bool token_length_check(const char* flag_name ,const std::string& value){
    if(!value.empty() && value.length() != 128)
	    throw std::logic_error("the length of token should be 128");
    else
	    return true;
}

bool addr_check(const char* flag_name ,const std::string& value){
    if((!value.empty()) & !(check_IPV6(value) | check_IPV4(value)))
	    throw std::logic_error("illegal address");
    else
        return true;
}

bool required_addr_check(const char* flag_name ,const std::string& value){
    if(value.empty())
        throw std::logic_error("missing required field");
    if(!(check_IPV4(value) | check_IPV6(value)))
	    throw std::logic_error("illegal address"); 
    else
	    return true;
   
}
//TODO 可能需要把这两个正则表达式改一下

//这一部分是非必填项的地址检查
DEFINE_validator(comp_api_address ,&addr_check);
DEFINE_validator(comp_comp_addr ,&addr_check);
DEFINE_validator(comp_datatrans_bind_addr ,&addr_check);
DEFINE_validator(comp_taskdep_addr ,&addr_check);
DEFINE_validator(comp_taskdep_cli_addr ,&addr_check);
DEFINE_validator(stor_api_address ,&addr_check);
DEFINE_validator(stor_datatrans_addr ,&addr_check);
DEFINE_validator(stor_remote_addr ,&addr_check);
DEFINE_validator(stor_taskdep_addr ,&addr_check);
DEFINE_validator(mana_api_address ,&addr_check);
DEFINE_validator(mana_datasource_ca ,&addr_check);
DEFINE_validator(mana_taskdep_addr ,&addr_check);

//下面是对token的检查
//首先是非必填的token
DEFINE_validator(comp_comp_token ,&token_length_check);
DEFINE_validator(stor_stor_token ,&token_length_check);
DEFINE_validator(mana_mana_token ,&token_length_check);
//然后是必填项的token
DEFINE_validator(comp_mana_token ,&required_token);
DEFINE_validator(comp_stor_token ,&required_token);
DEFINE_validator(stor_comp_token ,&required_token);
DEFINE_validator(stor_mana_token ,&required_token);
DEFINE_validator(mana_comp_token ,&required_token);
DEFINE_validator(mana_stor_token ,&required_token);

//下面是对必填端口号的检查
DEFINE_validator(comp_datatrans_remote_port ,&required_int_field);

//下面是对一些必填字符串（诸如路径和名字之类的）的检查
DEFINE_validator(comp_comp_name ,&required_string_field);
DEFINE_validator(stor_backend ,&required_string_field);
DEFINE_validator(stor_path ,&required_string_field);

//随机token生成的函数
std::string rand_str(int str_leng){
    std::default_random_engine e;
    std::uniform_int_distribution<int> u(33,126);
    std::string rand_string;
    char letter;
    e.seed(time(0));
    for(int i = 0 ;i < str_leng ;i++){
        letter = u(e);
	rand_string.push_back(letter);
    }
    return rand_string;
}

//实现${*/".crt"}.key的函数
std::string key_default_value(std::string str){
    std::string substring = str.substr(0, str.length() - 4);
    std::stringstream sstream;
    sstream << substring << ".key";
    return sstream.str();
}

void jxtd::misc::flags::parse_flags(int* argc ,char*** argv ,bool removal){
    gflags::ParseCommandLineFlags(argc ,argv ,removal);

    //这一部分做有条件触发的必填项的检查
    if(!FLAGS_comp_comp_boot)
        required_addr_check("comp_comp_peer_addr" ,FLAGS_comp_comp_peer_addr); 
    if(!FLAGS_comp_comp_boot)
        required_int_field("comp_comp_peer_port" ,FLAGS_comp_comp_peer_port);
    if(!FLAGS_stor_remote_addr.empty())
	required_int_field("stor_remote_port" ,FLAGS_stor_remote_port);
    
    //这一部分做key默认值的设置
    if(FLAGS_comp_key.empty())
        FLAGS_comp_key = key_default_value(FLAGS_comp_cert);
    if(FLAGS_stor_key.empty())
        FLAGS_stor_key = key_default_value(FLAGS_stor_cert);
    if(FLAGS_mana_key.empty())
        FLAGS_mana_key = key_default_value(FLAGS_mana_cert);

    //这一部分做token随机默认值的设置
    if(FLAGS_comp_comp_token.empty())
        FLAGS_comp_comp_token = rand_str(128);
    if(FLAGS_stor_stor_token.empty())
        FLAGS_stor_stor_token = rand_str(128);
    if(FLAGS_mana_mana_token.empty())
        FLAGS_mana_mana_token = rand_str(128);

    //这一部分做各种与其他flag相等的默认值的设置
    if(FLAGS_comp_api_cert.empty())
        FLAGS_comp_api_cert = FLAGS_comp_cert;
    if(FLAGS_comp_api_key.empty())
        FLAGS_comp_api_key = FLAGS_comp_key;
    if(FLAGS_comp_api_ca.empty())
        FLAGS_comp_api_ca =FLAGS_comp_ca;
    if(FLAGS_comp_comp_cert.empty())
        FLAGS_comp_comp_cert = FLAGS_comp_cert;
    if(FLAGS_comp_comp_key.empty())
        FLAGS_comp_comp_key = FLAGS_comp_key;
    if(FLAGS_comp_comp_ca.empty())
        FLAGS_comp_comp_ca = FLAGS_comp_ca;
    if(FLAGS_comp_taskdep_cert.empty())
        FLAGS_comp_taskdep_cert = FLAGS_comp_cert;
    if(FLAGS_comp_taskdep_key.empty())
        FLAGS_comp_taskdep_key = FLAGS_comp_key;
    if(FLAGS_comp_taskdep_ca.empty())
        FLAGS_comp_taskdep_ca = FLAGS_comp_ca;
    if(FLAGS_stor_api_cert.empty())
        FLAGS_stor_api_cert = FLAGS_stor_cert;
    if(FLAGS_stor_api_key.empty())
        FLAGS_stor_api_key = FLAGS_stor_key;
    if(FLAGS_stor_api_ca.empty())
        FLAGS_stor_api_ca = FLAGS_stor_ca;
    if(FLAGS_stor_datatrans_cert.empty())
        FLAGS_stor_datatrans_cert = FLAGS_stor_cert;
    if(FLAGS_stor_datatrans_key.empty())
        FLAGS_stor_datatrans_key = FLAGS_stor_key;
    if(FLAGS_stor_datatrans_ca.empty()) 
        FLAGS_stor_datatrans_ca = FLAGS_stor_ca;
    if(FLAGS_stor_taskdep_cert.empty())
        FLAGS_stor_taskdep_cert = FLAGS_stor_cert;
    if(FLAGS_stor_taskdep_key.empty())
        FLAGS_stor_taskdep_key = FLAGS_stor_key;
    if(FLAGS_stor_taskdep_ca.empty())
        FLAGS_stor_taskdep_ca = FLAGS_stor_ca;
    if(FLAGS_mana_api_cert.empty())
        FLAGS_mana_api_cert = FLAGS_mana_cert;
    if(FLAGS_mana_api_key.empty())
        FLAGS_mana_api_key = FLAGS_mana_key;
    if(FLAGS_mana_api_ca.empty())
        FLAGS_mana_api_ca = FLAGS_mana_ca;

}
