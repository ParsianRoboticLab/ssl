#ifndef AINODELET_H
#define AINODELET_H

#include <ros/ros.h>
#include <pluginlib/class_list_macros.h>
#include <nodelet/nodelet.h>

#include <parsian_msgs/parsian_skill_gotoPoint.h>
#include <parsian_msgs/parsian_skill_gotoPointAvoid.h>
#include <parsian_msgs/parsian_skill_receivePass.h>
#include <parsian_msgs/parsian_skill_kick.h>

#include <parsian_msgs/parsian_plan.h>
#include <parsian_msgs/parsian_ai_plan_request.h>

#include <parsian_msgs/parsian_draw.h>
#include <parsian_msgs/parsian_team_config.h>
#include <parsian_msgs/parsian_behavior.h>
#include <parsian_msgs/parsian_ai_status.h>
#include <parsian_ai/ai.h>

#include <dynamic_reconfigure/server.h>
#include "parsian_ai/aiConfig.h"
#include <parsian_ai/config.h>


namespace parsian_ai {
class AINodelet : public nodelet::Nodelet {

private:
    boost::shared_ptr<AI> ai;
    ros::Subscriber worldModelSub, robotStatusSub, refereeSub, teamConfSub, behaviorSub, mousePosSub;
    ros::Publisher drawPub;

        ros::Publisher *robTask;
        ros::Publisher behaviorPub;
        ros::Timer timer_;

    ros::ServiceClient plan_client;

    //config server setup
    boost::shared_ptr<dynamic_reconfigure::Server<ai_config::aiConfig>> server;
    void ConfigServerCallBack(const ai_config::aiConfig &config, uint32_t level) ;

    void onInit() override;

    void worldModelCallBack(const parsian_msgs::parsian_world_modelConstPtr &_wm);
    void refereeCallBack(const parsian_msgs::ssl_refree_wrapperConstPtr & _ref);
    void robotStatusCallBack(const parsian_msgs::parsian_robotConstPtr & _rs);
    void teamConfCb(const parsian_msgs::parsian_team_configConstPtr& _conf);
    void behaviorCb(const parsian_msgs::parsian_behaviorConstPtr& _behavior);
    void timerCb(const ros::TimerEvent &event);
    void mousePosCb(const parsian_msgs::vector2DConstPtr& _mousePos);

};
}
#endif //AINODELET_H
