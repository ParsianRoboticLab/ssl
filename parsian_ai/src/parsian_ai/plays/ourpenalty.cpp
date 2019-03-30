#include <parsian_ai/plays/ourpenalty.h>

COurPenalty::COurPenalty() : CMasterPlay()
{
    initMaster();
    PMgotopoint = new GotopointavoidAction();
    PMkick = new KickAction();
    time = 0;
}

COurPenalty::~COurPenalty() = default;

void COurPenalty::reset() {
    positioningPlan.reset();
    executedCycles = 0;
}

void COurPenalty::init(QList<Agent*>& _agents) {
    agents = _agents;
}

void COurPenalty::execute_x() {
    ROS_INFO_STREAM("penalty: execute_x");
    if (playMakeAgent == nullptr || (playMakeAgent->id() == -1)) {
        ROS_INFO_STREAM("penalty: playmakeagent is null");
        return;
    }
    if(penaltyState == PenaltyState::Positioning)
    {
        executeNormalPositioning();
        playmakePositioning();
    }
    if(penaltyState == PenaltyState::Kicking)
        playmakeKick();
}

void COurPenalty::setPlaymake(Agent* _playmakeAgent)
{
    if(_playmakeAgent != nullptr)
    {
        playMakeAgent = _playmakeAgent;
    }
}

void COurPenalty::executeNormalPositioning()
{
    if(agents.isEmpty())
        return;
    generatePositions();
    assignSkills();
}


void COurPenalty::generatePositions()
{
    positions.clear();
    double penaltyPositioningOffset = 0.4;
    double penaltyRuleOffset = 0.4;
    double maximum_x_width =  3;
    for(int i{}; i < agents.size(); i++)
    {
        positions.append(getEmptyTarget(Vector2D{maximum_x_width, pow(-1, i)* i/2}, penaltyPositioningOffset));
    }
}


Vector2D COurPenalty::getEmptyTarget(Vector2D _position, double _radius)
{
    Vector2D tempTarget, finalTarget, position;
    double escapeRad;
    int oppCnt = 0;
    bool posFound;
    escapeRad = _radius;
    position  = _position;
    finalTarget = position;
    for (double dist = 0.0 ; dist <= 0.6 ; dist += 0.1) {
        for (double ang = -180.0 ; ang <= 180.0 ; ang += 20.0) {
            tempTarget = position + Vector2D::polar2vector(dist, ang);
            ////should check
            if (wm->field->isInOppPenaltyArea(tempTarget + (wm->field->oppGoal() - tempTarget).norm() * 0.3)) {
                continue;
            }
            for (int i = 0; i < wm->our.activeAgentsCount(); i++) {
                if (Circle2D(wm->our.active(i)->pos, 0.1).contains(tempTarget)) {
                    oppCnt = 1;
                    break;
                }

            }
            if (!oppCnt) {
                finalTarget = tempTarget;
                posFound = true;
                break;
            }
        }
        if (posFound) {
            break;
        }
    }

    return finalTarget;
}

void COurPenalty::assignSkills()
{
    moveSkills.clear();
    for (int i{0}; i < agents.count(); i++) {
        moveSkills.append(new GotopointavoidAction());


        if(i!=5) {
        moveSkills[i]->setTargetpos(positions[i]);
        moveSkills[i]->setTargetdir(getEmptyTarget(wm->field->oppGoal(), 0.05));//should change
        moveSkills[i]->setSlowmode(true);
        moveSkills[i]->setBallobstacleradius(0.1);
        agents[i]->action = moveSkills[i];
         }
        }
         moveSkills[5]->setTargetdir(getEmptyTarget(wm->field->oppGoal(), 1) );
         moveSkills[5]->setPenaltykick(true);
         ROS_INFO_STREAM("I kick!");
        agents[5]->action = moveSkills[1];
    }


void COurPenalty::playmakePositioning()
{
    Vector2D direction, position;
    direction = wm->ball->pos - playMakeAgent->pos();
    direction.y *= 1.2;
    position = wm->ball->pos + (wm->ball->pos - wm->field->oppGoal() + Vector2D(0, 0.2)).norm() * (0.18);
    PMgotopoint->setTargetpos(position);
    PMgotopoint->setTargetdir(direction);
    PMgotopoint->setSlowmode(true);
    PMgotopoint->setNoavoid(true);
    PMgotopoint->setPenaltykick(true);
    PMgotopoint->setAvoidpenaltyarea(false);
    PMgotopoint->setAvoidcentercircle(false);
    PMgotopoint->setBallobstacleradius(0.2);
    changeDirPenaltyStrikerTime.restart();
    timerStartFlag = true;
    playMakeAgent->action = PMgotopoint;
}

void COurPenalty::playmakeKick()
{
    Vector2D shift;
    Vector2D position;
    penaltyTarget = know->getEmptyPosOnGoalForPenalty(1.0 / 8.0, true, 0.03); //////// tune
    drawer->draw(penaltyTarget, "blue");
    ROS_INFO_STREAM("Mahdi: penalty target" << penaltyTarget);
    PMgotopoint->setRoller(1);
    ////////////// change robot direction before kicking //////////////
    if (timerStartFlag) {
        if (changeDirPenaltyStrikerTime.elapsed() < 2500)
        {
            if (true || penaltyTarget.y * wm->field->oppGoalL().y < 0 && penaltyTarget.dist(wm->field->oppGoal()) > 0.25) {
                penaltyTarget.y = wm->field->oppGoalR().y * 2;
                shift = Vector2D(0, 0.3);
            } else {
                penaltyTarget.y = wm->field->oppGoalL().y * 2;
                shift = Vector2D(0, -0.3);
            }
            position = wm->ball->pos + (wm->ball->pos - wm->field->oppGoal() + shift).norm() * (0.15);
            PMgotopoint->setTargetdir(penaltyTarget);
            PMgotopoint->setTargetpos(position);
            PMgotopoint->setLookat(wm->ball->pos);
        } else {
            timerStartFlag = false;
        }
    }
    PMgotopoint->setDivemode(false);

    PMgotopoint->setSlowmode(true);
    PMkick->setSpin(0);
    PMkick->setTarget(penaltyTarget);
    PMkick->setKickspeed(4);
    PMkick->setPenaltykick(true);
    PMkick->setInterceptmode(false);
    PMkick->setSpin(false);
    PMkick->setChip(false);
    PMkick->setVeryfine(false);
    PMkick->setAvoidopppenaltyarea(false);
    PMkick->setTolerance(20);
    PMkick->setChip(false);
    if (timerStartFlag) {
        ROS_INFO_STREAM("penalty: assign gotopointAction");
        playMakeAgent->action = PMgotopoint;
    } else {
        ROS_INFO_STREAM("penalty: assign kickAction");
        playMakeAgent->action = PMkick;
    }
}

double COurPenalty::angleOfTwoSegment(const Segment2D &xp, const Segment2D &yp)
{
    double theta1 = std::atan2(xp.a().y-xp.b().y,xp.a().x-xp.b().x);
    double theta2 = std::atan2(yp.a().y-yp.b().y,yp.a().x-yp.b().x);
    double diff = fabs(theta1-theta2);
    return diff;
}
