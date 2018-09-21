#include <planner/planner_nodelet.h>
#include <cstring>

PLUGINLIB_EXPORT_CLASS(parsian_agent::PlannerNodelet, nodelet::Nodelet);

using namespace parsian_agent;

void PlannerNodelet::onInit() {
    ROS_INFO("%s onInits", getName().c_str());

    nh = getNodeHandle();
    private_nh = getPrivateNodeHandle();

    drawer   = new Drawer;
    wm = new CWorldModel;

    QString name(getName().c_str());

    planner.reset(new CPlanner(name.split('_').at(1).toInt()));

    common_config_sub = nh.subscribe("/commonconfig/parameter_updates", 10, &PlannerNodelet::commonConfigCb, this);
    world_model_sub   = nh.subscribe("world_model", 10, &PlannerNodelet::wmCb, this);
    planner_sub       = nh.subscribe(QString("agent_%1/plan").arg(planner->getID()).toStdString(), 1, &PlannerNodelet::plannerCb, this);

    draw_pub  = nh.advertise<parsian_msgs::parsian_draws>("draws", 1000);
    planner->path_pub = private_nh.advertise<parsian_msgs::parsian_path>("path", 5);

}

void PlannerNodelet::commonConfigCb(const dynamic_reconfigure::ConfigConstPtr &_cnf) {
    auto * a = const_cast<dynamic_reconfigure::Config*>(_cnf.get());
    conf->__fromMessage__(*a);
}

void PlannerNodelet::wmCb(const parsian_msgs::parsian_world_modelConstPtr& _wm) {
    wm->update(_wm);
    planner->run();
    if (drawer   != nullptr) {
        draw_pub.publish(drawer->getDraws());
        drawer->clear();
    }
}

void PlannerNodelet::plannerCb(const parsian_msgs::parsian_get_planConstPtr & _plan) {
    QList<int> ourRL;
    QList<int> oppRL;
    for (const auto& id : _plan->ourRelaxList) {
        ourRL.append(id);
    }
    for (const auto& id : _plan->oppRelaxList) {
        oppRL.append(id);
    }
    planner->initPathPlanner(Vector2D(_plan->goal), ourRL, oppRL, _plan->avoidPenaltyArea, _plan->avoidCenterCircle, _plan->ballObstacleRadius);
}
