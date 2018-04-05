#include <parsian_ai/plays/ourpenalty.h>

COurPenalty::COurPenalty() : CMasterPlay()
{
    initMaster();
    playmakeRole = new CRolePlayMake(nullptr);
    PMgotopoint = new GotopointavoidAction();
    PMkick = new KickAction();
}

COurPenalty::~COurPenalty() = default;

void COurPenalty::reset() {
    positioningPlan.reset();
    executedCycles = 0;
}

void COurPenalty::init(const QList<Agent*>& _agents) {
    setAgentsID(_agents);
    ROS_INFO_STREAM("penalty: init our agents");
}

void COurPenalty::setPlaymake(Agent* _playmakeAgent)
{
    ROS_INFO_STREAM("penalty:  before set playmake to: ");
    if(_playmakeAgent != nullptr)
    {
        playMakeAgent = _playmakeAgent;
        playmakeRole->assign(playMakeAgent);
    }
    ROS_INFO_STREAM("penalty: set playmake to: " << playMakeAgent->id());
}

void COurPenalty::executeShootoutPositioning()
{

}

void COurPenalty::executeNormalPositioning()
{
    ROS_INFO_STREAM("penalty: normal positioning");
    if(agents.isEmpty())
        return;
    generatePositions();
    assignSkills();
}

void COurPenalty::execute_x() {
    ROS_INFO_STREAM("penalty: execute_x");
    if (playMakeAgent == nullptr || (playMakeAgent->id() == -1)) {
        ROS_INFO_STREAM("penalty: playmakeagent is null");
        return;
    }
    isPenaltyShootOut = gameState->ourPenaltyShootout();
    //playmakeRole->execute();
    if(penaltyState == PenaltyState::Positioning)
    {
        if(isPenaltyShootOut)
        {
            ROS_INFO_STREAM("penalty: shootout");
            executeShootoutPositioning();
        }
        else
        {
            ROS_INFO_STREAM("penalty: norma");
            executeNormalPositioning();
        }

        playmakePositioning();
    }
    if(penaltyState == PenaltyState::Kicking)
    {
        ROS_INFO_STREAM("penalty: in kicking state");
        playmakeKick();
    }
}

void COurPenalty::generatePositions()
{
    ROS_INFO_STREAM("penalty: generate positions");
    positions.clear();
    double penaltyPositioningOffset = 0.4;
    double penaltyRuleOffset = 0.4;
    double maximum_x_width = wm->field->oppGoalL().x - (wm->field->_PENALTY_DEPTH + penaltyRuleOffset + penaltyPositioningOffset);
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
    for (double dist = 0.0 ; dist <= 0.5 ; dist += 0.2) {
        for (double ang = -180.0 ; ang <= 180.0 ; ang += 60.0) {
            tempTarget = position + Vector2D::polar2vector(dist, ang);
            ////should check
            if (wm->field->isInOppPenaltyArea(tempTarget + (wm->field->oppGoal() - tempTarget).norm() * 0.3)) {
                continue;
            }
            for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
                if (Circle2D(wm->opp.active(i)->pos, 0.07).contains(tempTarget)) {
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
    ROS_INFO_STREAM("penalty: assign skills");
    moveSkills.clear();
    for(int i{0}; i<agents.count(); i++)
    {
        moveSkills.append(new GotopointavoidAction());
        moveSkills[i]->setTargetpos(positions[i]);
        moveSkills[i]->setTargetdir(wm->field->oppGoal());
        moveSkills[i]->setSlowmode(true);
        moveSkills[i]->setBallobstacleradius(0.1);
        agents[i]->action = moveSkills[i];
    }
}

void COurPenalty::playmakePositioning()
{
    Vector2D direction, position;
    direction = wm->ball->pos - playMakeAgent->pos();
    direction.y *= 1.2;
    position = wm->ball->pos + (wm->ball->pos - wm->field->oppGoal() + Vector2D(0, 0.2)).norm() * (0.13);
    PMgotopoint->setTargetpos(position);
    PMgotopoint->setTargetdir(direction);
    PMgotopoint->setSlowmode(true);
    PMgotopoint->setNoavoid(false);
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
    ROS_INFO_STREAM("kian: penalty target" << penaltyTarget);
    PMgotopoint->setRoller(1);
    ////////////// change robot direction before kicking //////////////
    if (timerStartFlag) {
        if (changeDirPenaltyStrikerTime.elapsed() < 2500)
        {
            if (penaltyTarget.y * wm->field->oppGoalL().y < 0 && penaltyTarget.dist(wm->field->oppGoal()) > 0.25) {
                penaltyTarget.y = wm->field->oppGoalR().y * 2;
                shift = Vector2D(0, 0.3);
            } else {
                penaltyTarget.y = wm->field->oppGoalL().y * 2;
                shift = Vector2D(0, -0.3);
            }
            position = wm->ball->pos + (wm->ball->pos - wm->field->oppGoal() + shift).norm() * (0.13);
            PMgotopoint->setTargetdir(penaltyTarget);
            PMgotopoint->setTargetpos(position);
            PMgotopoint->setLookat(wm->ball->pos);
        } else {
            timerStartFlag = false;
        }
    }
    PMgotopoint->setDivemode(true);
    PMgotopoint->setSlowmode(false);
    PMkick->setSpin(1);
    PMkick->setTarget(penaltyTarget);
    PMkick->setKickspeed(6);
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
