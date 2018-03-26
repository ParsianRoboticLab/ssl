//
// Created by ali on 1/10/18.
//

#include <latency/latency_nodelet.h>

PLUGINLIB_EXPORT_CLASS(parsian_tools::Latency, nodelet::Nodelet);

using namespace parsian_tools;



void Latency::onInit() {
    ROS_INFO("%s oninit", getName().c_str());

    nh = getNodeHandle();
    world_model = new LatencyInfo<parsian_msgs::parsian_world_modelConstPtr> (nh, "world model"  , "/world_model") ;
    robot_command = new LatencyInfo<parsian_msgs::parsian_robot_commandConstPtr> (nh, "robot command", "/agent_0/command") ;
    vision = new LatencyInfo<parsian_msgs::ssl_vision_detectionConstPtr> (nh, "robot task"   , "/agent_0/task")  ;
    plan = new LatencyInfo<parsian_msgs::parsian_get_planConstPtr    > (nh, "vision"       , "/vision_detection");
    //robot_task = new LatencyInfo<parsian_msgs::parsian_robot_taskConstPtr>    (nh,"plan"         ,"/agent_0/plan")  ;
    path = new LatencyInfo<parsian_msgs::parsian_pathConstPtr         > (nh, "path"         , "/planner_0/path");
    packet = new LatencyInfo<parsian_msgs::parsian_packetsConstPtr> (nh, "packet"       , "/packets");

}
