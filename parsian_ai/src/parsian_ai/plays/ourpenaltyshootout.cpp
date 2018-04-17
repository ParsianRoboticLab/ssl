#include <parsian_ai/plays/ourpenaltyshootout.h>

COurPenaltyShootout::COurPenaltyShootout() : CMasterPlay()
{
    initMaster();
    PMgotopoint  =   new GotopointavoidAction();
    PMkick       =   new KickAction();
}

COurPenaltyShootout::~COurPenaltyShootout() = default;

void COurPenaltyShootout::reset()
{
    positioningPlan.reset();
    executedCycles = 0;
}

void COurPenaltyShootout::init(const QList<Agent*>& _agents)
{
    setAgentsID(_agents);
}

void COurPenaltyShootout::setPlaymake(Agent* _playmakeAgent)
{
    if(_playmakeAgent != nullptr)
    {
        playMakeAgent = _playmakeAgent;
    }
}

void COurPenaltyShootout::execute_x()
{
    if (playMakeAgent == nullptr || (playMakeAgent->id() == -1))
    {
        return;
    }
    if(penaltyState == PenaltyShootoutState::Positioning)
    {
        generatePositions();
        assignSkills();
    }
    else
    {
        runShootOut();
    }
}

void COurPenaltyShootout::generatePositions()
{
    positions.clear();
    double penaltyPositioningOffset = 0.9;
    double maximum_x_width = wm->field->ourGoal().x + (wm->field->_PENALTY_DEPTH + penaltyPositioningOffset);
    for(int i{}; i < agents.size(); i++)
    {
        positions.append(getEmptyTarget(Vector2D{maximum_x_width, pow(-1, i)* i/2}, penaltyPositioningOffset));
    }
}

Vector2D COurPenaltyShootout::getEmptyTarget(Vector2D _position, double _radius)
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

void COurPenaltyShootout::assignSkills()
{
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

void COurPenaltyShootout::playmakeInitialPositioning()
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
    shootoutTimer.restart();
    timerFlag = true;
    playMakeAgent->action = PMgotopoint;
}

void COurPenaltyShootout::runShootOut()
{

}
