//
// Created by parsian-ai on 10/5/17.
//

#ifndef PARSIAN_WORLD_MODEL_WORLDMODEL_NODELET_H
#define PARSIAN_WORLD_MODEL_WORLDMODEL_NODELET_H

#include  <ros/ros.h>
#include <pluginlib/class_list_macros.h>
#include <nodelet/nodelet.h>

#include <parsian_msgs/parsian_world_model.h>
#include <parsian_msgs/ssl_vision_geometry.h>
#include <parsian_msgs/ssl_vision_detection.h>

#include <parsian_world_model/worldmodel_nodelet.h>
#include <parsian_world_model/wm/worldmodel.h>

#include <dynamic_reconfigure/server.h>

#include <parsian_msgs/parsian_team_config.h>
#include <parsian_world_model/config.h>

#include <parsian_msgs/parsian_robot_command.h>
#include <parsian_util/tools/drawer.h>
#include <QString>
//#include <parsian_world_model/world_modelConfig.h>

namespace parsian_world_model {

class WMNodelet : public nodelet::Nodelet {

private:
    virtual void onInit();

    // Timer CallBack (to publish)
    //void timerCb(const ros::TimerEvent& event);

//        Subs CallBack
    void geomCb(const parsian_msgs::ssl_vision_geometryConstPtr& _geom);
    void detectionCb(const parsian_msgs::ssl_vision_detectionConstPtr& _detection);
    void robotsCommandCb(const parsian_msgs::parsian_robot_commandConstPtr& _robotCommad);
    void teamConfigCb(const parsian_msgs::parsian_team_configConstPtr& msg);


    boost::shared_ptr<WorldModel> wm;

    ros::Publisher wm_pub;

    ros::Subscriber vision_detection_sub;
    ros::Subscriber vision_geom_sub;
    ros::Subscriber robots_command_sub[12];
    ros::Subscriber team_config_sub;


    boost::shared_ptr<dynamic_reconfigure::Server<world_model_config::world_modelConfig>> server;
    void ConfigServerCallBack(const world_model_config::world_modelConfig &config, uint32_t level);


    int frame, packs;

//        ros::Timer timer;



};
}

#endif //PARSIAN_WORLD_MODEL_WORLDMODEL_NODELET_H
