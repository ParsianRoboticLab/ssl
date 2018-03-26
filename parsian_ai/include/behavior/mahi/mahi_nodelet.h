#ifndef MAHINODELET_H
#define MAHINODELET_H

#include <ros/ros.h>
#include <pluginlib/class_list_macros.h>
#include <nodelet/nodelet.h>

#include <parsian_msgs/parsian_debugs.h>
#include <parsian_msgs/parsian_draw.h>
#include <parsian_msgs/ssl_refree_wrapper.h>
#include <parsian_msgs/parsian_robots_status.h>
#include <parsian_msgs/parsian_behavior.h>
#include <behavior/mahi/mahi.h>

#include <dynamic_reconfigure/server.h>
#include <parsian_ai/mahiConfig.h>
#include <parsian_ai/config.h>


namespace parsian_ai {
class MahiNodelet : public nodelet::Nodelet {

private:
    boost::shared_ptr<BehaviorMahi> mahi;
    ros::Subscriber worldModelSub;
    ros::Subscriber refereeSub;
    ros::Subscriber teamConfSub;
    ros::Subscriber robotStatusSub;
    ros::Publisher drawPub;
    ros::Publisher behavPub;
    ros::Publisher debugPub;

    ros::Timer timer_;

    //config server setup
    boost::shared_ptr<dynamic_reconfigure::Server<ai_config::mahiConfig>> server;
    void ConfigServerCallBack(const ai_config::mahiConfig &config, uint32_t level) ;

    void onInit() override;
    void robotStatusCallBack(const parsian_msgs::parsian_robots_statusConstPtr& _rs);
    void worldModelCallBack(const parsian_msgs::parsian_world_modelConstPtr &_wm);
    void refereeCallBack(const parsian_msgs::ssl_refree_wrapperConstPtr & _ref);
    void timerCb(const ros::TimerEvent &event);
};
}
#endif //MAHINODELET_H
