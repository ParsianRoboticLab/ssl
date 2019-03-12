#include <search.h>
#include <parsian_ai/plays/ourballplacement.h>
#include <math.h>
#include "parsian_ai/plays/ourballplacement.h"

COurBallPlacement::COurBallPlacement() {

    loopCounter = 0;
    first = true;
    loop = false;
    recivePass = new ReceivepassAction;
    pass = new KickAction;
    gpaP = new GotopointavoidAction; gpaH = new GotopointavoidAction;
    gpaK = new GotopointavoidAction; gpaR = new GotopointavoidAction;
    for (auto& g : gpa) g = new GotopointavoidAction;
    nearFlag = false;  ///// near agent flag
    shotFlag = true;  ///// near agent kick the ball or not
    updateFlag = false;   /////
    lastBallPos =  Vector2D(); lastBallPos.invalidate();
    ballPosBeforKick = Vector2D(); ballPosBeforKick.invalidate();
}

COurBallPlacement::~COurBallPlacement() {
    delete gpaP;
    delete pass;
    delete recivePass;
    delete gpaH;
    delete gpaK;
    delete gpaR;
    for (auto& g : gpa) delete g;
}

void COurBallPlacement::reset(){
    first = false;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void COurBallPlacement::init(QList<Agent*>& _agents) {
    //agents = _agents;
    //ROS_INFO_STREAM("shiit : "<<agents.size());
    initMaster();
}
/**
 *
 * @param currentBallPos
 * @param lastBallPos is the posiotion ball hase befor kick
 * @return is ball moved or not!!
 */
bool COurBallPlacement::isBallHaseMoved(const Vector2D &currentBallPos, const Vector2D &lastBallPos, const double &dist) {
    return currentBallPos.dist(lastBallPos) > dist;
}

/**
 *
 * @param ballPos
 * @param ballPosBeforKick
 * @return ball kicked well or not
 */
bool COurBallPlacement::isBallDidntKickedWell(const Vector2D &ballPos, const Vector2D &ballPosBeforKick, const double dist) {
    return ballPosBeforKick.dist(ballPos) < dist;
}

/**
 * @param agent that going to receive the ball
 * @param targetPos
 * @return is receiver agent near to the position or not
 */
bool COurBallPlacement::isAgentOnThePosition(Agent* agent, const Vector2D &targetPos, const double dist) {
    return agent->pos().dist(targetPos) < dist;
}

/**
 *
 * @param ballPos current position of ball
 * @param targetPos
 * @return is ball near to the position that must be or not!!
 */
bool COurBallPlacement::isBallNearToTarget(const Vector2D &ballPos, const Vector2D &targetPos , const double &dist) {
    return targetPos.dist(ballPos) < dist;
}

/**
 *
 * @param speed is the speed that we want to compare with
 * @param velocity is a vector2D of velocity in x and y cordinate
 * @return is velocity smaller tha desierd speed or not
 */
bool COurBallPlacement::isBallSpeedLow(const double &speed, const Vector2D &velocity) {
    return sqrt(pow(velocity.x, 2) + pow(velocity.y, 2)) < speed;
}

/**
 * find the nearest agent to our desired position
 * @param pos that we want to assign a robot to it
 * @param blockedAgent is the robot that we dont want to choos among the agents
 * @return near agents id
 *
 */
Agent* COurBallPlacement::reciverFinder(const Vector2D& target , Agent* blockedAgent) {

    ROS_INFO_STREAM("agentfinder blocked agent id : "<<blockedAgent->id());
    double minDist = 1000;
    Agent* nearAgent = nullptr;
    ROS_INFO_STREAM("fil"<<agents.size());
    for(auto agent : agents){
        ROS_INFO_STREAM("fil raft to for");
        if ( agent->id() != blockedAgent->id() && agent->pos().dist(target) < minDist){
            ROS_INFO_STREAM("fil raft to if");
            nearAgent = agent;
            minDist = agent->pos().dist(target);
        }
    }
    //ROS_INFO_STREAM("agentfinder reciver agent id : "<<nearAgent->id());
    return nearAgent;

}

Agent *COurBallPlacement::kickerfinder(const Vector2D & target) {

    double minDist = 1000;
    Agent* nearAgent = nullptr;
    for (auto agent : agents) {
        if (agent->pos().dist(target) < minDist ){
            nearAgent = agent;
            minDist = agent->pos().dist(target);
        }
    }
    ROS_INFO_STREAM("agentfinder kicker agent id : "<<nearAgent->id());
    return nearAgent;
}


/**
 * this function will order not selected robots to what to do when ballPLacement is on
 * now they go in the corner and step out of the way
 * @param nearAgent the robot that is nearest to ball
 * @param restFlag rest is all the agent except the nearAgent
 */
void COurBallPlacement::otherRobotsFormation(Agent* kickerAgent , Agent* receiverAgent) const {

    for(int i=0 ;i<agents.size() ;i++) {
        if(i != kickerAgent->id() && i != receiverAgent->id()) {                            /////////TODO: is ID uses corectly here??????
            gpa[i]->setTargetpos(wm->field->center() - Vector2D((i * 0.5 - 5), 4));
            gpa[i]->setTargetdir(Vector2D(1, 0));
            drawer->draw(wm->field->center() - Vector2D(i * 0.5 - 5, 4), QColor("red"));
            agents[i]->action = gpa[i];
        }
    }
}

/**
 * this function will find the near robot to the ball and order the oder agent to go some were else
 * @param ballPos
 * @return near agent number
 */
Agent* COurBallPlacement::firstStep(const Vector2D& ballPos ) {

    Agent* nearAgentToBall = kickerfinder(ballPos);
    ROS_INFO_STREAM("ga nearAgent in firstStepFunction: " << nearAgentToBall);
    if (!nearFlag) {
        nearFlag = true;
        nearID = nearAgentToBall;
        lastBallPos = ballPos;
    } else {
        ROS_WARN("NO AGENT");
    }

    if (isBallHaseMoved(ballPos, lastBallPos, 2) ){
        nearFlag = false;
    }
    ROS_INFO_STREAM("ga agentID in firstStepFunction: " << nearID);
    return nearID;

}

/**
 * check if pass received to desired position
 * @param ballPos
 * @param desiredPos
 * @return
 */
bool COurBallPlacement::isPassReceived(const Vector2D &ballPos, const Vector2D &desiredPos) {

    ROS_INFO_STREAM("debug KickAndRecive");
    if (!isBallNearToTarget(ballPos, desiredPos, 0.25)) {              ////////////////ball did not arrived in near of target
        ROS_INFO_STREAM("debug agent shoted");
        pass->setDontkick(false);
        return false;
    }else{                                                              ////////////////ball arrived in near of target
        ROS_INFO_STREAM("debug ball_recived");
        return true;
    }

}

/**
 *
 * @param kickerAgent
 * @param reciverAgent
 * @return
 */
bool COurBallPlacement::isAgentsOnThePosition(Agent* kickerAgent, Agent* reciverAgent) {
    ROS_INFO_STREAM(" isAgentsOnThePosition function");
    Vector2D targetedPoint = Vector2D(0,0);
    Vector2D difference = wm->ball->pos - targetedPoint;
    return isAgentOnThePosition(reciverAgent, targetedPoint, 0.25) && isAgentOnThePosition(kickerAgent, wm->ball->pos + difference.norm()*0.4, 0.1);
}


/**
 * this func will execute in execute function of masterPlay class
 */
void COurBallPlacement::execute_x(){
    Vector2D targetedPoint = Vector2D(0,0);
    ///Vector2D targetedPoint = wm->ballplacementPoint();
    kickerAgent = firstStep(wm->ball->pos);
    //if( loopCounter == 30) {
    receiverAgent = reciverFinder(targetedPoint, kickerAgent);        //////TODO: is kickerAgent id good for here//////////////////////////////////////
    //}
    if(receiverAgent == nullptr){
        ROS_INFO_STREAM("fil its null");
    }
    otherRobotsFormation(kickerAgent, receiverAgent);
    Vector2D difference = wm->ball->pos - targetedPoint;
    gpaK->setLookat(targetedPoint);
    gpaK->setTargetpos(wm->ball->pos + difference.norm()*0.4);            //////////30 centimeter behind the ball locking at targeted position
    gpaK->setTargetdir(targetedPoint);
    gpaK->setBallobstacleradius(0.15);
    gpaR->setLookat(wm->ball->pos);
    gpaR->setTargetpos(targetedPoint);
    gpaR->setTargetdir(targetedPoint);
    ///set pass action for nearAgent
    pass->setTarget(targetedPoint);
    pass->setKickspeed(4.5);
    pass->setSlow(true);
    pass->setDontkick(true);
    ROS_INFO_STREAM("speed:" << pass->getKickspeed());
    ///recive pass action
    recivePass->setReceiveradius(0.3);
    recivePass->setTarget(targetedPoint);
    recivePass->setSlow(true);
    if(isAgentsOnThePosition(kickerAgent, receiverAgent)){
        ROS_INFO_STREAM("shit agents on the positions");
        if(!isPassReceived(wm->ball->pos, targetedPoint)){
            ROS_INFO_STREAM("hamit invalid pass");
            kickerAgent->action = pass;
            receiverAgent->action = recivePass;
        } else{
            ROS_INFO_STREAM("shit pass received");
            const Vector2D position = wm->ball->pos - targetedPoint;
            gpaP->setLookat(targetedPoint);
            gpaP->setTargetpos(wm->ball->pos + position.norm()*0.4);
            gpaP->setTargetdir(targetedPoint);
            gpaH->setLookat(wm->ball->pos);
            gpaH->setTargetpos(wm->ball->pos - position.norm()*0.25);
            gpaH->setTargetdir(wm->ball->pos);
            kickerAgent->action = gpaP;
            receiverAgent->action = gpaH;
        }
    }
    else{
        ROS_INFO_STREAM("shit kickerAgent"<<kickerAgent->id()<<"reciver"<<receiverAgent->id());
            kickerAgent->action = gpaK;
            receiverAgent->action = gpaR;
    }
}

