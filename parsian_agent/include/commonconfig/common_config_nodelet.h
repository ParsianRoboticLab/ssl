#ifndef COMMONCONFIGNODELET_H
#define COMMONCONFIGNODELET_H

#include <nodelet/nodelet.h>
#include <ros/ros.h>
#include <pluginlib/class_list_macros.h>
#include <dynamic_reconfigure/server.h>
#include <parsian_agent/agentConfig.h>
#include <parsian_agent/config.h>


class CommonConfig : public nodelet::Nodelet {
private:

    void onInit() override;

    ros::NodeHandle private_nh;

    boost::shared_ptr<dynamic_reconfigure::Server<agent_common::agentConfig>> server;

};

#endif //COMMONCONFIGNODELET_H

