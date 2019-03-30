#include <parsian_ai/ai_nodelet.h>

PLUGINLIB_EXPORT_CLASS(parsian_ai::AINodelet, nodelet::Nodelet);

using namespace parsian_ai;

void AINodelet::onInit() {

    ros::NodeHandle& nh = getNodeHandle();
    ros::NodeHandle& private_nh = getPrivateNodeHandle();
    ai.reset(new AI());
    ROS_INFO("init2");
    robTask = new ros::Publisher[_MAX_NUM_PLAYERS];
    for (int i = 0; i < _MAX_NUM_PLAYERS; ++i) {
        std::string topic(QString("/agent_%1/task").arg(i).toStdString());
        robTask[i] = nh.advertise<parsian_msgs::parsian_robot_task>(topic, 10);
    }
    drawer = new Drawer();

    worldModelSub = nh.subscribe("/world_model", 10, &AINodelet::worldModelCallBack, this);
    robotStatusSub = nh.subscribe("/robot_status", 100, &AINodelet::robotStatusCallBack, this);
    refereeSub = nh.subscribe("/referee", 100,  &AINodelet::refereeCallBack, this);
    teamConfSub = nh.subscribe("/team_config", 100, &AINodelet::teamConfCb, this);
    mousePosSub = nh.subscribe("/mousePos", 100, &AINodelet::mousePosCb, this);
    forceRefereeSub = nh.subscribe("/force_referee", 100, &AINodelet::forceRefereeCallBack, this);
    robotSubstituteSub = nh.subscribe("/substitute", 100, &AINodelet::substitutedetectionCallBack, this);

    drawPub = nh.advertise<parsian_msgs::parsian_draws>("/draws", 1000);

    plan_client.reset(new ros::ServiceClient);
    *plan_client = nh.serviceClient<parsian_msgs::plan_service> ("/get_plans", true);
    ai->getSoccer()->getCoach()->setPlanClient(plan_client);

    //config server settings
    server.reset(new dynamic_reconfigure::Server<ai_config::aiConfig>(private_nh));
    dynamic_reconfigure::Server<ai_config::aiConfig>::CallbackType f;
    f = boost::bind(&AINodelet::ConfigServerCallBack, this, _1, _2);
    server->setCallback(f);

}
void AINodelet::mousePosCb(const parsian_msgs::vector2DConstPtr &_mousePos) {
    mousePos.assign(_mousePos.get()->x ,_mousePos.get()->y );
}
void AINodelet::teamConfCb(const parsian_msgs::parsian_team_configConstPtr& _conf) {
    teamConfig = *_conf;
}

void AINodelet::worldModelCallBack(const parsian_msgs::parsian_world_modelConstPtr &_wm) {
    ai->updateWM(_wm);
    ai->execute();

    for (int i = 0; i < wm->our.activeAgentsCount(); i++) {

        robTask[wm->our.activeAgentID(i)].publish(ai->getTask(wm->our.activeAgentID(i)));
    }

    if (drawer != nullptr)  {
        drawPub.publish(drawer->getDraws());
        drawer->clear();
    }

}

void AINodelet::refereeCallBack(const parsian_msgs::ssl_refree_wrapperConstPtr & _ref) {
    ai->updateReferee(_ref);
}

void AINodelet::substitutedetectionCallBack(const parsian_msgs::parsian_robot_substitution & _rs) {
    ai->updateRobotSubstitutes(_rs);
}

void AINodelet::forceRefereeCallBack(const parsian_msgs::ssl_force_refereeConstPtr & _command){
    ai->forceUpdateReferee(_command);
}

void AINodelet::robotStatusCallBack(const parsian_msgs::parsian_robotConstPtr & _rs) {
    ai->updateRobotStatus(_rs);
}

void AINodelet::ConfigServerCallBack(const ai_config::aiConfig &config, uint32_t level) {
    conf = config;
}
