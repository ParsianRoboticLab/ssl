#include <commonconfig/common_config_nodelet.h>

void CommonConfig::onInit() {
    ROS_INFO("CommonConfig onInit");
    private_nh = getPrivateNodeHandle();
    server.reset(new dynamic_reconfigure::Server<agent_common::agentConfig>(private_nh));
}

PLUGINLIB_EXPORT_CLASS(CommonConfig, nodelet::Nodelet);