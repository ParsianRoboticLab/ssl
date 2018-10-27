#include <search.h>
#include <parsian_ai/plays/ourballplacement.h>
#include <math.h>
#include "parsian_ai/plays/ourballplacement.h"

COurBallPlacement::COurBallPlacement() {

    minIndexPos = 0;
    nearTargetAgent = 0;
    nearAgentToBall = 0;
    ap = nullptr;
    a = nullptr;
    first = true;
    state = BallPlacement :: GO_FOR_BALL;
    gpa = new GotopointavoidAction;
    recivePass = new ReceivepassAction;
    pass = new KickAction;
    //gpar = new GotopointavoidAction;
    gpa0 = new GotopointavoidAction;gpa1 = new GotopointavoidAction;gpa2 = new GotopointavoidAction;gpa3 = new GotopointavoidAction;
    gpa4 = new GotopointavoidAction;gpa5 = new GotopointavoidAction;gpa6 = new GotopointavoidAction;gpa7 = new GotopointavoidAction;
    nearFlag = false;  ///// near agent flag
    shotFlag = false;  ///// near agent kick the ball or not
    updateFlag = false;   /////
    reciveFlag = false;   ///// reciver robot have recived the ball or not
    phFlag = false;       /////pusher and holder robots are set
    lastBallPos =  Vector2D();
    ballPosBeforKick = Vector2D();

}

COurBallPlacement::~COurBallPlacement() {

}

