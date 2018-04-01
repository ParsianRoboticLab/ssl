#ifndef GRSIMNODELET_H_
#define GRSIMNODELET_H_
#include <nodelet/nodelet.h>
#include <dynamic_reconfigure/server.h>
#include <parsian_protobuf_wrapper/grsimConfig.h>
#include "ros/ros.h"
#include <vector>
#include <string>
//#include "parsian_msgs/grsim_robot_command.h"
#include "parsian_msgs/parsian_robot_command.h"
//#include "parsian_msgs/grsim_replacement.h"
#include <parsian_msgs/parsian_world_model.h>
#include "parsian_protobuf_wrapper/grSim_Packet.pb.h"
#include "parsian_protobuf_wrapper/grSim_Commands.pb.h"
#include "parsian_protobuf_wrapper/grSim_Replacement.pb.h"
#include "parsian_protobuf_wrapper/common/net/udpsend.h"
#include "parsian_msgs/grsim_robot_replacement.h"
#include "parsian_msgs/grsim_ball_replacement.h"
#include <ros/package.h>
#include <fstream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <parsian_msgs/parsian_team_config.h>

#define MAX_ROBOT_NUM 12


class GrsimNodelet : public nodelet::Nodelet {
public:
    GrsimNodelet();
    ~GrsimNodelet();

    virtual void onInit();
    void visionCB(const parsian_msgs::parsian_world_modelConstPtr & msg);
    //void timerCb(const ros::TimerEvent& event);
    void GrsimBotCmd(const parsian_msgs::parsian_robot_command::ConstPtr& msg);

    bool GrsimBallReplacesrv(parsian_msgs::grsim_ball_replacement::Request& req,
                             parsian_msgs::grsim_ball_replacement::Response& res);
    bool GrsimRobotReplacesrv(parsian_msgs::grsim_robot_replacement::Request& req,
                              parsian_msgs::grsim_robot_replacement::Response& res);
    void teamConfigCb(const parsian_msgs::parsian_team_configConstPtr& msg);


    boost::shared_ptr<dynamic_reconfigure::Server<protobuf_wrapper_config::grsimConfig>> server;
    dynamic_reconfigure::Server<protobuf_wrapper_config::grsimConfig>::CallbackType f;
    void conf(protobuf_wrapper_config::grsimConfig &config, uint32_t level);



    void send();
    std::string ip;
    int port;

    bool isYellow;

    grSim_Packet packet;
    grSim_Commands* GrsimCommand;
    grSim_Replacement* GrsimReplacement;

    UDPSend* udp;


    ros::NodeHandle n;
    ros::NodeHandle pn;
    ros::Subscriber vision_sub;

    ros::Subscriber robot_command_sub[MAX_ROBOT_NUM];
    ros::Subscriber _sub;
    ros::Subscriber team_config_sub;
    ros::ServiceServer service0;
    ros::ServiceServer service1;

    //kick profiler usage
    std::vector<std::vector<std::string>> dataList;
    void getprofilerdata();
    std::vector<bool> gotprofilerdatas;
    double convertkickchargetimetokickspeed(int id, double chargetime);
    std::vector<double> coef_a;
    std::vector<double> coef_b;
    std::vector<double> coef_c;
};

#endif /* GRSIMNODELET_H_ */
