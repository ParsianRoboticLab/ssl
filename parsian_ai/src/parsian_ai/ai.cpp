#include <parsian_ai/ai.h>

AI::AI() {
    wm = new WorldModel();
    gameState = new GameState();
    soccer = new CSoccer();
}

AI::~AI() {
    delete soccer;
    delete wm;
    delete gameState;
}

void AI::execute() {
    soccer->execute();
}

parsian_msgs::parsian_robot_task AI::getTask(int robotID) {
    if (wm->our.data->activeAgents.contains(robotID) != false) {
        if (soccer->agents[robotID]->action != nullptr) {
            if (soccer->agents[robotID]->action->getActionName() == KickAction::SActionName()) {
                parsian_msgs::parsian_skill_kick *task;
                task = reinterpret_cast<parsian_skill_kick *>(soccer->agents[robotID]->action->getMessage());
                robotsTask[robotID].kickTask = *task;
                robotsTask[robotID].select = robotsTask[robotID].KICK;

            } else if (soccer->agents[robotID]->action->getActionName() == GotopointavoidAction::SActionName()) {
                parsian_msgs::parsian_skill_gotoPointAvoid *task;
                task = reinterpret_cast<parsian_skill_gotoPointAvoid *>(soccer->agents[robotID]->action->getMessage());
                robotsTask[robotID].gotoPointAvoidTask = *task;
                robotsTask[robotID].select = robotsTask[robotID].GOTOPOINTAVOID;

            } else if (soccer->agents[robotID]->action->getActionName() == GotopointAction::SActionName()) {
                parsian_msgs::parsian_skill_gotoPoint *task;
                task = reinterpret_cast<parsian_skill_gotoPoint *>(soccer->agents[robotID]->action->getMessage());
                robotsTask[robotID].gotoPointTask = *task;
                robotsTask[robotID].select = robotsTask[robotID].GOTOPOINT;

            } else if (soccer->agents[robotID]->action->getActionName() == ReceivepassAction::SActionName()) {
                parsian_msgs::parsian_skill_receivePass *task;
                task = reinterpret_cast<parsian_skill_receivePass *>(soccer->agents[robotID]->action->getMessage());
                robotsTask[robotID].receivePassTask = *task;
                robotsTask[robotID].select = robotsTask[robotID].RECIVEPASS;

            } else if (soccer->agents[robotID]->action->getActionName() == OnetouchAction::SActionName()) {
                parsian_msgs::parsian_skill_oneTouch *task;
                task = reinterpret_cast<parsian_skill_oneTouch *>(soccer->agents[robotID]->action->getMessage());
                robotsTask[robotID].oneTouchTask = *task;
                robotsTask[robotID].select = robotsTask[robotID].ONETOUCH;

            } else if (soccer->agents[robotID]->action->getActionName() == NoAction::SActionName()) {
                parsian_msgs::parsian_skill_no *task;
                task = reinterpret_cast<parsian_skill_no*>(soccer->agents[robotID]->action->getMessage());
                robotsTask[robotID].noTask = *task;
                robotsTask[robotID].select = robotsTask[robotID].NOTASK;
            }
        } else {
            auto *task = new parsian_skill_no;
            task->waithere = static_cast<unsigned char>(false);
            robotsTask[robotID].noTask = *task;
            robotsTask[robotID].select = robotsTask[robotID].NOTASK;

        }
    }

    return robotsTask[robotID];
}

void AI::updateRobotStatus(const parsian_msgs::parsian_robotConstPtr & _rs) {

}

void AI::updateRobotFaults(const parsian_msgs::parsian_robot_fault & _rs)
{
    if(_rs.select == 0)
    {
        soccer->agents[_rs.robot_id]->fault = false;
        soccer->agents[_rs.robot_id]->faultstate = Agent::FaultState::HEALTHY;
    }
    if(_rs.select == 1)
    {
        ROS_INFO_STREAM("kian: " << soccer->agents[_rs.robot_id]->id() << " disrepaired");
    }
    if(_rs.select == 2)
    {
        soccer->agents[_rs.robot_id]->fault = true;
        soccer->agents[_rs.robot_id]->faultstate = Agent::FaultState::DAMEGED;
    }
    if(_rs.select == 3)
    {
        soccer->agents[_rs.robot_id]->fault = true;
        soccer->agents[_rs.robot_id]->faultstate = Agent::FaultState::DESTROYED;
    }
}

void AI::updateWM(const parsian_msgs::parsian_world_modelConstPtr & _wm) {
    wm->update(_wm);
    for (int i = 0 ; i < _MAX_NUM_PLAYERS ; i++) {
        soccer->agents[i]->self = *wm->our[i];
    }
}

void AI::updateReferee(const parsian_msgs::ssl_refree_wrapperConstPtr & _ref) {
    gameState->setRefree(_ref);
    wm->updateRef(_ref);
}

void AI::forceUpdateReferee(const parsian_msgs::ssl_force_refereeConstPtr & _command){
    gameState->setForceRefree(_command);
    wm->setBallplacementPoin(_command->ballPlacementPos);
}

CSoccer* AI::getSoccer() {
    return soccer;
}
