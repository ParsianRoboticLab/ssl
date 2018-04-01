//
// Created by ali on 10/11/17.
//

#include "parsian_protobuf_wrapper/ssl-refbox/parsian_referee_nodelet.h"


PLUGINLIB_EXPORT_CLASS(parsian_protobuf_wrapper::RefreeNodelet, nodelet::Nodelet);

using namespace parsian_protobuf_wrapper;

void RefreeNodelet::onInit() {
    ros::NodeHandle &nh = getNodeHandle();
    ros::NodeHandle& nh_private = getPrivateNodeHandle();

    team_config_sub = nh.subscribe<parsian_msgs::parsian_team_config>("/team_config", 1000, boost::bind(& RefreeNodelet::teamConfigCb, this, _1));
    ssl_referee_pub  = nh_private.advertise<parsian_msgs::ssl_refree_wrapper>("/referee", 1000);

    //ros::param::get("/team_color", teamColor);



    refBox = nullptr;
    networkConfig.port = 10003;
    networkConfig.ip = "224.5.23.1";
    reconnect();
    std::chrono::high_resolution_clock::time_point last_param_update_time;

    server.reset(new dynamic_reconfigure::Server<protobuf_wrapper_config::refereeConfig>);
    dynamic_reconfigure::Server<protobuf_wrapper_config::refereeConfig>::CallbackType f;
    f = boost::bind(&RefreeNodelet::callback, this, _1, _2);
    server->setCallback(f);
    timer = nh.createTimer(ros::Duration(.010), boost::bind(&RefreeNodelet::timerCb, this, _1));

}

void RefreeNodelet::reconnect() {
    delete refBox;
    refBox = new RoboCupSSLClient(networkConfig.port, networkConfig.ip);
    if (!refBox->open(false)) {
        ROS_WARN("Connection Failed.");
    } else {
        ROS_INFO("Connected!");
    }

}

void RefreeNodelet::callback(const protobuf_wrapper_config::refereeConfig &config, uint32_t level) {
    networkConfig.ip = config.refree_multicast_ip;
    networkConfig.port = config.refree_multicast_port;
    reconnect();
}


void RefreeNodelet::timerCb(const ros::TimerEvent& event) {
    parsian_msgs::ssl_refree_wrapper referee;
    while (refBox->receive(ssl_referee)) {
        if (ssl_referee.has_command() || ssl_referee.has_stage()) {
            referee = pr::convert_referee_data(ssl_referee, isOurColorYellow);
            ssl_referee_pub.publish(referee);
        }
    }

}

void RefreeNodelet::teamConfigCb(const parsian_msgs::parsian_team_configConstPtr& msg) {
    isOurColorYellow = msg->color == parsian_msgs::parsian_team_config::YELLOW;
}

