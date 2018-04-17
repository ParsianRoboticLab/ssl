#ifndef AGENTNODELET_H
#define AGENTNODELET_H

#include <nodelet/nodelet.h>
#include <ros/ros.h>
#include <pluginlib/class_list_macros.h>
#include <parsian_util/tools/blackboard.h>
#include <parsian_util/tools/drawer.h>
#include <parsian_msgs/parsian_robot_task.h>
#include <parsian_msgs/parsian_world_model.h>
#include <parsian_msgs/parsian_robot_command.h>
#include <parsian_msgs/parsian_ai_status.h>
#include <parsian_msgs/parsian_draw.h>
#include <parsian_agent/agent.h>
#include <parsian_agent/config.h>
#include <parsian_agent/skills.h>


namespace parsian_agent {
class AgentNodelet : public nodelet::Nodelet {
private:

    void onInit() override;

    void timerCb(const ros::TimerEvent &event);

    ros::NodeHandle nh;
    ros::NodeHandle private_nh;

    ros::Subscriber world_model_sub;
    ros::Subscriber ai_sub;
    ros::Subscriber robot_task_sub;
    ros::Subscriber robot_status_sub;
    ros::Subscriber common_config_sub;
    ros::Subscriber planner_sub;

    ros::Publisher draw_pub;
    ros::Publisher parsian_robot_command_pub;

    ros::Timer timer_;

    void commonConfigCb(const dynamic_reconfigure::ConfigConstPtr & _cnf);

    void wmCb(const parsian_msgs::parsian_world_modelConstPtr &);

    void rtCb(const parsian_msgs::parsian_robot_taskConstPtr &);

    void aiCb(const parsian_msgs::parsian_ai_statusConstPtr &);

    void plannerCb(const parsian_msgs::parsian_pathConstPtr&);

    boost::shared_ptr<Agent> agent;

    CSkill* getSkill(const parsian_msgs::parsian_robot_taskConstPtr &);

    CSkillGotoPoint* gotoPoint;
    CSkillGotoPointAvoid* gotoPointAvoid;
    CSkillKick* skillKick;
    CSkillKickOneTouch* oneTouch;
    CSkillReceivePass* receivePass;
    bool finished = true;
    int watchdog;

};
}

#endif //PARSIAN_AGENT_AGENTNODELET_H

