#include <parsian_world_model/worldmodel_nodelet.h>

PLUGINLIB_EXPORT_CLASS(parsian_world_model::WMNodelet, nodelet::Nodelet);

using namespace parsian_world_model;

void WMNodelet::onInit() {
    ros::NodeHandle& nh = getNodeHandle();
    ros::NodeHandle& private_nh = getPrivateNodeHandle();

    wm.reset(new WorldModel);
    drawer = new Drawer;
//    timer = nh.createTimer(ros::Duration(.062), boost::bind(&WMNodelet::timerCb, this, _1));
    wm_pub = nh.advertise<parsian_msgs::parsian_world_model>("/world_model", 1000);
    team_config_sub = nh.subscribe("/team_config", 1000, & WMNodelet::teamConfigCb, this);
    vision_detection_sub = nh.subscribe("vision_detection", 1000, &WMNodelet::detectionCb, this);
    QString robotCommandSubName;
    for (int i = 0 ; i < 12 ; i++) {
        robotCommandSubName = QString("/agent_%1/command").arg(i);
        robots_command_sub[i] = nh.subscribe(robotCommandSubName.toStdString(), 1000, &WMNodelet::robotsCommandCb, this);
    }
//    vision_geom_sub = nh.subscribe("vision_geom", 10, boost::bind(& WMNodelet::geomCb, this, _1));

    server.reset(new dynamic_reconfigure::Server<world_model_config::world_modelConfig>(private_nh));
    dynamic_reconfigure::Server<world_model_config::world_modelConfig>::CallbackType f;
    f = boost::bind(&WMNodelet::ConfigServerCallBack, this, _1, _2);
    server->setCallback(f);

}

//void WMNodelet::geomCb(const parsian_msgs::ssl_vision_geometryConstPtr &_geom) {
//
//}

void WMNodelet::robotsCommandCb(const parsian_msgs::parsian_robot_commandConstPtr &_robotCommad) {
    wm->vForwardCmd[_robotCommad->robot_id] = _robotCommad->vel_F;
    wm->vNormalCmd [_robotCommad->robot_id] = _robotCommad->vel_N;
    wm->vAngCmd    [_robotCommad->robot_id] = _robotCommad->vel_w;

}

void WMNodelet::detectionCb(const parsian_msgs::ssl_vision_detectionConstPtr &_detection) {
    wm->updateDetection(_detection);
    wm->execute();
    frame ++;
    packs ++;
    if (packs >= 4) {
        packs = 0;
        wm->merge(frame);
        parsian_msgs::parsian_world_modelPtr temp = wm->getParsianWorldModel();
        temp->header.stamp = ros::Time::now();
        temp->header.frame_id = std::to_string(_detection->frame_number);
        wm_pub.publish(temp);
    }

}


void WMNodelet::teamConfigCb(const parsian_msgs::parsian_team_configConstPtr& msg) {
    isOurSideLeft = msg->side == parsian_msgs::parsian_team_config::LEFT;
    wm->setMode(msg->mode == parsian_msgs::parsian_team_config::SIMULATION);
    NODELET_INFO("team config received!");
}

void WMNodelet::ConfigServerCallBack(const world_model_config::world_modelConfig &config, uint32_t level) {
    m_config.active_cam_num = config.active_cam_num;
    m_config.camera_one_active = config.camera_one_active;
    m_config.camera_two_active = config.camera_two_active;
    m_config.camera_three_active = config.camera_three_active;
    m_config.camera_four_active = config.camera_four_active;
}
