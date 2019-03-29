
#include <parsian_ai/plays/playoff/dynamicplayoff.h>

CDynamicPlayOff::CDynamicPlayOff() {
//    dummyPositions[1] = Vector2D{3.2, 0.7};
//    dummyPositions[2] = Vector2D{3.2, -0.7};
//    dummyPositions[3] = Vector2D{3, 1.5};
//    dummyPositions[4] = Vector2D{3, -2.5};
//    dummyPositions[5] = Vector2D{3, 2.5};
//    dummyPositions[6] = Vector2D{3, 3.5};
//    dummyPositions[7] = Vector2D{3, -3.5};

    reset();
}

CDynamicPlayOff::~CDynamicPlayOff(){
}

void CDynamicPlayOff::reset() {
    state = DynamicState::None;
    dynamicStartTime = 0;
}
void CDynamicPlayOff ::Setposition(){



    theirdist=100;
    for(int i=0;i< wm->opp.activeAgentsCount();i++)
    {
         if(wm->opp.active(i)->pos.dist(wm->ball->pos)<theirdist )
         {
              theirdist=wm->opp.active(i)->pos.dist(wm->ball->pos);
              theirpos=wm->opp.active(i)->pos;
              n=i;
         }
    }
    ROS_INFO_STREAM("maral: blocker"<<wm->opp.active(n)->id);


    Line2D line1= Line2D(wm->ball->pos,wm->field->oppGoal());
    Segment2D segment1=Segment2D(wm->ball->pos,wm->field->oppGoal());
    drawer->draw(segment1,QColor("blue"));

    Line2D line2= line1.perpendicular(theirpos);

    Vector2D vect=line1.intersection(line2);
    Segment2D segment2=Segment2D(vect,theirpos);
    drawer->draw(vect,QColor("red"), 20);
    drawer->draw(segment2,QColor("black"));
    Vector2D VECTOR=(wm->field->oppGoal()-vect).norm()*0.4;
    dummyPositions[0]=VECTOR+vect;
    drawer->draw(dummyPositions[0],QColor("red"), 20);


        dummyPositions[1] = Vector2D{0, 0};
        dummyPositions[2] = Vector2D{0, 0};
        dummyPositions[3] = Vector2D{0, 0};
        dummyPositions[4] = Vector2D{0, 0};
        dummyPositions[5] = Vector2D{0, 0};
        dummyPositions[6] = Vector2D{0, 0};
        dummyPositions[7] = Vector2D{0, 0};


//    dummyPositions[1]=vector+theirpos;
//    drawer->draw(dummyPositions[0], QColor("black"), 2);

//    if(agents.size()>4){
//        dummyPositions[1] = Vector2D{3.2, 0.7};
//        dummyPositions[2] = Vector2D{3, -0.7};
//        dummyPositions[3] = Vector2D{3, 1.5};
//        dummyPositions[4] = Vector2D{-3, -2.5};
//        dummyPositions[5] = Vector2D{-3, 2.5};
//        dummyPositions[6] = Vector2D{-3, 3.5};
//        dummyPositions[7] = Vector2D{-3, -3.5};

//    }
//    else
//    {
//        dummyPositions[1] = Vector2D{3, 0.7};
//        dummyPositions[2] = Vector2D{3, -0.7};
//        dummyPositions[3] = Vector2D{2.5, 1.5};
//        dummyPositions[4] = Vector2D{2.5, -2.5};
//        dummyPositions[5] = Vector2D{2.5, 2.5};
//        dummyPositions[6] = Vector2D{2.5, 3.5};
//        dummyPositions[7] = Vector2D{2.5, -3.5};

//    }






}

void CDynamicPlayOff::execute() {

    switch (dynamicSelect) {
        case DynamicSelect::NoSelect:
            break;
        case DynamicSelect::Chip:
            dynamicPlayChipToGoal(true);
            matchAgent();
            checkEndChipToGoal();
            break;
        case DynamicSelect::Kick:
            dynamicPlayChipToGoal(false);
            matchAgent();
            checkEndChipToGoal();
            break;
        case DynamicSelect::Khafan:
            Setposition();
            dynamicPlayKhafan();
          if (state == DynamicState::Ready) matchAgent();
            checkEndKhafan();
            break;
    }


    for (int i = 0; i < agents.size(); i++) {
        roleAgents[i]->execute();
    }
}

