#include "parsian_protobuf_wrapper/grsim/grsim_nodelet.h"
#include <pluginlib/class_list_macros.h>



/*----------filing grSim_Robot_Command with ROS datas for each robots----------*/
void GrsimNodelet::GrsimBotCmd(const parsian_msgs::parsian_robot_command::ConstPtr& msg) {
    grSim_Robot_Command* GrsimRobotCommand = GrsimCommand->add_robot_commands();
    GrsimRobotCommand->set_id(msg->robot_id);
    GrsimRobotCommand->set_kickspeedx(convertkickchargetimetokickspeed(msg->robot_id, msg->kickSpeed));
    GrsimRobotCommand->set_kickspeedz(msg->kickspeedz / 200.0);
    GrsimRobotCommand->set_veltangent(0);
    GrsimRobotCommand->set_velnormal(0);
    GrsimRobotCommand->set_velangular(0);
    GrsimRobotCommand->set_wheel1(msg->wheel1);
    GrsimRobotCommand->set_wheel2(msg->wheel2);
    GrsimRobotCommand->set_wheel3(msg->wheel3);
    GrsimRobotCommand->set_wheel4(msg->wheel4);
    GrsimRobotCommand->set_spinner(msg->spinner != 0u);
    GrsimRobotCommand->set_wheelsspeed(msg->wheelsspeed != 0u);

}


/*----------filing grsim_robot_replacement with ROS datas for each robots ##service##----------*/
bool GrsimNodelet::GrsimRobotReplacesrv(parsian_msgs::grsim_robot_replacement::Request& req,
                                        parsian_msgs::grsim_robot_replacement::Response& res) {
    grSim_RobotReplacement* grsimRReplacement = GrsimReplacement->add_robots();
    grsimRReplacement->set_x(req.x);
    grsimRReplacement->set_y(req.y);
    grsimRReplacement->set_dir(req.dir);
    grsimRReplacement->set_id(req.id);
    grsimRReplacement->set_yellowteam(req.yellowteam);
    return true;
}

/*----------filing grsim_ball_replacement with ROS datas ##service##----------*/
bool GrsimNodelet::GrsimBallReplacesrv(parsian_msgs::grsim_ball_replacement::Request& req,
                                       parsian_msgs::grsim_ball_replacement::Response& res) {
    grSim_BallReplacement* GrsimBReplacement = new grSim_BallReplacement;
    GrsimBReplacement->set_x(req.x);
    GrsimBReplacement->set_y(req.y);
    GrsimBReplacement->set_vx(req.vx);
    GrsimBReplacement->set_vy(req.vy);
    GrsimReplacement->set_allocated_ball(GrsimBReplacement);
    return true;
}

/*----------creating a full grSim_Packet protocol buffer and sending it----------*/
void GrsimNodelet::send() {

    GrsimCommand->set_isteamyellow(static_cast<int>(isYellow));
    GrsimCommand->set_timestamp(0.0);                       //should fix this
    packet.set_allocated_commands(GrsimCommand);
    packet.set_allocated_replacement(GrsimReplacement);
    std::string buffer;
    packet.SerializeToString(&buffer);
    udp->send(buffer);
    packet.clear_commands();
    packet.clear_replacement();
    GrsimCommand = new grSim_Commands;
    GrsimReplacement = new grSim_Replacement;
}

void GrsimNodelet::visionCB(const parsian_msgs::parsian_world_modelConstPtr & msg) {
    send();
}

//void GrsimNodelet::timerCb(const ros::TimerEvent& event){
//    // Using timers is the preferred 'ROS way' to manual threading
//    send();
//}

/*----------get ip address and port from configue server----------*/
void GrsimNodelet::conf(protobuf_wrapper_config::grsimConfig &config, uint32_t level) {
    if (this->ip != config.grsim_send_ip) {
        this->ip = config.grsim_send_ip;
        udp->setIP(this->ip);
    }
    if (this->port  != config.grsim_send_port) {
        this->port  = config.grsim_send_port;
        udp->setport(this->port);
    }

    NODELET_INFO("grsim_nodelet binded on %s : %d", this->ip.c_str(), this->port);

}