void COurBallPlacement::reset(){
    flag = false;
    first = false;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void COurBallPlacement::init(QList<Agent*>& _agents) {
    agents = _agents;
    initMaster();
}

bool COurBallPlacement::ballHaseMoved(const Vector2D& currentBallPos , const Vector2D& lastBallPos) const{
    return currentBallPos.dist(lastBallPos) > 0.5;
}

bool COurBallPlacement::ballDidntAriveToTarget(const Vector2D& ballPos , const Vector2D& desiredPos , const Vector2D& ballPosBeforKick) const {
    return ballPosBeforKick.dist(ballPos) > ballPosBeforKick.dist(desiredPos) + 2;
}

bool COurBallPlacement::ballDidntKickedWell(const Vector2D& ballPos, const Vector2D& ballPosBeforKick ) const {
    return ballPosBeforKick.dist(ballPos) < 0.3;
}

bool COurBallPlacement::reciverAgentIsOnThePosition(const int &agent, const Vector2D &targetPos) const{
    return agents[nearTargetAgent]->pos().dist(targetPos) < 0.2;
}

bool COurBallPlacement::ballIsNearToTarget(const Vector2D &ballPos, const Vector2D &targetPos) const {
    return targetPos.dist(ballPos) < 0.5;
}
bool COurBallPlacement::ballSpeedIsLow() {
    return sqrt(pow(wm->ball->vel.x, 2) + pow(wm->ball->vel.x, 2)) < 0.2;
}

bool COurBallPlacement::kickerAgentIsNearToBall(const Vector2D &ballPos, const int &agent) const{
    return agents[agent]->pos().dist(ballPos) < 0.2;
}

/**
 * find the nearest agent to our desired position
 * @param pos that we want to assign a robot to it
 * @param blockedAgent is the robot that we dont want to choos among the agents
 * @return near agents id
 *
 */
int COurBallPlacement::agentFinder(const Vector2D& pos , int blockedAgent) const {

    double minDist = 1000;
    int nearAgent=0;

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
void COurBallPlacement::otherRobotsFormation(const int& nearAgent) const {

    for(int i=0 ;i<agents.size() ;i++) {
        if(i != nearAgent) {
            switch (i) {
                case 0: {
                    gpa0->setTargetpos(wm->field->center() - Vector2D((i * 0.5 - 5), 4));
                    gpa1->setTargetdir(Vector2D(1, 0));
                    drawer->draw(wm->field->center() - Vector2D(i * 0.5 - 5, 4), QColor("red"));
                    agents[i]->action = gpa0;
                    break;
                }
                case 1: {
                    gpa0->setTargetpos(wm->field->center() - Vector2D((i * 0.5 - 5), 4));
                    gpa1->setTargetdir(Vector2D(1, 0));
                    drawer->draw(wm->field->center() - Vector2D(i * 0.5 - 5, 4), QColor("red"));
                    agents[i]->action = gpa1;
                    break;
                }
                case 2: {
                    gpa0->setTargetpos(wm->field->center() - Vector2D((i * 0.5 - 5), 4));
                    gpa1->setTargetdir(Vector2D(1, 0));
                    drawer->draw(wm->field->center() - Vector2D(i * 0.5 - 5, 4), QColor("red"));
                    agents[i]->action = gpa2;
                    break;
                }
                case 3: {
                    gpa0->setTargetpos(wm->field->center() - Vector2D((i * 0.5 - 5), 4));
                    gpa1->setTargetdir(Vector2D(1, 0));
                    drawer->draw(wm->field->center() - Vector2D(i * 0.5 - 5, 4), QColor("red"));
                    agents[i]->action = gpa3;
                    break;
                }
                case 4: {
                    gpa0->setTargetpos(wm->field->center() - Vector2D((i * 0.5 - 5), 4));
                    gpa1->setTargetdir(Vector2D(1, 0));
                    drawer->draw(wm->field->center() - Vector2D(i * 0.5 - 5, 4), QColor("red"));
                    agents[i]->action = gpa4;
                    break;
                }
                case 5: {
                    gpa0->setTargetpos(wm->field->center() - Vector2D((i * 0.5 - 5), 4));
                    gpa1->setTargetdir(Vector2D(1, 0));
                    drawer->draw(wm->field->center() - Vector2D(i * 0.5 - 5, 4), QColor("red"));
                    agents[i]->action = gpa5;
                    break;
                }
                case 6: {
                    gpa0->setTargetpos(wm->field->center() - Vector2D((i * 0.5 - 5), 4));
                    gpa1->setTargetdir(Vector2D(1, 0));
                    drawer->draw(wm->field->center() - Vector2D(i * 0.5 - 5, 4), QColor("red"));
                    agents[i]->action = gpa6;
                    break;
                }
                case 7: {
                    gpa0->setTargetpos(wm->field->center() - Vector2D((i * 0.5 - 5), 4));
                    gpa1->setTargetdir(Vector2D(1, 0));
                    drawer->draw(wm->field->center() - Vector2D(i * 0.5 - 5, 4), QColor("red"));
                    agents[i]->action = gpa7;
                    break;
                }
                default:break;
            }
        }
    }
}

/**
 * @param desired desired target position
 * @param ballPos current ball position
 * @return teta between ball and desierd target position
 */
double COurBallPlacement::tetaFinder(const Vector2D& desired, const Vector2D& ballPos ) const {
    double x = desired.x - ballPos.x;
    double y = desired.y - ballPos.y;
    return atan(y/x);
}

/**
 * @param desired  position
 * @param ballPos positino
 * @param dist distance between ball and agent
 * @return distance between ball and agent in y axis
 */
double COurBallPlacement::yFinder(const Vector2D& desired , const Vector2D& ballPos , const double& dist) const {
    return dist*sin(tetaFinder(desired , ballPos));
}
/**
 * @param desired position
 * @param ballPos position
 * @param dist distance between ball and agent
 * @return distance between ball and agent in x axis
 */
double COurBallPlacement::xFinder(const Vector2D& desired , const Vector2D& ballPos , const double& dist) const {
    return dist*cos(tetaFinder(desired , ballPos));
}


/**
 * this function will find the near robot to the ball and order the oder agent to go some were else
 * @param ballPos
 * @return near agent number
 */
int COurBallPlacement::firstStep(const Vector2D& ballPos ) {

    if (nearFlag)
        return nearAgentToBall;

    nearAgentToBall = agentFinder(ballPos , 100);

    if (agents.size() > nearAgentToBall) {
        nearFlag = true;
        ROS_INFO_STREAM("hamit agent ID: " << agents[nearAgentToBall]->id());
        ROS_INFO_STREAM("hamit agent ID: " << nearAgentToBall);
        lastBallPos = ballPos;
    } else {
        ROS_WARN("NO AGENT");
    }

    if ( ballHaseMoved(ballPos,lastBallPos) ){
        nearFlag = false;
    }

    otherRobotsFormation(nearAgentToBall);

    return nearAgentToBall;


}

/**
 * this func will execute in execute function of masterPlay class
 */
void COurBallPlacement::execute_x(){

    int pusherAgent = 0;
    int holderAgent = 0;
    const Vector2D& ballPos = wm->ball->pos;

    const Vector2D& desiredPos = wm->ballplacementPoint();
    const int& nearAgent = firstStep(ballPos);

    if(!phFlag) {
        ///set pass action for nearAgent
        pass->setTarget(desiredPos);
        pass->setKickspeed(30);
        ROS_INFO_STREAM("speed:" << pass->getKickspeed());
        pass->setSlow(true);
        pass->setDontkick(true);

        ///recive pass action
        recivePass->setReceiveradius(1);
        recivePass->setTarget(desiredPos);
        recivePass->setSlow(true);
    }

    if(!shotFlag && !updateFlag) {
        ballPosBeforKick = ballPos;
    }
    if ( ballDidntAriveToTarget(ballPos,desiredPos,ballPosBeforKick) || ballDidntKickedWell(ballPos,ballPosBeforKick) ) {
        ROS_INFO_STREAM("hamid agent can shot again");
        shotFlag = false;
    }
    if ( reciverAgentIsOnThePosition(nearTargetAgent , desiredPos) && kickerAgentIsNearToBall(ballPos , nearAgent) && !shotFlag){
        ROS_INFO_STREAM("hamid agent shoted");
        shotFlag = true;
        updateFlag = true;
        pass->setDontkick(false);
    }

    if ( ballIsNearToTarget(ballPos , desiredPos)) {
        reciveFlag = ballSpeedIsLow();
    } else {
        reciveFlag = false;
    }
    if(reciveFlag && desiredPos.dist(ballPos) > 2) {
        updateFlag = false;
        shotFlag = false;
        phFlag = false;
    }
    else if(reciveFlag && shotFlag) {
        phFlag = true;
        pusherAgent = nearTargetAgent;
        holderAgent = agentFinder(ballPos , pusherAgent);
        gpa->setLookat(desiredPos);
        gpa->setTargetpos(ballPos - Vector2D(xFinder(desiredPos , ballPos , 0.1) , yFinder(desiredPos , ballPos , 0.1)));
        gpa->setTargetdir(Vector2D(1,0));
    }
    if (phFlag){
        agents[pusherAgent]->action = gpa;
    } else{
        nearTargetAgent = agentFinder(desiredPos , nearAgent);
        agents[nearTargetAgent]->action = recivePass;
        agents[nearAgent]->action = pass;
    }


    /*
    double mindist = 10000;
    double dist;
    CAgent *pa = a;
    for(int i = 0 ; i < agents.size() ; i++){
        dist = agents[i]->pos().dist(ballPos);
        if(dist < mindist && minIndexPos != i){
            mindist = dist;
            minIndex = i;
            a = agents[i];
        }
    }
    a->action = gpa;

    return;

      Vector2D ballPos = Vector2D(wm->ball->pos.x , wm->ball->pos.y);
      Vector2D robotTarget = Vector2D(ballPos.x - 1 , ballPos.y);
      Vector2D pos = Vector2D(6, 0);
      auto *gpa = new GotopointavoidAction;
      gpa->setTargetpos(robotTarget);
      gpa->setSlowmode(true);
      gpa->setBallobstacleradius(0.15);
      agents[1]->action = gpa;

    ROS_INFO("Executaion X");
    ROS_INFO_STREAM(flag);
    if(agents.size() <= 1)
        return;
    Vector2D ballpos = Vector2D(wm->ball->pos.x, wm->ball->pos.y);
    Vector2D pos = wm->ballplacementPoint();
    double dist = 0;
    double mindist = 10000;
    if (first) {
        a = ap = agents[0];
        first = false;
        for(int i = 0 ; i < agents.size()   ; i++){
            dist = agents[i]->pos().dist(pos);
            if (dist < mindist){
                mindist = dist;
                minIndexPos = i;
                ap = agents[i];
            }
        }
    }
    mindist = 10000;
    CAgent *pa = a;
    for(int i = 0 ; i < agents.size() ; i++){
        dist = agents[i]->pos().dist(ballpos);
        if(dist < mindist && minIndexPos != i){
            mindist = dist;
            minIndex = i;
            a = agents[i];
        }
    }
    ROS_INFO("Executaion 1");
    auto *nothing = new NoAction;
    ROS_INFO("Executaion 1.2");
    if (pa->id() != a->id()) {
        pa->action = nothing;
        if(state == BallPlacement :: PASS){
            state = BallPlacement :: GO_FOR_BALL;
        }
    }
    ROS_INFO("Executaion 1.5");

    Vector2D behindBall = ballpos - Vector2D(pos - ballpos).norm() * 0.20;
    Circle2D validcir{pos , 0.2};
    Circle2D invalidcir{pos, 1 - 0.1};
    Vector2D sol1, sol2;
    drawer->draw(Segment2D(ballpos , ballpos + wm->ball->vel.norm()) , QColor(Qt::blue));

    if(state == BallPlacement :: GO_FOR_BALL && agents[minIndexPos]->pos().dist(pos) < 0.1 && agents[minIndex]->pos().dist(behindBall) < 0.1){
        state = BallPlacement :: PASS;
        passballpos = ballpos;
    }
    if(state == BallPlacement :: PASS &&
    (invalidcir.contains(ballpos) || invalidcir.intersection(Segment2D(ballpos, ballpos + wm->ball->vel.norm()),&sol1,&sol2) > 0)){
        state = BallPlacement :: GO_FOR_VALID_PASS;
    }
    ROS_INFO_STREAM("EDs: " << agents[minIndexPos]->pos().dist(pos) << " -- " << agents[minIndex]->pos().dist(behindBall) << " -- " << validcir.contains(ballpos) <<
    " -- " << validcir.intersection(Segment2D(ballpos, ballpos + wm->ball->vel.norm()),&sol1,&sol2) << " -- " << agents[minIndexPos]->pos().dist(ballpos));
    drawer->draw(validcir,QColor(Qt::red));
    if(state == BallPlacement :: GO_FOR_VALID_PASS && agents[minIndexPos]->pos().dist(pos) < 0.1 && agents[minIndex]->pos().dist(behindBall) < 0.1){
        state = BallPlacement  :: VALID_PASS;
    }
    if(state == BallPlacement :: VALID_PASS &&
    (validcir.contains(ballpos) || validcir.intersection(Segment2D(ballpos, ballpos + wm->ball->vel.norm()),&sol1,&sol2) > 0)){
        state = BallPlacement :: RECIVE_AND_POS;
    }
    if(state == BallPlacement :: RECIVE_AND_POS && invalidcir.contains(ballpos) && !validcir.contains(ballpos) &&
    validcir.intersection(Segment2D(ballpos, ballpos + wm->ball->vel.norm()),&sol1,&sol2) == 0){
        state = BallPlacement :: GO_FOR_VALID_PASS;
    }
    if(state == BallPlacement :: RECIVE_AND_POS && !invalidcir.contains(ballpos) && !validcir.contains(ballpos)){
        state = BallPlacement :: GO_FOR_BALL;
    }
    if(state == BallPlacement :: VALID_PASS && agents[minIndex]->pos().dist(ballpos) > 0.3){
        state = BallPlacement :: GO_FOR_VALID_PASS;
    }
    if(state == BallPlacement :: GO_FOR_VALID_PASS && !invalidcir.contains(ballpos) && !validcir.contains(ballpos)){
        state = BallPlacement :: GO_FOR_BALL;
    }
    if(state == BallPlacement :: PASS && agents[minIndex]->pos().dist(ballpos) > 2){
        state = BallPlacement :: GO_FOR_BALL;
    }

    //GO_FOR_BALL
    auto *rec = new ReceivepassAction();
    rec->setReceiveradius(1);
    rec->setTarget(pos);
    rec->setSlow(true);
    auto *gpa = new GotopointavoidAction;
    gpa->setTargetpos(behindBall);
    gpa->setSlowmode(true);
    gpa->setBallobstacleradius(0.15);
    gpa->setLookat(pos);

    //PASS
    auto *pass = new KickAction();
    pass->setTarget(pos);
    double power = .3 * pos.dist(ballpos);
    if(power<.5) power = .5;
    if(power>1.5) power = 1.5;
    ROS_INFO_STREAM("shoot speed:"<< power);
    pass->setKickspeed(power);
//    pass->setSpin(1);
    pass->setSlow(true);

    //GO_FOR_VALID_PASS
    auto *vrec = new ReceivepassAction();
    vrec->setReceiveradius(0.2);
    vrec->setTarget(pos);
    vrec->setSlow(true);

    //VALID_PASS

    //RECIVE_AND_POS
    auto *recSpin = new ReceivepassAction();
    recSpin->setReceiveradius(0.2);
    recSpin->setTarget(pos);
    recSpin->setSlow(true);
    //spin

    //FINAL_POS
    auto *gpas = new GotopointavoidAction();
    gpas->setTargetpos(pos);
    gpas->setLookat(ballpos);
    gpas->setSlowmode(true);
//    gpas->setRoller(1);

    switch(state){
        case BallPlacement :: NoState:
            //:))
            break;
        case BallPlacement :: GO_FOR_BALL://noghtash doroste vali mikhore be top:-?
            ROS_INFO_STREAM("ED: " << "GO_FOR_BALL");
            agents[minIndexPos]->action = rec;
            agents[minIndex]->action = gpa;
            break;
        case BallPlacement :: PASS:
            ROS_INFO_STREAM("ED: " << "PASS");
            agents[minIndexPos]->action = rec;
            agents[minIndex]->action = pass;
            break;
        case BallPlacement :: GO_FOR_VALID_PASS:
            ROS_INFO_STREAM("ED: " << "GO_FOR_VALID_PASS");
            agents[minIndexPos]->action = vrec;
            agents[minIndex]->action = gpa;
            break;
        case BallPlacement :: VALID_PASS:
            ROS_INFO_STREAM("ED: " << "VALID_PASS");
            agents[minIndexPos]->action = vrec;
            agents[minIndex]->action = pass;
            break;
        case BallPlacement :: RECIVE_AND_POS:
            ROS_INFO_STREAM("ED: " << "RECIVE_AND_POS");
            agents[minIndexPos]->action = recSpin;
            agents[minIndex]->action = nothing;
            break;
        case BallPlacement :: FINAL_POS:
            ROS_INFO_STREAM("ED: " << "FINAL_POS");
            agents[minIndexPos]->action = gpas;
            agents[minIndex]->action = nothing;
            break;
        case BallPlacement :: DONE:
            ROS_INFO_STREAM("ED: " << "DONE");
            agents[minIndexPos]->action = nothing;
            agents[minIndex]->action = nothing;
            break;
        default:
            break;
    }
    */
}

int COurBallPlacement::chooseFirst() {
    return -1;
}








