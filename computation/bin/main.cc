#include<misc/flags/flag.h>
#include<computation/infer/infer.h>
#include<computation/drccp/server.h>
#include<string>
#include<computation/bin/flag.h>
#include<workflow/WFFacilities.h>
#include<proto/taskdep/message.h>
#include<computation/taskdep/taskdep.h>
#include<regex>

static bool check_IPV4(const std::string& str){
    std::regex r("(^localhost)|(^(([0-9]){1,3}\\.){3}([0-9]){1,3})");
    return std::regex_match(str ,r);
}

int main(int argc ,char** argv){ 
    jxtd::misc::flags::parse_flags(&argc ,&argv ,true);

    //启动DrccpServerManager
    jxtd::computation::infer::DrccpServerManager server_manager;
    jxtd::computation::drccp::DrccpServer::DrccpSetupContext server_setup_context;
    jxtd::computation::drccp::DrccpServer::DrccpProxyOriginContext server_origin_context;
    server_setup_context.listen_addr = FLAGS_comp_comp_addr;
    server_setup_context.family = check_IPV4(FLAGS_comp_comp_addr) ? AF_INET :AF_INET6;
    server_setup_context.port = FLAGS_comp_comp_port;
    server_setup_context.ssl = !FLAGS_comp_comp_cert.empty();
    server_setup_context.ca = FLAGS_comp_comp_ca;
    server_setup_context.cert = FLAGS_comp_comp_cert;
    server_setup_context.key = FLAGS_comp_comp_key;
    server_origin_context.origin_addr = FLAGS_comp_comp_addr;
    server_origin_context.origin_port = FLAGS_comp_comp_port;
    server_manager.load_server_config(FLAGS_comp_comp_token ,FLAGS_comp_datatrans_bind_addr  ,FLAGS_comp_comp_port ,FLAGS_comp_datatrans_ssl ,FLAGS_comp_comp_name ,server_setup_context ,server_origin_context);
    //FIXME 这里的comp_comp_port存疑
    server_manager.start(FLAGS_comp_comp_name);
    if(!FLAGS_comp_comp_boot)
        server_manager.signin(FLAGS_comp_comp_name ,"cluster" ,FLAGS_comp_comp_peer_addr ,FLAGS_comp_comp_peer_port);
    else
	    server_manager.signin(FLAGS_comp_comp_name ,"cluster" ,FLAGS_comp_comp_addr ,FLAGS_comp_comp_port);
    //FIXME 这里的claster是什么还不确定

    //启动taskManager    
    jxtd::computation::infer::Taskmanager task_manager;
    
    jxtd::proto::taskdep::Endpoint src_endpoint;
    jxtd::proto::taskdep::Endpoint dest_endpoint;

    src_endpoint.set_proto("jxtd");
    src_endpoint.set_authtype(0x0);
    src_endpoint.set_url(FLAGS_comp_taskdep_cli_addr);
    //src_endpoint.set_port();

    dest_endpoint.set_proto("jxtd");
    dest_endpoint.set_authtype(0x0);
    dest_endpoint.set_url(FLAGS_comp_taskdep_stor_addr);
    dest_endpoint.set_port(FLAGS_comp_taskdep_stor_port);

    task_manager.set_src(src_endpoint ,!FLAGS_comp_taskdep_cert.empty() ,FLAGS_comp_stor_token);
    //FIXME 这里的stor_token可能是错的
    task_manager.set_dest(dest_endpoint);

    jxtd::computation::infer::set_storage_datatransfer(FLAGS_comp_datatrans_remote_addr ,FLAGS_comp_datatrans_remote_port ,FLAGS_comp_datatrans_ssl);

    //把taskdepServer和DrccpServerManager移交给taskdepServer管理    
    jxtd::computation::taskdep::ClusterServer taskdep_server;
    jxtd::computation::taskdep::TaskdepSetupContext taskdep_setup_context;
    jxtd::proto::taskdep::Endpoint setup_endpoint;

    setup_endpoint.set_proto("jxtd");
    setup_endpoint.set_authtype(0x0);
    setup_endpoint.set_url(FLAGS_comp_taskdep_addr);
    setup_endpoint.set_port(FLAGS_comp_taskdep_port);

    taskdep_setup_context.endpoint = setup_endpoint;
    taskdep_setup_context.ssl = !FLAGS_comp_taskdep_cert.empty();
    taskdep_setup_context.family = check_IPV4(FLAGS_comp_taskdep_addr) ? AF_INET :AF_INET6;
    taskdep_setup_context.ca = FLAGS_comp_taskdep_ca;
    taskdep_setup_context.cert = FLAGS_comp_taskdep_cert;
    taskdep_setup_context.key = FLAGS_comp_taskdep_key;

    taskdep_server.setup(taskdep_setup_context);
    taskdep_server.start(FLAGS_comp_comp_name ,task_manager ,server_manager);

    for(;;);
    
    return 0; 
}
