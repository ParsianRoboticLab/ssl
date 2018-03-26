#include <agent_nodelet.h>

PLUGINLIB_EXPORT_CLASS(parsian_agent::AgentNodelet, nodelet::Nodelet);

using namespace parsian_agent;
void AgentNodelet::onInit() {
    ROS_INFO("%s oninit", getName().c_str());

    debugger = new Debugger;
    drawer   = new Drawer;
    wm = new CWorldModel;

    nh = getNodeHandle();
    private_nh = getPrivateNodeHandle();
    QString name(getName().c_str());
    agent.reset(new Agent(name.split('_').at(1).toInt()));

    gotoPoint = new CSkillGotoPoint(agent.get());
    gotoPointAvoid = new CSkillGotoPointAvoid(agent.get());
    skillKick = new CSkillKick(agent.get());
    oneTouch = new CSkillKickOneTouch(agent.get());
    receivePass = new CSkillReceivePass(agent.get());

    common_config_sub = nh.subscribe("/commonconfig/parameter_updates", 1000, &AgentNodelet::commonConfigCb, this);
    world_model_sub   = nh.subscribe("world_model", 10000, &AgentNodelet::wmCb, this);
    robot_task_sub    = private_nh.subscribe("task", 10, &AgentNodelet::rtCb, this);
    planner_sub       = nh.subscribe(QString("planner_%1/path").arg(agent->id()).toStdString(), 5, &AgentNodelet::plannerCb, this);

    debug_pub = nh.advertise<parsian_msgs::parsian_debugs>("debugs", 1000);
    draw_pub  = nh.advertise<parsian_msgs::parsian_draw>("draws", 1000);

    parsian_robot_command_pub = private_nh.advertise<parsian_msgs::parsian_robot_command>("command", 1000);
    agent->planner_pub = private_nh.advertise<parsian_msgs::parsian_get_plan>("plan", 5);

    timer_ = nh.createTimer(ros::Duration(0.01), &AgentNodelet::timerCb, this);
    watchdog = 0;
}

void AgentNodelet::commonConfigCb(const dynamic_reconfigure::ConfigConstPtr &_cnf) {
    auto * a = const_cast<dynamic_reconfigure::Config*>(_cnf.get());
    conf->__fromMessage__(*a);
}

void AgentNodelet::wmCb(const parsian_msgs::parsian_world_modelConstPtr& _wm) {
    wm->update(_wm);
    watchdog++;
    if (watchdog >= 2*conf->watchdog) {
        agent->skill = nullptr;
        watchdog = 2*conf->watchdog;
    } else if (watchdog >= conf->watchdog) {
        agent->waitHere();
        agent->skill = nullptr;
        parsian_robot_command_pub.publish(agent->getCommand());

    } else
    if (agent->skill != nullptr && finished) {
        finished = false;
        agent->execute();
        parsian_robot_command_pub.publish(agent->getCommand());
        finished = true;
    }

}

void AgentNodelet::timerCb(const ros::TimerEvent& event) {
    if (debugger != nullptr) {
        debug_pub.publish(debugger->debugs);
    }
    if (drawer   != nullptr) {
        // ROS_INFO_STREAM("agent drawer"<<drawer);
        draw_pub.publish(drawer->draws);
        drawer->draws.texts.clear();

        drawer->draws.circles.clear();
        drawer->draws.segments.clear();
        drawer->draws.vectors.clear();
        drawer->draws.rects.clear();
    }
    //ROS_INFO("draawwwerrr");
}

void AgentNodelet::rtCb(const parsian_msgs::parsian_robot_taskConstPtr& _robot_task) {
    watchdog = 0;
    agent->skill = getSkill(_robot_task);
}


CSkill* AgentNodelet::getSkill(const parsian_msgs::parsian_robot_taskConstPtr &_task) {
    CSkill *skill = nullptr;
    switch (_task->select) {
    case parsian_msgs::parsian_robot_task::GOTOPOINT:
        gotoPoint->setMessage(&_task->gotoPointTask);
        skill = gotoPoint;
        // ROS_INFO("GOTOPOINT executed!");
        break;
    case parsian_msgs::parsian_robot_task::GOTOPOINTAVOID:
        gotoPointAvoid->setMessage(&_task->gotoPointAvoidTask);
//            gotoPointAvoid->setNoavoid(true);
        skill = gotoPointAvoid;
        ROS_INFO_STREAM("GOTOPOINTAVOID executed!" << gotoPointAvoid->getTargetpos().y);
        break;
    case parsian_msgs::parsian_robot_task::KICK:
			skillKick->setMessage(&_task->kickTask);
            if(!_task->kickTask.chip)
            {
                if (!_task->kickTask.iskickchargetime)
                    skillKick->setKickspeed(agent->kickSpeedValue(_task->kickTask.kickSpeed,_task->kickTask.spin));
                else
                    skillKick->setKickspeed(_task->kickTask.kickchargetime);
            }
            else
            {
                if (!_task->kickTask.iskickchargetime)
                    skillKick->setKickspeed(agent->chipDistanceValue(_task->kickTask.kickSpeed,_task->kickTask.spin));
                else
                    skillKick->setKickspeed(_task->kickTask.kickchargetime);
            }
            skill = skillKick;
            //ROS_INFO("KICK executed!");
            break;

        break;
    case parsian_msgs::parsian_robot_task::ONETOUCH:
        oneTouch->setMessage(&_task->oneTouchTask);
        skill = oneTouch;
        //ROS_INFO("ONETOUCH executed!");
        break;
    case parsian_msgs::parsian_robot_task::RECIVEPASS:
        receivePass->setMessage(&_task->receivePassTask);
        skill = receivePass;
        //ROS_INFO("RECIVEPASS executed!");
        break;
    case parsian_msgs::parsian_robot_task::NOTASK:
        if (_task->noTask.waithere) {
            agent->waitHere();
            parsian_robot_command_pub.publish(agent->getCommand());
        } else {
            // TODO : Release Agent
            agent->waitHere();
            parsian_msgs::parsian_robot_commandPtr a = agent->getCommand();
            a->release = static_cast<unsigned char>(true);
            parsian_robot_command_pub.publish(a);

        }
        skill = nullptr;
        break;
    default:
        break;
    }
    return skill;
}

void AgentNodelet::plannerCb(const parsian_msgs::parsian_pathConstPtr & _path) {
    std::vector<Vector2D> path;
    for (const auto& p : _path->results) {
        path.emplace_back(p);
    }
    agent->getPathPlannerResult(path, Vector2D(_path->averageDir));
}
