//
// Created by ali on 10/11/17.
//

#ifndef PARSIAN_PROTOBUF_WRAPPER_PARSIAN_REFEREE_NODELET_H
#define PARSIAN_PROTOBUF_WRAPPER_PARSIAN_REFEREE_NODELET_H

#include <ros/ros.h>
#include <pluginlib/class_list_macros.h>
#include <nodelet/nodelet.h>
#include <chrono>

#include <dynamic_reconfigure/server.h>
#include <parsian_protobuf_wrapper/common/net/robocup_ssl_client.h>
#include <parsian_protobuf_wrapper/ssl-refbox/convert/convert_referee.h>

#include <parsian_msgs/parsian_team_config.h>

#include "parsian_protobuf_wrapper/refereeConfig.h"

namespace parsian_protobuf_wrapper {

class RefreeNodelet: public nodelet::Nodelet {
private:
    virtual void onInit();

    RoboCupSSLClient *refBox;
    bool isOurColorYellow = false;

    struct SNetworkConfig {
        std::string ip;
        int port;
    } networkConfig;

    void reconnect();
    void timerCb(const ros::TimerEvent& event);
    void callback(const protobuf_wrapper_config::refereeConfig &config, uint32_t level) ;
    void teamConfigCb(const parsian_msgs::parsian_team_configConstPtr& msg);


    boost::shared_ptr<dynamic_reconfigure::Server<protobuf_wrapper_config::refereeConfig>> server;


    SSL_Referee ssl_referee;
    //std::string teamColor;
    ros::Publisher ssl_referee_pub;
    ros::Timer timer;

    ros::Subscriber team_config_sub;


};

}
#endif //PARSIAN_PROTOBUF_WRAPPER_PARSIAN_REFEREE_NODELET_H
