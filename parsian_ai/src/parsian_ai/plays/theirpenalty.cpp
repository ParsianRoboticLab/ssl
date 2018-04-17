#include "parsian_ai/plays/theirpenalty.h"
#include "parsian_ai/roles/playmake.h"
#include "parsian_ai/soccer.h"

CTheirPenalty::CTheirPenalty() {
    initMaster();

}

CTheirPenalty::~CTheirPenalty() {

}

void CTheirPenalty::reset() {
    positioningPlan.reset();
    executedCycles = 0;
}

void CTheirPenalty::init(const QList<Agent*>& _agents) {
    setAgentsID(_agents);
}

void CTheirPenalty::execute_x() {
    if (agents.empty()) {
        return;
    }
    executePositioning();
}

void CTheirPenalty::executePositioning()
{
    ROS_INFO_STREAM("penalty: normal positioning");
    if(agents.isEmpty())
        return;
    generatePositions();
    assignSkills();
}

void CTheirPenalty::assignSkills()
{
    ROS_INFO_STREAM("penalty: assign skills");
    moveSkills.clear();
    for(int i{0}; i<agents.count(); i++)
    {
        moveSkills.append(new GotopointavoidAction());
        moveSkills[i]->setTargetpos(positions[i]);
        moveSkills[i]->setTargetdir(wm->field->ourGoal());
        moveSkills[i]->setSlowmode(true);
        moveSkills[i]->setBallobstacleradius(0.1);
        agents[i]->action = moveSkills[i];
    }
}


void CTheirPenalty::generatePositions()
{
    ROS_INFO_STREAM("penalty: generate positions");
    positions.clear();
    double penaltyPositioningOffset = 0.4;
    double penaltyRuleOffset = 0.4;
    double maximum_x_width = wm->field->ourGoalL().x + (wm->field->_PENALTY_DEPTH + penaltyRuleOffset + penaltyPositioningOffset);
    for(int i{}; i < agents.size(); i++)
    {
        positions.append(getEmptyTarget(Vector2D{maximum_x_width, pow(-1, i)* i/2}, penaltyPositioningOffset));
    }
}

Vector2D CTheirPenalty::getEmptyTarget(Vector2D _position, double _radius)
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
