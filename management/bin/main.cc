#include<misc/flags/flag.h>
#include<management/datasource/DataSourceServer.h>
#include<workflow/WFFacilities.h>
#include<proto/taskdep/message.h>
#include<management/taskdep/taskdep.h>
#include<string>
#include<management/bin/flag.h>
#include<regex>

static bool check_IPV4(const std::string& str){
    std::regex r("(^localhost)|(^(([0-9]){1,3}\\.){3}([0-9]){1,3})");
    return std::regex_match(str ,r);
}

int main(int argc ,char** argv){
    jxtd::misc::flags::parse_flags(&argc ,&argv ,true);
    
    //启动DataSourceServer
    jxtd::management::datasource::DataSourceServerSetupContext datasource_server_setup_context;
    datasource_server_setup_context.family = check_IPV4(FLAGS_mana_datasource_addr) ? AF_INET : AF_INET6;
    datasource_server_setup_context.addr = FLAGS_mana_datasource_addr;
    datasource_server_setup_context.port = FLAGS_mana_datasource_port;
    datasource_server_setup_context.ssl = !FLAGS_mana_datasource_cert.empty();
    datasource_server_setup_context.ca_path = FLAGS_mana_datasource_ca;
    datasource_server_setup_context.key_path = FLAGS_mana_datasource_key;
    datasource_server_setup_context.cert_path = FLAGS_mana_datasource_cert;
    jxtd::management::datasource::DataSourceServer datasource_server(datasource_server_setup_context);
    datasource_server.start();

    //启动ManagementClient
    jxtd::proto::taskdep::Endpoint mana_endpoint;
    mana_endpoint.set_proto("jxtd");
    mana_endpoint.set_authtype(0x0);
    mana_endpoint.set_url(FLAGS_mana_taskdep_comp_addr);
    mana_endpoint.set_port(FLAGS_mana_taskdep_comp_port);

    jxtd::proto::taskdep::Endpoint comp_endpoint;
    comp_endpoint.set_proto("jxtd");
    comp_endpoint.set_url(FLAGS_mana_taskdep_comp_addr);
    comp_endpoint.set_port(FLAGS_mana_taskdep_comp_port);

    jxtd::proto::taskdep::Endpoint stor_endpoint;
    stor_endpoint.set_proto("jxtd");
    stor_endpoint.set_url(FLAGS_mana_taskdep_stor_addr);
    stor_endpoint.set_port(FLAGS_mana_taskdep_stor_port);

    jxtd::management::taskdep::TaskdepClientContext task_client_context;
    task_client_context.management = mana_endpoint;
    task_client_context.computation = comp_endpoint;
    task_client_context.storage = stor_endpoint;
    task_client_context.ssl = !FLAGS_mana_cert.empty();
    //FIXME task_client_context.token = 
    jxtd::management::taskdep::ManagementClient management_client;
    management_client.setup(task_client_context);
    
    for(;;);
    
    return 0;    
}
