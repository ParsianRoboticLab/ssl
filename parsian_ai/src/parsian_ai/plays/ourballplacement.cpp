#include <search.h>
#include <parsian_ai/plays/ourballplacement.h>
#include <math.h>
#include "parsian_ai/plays/ourballplacement.h"

COurBallPlacement::COurBallPlacement() {

    nearID = 0 ;
    kickerAgent = 0;
    receiverAgent = 0;
    first = true;
    loop = false;
    recivePass = new ReceivepassAction;
    pass = new KickAction;
    gpaP = new GotopointavoidAction; gpaH = new GotopointavoidAction;
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
    for (auto& g : gpa) delete g;
}

void COurBallPlacement::reset(){
    first = false;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void COurBallPlacement::init(const QList<Agent*>& _agents) {
    setAgentsID(_agents);
    initMaster();
    //if(knowledge->getLastPlayExecuted() != OurBallPlacement ){
    //    reset();

    //}
    // knowledge->setLastPlayExecuted(OurBallPlacement);
}
/**
 *
 * @param currentBallPos
 * @param lastBallPos is the posiotion ball hase befor kick
 * @return is ball moved or not!!
 */
bool COurBallPlacement::isBallHaseMoved(const Vector2D &currentBallPos, const Vector2D &lastBallPos) {
    return currentBallPos.dist(lastBallPos) > 0.5;
}

/**
 *
 * @param ballPos
 * @param ballPosBeforKick
 * @return ball kicked well or not
 */
bool COurBallPlacement::isBallDidntKickedWell(const Vector2D &ballPos, const Vector2D &ballPosBeforKick) {
    return ballPosBeforKick.dist(ballPos) < 0.3;
}

/**
 * @param agent that going to receive the ball
 * @param targetPos
 * @return is receiver agent near to the position or not
 */
bool COurBallPlacement::isReciverAgentOnThePosition(const int &agent, const Vector2D &targetPos) {
    return agents[agent]->pos().dist(targetPos) < 0.2;
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
    return sqrt(pow(velocity.x, 2) + pow(velocity.y, 2)) < 1;
}

/**
 *
 * @param ballPos
 * @param agent the agent that want to kick the ball
 * @return is kicker agent on the position or not
 */
bool COurBallPlacement::isKickerOnThePosition(const Vector2D &ballPos, const int &agent) const{
    Vector2D diffrence = agents[agent]->pos() - ballPos;
    double x = diffrence.x;
    double y = diffrence.y;
    double currentDist = sqrt(pow(x,2) + pow(y,2));
    Vector2D distance = diffrence.norm()*0.1;
    return ((diffrence.norm() * 0.15).x >= (diffrence.norm() * currentDist).x) && ((diffrence.norm() * 0.15).y >= (diffrence.norm() * currentDist).y);
    //return agents[agent]->pos().dist(ballPos) < 0.2;
}
/**
 * find the nearest agent to our desired position
 * @param pos that we want to assign a robot to it
 * @param blockedAgent is the robot that we dont want to choos among the agents
 * @return near agents id
 *
 */
int COurBallPlacement::agentFinder(const Vector2D& pos , const int &blockedAgent) {

    ROS_INFO_STREAM("gas blockedAgent : "<<blockedAgent);
    double minDist = 1000;
    int nearAgent = 0;
    for(int i = 0 ; i < agents.size() ; i++){
        if ( agents[i]->pos().dist(pos) < minDist && i != blockedAgent){
            nearAgent = i;
            minDist = agents[i]->pos().dist(wm->ball->pos);
        }
    }
    return nearAgent;

}
/**
 * this function will order not selected robots to what to do when ballPLacement is on
 * now they go in the corner and step out of the way
 * @param nearAgent the robot that is nearest to ball
 * @param restFlag rest is all the agent except the nearAgent
 */
void COurBallPlacement::otherRobotsFormation(const int& nearAgent , const int& nearTargetAgent) const {

    ROS_INFO_STREAM("debug oherRobotFormation");
    for(int i=0 ;i<agents.size() ;i++) {
        if(i != nearAgent && i != nearTargetAgent) {
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
int COurBallPlacement::firstStep(const Vector2D& ballPos ) {

    int nearAgentToBall = agentFinder(ballPos , 100);
    ROS_INFO_STREAM("ga nearAgent in firstStepFunction: " << nearAgentToBall);
    if (agents.size() > nearAgentToBall && !nearFlag) {
        nearFlag = true;
        nearID = nearAgentToBall;
        lastBallPos = ballPos;
    } else {
        ROS_WARN("NO AGENT");
    }

    if (isBallHaseMoved(ballPos, lastBallPos) ){
        nearFlag = false;
    }
    ROS_INFO_STREAM("ga agentID in firstStepFunction: " << nearID);
    return nearID;

}

bool COurBallPlacement::isPassReceived(const Vector2D &ballPos, const Vector2D &desiredPos, const int &nearAgent, const int &nearTargetAgent) {

    ROS_INFO_STREAM("debug KickAndRecive");
    ///set pass action for nearAgent
    pass->setTarget(desiredPos);
    pass->setKickspeed(6);
    ROS_INFO_STREAM("speed:" << pass->getKickspeed());
    pass->setSlow(true);
    pass->setDontkick(true);

    ///recive pass action
    recivePass->setReceiveradius(0.3);
    recivePass->setTarget(desiredPos);
    recivePass->setSlow(true);

    if(shotFlag && updateFlag) {
        ballPosBeforKick = ballPos;
    }
    if ( !isBallNearToTarget(ballPos, desiredPos, 1) || isBallDidntKickedWell(ballPos, ballPosBeforKick) ) {
        ROS_INFO_STREAM("debug agent can shot again");
        shotFlag = true;
    }
    if (isReciverAgentOnThePosition(nearTargetAgent, desiredPos) && isKickerOnThePosition(ballPos, nearAgent) && shotFlag){
        ROS_INFO_STREAM("debug agent shoted");
        pass->setDontkick(false);
        shotFlag = false;
        updateFlag = false;
    }

    bool isReceived;
    if (isBallNearToTarget(ballPos, desiredPos, 1.2) && isBallSpeedLow(1 , wm->ball->vel) ) {
        isReceived = true;
        updateFlag = false;
        shotFlag = false;
        ROS_INFO_STREAM("debug ball_recived");
    } else {
        isReceived = false;
        updateFlag = true;
        shotFlag = true;
        ROS_INFO_STREAM("debug ball_didnt_reciveed");
    }

    return isReceived;
}


/**
 * this func will execute in execute function of masterPlay class
 */
void COurBallPlacement::execute_x(){
    ROS_INFO_STREAM("gas -------------------------------");

    int pusherAgent = 0;
    int holderAgent = 0;
//    const int nearAgent = 0;
//    const int nearTargetAgent = 0;

    if(!loop) {
        kickerAgent = firstStep(wm->ball->pos);
        ROS_INFO_STREAM("gas shooter Robot : " << kickerAgent);
        receiverAgent = agentFinder(wm->ballplacementPoint(), kickerAgent);
        ROS_INFO_STREAM("gas reciver Robot : " << agents[receiverAgent]->id());
        otherRobotsFormation(kickerAgent, receiverAgent);
        loop = true;
    }
    bool isReceived = isPassReceived(wm->ball->pos, wm->ballplacementPoint(), kickerAgent, receiverAgent);
    bool phFlag;
    if(!isReceived) {
        phFlag = false;
    } else{
        phFlag = true;
        const Vector2D position = wm->ball->pos - wm->ballplacementPoint();
        pusherAgent = kickerAgent;
        holderAgent = receiverAgent;
        gpaP->setLookat(wm->ballplacementPoint());
        gpaP->setTargetpos(wm->ball->pos + position.norm()*0.3 );
        gpaP->setTargetdir(Vector2D(1,0));
        gpaH->setLookat(wm->ball->pos);
        gpaH->setTargetpos(wm->ball->pos - position.norm()*0.1);
        gpaH->setTargetdir(Vector2D(1,0));
    }

    if (phFlag){
        ROS_INFO_STREAM("checkk pusher agent : "<< agents[pusherAgent]->id());
        agents[pusherAgent]->action = gpaP;
        ROS_INFO_STREAM("checkk holder agent : "<< agents[holderAgent]->id());
        agents[holderAgent]->action = gpaH;
    } else{

        ROS_INFO_STREAM("ga khob");
        agents[kickerAgent]->action = pass;
        agents[receiverAgent]->action = recivePass;
    }


}