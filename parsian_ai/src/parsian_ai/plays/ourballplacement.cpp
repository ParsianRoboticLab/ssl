#include <search.h>
#include <parsian_ai/plays/ourballplacement.h>
#include <math.h>
#include "parsian_ai/plays/ourballplacement.h"

COurBallPlacement::COurBallPlacement() {

    loopCounter = 0;
    find = false;
    fuckOff = -1;
    first = true;
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
 * @return kick speed
 */
double COurBallPlacement::kickSpeedCalculator(const Vector2D &ballPos ,const Vector2D &targetPos) {
    if(ballPos.dist(targetPos) > 8 ) {
        return conf.ballPlacementLongKick+2;
    }
    else if(ballPos.dist(targetPos) > 3 ) {
        return conf.ballPlacementMedimKick+2;
    }
    else {
        return conf.ballPlacementSlowKick+2;
    }
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
 * @param nearAgent the robot that is nearest to ball should
 * @param restFlag rest is all the agent except the nearAgent
 */
void COurBallPlacement::otherRobotsFormation(Agent* kickerAgent , Agent* receiverAgent)  {

    Segment2D ballToTargetLine = Segment2D(wm->ball->pos, Vector2D(-4, -1));
    Segment2D robotXLine = Segment2D(Vector2D(-1, -1), Vector2D(-1, -1));
    Segment2D robotYLine = Segment2D(Vector2D(-1, -1), Vector2D(-1, -1));
    for(auto agent : agents){
        if(agent->id() != kickerAgent->id() && agent->id() != receiverAgent->id()){
            robotXLine = Segment2D(Vector2D(agent->pos().x-robot_radius_new, agent->pos().y), Vector2D(agent->pos().x + robot_radius_new, agent->pos().y));
            robotYLine = Segment2D(Vector2D(agent->pos().x, agent->pos().y + robot_radius_new), Vector2D(agent->pos().x, agent->pos().y - robot_radius_new));
            if(ballToTargetLine.existIntersection(robotXLine) || ballToTargetLine.existIntersection(robotYLine)){
                fuckOff = agent->id();
                gpa[agent->id()]->setTargetpos(wm->field->center() - Vector2D(-5, -3));
                gpa[agent->id()]->setTargetdir(Vector2D(100, 0));
                //drawer->draw(wm->field->center() - Vector2D(-5, - 3)), QColor("red"));
                agents[agent->id()]->action = gpa[agent->id()];
                //loopCounter++;
                //ROS_INFO_STREAM("checkintersect  ....  exist "<< agent->id() << loopCounter);
            }else{
                if(fuckOff == agent->id()){
                    gpa[agent->id()]->setTargetpos(wm->field->center() - Vector2D(-4, -3));
                    gpa[agent->id()]->setTargetdir(Vector2D(100, 0));
                    //drawer->draw(wm->field->center() - Vector2D(-5, - 3)), QColor("red"));
                    agents[agent->id()]->action = gpa[agent->id()];
                } else {
                    gpa[agent->id()]->setTargetpos(wm->field->center() - Vector2D(-4, agent->id() * 0.3 - 2));
                    gpa[agent->id()]->setTargetdir(Vector2D(100, 0));
                    //drawer->draw(wm->field->center() - Vector2D(-4, agent->id()*0.3 - 2)), QColor("red"));
                    agents[agent->id()]->action = gpa[agent->id()];
                }
            }
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
    if (isBallNearToTarget(ballPos, desiredPos, 0.3) && isBallSpeedLow(0.2, wm->ball->vel)) {
        ROS_INFO_STREAM("hahaha ball_recived");
        return true;
    }else{
        ROS_INFO_STREAM("debug agent shoted");
        pass->setDontkick(false);
        return false;////////////////ball arrived in near of targe
    }

}

/**
 *
 * @param kickerAgent
 * @param reciverAgent
 * @return is kicker AND receiver agents are on their position
 */
bool COurBallPlacement::isAgentsOnThePosition(Agent* kickerAgent, Agent* reciverAgent) {
    ROS_INFO_STREAM(" isAgentsOnThePosition function");
    Vector2D targetedPoint = Vector2D(-4,-1);
    Vector2D difference = wm->ball->pos - targetedPoint;
    const double threshold = 0.45;
    return reciverAgent->pos().dist(targetedPoint) < 0.25 && kickerAgent->pos().dist(wm->ball->pos + difference.norm()*threshold) < threshold - robot_radius_new + 0.06;
}


/**
 * this func will execute in execute function of masterPlay class
 */
void COurBallPlacement::execute_x(){
    Vector2D targetedPoint = Vector2D(-4, -1);
    //Vector2D targetedPoint = wm->ballplacementPoint()
    if(!find){
        kickerAgent = firstStep(wm->ball->pos);
        receiverAgent = reciverFinder(targetedPoint, kickerAgent);
    }
    if(!find && kickerAgent != nullptr && receiverAgent != nullptr) {
        find = true;
    }
    if(wm->ball->pos.dist(kickerAgent->pos()) > 8){
        find = false;
    }
    otherRobotsFormation(kickerAgent, receiverAgent);
    Vector2D difference = wm->ball->pos - targetedPoint;
    gpaK->setLookat(targetedPoint);
    gpaK->setTargetpos(wm->ball->pos + difference.norm()*0.45);
    gpaK->setTargetdir(targetedPoint);
    gpaK->setBallobstacleradius(0.20);
    gpaR->setLookat(wm->ball->pos);
    gpaR->setTargetpos(targetedPoint);
    gpaR->setTargetdir(targetedPoint);
    ///set pass action for nearAgent
    pass->setTarget(targetedPoint);
    pass->setKickspeed(kickSpeedCalculator(wm->ball->pos, targetedPoint));
    pass->setSlow(true);
    pass->setDontkick(true);
    pass->setIsplayoff(true);
    ROS_INFO_STREAM("speed:" << pass->getKickspeed());
    ///recive pass action
    recivePass->setReceiveradius(0.6);
    recivePass->setTarget(targetedPoint);
    recivePass->setSlow(true);
    if(isAgentsOnThePosition(kickerAgent, receiverAgent)){
        ROS_INFO_STREAM("shit agents on the positions");
        if(isPassReceived(wm->ball->pos, targetedPoint)){
            ROS_INFO_STREAM("shit pass received");
            fuckOff = -1;
            const Vector2D position = wm->ball->pos - targetedPoint;
            gpaP->setLookat(targetedPoint);
            gpaP->setTargetpos(wm->ball->pos + position.norm()*0.4);
            gpaP->setTargetdir(targetedPoint);
            gpaH->setLookat(wm->ball->pos);
            gpaH->setTargetpos(wm->ball->pos - position.norm()*0.25);
            gpaH->setTargetdir(wm->ball->pos);
            kickerAgent->action = gpaP;
            receiverAgent->action = gpaH;
            //receiverAgent->action = recivePass;

        } else{
            ROS_INFO_STREAM("hamit invalid pass");
            kickerAgent->action = pass;
            receiverAgent->action = recivePass;
        }
    }
    else{
        ROS_INFO_STREAM("shit kickerAgent"<<kickerAgent->id()<<"reciver"<<receiverAgent->id());
            kickerAgent->action = gpaK;
            receiverAgent->action = gpaR;
    }
}

