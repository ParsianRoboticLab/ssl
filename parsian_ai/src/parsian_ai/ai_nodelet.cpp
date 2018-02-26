#include <parsian_ai/ai_nodelet.h>

PLUGINLIB_EXPORT_CLASS(parsian_ai::AINodelet, nodelet::Nodelet);

using namespace parsian_ai;

void AINodelet::onInit() {

    ros::NodeHandle& nh = getNodeHandle();
    ros::NodeHandle& private_nh = getPrivateNodeHandle();
    ai.reset(new AI());
    ROS_INFO("init");
    robTask = new ros::Publisher[_MAX_NUM_PLAYERS];
    for (int i = 0; i < _MAX_NUM_PLAYERS; ++i) {
        std::string topic(QString("/agent_%1/task").arg(i).toStdString());
        robTask[i] = nh.advertise<parsian_msgs::parsian_robot_task>(topic, 1000);
    }
    drawer = new Drawer();
    debugger = new Debugger();

    worldModelSub = nh.subscribe("/world_model", 1000, &AINodelet::worldModelCallBack, this);
    robotStatusSub = nh.subscribe("/robot_status", 1000, &AINodelet::robotStatusCallBack, this);

    refereeSub = nh.subscribe("/referee", 1000,  &AINodelet::refereeCallBack, this);
    teamConfSub = nh.subscribe("/team_config", 100, &AINodelet::teamConfCb, this);

    drawPub = nh.advertise<parsian_msgs::parsian_draw>("/draws", 1000);
    debugPub = nh.advertise<parsian_msgs::parsian_debugs>("/debugs", 1000);
//    timer_ = nh.createTimer(ros::Duration(.062), boost::bind(&AINodelet::timerCb, this, _1));

    plan_client = nh.serviceClient<parsian_msgs::plan_service> ("/get_plans", true);

    ai->getSoccer()->getCoach()->setPlanClient(plan_client);

    //config server settings
    server.reset(new dynamic_reconfigure::Server<ai_config::aiConfig>(private_nh));
    dynamic_reconfigure::Server<ai_config::aiConfig>::CallbackType f;
    f = boost::bind(&AINodelet::ConfigServerCallBack,this, _1, _2);
    server->setCallback(f);

}

void AINodelet::teamConfCb(const parsian_msgs::parsian_team_configConstPtr& _conf) {
    teamConfig = *_conf;
}

void AINodelet::timerCb(const ros::TimerEvent& event) {

    // ai->execute();

    if (drawer != nullptr)   drawPub.publish(drawer->draws);
    if (debugger != nullptr) debugPub.publish(debugger->debugs);
}

void AINodelet::worldModelCallBack(const parsian_msgs::parsian_world_modelConstPtr &_wm) {
    ai->updateWM(_wm);
    ROS_INFO("wm updated");
    ai->execute();

    for(int i=0; i < wm->our.activeAgentsCount(); i++) {
        robTask[wm->our.activeAgentID(i)].publish(ai->getTask(wm->our.activeAgentID(i)));
    }

    parsian_msgs::plan_serviceResponse lastPlan = ai->getSoccer()->getCoach()->getLastPlan();
    ROS_INFO_STREAM("last plan name: "<<lastPlan.the_plan.planFile);
    /// handle plan request
//    if(ai->getSoccer()->getCoach()->requestForPlan){
//        parsian_msgs::plan_service req;
//        req.request = ai->getSoccer()->getCoach()->getPlanRequest();
//        if(plan_client.call(req)){
//            ai->getSoccer()->getCoach()->setPlanResponse(req.response);
//            ai->getSoccer()->getCoach()->requestForPlan = false;
//            ROS_INFO("ai requesting for plan...");
//
//        } else {
//            ROS_INFO("ERROR! in ai plan request");
//        }
//    } else {
//        ROS_INFO("No plan requested");
//    }

}

void AINodelet::refereeCallBack(const parsian_msgs::ssl_refree_wrapperConstPtr & _ref) {

    ai->updateReferee(_ref);
}

void AINodelet::robotStatusCallBack(const parsian_msgs::parsian_robotConstPtr & _rs) {
    ai->updateRobotStatus(_rs);
}

void AINodelet::ConfigServerCallBack(const ai_config::aiConfig &config, uint32_t level) {
    conf = config;
}
