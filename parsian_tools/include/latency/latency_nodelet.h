//
// Created by ali on 1/10/18.
//

#ifndef PARSIAN_TOOLS_LATENCY_NODELET_H
#define PARSIAN_TOOLS_LATENCY_NODELET_H


#include <nodelet/nodelet.h>
#include <ros/ros.h>
#include <pluginlib/class_list_macros.h>
#include <latency/latency_info.h>




namespace parsian_tools {

class Latency : public nodelet::Nodelet {
private:
    void onInit() override;

    ros::NodeHandle nh;

    LatencyInfo<parsian_msgs::parsian_world_modelConstPtr  > *world_model  ;
    LatencyInfo<parsian_msgs::parsian_robot_commandConstPtr> *robot_command;
    LatencyInfo<parsian_msgs::ssl_vision_detectionConstPtr > *vision       ;
    LatencyInfo<parsian_msgs::parsian_get_planConstPtr     > *plan         ;
    LatencyInfo<parsian_msgs::parsian_robot_taskConstPtr   > *robot_task   ;
    LatencyInfo<parsian_msgs::parsian_pathConstPtr         > *path         ;
    LatencyInfo<parsian_msgs::parsian_packetsConstPtr      > *packet       ;


};
}


#endif //PARSIAN_TOOLS_LATENCY_NODELET_H
