#include <behavior/direct/direct_nodelet.h>

PLUGINLIB_EXPORT_CLASS(parsian_ai::DirectNodelet, nodelet::Nodelet);

using namespace parsian_ai;

void DirectNodelet::onInit() {

    ros::NodeHandle& nh = getNodeHandle();
    ros::NodeHandle& private_nh = getPrivateNodeHandle();
    direct.reset(new BehaviorDirect());

    drawer = new Drawer();

    worldModelSub = nh.subscribe("/world_model", 1000, &DirectNodelet::worldModelCallBack, this);
    robotStatusSub = nh.subscribe("/robot_status", 1000, &DirectNodelet::robotStatusCallBack, this);
    aiStatusSub = nh.subscribe("/status", 1000, &DirectNodelet::aiStatusCallBack, this);

    refereeSub = nh.subscribe("/referee", 1000,  &DirectNodelet::refereeCallBack, this);

    drawPub = nh.advertise<parsian_msgs::parsian_draw>("/draws", 1000);
    timer_ = nh.createTimer(ros::Duration(.016), boost::bind(&DirectNodelet::timerCb, this, _1));

    behavPub = nh.advertise<parsian_msgs::parsian_behavior>("/eval", 10); // TODO: make it private
    //config server settings
    server.reset(new dynamic_reconfigure::Server<ai_config::directConfig>(private_nh));
    dynamic_reconfigure::Server<ai_config::directConfig>::CallbackType f;
    f = boost::bind(&DirectNodelet::ConfigServerCallBack,this, _1, _2);
    server->setCallback(f);
    wm = new WorldModel();
}

void DirectNodelet::timerCb(const ros::TimerEvent& event) {

    if (drawer != nullptr)   drawPub.publish(drawer->draws);

    drawer->draws.texts.clear();
    drawer->draws.circles.clear();
    drawer->draws.segments.clear();
    drawer->draws.vectors.clear();
    drawer->draws.rects.clear();

}

void DirectNodelet::worldModelCallBack(const parsian_msgs::parsian_world_modelConstPtr &_wm) {
    direct->updateWM(_wm);
    parsian_msgs::parsian_behaviorPtr behav;
    behav.reset(new parsian_msgs::parsian_behavior);
    auto prob = direct->eval(behav);
    if (prob == 0.0 || behav == nullptr) return;
    behav->name = "direct";
    behav->probability = prob;
    behavPub.publish(behav);

}

void DirectNodelet::refereeCallBack(const parsian_msgs::ssl_refree_wrapperConstPtr & _ref) {

    direct->updateReferee(_ref);
}

void DirectNodelet::ConfigServerCallBack(const ai_config::directConfig &config, uint32_t level) {

}

void DirectNodelet::robotStatusCallBack(const parsian_msgs::parsian_robots_statusConstPtr& _rs) {

}

void DirectNodelet::aiStatusCallBack(const parsian_msgs::parsian_ai_statusConstPtr& _ais) {
    direct->updateAIStatus(_ais);
}

