#include <search.h>
#include "parsian_ai/plays/ourballplacement.h"

COurBallPlacement::COurBallPlacement() {

    minIndexPos = 0;
    ap = nullptr;
    minIndex = 0;
    a = nullptr;
    first = true;
    state = BallPlacement :: GO_FOR_BALL;
}

COurBallPlacement::~COurBallPlacement() {

}

void COurBallPlacement::reset(){
    flag = false;
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


void COurBallPlacement::execute_x(){

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

    Vector2D behindBall = ballpos - Vector2D(pos - ballpos).norm() * 0.15;
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
    gpa->setBallobstacleradius(0.18);
    gpa->setLookat(pos);

    //PASS
    auto *pass = new KickAction();
    pass->setTarget(pos);
    int power = (int)(50 * pos.dist(ballpos));
    ROS_INFO_STREAM(power);
    pass->setKickspeed(power);
    pass->setSpin(5);
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
    gpas->setRoller(4);

    switch(state){
        case BallPlacement :: NoState:
            //:)
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
}