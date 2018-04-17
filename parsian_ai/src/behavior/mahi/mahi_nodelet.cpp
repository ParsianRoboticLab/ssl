#include <behavior/mahi/mahi_nodelet.h>

PLUGINLIB_EXPORT_CLASS(parsian_ai::MahiNodelet, nodelet::Nodelet);

using namespace parsian_ai;

void MahiNodelet::onInit() {

    ros::NodeHandle& nh = getNodeHandle();
    ros::NodeHandle& private_nh = getPrivateNodeHandle();
    mahi.reset(new BehaviorMahi());

    drawer = new Drawer();

    worldModelSub = nh.subscribe("/world_model", 1000, &MahiNodelet::worldModelCallBack, this);
    robotStatusSub = nh.subscribe("/robot_status", 1000, &MahiNodelet::robotStatusCallBack, this);

    refereeSub = nh.subscribe("/referee", 1000,  &MahiNodelet::refereeCallBack, this);

    drawPub = nh.advertise<parsian_msgs::parsian_draw>("/draws", 1000);
    timer_ = nh.createTimer(ros::Duration(.016), boost::bind(&MahiNodelet::timerCb, this, _1));

    behavPub = nh.advertise<parsian_msgs::parsian_behavior>("/eval", 10); // TODO: make it private
    //config server settings
    server.reset(new dynamic_reconfigure::Server<ai_config::mahiConfig>(private_nh));
    dynamic_reconfigure::Server<ai_config::mahiConfig>::CallbackType f;
    f = boost::bind(&MahiNodelet::ConfigServerCallBack, this, _1, _2);
    server->setCallback(f);

}

void MahiNodelet::timerCb(const ros::TimerEvent& event) {

    if (drawer != nullptr) {
        drawPub.publish(drawer->draws);
    }
//    if (debugger != nullptr) debugPub.publish(debugger->debugs);

    drawer->draws.texts.clear();
    drawer->draws.circles.clear();
    drawer->draws.segments.clear();
    drawer->draws.vectors.clear();
    drawer->draws.rects.clear();

//    debugger->debugs.clear();

}

void MahiNodelet::worldModelCallBack(const parsian_msgs::parsian_world_modelConstPtr &_wm) {
    mahi->updateWM(_wm);
    parsian_msgs::parsian_behaviorPtr behav;
    behav.reset(new parsian_msgs::parsian_behavior);
    auto prob = mahi->eval(behav);
    if (prob == 0.0 || behav == nullptr) return;
    behav->name = "mahi";
    behav->probability = prob;
    behavPub.publish(behav);

}

void MahiNodelet::refereeCallBack(const parsian_msgs::ssl_refree_wrapperConstPtr & _ref) {

    mahi->updateReferee(_ref);
}

void MahiNodelet::ConfigServerCallBack(const ai_config::mahiConfig &config, uint32_t level) {
}

void MahiNodelet::robotStatusCallBack(const parsian_msgs::parsian_robots_statusConstPtr& _rs) {

}
