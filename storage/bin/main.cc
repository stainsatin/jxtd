#include<misc/flags/flag.h>
#include<storage/taskdep/taskdep.h>
#include<storage/bin/flag.h>
#include<proto/taskdep/message.h>
#include<storage/datamanagement/datamanagement.h>
#include<storage/datatrans/DatatranServer.h>
#include<workflow/WFFacilities.h>
#include<regex>

static bool check_IPV4(const std::string& str){
    std::regex r("(^localhost)|(^(([0-9]){1,3}\\.){3}([0-9]){1,3})");
    return std::regex_match(str ,r);
}

int main(int argc ,char** argv){
    jxtd::misc::flags::parse_flags(&argc ,&argv ,true);

    //启动后端    
    jxtd::storage::datamanagement::Datamanager data_manager;
    data_manager.init(FLAGS_stor_path ,FLAGS_stor_creat);
    data_manager.backend_choose(FLAGS_stor_backend);

    //下面启动storage_server
    std::mutex task_mutex;

    jxtd::storage::taskdep::TaskdepSetupContext taskdep_setup_context;
    jxtd::proto::taskdep::Endpoint endpoint;

    endpoint.set_proto("jxtd");
    endpoint.set_authtype(0x0);
    endpoint.set_url(FLAGS_stor_taskdep_addr);
    endpoint.set_port(FLAGS_stor_taskdep_port);
    
    taskdep_setup_context.endpoint = endpoint;
    taskdep_setup_context.ssl = !FLAGS_stor_taskdep_cert.empty();
    taskdep_setup_context.ca = FLAGS_stor_taskdep_ca;
    taskdep_setup_context.cert = FLAGS_stor_taskdep_cert;
    taskdep_setup_context.key = FLAGS_stor_taskdep_key;

    jxtd::storage::taskdep::StorageServer storage_server;
    storage_server.setup(taskdep_setup_context);
    storage_server.start(data_manager ,task_mutex);

    //下面启动DatatransServer
    jxtd::storage::datatransfer::DatatranServerSetupContext datatrans_server_setup_context;
    datatrans_server_setup_context.family = check_IPV4(FLAGS_stor_datatrans_addr) ? AF_INET :AF_INET6;
    datatrans_server_setup_context.addr = FLAGS_stor_datatrans_addr;
    datatrans_server_setup_context.port = FLAGS_stor_datatrans_port;
    datatrans_server_setup_context.ssl = !FLAGS_stor_datatrans_cert.empty();
    datatrans_server_setup_context.ca_path = FLAGS_stor_datatrans_ca;
    datatrans_server_setup_context.key_path = FLAGS_stor_datatrans_key;
    datatrans_server_setup_context.cert_path = FLAGS_stor_datatrans_cert;

    jxtd::storage::datatransfer::DatatranServer datatrans_server;
    datatrans_server.setup_context(datatrans_server_setup_context);
    datatrans_server.start(data_manager ,task_mutex);

    for(;;);

    return 0;
}
