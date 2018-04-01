#include <behavior/move_forward/move_forward_nodelet.h>

PLUGINLIB_EXPORT_CLASS(parsian_ai::Move_ForwardNodelet, nodelet::Nodelet);

using namespace parsian_ai;

void Move_ForwardNodelet::onInit() {

    ros::NodeHandle& nh = getNodeHandle();
    ros::NodeHandle& private_nh = getPrivateNodeHandle();
    move_forward.reset(new BehaviorMove_Forward());

    drawer = new Drawer();
    debugger = new Debugger();
    wm = new WorldModel;
    worldModelSub = nh.subscribe("/world_model", 1000, &Move_ForwardNodelet::worldModelCallBack, this);
    robotStatusSub = nh.subscribe("/robot_status", 1000, &Move_ForwardNodelet::robotStatusCallBack, this);

    refereeSub = nh.subscribe("/referee", 1000,  &Move_ForwardNodelet::refereeCallBack, this);

    drawPub = nh.advertise<parsian_msgs::parsian_draw>("/draws", 1000);
    debugPub = nh.advertise<parsian_msgs::parsian_debugs>("/debugs", 1000);
    timer_ = nh.createTimer(ros::Duration(.062), boost::bind(&Move_ForwardNodelet::timerCb, this, _1));

    //config server settings
    server.reset(new dynamic_reconfigure::Server<ai_config::move_forwardConfig>(private_nh));
    dynamic_reconfigure::Server<ai_config::move_forwardConfig>::CallbackType f;
    f = boost::bind(&Move_ForwardNodelet::ConfigServerCallBack,this, _1, _2);
    server->setCallback(f);

}

void Move_ForwardNodelet::timerCb(const ros::TimerEvent& event) {

    if (drawer != nullptr)   drawPub.publish(drawer->draws);
//    if (debugger != nullptr) debugPub.publish(debugger->debugs);

    drawer->draws.texts.clear();
    drawer->draws.circles.clear();
    drawer->draws.segments.clear();
    drawer->draws.vectors.clear();
    drawer->draws.rects.clear();

//    debugger->debugs.clear();

}

void Move_ForwardNodelet::worldModelCallBack(const parsian_msgs::parsian_world_modelConstPtr &_wm) {
    move_forward->updateWM(_wm);
    move_forward->eval();
}

void Move_ForwardNodelet::refereeCallBack(const parsian_msgs::ssl_refree_wrapperConstPtr & _ref) {

    move_forward->updateReferee(_ref);
}

void Move_ForwardNodelet::ConfigServerCallBack(const ai_config::move_forwardConfig &config, uint32_t level) {
}

void Move_ForwardNodelet::robotStatusCallBack(const parsian_msgs::parsian_robots_statusConstPtr& _rs) {

}
