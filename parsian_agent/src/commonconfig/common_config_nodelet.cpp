#include <commonconfig/common_config_nodelet.h>
PLUGINLIB_DECLARE_CLASS(parsian_agent, CommonConfig, CommonConfig, nodelet::Nodelet);

void CommonConfig::onInit() {
    ROS_INFO("CommonConfig onInit");
    private_nh = getPrivateNodeHandle();
    wm = new CWorldModel;
    server.reset(new dynamic_reconfigure::Server<agent_common::agentConfig>(private_nh));
}