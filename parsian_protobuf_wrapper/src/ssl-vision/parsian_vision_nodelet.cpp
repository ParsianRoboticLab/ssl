#include <parsian_protobuf_wrapper/ssl-vision/parsian_vision_nodelet.h>

PLUGINLIB_EXPORT_CLASS(parsian_protobuf_wrapper::VisionNodelet, nodelet::Nodelet);


using namespace parsian_protobuf_wrapper;

void VisionNodelet::onInit() {
    ros::NodeHandle& nh = getNodeHandle();
    ros::NodeHandle& nh_private = getPrivateNodeHandle();


    //ros::param::get("/team_color", teamColor);
    packs = 0;

    timer = nh.createTimer(ros::Duration(.004), boost::bind(&VisionNodelet::timerCb, this, _1));

    ssl_geometry_pub  = nh.advertise<parsian_msgs::ssl_vision_geometry>("vision_geom", 1000);
    ssl_detection_pub = nh.advertise<parsian_msgs::ssl_vision_detection>("vision_detection", 1000);
    team_config_sub = nh.subscribe("/team_config", 1000, & VisionNodelet::teamConfigCb, this);


//    ssl_wrapper_pub = nh.advertise<parsian_msgs::ssl_vision_wrapper>("vision", 1000);
    //ROS_INFO("on init");
    vision = nullptr;
    visionConfig.vision_multicast_port = 10020;
    visionConfig.vision_multicast_ip = "224.5.23.2";
    reconnect();

    configServer.reset(new dynamic_reconfigure::Server<protobuf_wrapper_config::visionConfig>(nh_private));
    dynamic_reconfigure::Server<protobuf_wrapper_config::visionConfig>::CallbackType f;
    f = boost::bind(&VisionNodelet::configCb, this, _1, _2);
    configServer->setCallback(f);

}

void VisionNodelet::reconnect() {
    delete vision;
    vision = new RoboCupSSLClient(visionConfig.vision_multicast_port, visionConfig.vision_multicast_ip);
    if (!vision->open(false)) {
        NODELET_WARN("Connection Failed.");
    } else {
        NODELET_INFO("Connected!");
    }

}

void VisionNodelet::configCb(const protobuf_wrapper_config::visionConfig &config , uint32_t level) {
    visionConfig.vision_multicast_ip = config.vision_multicast_ip;
    visionConfig.vision_multicast_port = config.vision_multicast_port;
    //isOurColorYellow = config.is_yellow;
    reconnect();

}

void VisionNodelet::timerCb(const ros::TimerEvent &event) {

    while (vision != nullptr && vision->receive(vision_packet)) {
        //ROS_INFO("v");
        if (vision_packet.has_detection()) {
            parsian_msgs::ssl_vision_detectionPtr detection{new parsian_msgs::ssl_vision_detection};
            *detection = pr::convert_detection_frame(vision_packet.detection(), isOurColorYellow, isOurSideLeft);
            detection->header.stamp = ros::Time::now();
            ssl_detection_pub.publish(detection);
        }

        if (vision_packet.has_geometry()) {
            parsian_msgs::ssl_vision_geometryPtr geometry{new parsian_msgs::ssl_vision_geometry};
            *geometry = pr::convert_geometry_data(vision_packet.geometry());
            ssl_geometry_pub.publish(geometry);
        }
    }
}

void VisionNodelet::teamConfigCb(const parsian_msgs::parsian_team_configConstPtr& msg) {
    isOurColorYellow = msg->color == parsian_msgs::parsian_team_config::YELLOW;
    isOurSideLeft = msg->side == parsian_msgs::parsian_team_config::LEFT;
}