void CDynamicPlayOff::dynamicPlayChipToGoal(bool isChip) {
    switch (state) {
        case DynamicState::Ready:
            roleAgents[0] -> setAvoidCenterCircle(false);
            roleAgents[0] -> setAvoidPenaltyArea(true);
            roleAgents[0] -> setChip(isChip);
            roleAgents[0] -> setKickSpeed(6.5); // TODO: Use Global Constants
            roleAgents[0] -> setTarget(wm->field->oppGoal());
            roleAgents[0] -> setDoPass(false);
            roleAgents[0] -> setIntercept(false);
            roleAgents[0] -> setLookForward(false);
            roleAgents[0] -> setSelectedSkill(RoleSkill::Kick);

            for (int i = 1; i < dynamicAgentSize; i++) {
                if (dynamicMatch[i] != -1) {
                    roleAgents[i] -> setAvoidPenaltyArea(true);
                    roleAgents[i] -> setAvoidBall(true);
                    roleAgents[i] -> setTimeBased(false);
                    roleAgents[i] -> setTarget(dummyPositions[i + 1]);
                    roleAgents[i] -> setLookAt(-wm->field->oppGoal());
                    roleAgents[i] -> setEventDist(0.3);
                    roleAgents[i] -> setSlow(false);
                    roleAgents[i] -> setSelectedSkill(RoleSkill::GotopointAvoid);
                }
            }
            break;
        case DynamicState::None:break;
        case DynamicState::Pass:break;
        case DynamicState::Shot:
            roleAgents[0]->setDoPass(true);
            break;
    }

}

void CDynamicPlayOff::dynamicPlayKhafan() {
    switch (state) {
        case DynamicState::Ready:
            roleAgents[0] -> setAvoidCenterCircle(false);
            roleAgents[0] -> setAvoidPenaltyArea(true);
            roleAgents[0] -> setChip(true);
            roleAgents[0] -> setKickSpeed(conf.LowDistChip); // Vartypes This
            roleAgents[0] -> setTarget(wm->field->oppGoal());
            roleAgents[0] -> setDoPass(false);
            roleAgents[0] -> setIntercept(false);
            roleAgents[0] -> setTargetDir(wm->field->oppGoal());
            roleAgents[0] -> setSelectedSkill(RoleSkill::Kick);

            for (int i = 1; i < agents.size(); i++) {
                    roleAgents[i] -> setAvoidPenaltyArea(true);
                    roleAgents[i] -> setAvoidBall(true);
                    roleAgents[i] -> setTimeBased(false);
                    roleAgents[i] -> setTarget(dummyPositions[i - 1]);
                    roleAgents[i] -> setLookAt(wm->field->oppGoal());
                    roleAgents[i] -> setEventDist(0.3);
                    roleAgents[i] -> setSlow(false);
                    roleAgents[i] -> setSelectedSkill(RoleSkill::GotopointAvoid);
            }
            break;
        case DynamicState::Pass:
            roleAgents[0] -> setDoPass(true);


            break;
        case DynamicState::Shot:
            roleAgents[1] -> setAvoidCenterCircle(false);
            roleAgents[1] -> setAvoidPenaltyArea(true);
            roleAgents[1] -> setChip(false);
            roleAgents[1] -> setKickSpeed(conf.MediumSpeedPass); // Vartypes This
            roleAgents[1] -> setTarget(wm->field->oppGoal());
            roleAgents[1] -> setDoPass(true);
            roleAgents[1] -> setIntercept(false);
            roleAgents[1] -> setTargetDir(wm->field->oppGoal());
            roleAgents[1] -> setSelectedSkill(RoleSkill::Kick);
            roleAgents[0] -> setAvoidPenaltyArea(true);
            roleAgents[0] -> setAvoidBall(true);
            roleAgents[0] -> setTimeBased(false);
            roleAgents[0] -> setTarget(Vector2D(0, -2));
            roleAgents[0] -> setLookAt(wm->field->oppGoal());
            roleAgents[0] -> setEventDist(0.3);
            roleAgents[0] -> setSlow(false);
            roleAgents[0] -> setSelectedSkill(RoleSkill::GotopointAvoid);
            break;
        case DynamicState::None:break;
    }

}
int CDynamicPlayOff:: EvalPlayKhafan(){
    if(agents.size()<2)
        eval=0;
    if(wm->opp.activeAgentsCount()==0)
        eval=100;

theirdist=100;
n=0;
for(int i=0;i< wm->opp.activeAgentsCount();i++)
{
    if(wm->opp.active(i)->pos.dist(wm->ball->pos)<theirdist){
        theirdist=wm->opp.active(i)->pos.dist(wm->ball->pos);
        theirpos= wm->opp.active(i)->pos;
        n=i;
    }
}
for (int i=0;i< wm->opp.activeAgentsCount();i++){
    if(Triangle2D(theirpos,wm->field->oppGoalL(),wm->field->oppGoalR()).contains(wm->opp.active(i)->pos))
        sum++;
}
if(sum==0){
    eval=100;
}
for(int i=0 ;i< wm->opp.activeAgentsCount();i++)
{
    if(Triangle2D(theirpos,wm->field->oppGoalR(),wm->field->oppGoalL()).contains(wm->opp.active(i)->pos))
    {
        if(wm->opp.active(i)->pos.dist(theirpos)<Robot::robot_radius_new*3)
            eval=0;
        else if(wm->opp.active(i)->pos.dist(theirpos)<0.2)
            eval=20;
        else if(wm->opp.active(i)->pos.dist(theirpos)<0.4)
            eval=40;
        else if(wm->opp.active(i)->pos.dist(theirpos)<0.6)
            eval=60;
        else if(wm->opp.active(i)->pos.dist(theirpos)<0.8)
            eval=80;
        else if(wm->opp.active(i)->pos.dist(theirpos)<1)
            eval=100;

        return eval;

    }
}


return eval;
}