GrsimNodelet::GrsimNodelet() {
    ROS_INFO("grsim_nodelet is running");
}

GrsimNodelet::~GrsimNodelet() {
    //ROS_INFO("Destructor");
}

void GrsimNodelet::onInit() {
    NODELET_INFO("grsim_nodelet onInit");


    getprofilerdata();

    udp = new UDPSend(ip, port);

    GrsimCommand = new grSim_Commands;
    GrsimReplacement = new grSim_Replacement;


    n = getNodeHandle();
    pn = getPrivateNodeHandle();
    for (int i = 0; i < MAX_ROBOT_NUM; ++i) {
        robot_command_sub [i] = n.subscribe<parsian_msgs::parsian_robot_command>(("agent_" + QString::number(i) + "/command").toStdString().c_str(), 5,
                                boost::bind(& GrsimNodelet::GrsimBotCmd, this, _1));
    }

    team_config_sub = n.subscribe("/team_config", 5, & GrsimNodelet::teamConfigCb, this);



    service0 = n.advertiseService<parsian_msgs::grsim_robot_replacementRequest,
    parsian_msgs::grsim_robot_replacementResponse>
    ("GrsimRobotReplacesrv", boost::bind(& GrsimNodelet::GrsimRobotReplacesrv, this, _1, _2));
    service1 = n.advertiseService<parsian_msgs::grsim_ball_replacement::Request,
    parsian_msgs::grsim_ball_replacement::Response>
    ("GrsimBallReplacesrv", boost::bind(& GrsimNodelet::GrsimBallReplacesrv, this, _1, _2));

    server.reset(new dynamic_reconfigure::Server<protobuf_wrapper_config::grsimConfig>(pn));
    f = boost::bind(& GrsimNodelet::conf, this, _1, _2);
    server->setCallback(f);

    vision_sub = n.subscribe("/world_model", 10, &GrsimNodelet::visionCB, this);

}

void GrsimNodelet::teamConfigCb(const parsian_msgs::parsian_team_configConstPtr& msg) {
    isYellow = (msg->color == parsian_msgs::parsian_team_config::YELLOW);
}

//kick profiler usage
void GrsimNodelet::getprofilerdata()
{
    std::ifstream file(ros::package::getPath("parsian_agent") + "/profiler_data/coefficients/coeffs_kick_0.csv");
    if (!file.is_open())
        ROS_INFO_STREAM("profiler datas failed to open");
    else
    {
        ROS_INFO_STREAM("file opened");
        std::string line = "";
        // Iterate through each line and split the content using delimeter
        while (getline(file, line))
        {
            std::vector<std::string> vec;
            boost::algorithm::split(vec, line, boost::is_any_of(","));
            dataList.push_back(vec);
        }
        // Close the File
        file.close();
        for(int i{1}; i<dataList.size(); i++){
                coef_a.push_back(atof(dataList[i][1].c_str()));
                coef_b.push_back(atof(dataList[i][2].c_str()));
                coef_c.push_back(atof(dataList[i][3].c_str()));
                gotprofilerdatas.push_back(true);
            }
    }
}
double GrsimNodelet::convertkickchargetimetokickspeed(int id, double chargetime)
{
    if (gotprofilerdatas[id])
    {
        ROS_INFO_STREAM(coef_a[id] << " " << coef_b[id] << " " << coef_c[id]);
        double mkickspeed = (((-1*coef_b[id]) + sqrt(coef_b[id]*coef_b[id] - 4*coef_a[id]*(coef_c[id] - chargetime)))/(2*coef_a[id]));
        return mkickspeed;

    }
    else
    {
        ROS_INFO_STREAM("there was no result for this robot in profiler datas");
        return 5;
    }


}




PLUGINLIB_DECLARE_CLASS(parsian_protobuf_wrapper, GrsimNodelet, GrsimNodelet, nodelet::Nodelet);