void CDynamicPlayOff::checkEndKhafan() {
    ROS_INFO_STREAM("TIMENS: "<< ros::Time::now().sec << " TIMES: "<< ros::Time::now().sec);
    switch (state) {
        case DynamicState ::Ready:
            if (roleAgents[1] -> getAgent() -> pos().dist(roleAgents[1] -> getTarget())
                < roleAgents[1] -> getEventDist()) {
                state = DynamicState::Pass;
            }
            break;
        case DynamicState::None:
            break;
        case DynamicState::Pass:

            DBUG(QString("ENDKHAFAN : %1").arg(ros::Time::now().sec - dynamicStartTime), D_MAHI);
            if (wm->ball->pos.dist(wm->field->oppGoal()) - 0.5 < roleAgents[1]->getAgent()->pos().dist(wm->field->oppGoal())) {
                state = DynamicState::Shot;
            }
            if (!Circle2D(roleAgents[0]->getAgent()->pos(), 0.5).contains(wm->ball->pos) && dynamicStartTime == 0) {
                dynamicStartTime = ros::Time::now().sec;
            }

            if (wm->ball->vel.length() < 0.2 && dynamicStartTime != 0) {
                playOnFlag = true;
            }
            if ((ros::Time::now().sec - dynamicStartTime) > 5 && dynamicStartTime != 0) playOnFlag = true;
            break;
        case DynamicState::Shot:
            if (wm->ball->vel.length() < 0.2) {
                playOnFlag = true;
            }
            DBUG(QString("[dastan] : %1").arg(ros::Time::now().sec - dynamicStartTime), D_MAHI);

            if (ros::Time::now().sec - dynamicStartTime > 4 && dynamicStartTime != 0) {
                playOnFlag = true;
            }

            break;
    }


}

void CDynamicPlayOff::checkEndChipToGoal() {

    switch (state) {
        case DynamicState::Ready:
            if (Circle2D(wm->ball->pos, 0.5).contains(roleAgents[0]->getAgent()->pos())) {
                dynamicStartTime = ros::Time::now().sec;
                state = DynamicState::Shot;
            }
            break;
        case DynamicState::Shot:
            if (!Circle2D(roleAgents[0]->getAgent()->pos(), 0.5).contains(wm->ball->pos)) {
                playOnFlag = true;
            }

            if (ros::Time::now().sec - dynamicStartTime > 2 && dynamicStartTime != 0) {
                playOnFlag = true;
            }

            break;
        case DynamicState::None:break;
        case DynamicState::Pass:break;
    }

}


void CDynamicPlayOff::init(const QList<Agent *> &_agents) {
    agents.clear();
    agents.append(_agents);

    if(agents.size()>2)
        dynamicSelect = DynamicSelect::Khafan;
    else
        dynamicSelect = DynamicSelect::Chip;

        state = DynamicState::Ready;
}
