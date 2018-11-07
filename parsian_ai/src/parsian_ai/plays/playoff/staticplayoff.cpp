//
// Created by parsian-ai on 11/7/18.
//

#include <parsian_ai/plays/playoff/staticplayoff.h>



CStaticPlayOff::CStaticPlayOff() {

    ROS_INFO("Bring yourself back online playoff");

    for (auto &i : positionAgent) {
        i.stateNumber = 0;
    }
    for (int i = 0; i < _NUM_PLAYERS; i++) {
        roleAgent[i] = new CRolePlayOff();
    }
    isBallIn = false;
    tempAgent = new CRolePlayOff();
    doPass = false;
    doAfterlife = false;
    setTimer = true;
    ////////////

    masterPlan = nullptr;



    playOnFlag = false;
    firstPass  = true;
    havePassInPlan    = false;


    criticalInit = true;
    criticalKick = new KickAction();

}

CStaticPlayOff::~CStaticPlayOff() {
    ROS_INFO("Playoff is gone");
    for (int i = 0; i < _NUM_PLAYERS; i++) {
        delete roleAgent[i];
    }
    delete tempAgent;
}

void CStaticPlayOff::execute() {

    ROS_INFO_STREAM("lastTime: " <<ros::Time::now().sec - lastTime << "ball dist: "<<lastBallPos.dist(wm->ball->pos) );
    if (ros::Time::now().sec - lastTime > 10 && lastBallPos.dist(wm->ball->pos) <= 0.06 && wm->ball->vel.length() > 0.5) {
        criticalPlay();
        playOnFlag = true;
        return;
    }
    staticExecute();
}

void CStaticPlayOff::staticExecute() {

    if (masterPlan != nullptr) {
        if (gameState->canKickBall()) {

            fillRoleProperties(); //// Ready RoleAgents for Execution
            posExecute(); //// Execute RoleAgents
            checkEndState(); //// Check if an Execution is Over Move To Next State

            if (masterPlan->currentSize > 1 && havePassInPlan) passManager();

            if (isPlanEnd()) playOnFlag = true;

        }

    } else {
        ROS_ERROR("MASTER IS NULL!!");
    }

}

bool CStaticPlayOff::isPlanEnd() {
    return isPlanDone() || isPlanFailed();
}

bool CStaticPlayOff::isPlanDone() {

    return isFinalShotDone();
}


bool CStaticPlayOff::isPlanFailed() {
    return isTimeOver() || firstKickFailed() || isBallDirChanged();
}

bool CStaticPlayOff::isTimeOver() {

    if (setTimer) startTime = ros::Time::now().sec;

    if (!Circle2D(lastBallPos, 0.06).contains(wm->ball->pos)) {
        setTimer = false;
        ROS_INFO_STREAM("MAHI: Time That Left: " << ros::Time::now().sec - startTime);
        if(ros::Time::now().sec - startTime >= 3*masterPlan->execution.passCount) { // 3 Second TODO: ADD TO CFG
            setTimer = true;
            return true;
        }
    }
    return false;
}

bool CStaticPlayOff::isBallDirChanged() {
    // TODO: Need Test And Refine
    if (masterPlan->execution.passCount != 1) {
        return false;
    }

    //// USE PASSER FORM INITIAL LEVEL
    const int& passer = masterPlan->execution.passer.at(0).id;
    const int& receiver = masterPlan->execution.receiver.at(0).id;
    const int receiverID = masterPlan->matchedID.value(receiver);
    if (wm->ball->pos.dist(lastBallPos) > 0.5 && !roleAgent[passer]->getChip()) {
        Circle2D  c(roleAgent[receiverID]->getWaitPos(), 1); // TODO : CHECK radius
        drawer->draw(wm->ball->seg(), QColor(Qt::blue));
        drawer->draw(c, QColor(Qt::red));
        return !c.intersection(wm->ball->seg());
    }
    return false;

}

bool CStaticPlayOff::isFinalShotDone() {

    const int& tLastAgent = masterPlan->execution.theLastAgent;
    const int& tLastState = masterPlan->execution.theLastState;

    // Plan hasn't a final shoot
    if (tLastState == -1 || tLastAgent == -1) {
        return false;
    }

    Agent* tAgent = getAgent(tLastAgent);

    Circle2D cir(tAgent->pos() + tAgent->dir().norm() * 0.08, 0.16);
    Circle2D cir2(tAgent->pos() + tAgent->dir().norm() * 0.20, 0.40);

    drawer->draw(cir , QColor(Qt::blue));
    drawer->draw(cir2, QColor(Qt::blue));

    if (positionAgent[tLastAgent].stateNumber == tLastState) {
        if (cir.contains(wm->ball->pos)) {
            isBallIn = true;

        } else if (isBallIn && !cir2.contains(wm->ball->pos)) {
            isBallIn = false;
            return true;

        }
    }

    return false;
}

Vector2D CStaticPlayOff::getEmptyTarget(const Vector2D& _position, const double& _radius) {
    Vector2D tempTarget, finalTarget;
    bool opp;
    finalTarget = _position;
    for (double dist = 0.2 ; dist <= _radius ; dist += 0.2) {
        for (double ang = -180.0 ; ang <= 180.0 ; ang += 18.0/dist) {
            opp = false;
            tempTarget = _position + Vector2D::polar2vector(dist, ang);
            for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
                if (Circle2D(wm->opp.active(i)->pos, 0.25).contains(tempTarget)
                    || !wm->field->isInField(tempTarget)
                    || wm->field->isInOppPenaltyArea(tempTarget)
                    || wm->field->isInOurPenaltyArea(tempTarget)) {
                    opp = true;
                    break;
                }

            }
            if (!opp) {
                finalTarget = tempTarget;
                dist = _radius*2; // to break upper loop
                break;
            }
        }
    }

    return finalTarget;
}
///////////////PassManager///////////////////
void CStaticPlayOff::passManager() {
    const AgentPoint &r = masterPlan->execution.receiver.at(0);
    const AgentPoint &p = masterPlan->execution.passer.at(0);

    const int &i = masterPlan->matchedID.value(r.id);

    Agent *c = getAgent(r.id); //// Receiver
    if (positionAgent[r.id].stateNumber == r.state
        || positionAgent[r.id].stateNumber == r.state + 1) {
        DBUG(QString("RC : %1, %2").arg(r.id).arg(r.state), D_MAHI);
        drawer->draw(Circle2D(positionAgent[r.id].getAbsArgs(r.state).staticPos, masterPlan->lastDist),
                     QColor(Qt::darkMagenta));
        doPass = positionAgent[r.id].getAbsArgs(r.state).staticPos.dist(c->pos())
                 <= masterPlan->lastDist;
        doAfterlife = !Circle2D(lastBallPos, 0.1).contains(wm->ball->pos);
        roleAgent[p.id]->setDoPass(doPass);
    }

}

/**
 * @brief CStaticPlayOff::isTaskDone
 * @param _roleAgent
 * @return true if the task get done
 */
bool CStaticPlayOff::isTaskDone(CRolePlayOff* _roleAgent) {

    switch (_roleAgent->getSelectedSkill()) {
        case RoleSkill::Gotopoint:
        case RoleSkill::GotopointAvoid:
            return isMoveDone(_roleAgent);
            break;
        case RoleSkill::Kick:
            return isKickDone(_roleAgent);
            break;
        case RoleSkill::OneTouch:
            return isOneTouchDone(_roleAgent);
            break;
        case RoleSkill::ReceivePass:
            return isReceiveDone(_roleAgent);
            break;
            //  Life
        case RoleSkill::Mark:
        case RoleSkill::Support:
        case RoleSkill::Defense:
            ROS_DEBUG("got it");
            _roleAgent->setRoleUpdate(false);
            return false;
            break;
    }
}

void CStaticPlayOff::posExecute() {
    for (int i = 0; i < masterPlan->currentSize; i++) {
        if (roleAgent[i]->getAgent() != nullptr) {
            roleAgent[i]->execute();
        }
    }
}

void CStaticPlayOff::checkEndState() {

    for (int i = 0; i < masterPlan->currentSize; i++) {
        if (roleAgent[i]->getAgent() == nullptr) {
            continue;
        }

        Agent *firstPasser = getAgent(masterPlan->execution.passer.at(0).id);

        if (isTaskDone(roleAgent[i]) || (doAfterlife && roleAgent[i]->getAgent()->id() != firstPasser->id())) {

            roleAgent[i]->setRoleUpdate(false);
            roleAgent[i]->resetTime();

            POFFSKILL last_skill = positionAgent[i].positionArg.at(
                    positionAgent[i].positionArg.size() - 1).staticSkill;

            if (last_skill == POFFSKILL::Position) {
                if (!doAfterlife && positionAgent[i].stateNumber == positionAgent[i].positionArg.size() - 2) {
                    roleAgent[i]->setRoleUpdate(true);
                } else if (doAfterlife){
                    positionAgent[i].stateNumber = positionAgent[i].positionArg.size() - 1;
                }
            } else if (positionAgent[i].stateNumber + 1 < positionAgent[i].positionArg.size()) {
                if (positionAgent[i].getArgs(1).staticSkill    == POFFSKILL::Defense
                    || positionAgent[i].getArgs(1).staticSkill == POFFSKILL::Support
                    || positionAgent[i].getArgs(1).staticSkill == POFFSKILL::Position
                    || positionAgent[i].getArgs(1).staticSkill == POFFSKILL::Goalie
                    || positionAgent[i].getArgs(1).staticSkill == POFFSKILL::Mark) {

                    if (doAfterlife) {
                        positionAgent[i].stateNumber++;
                    }
                    continue;
                }
                positionAgent[i].stateNumber++;
                DBUG(QString("IDDD : %1, ST : %2").arg(i).arg(positionAgent[i].stateNumber), D_MAHI);
            } else {
                //                positionAgent[i].zombie = true;
                ///Temp
                //                SPositioningArg tempPA;
                //                tempPA = positionAgent[i].getAbsArgs(positionAgent[i].positionArg.size() - 1);
                //                tempPA.staticSkill = MoveSkill;
                //                positionAgent[i].positionArg.append(tempPA);
            }
        }
    }
}

void CStaticPlayOff::fillRoleProperties() {
    for (int i = 0; i < agents.size(); i++) {
        if (masterPlan->matchedID.contains(i) && !roleAgent[i]->isRoleUpdated()) {

            roleAgent[i]->setFirstMove(positionAgent[i].stateNumber == 0);
            roleAgent[i]->setAgent(getAgent(i));

            //// Handle OneTouch Faster
            if (positionAgent[i].stateNumber + 1 < positionAgent[i].positionArg.size()) {
                if (positionAgent[i].getArgs().staticSkill  == POFFSKILL::Move &&
                    positionAgent[i].getArgs(1).staticSkill == POFFSKILL::OneTouch) {
                    positionAgent[i].stateNumber++;
                }
            }

            //// Change Receive and Pass to OneTouch (2nd Pass)
            for (auto x : masterPlan->execution.passer) {
                if (x.id == i) {
                    if (i > 0 && masterPlan->execution.receiver.at(i - 1).id == x.id) {
                        if (positionAgent[i].getArgs().staticSkill == POFFSKILL::ReceivePass) {
                            int index = -1;
                            for (int j = 1;
                                 j < positionAgent[i].positionArg.size() - positionAgent[i].stateNumber; j++) {
                                if (positionAgent[i].getArgs(j).staticSkill == POFFSKILL::Pass) {
                                    index = j;
                                    break;
                                }
                            }
                            if (index != -1) {
                                Vector2D pass_target =
                                        positionAgent[positionAgent[i].getArgs(index).PassToId].getAbsArgs(
                                                positionAgent[i].getArgs(index).PassToState).staticPos;

                                Vector2D v1 = wm->ball->pos - positionAgent[i].getArgs(index).staticPos;
                                Vector2D v2 = pass_target - positionAgent[i].getArgs(index).staticPos;
                                double onetouchAngle = (v1.th() - v2.th()).degree();
                                ROS_INFO_STREAM("onetouchAngle " << onetouchAngle << " max OnetouchAngle"
                                                                 << conf.MaxOnetouchAngle);
                                if (onetouchAngle < conf.MaxOnetouchAngle) {
                                    positionAgent[i].stateNumber++;
                                    firstPass = false;
                                }
                            }
                        }
                    }
                }
            }

            roleAgent[i]->setRoleUpdate(true);
            roleAgent[i]->resetTime();
            assignTask(roleAgent[i], positionAgent[i]);
            ROS_INFO_STREAM("NEW TASK: " << roleAgent[i]->getTarget().x << roleAgent[i]->getTarget().y);

        } else {
            ROS_WARN("coach -> Match function doesn't work :( ");
            if (!roleAgent[i]->isRoleUpdated()) {
                roleAgent[i]->setAgent(agents.at(i));
                assignTask(roleAgent[i], positionAgent[i]);
                roleAgent[i]->setRoleUpdate(true);
            }
        }
    }
}

void CStaticPlayOff::assignTask(CRolePlayOff* _roleAgent, const SPositioningAgent& _positionAgent) {
    switch (_positionAgent.getArgs().staticSkill) {
        case POFFSKILL::Pass:
            assignPass(_roleAgent, _positionAgent);
            break;
        case POFFSKILL::ReceivePass:
            assignReceive(_roleAgent, _positionAgent, false);
            break;
        case POFFSKILL::ReceivePassIA:
            assignReceive(_roleAgent, _positionAgent, true);
            break;
        case POFFSKILL::ShotToGoal:
            assignKick(_roleAgent, _positionAgent, false);
            break;
        case POFFSKILL::ChipToGoal:
            assignKick(_roleAgent, _positionAgent, true);
            break;
        case POFFSKILL::OneTouch:
            assignOneTouch(_roleAgent, _positionAgent);
            break;
        case POFFSKILL::Move:
            assignMove(_roleAgent, _positionAgent);
            break;
            // After Life
        case POFFSKILL::Defense:
            _roleAgent->setRoleUpdate(false);
            assignDefense(_roleAgent, _positionAgent);
            break;
        case POFFSKILL::Support:
            _roleAgent->setRoleUpdate(false);
            assignSupport(_roleAgent, _positionAgent);
            break;
        case POFFSKILL::Position:
            _roleAgent->setRoleUpdate(false);
            assignPosition(_roleAgent, _positionAgent);
            break;
        case POFFSKILL::Goalie:
            _roleAgent->setRoleUpdate(false);
            assignGoalie(_roleAgent, _positionAgent);
            break;
        case POFFSKILL::Mark:
            _roleAgent->setRoleUpdate(false);
            assignMark(_roleAgent, _positionAgent);
            break;
        case POFFSKILL::None:
            break;
    }
}

void CStaticPlayOff::assignPass(CRolePlayOff* _roleAgent, const SPositioningAgent& _posAgent) {
    _roleAgent->setAvoidCenterCircle(false);
    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setChip(chipOrNot(_posAgent.getArgs()));
    if (_roleAgent->getChip()) _roleAgent->setKickSpeed(static_cast<double>(_posAgent.getArgs().rightData)/100.0);
    else _roleAgent->setKickSpeed(static_cast<double>(_posAgent.getArgs().leftData)/100.0);
    _roleAgent->setTarget(positionAgent[_posAgent.getArgs().PassToId].getAbsArgs(_posAgent.getArgs().PassToState).staticPos);
    _roleAgent->setDoPass(doPass);
    _roleAgent->setIntercept(false);
    _roleAgent->setTargetDir(_posAgent.getArgs().staticAng);
    _roleAgent->setSelectedSkill((firstPass) ? RoleSkill::Kick : RoleSkill::OneTouch);

}

void CStaticPlayOff::assignReceive(CRolePlayOff* _roleAgent, const SPositioningAgent& _posAgent, bool _ignoreAngle) {

    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setIgnoreAngle(_ignoreAngle);
    _roleAgent->setTarget(_posAgent.getArgs().staticPos);
    _roleAgent->setTargetDir(_posAgent.getArgs().staticAng); /** Just Matter when we use Ignore mode **/
    _roleAgent->setReceiveRadius(_posAgent.getArgs().leftData / 100);
    _roleAgent->setSelectedSkill(RoleSkill::ReceivePass);
}

void CStaticPlayOff::assignKick(CRolePlayOff* _roleAgent,
                          const SPositioningAgent& _posAgent, bool _chip) {

    _roleAgent->setChip(_chip);
    _roleAgent->setKickSpeed(static_cast<double>(_posAgent.getArgs().leftData) / 100.0);
    _roleAgent->setTarget(getGoalTarget(_posAgent.getArgs().rightData));
    _roleAgent->setIntercept(false);
    _roleAgent->setSelectedSkill(RoleSkill::Kick);
}

void CStaticPlayOff::assignOneTouch(CRolePlayOff* _roleAgent,
                              const SPositioningAgent& _posAgent) {

    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setWaitPos(_posAgent.getArgs().staticPos);
    _roleAgent->setKickSpeed(static_cast<double>(_posAgent.getArgs().leftData)/100.0);
    _roleAgent->setTarget(getGoalTarget(_posAgent.getArgs().rightData));
    _roleAgent->setSelectedSkill(RoleSkill::OneTouch);
}

void CStaticPlayOff::assignMove(CRolePlayOff* _roleAgent, const SPositioningAgent& _posAgent) {
    _roleAgent -> setAvoidPenaltyArea(true);
    _roleAgent -> setTime(_posAgent.getArgs().rightData); //// ignore duration -> time is wait
    _roleAgent -> setAvoidBall(true);
    _roleAgent -> setTimeBased(_roleAgent->getTime() != 0);
    _roleAgent -> setEventDist(_posAgent.getArgs().staticEscapeRadius);
    _roleAgent -> setTarget(getMoveTarget(_posAgent.getArgs()));
    _roleAgent -> setTargetDir(_posAgent.getArgs().staticAng);
    _roleAgent -> setSlow(false);
    _roleAgent -> setMaxVelocity(getMaxVel(_roleAgent, _posAgent.getArgs()));
    _roleAgent -> setSelectedSkill(RoleSkill::GotopointAvoid);

    if (_posAgent.getArgs().staticPos == BEHIND_BALL_POS) { //// First Passer
        _roleAgent -> setTimeBased(true);
        _roleAgent -> setTarget(wm->ball->pos - Vector2D(0.20, 0));
        _roleAgent -> setTargetDir(wm->ball->pos - _roleAgent->getAgent()->pos());
        _roleAgent -> setSlow(true);
        _roleAgent -> setMaxVelocity(1);
    }
}

void CStaticPlayOff::assignGoalie(CRolePlayOff * _roleAgent, const SPositioningAgent &_posAgent) {
    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setAvoidBall(false);
    _roleAgent->setSlow(false);
    _roleAgent->setTargetDir(Vector2D(0, 1));
    _roleAgent->setTarget(wm->field->ourGoal() + Vector2D(0, 1));
    _roleAgent->setSelectedSkill(RoleSkill::GotopointAvoid);
}

void CStaticPlayOff::assignMark(CRolePlayOff * _roleAgent, const SPositioningAgent &_posAgent) {

    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setAvoidBall(false);
    _roleAgent->setNoAvoid(true);
    _roleAgent->setTargetDir(Vector2D(0, 1));
    _roleAgent->setSlow(false);
    _roleAgent->setTarget(CDefPos::getStaticDefPositions(wm->ball->pos, 2, 2, 3).pos[1]);
    _roleAgent->setSelectedSkill(RoleSkill::GotopointAvoid);
}

void CStaticPlayOff::assignPosition(CRolePlayOff * _roleAgent,
                              const SPositioningAgent &_posAgent) {
    assignMove(_roleAgent, _posAgent); // TODO : check this
}

void CStaticPlayOff::assignSupport(CRolePlayOff * _roleAgent,
                             const SPositioningAgent &_posAgent) {
    Vector2D supportPosition = (lastBallPos.dist(wm->ball->pos) < 0.1)
                               ? _roleAgent->getAgent()->pos()
                               : (wm->ball->pos + wm->ball->vel * 0.5) - Vector2D(1, 0);

    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setAvoidBall(false);
    _roleAgent->setSlow(false);
    _roleAgent->setTargetDir(_roleAgent->getAgent()->pos() - wm->ball->pos);
    _roleAgent->setTarget(getEmptyTarget(supportPosition, .5));
    _roleAgent->setSelectedSkill(RoleSkill::GotopointAvoid); //GotoPointAvoid
}

void CStaticPlayOff::assignDefense(CRolePlayOff * _roleAgent,
                             const SPositioningAgent &_posAgent) {
    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setAvoidBall(false);
    _roleAgent->setNoAvoid(true);
    _roleAgent->setTargetDir(Vector2D(0, 1));
    _roleAgent->setSlow(false);
    _roleAgent->setTarget(CDefPos::getStaticDefPositions(wm->ball->pos, 2, 2, 3).pos[0]);
    _roleAgent->setSelectedSkill(RoleSkill::GotopointAvoid); //GotoPointAvoid
}

Vector2D CStaticPlayOff::getMoveTarget(const SPositioningArg& _posArg) {
    return getEmptyTarget(_posArg.staticPos, _posArg.staticEscapeRadius);
}

double CStaticPlayOff::getMaxVel(const CRolePlayOff* _roleAgent,
                           const SPositioningArg& _posArg) {
    Vector2D tAgentPos =  _roleAgent->getAgent()->pos();
    double dist = tAgentPos.dist(_posArg.staticPos);
    double vel = std::min(std::max(dist / _posArg.leftData, 1.5), 4.0);
    return vel;
}

Vector2D CStaticPlayOff::getGoalTarget(long _y) {
    _y = std::min(std::max(_y, 0L), 1000L);
    double tempYPos = (double)(_y) / 1000.0 + wm->field->oppGoalR().y;
    return Vector2D{wm->field->oppGoal().x, tempYPos};
}

bool CStaticPlayOff::chipOrNot(const SPositioningArg& _posArg) {
    if (_posArg.leftData < 0) return true;
    else if (_posArg.rightData < 0) return false;
    else {
        const int& id = _posArg.PassToId;
        const int& ps = _posArg.PassToState;
        return !isPathClear(wm->ball->pos,
                            positionAgent[id].getAbsArgs(ps).staticPos,
                            0.5,   // Radius
                            0.1);  // Threshold
    }
}

bool CStaticPlayOff::isPathClear(Vector2D _pos1,
                           Vector2D _pos2,
                           double _radius,
                           double threshold) {

    Polygon2D poly = getPathPolygon(_pos1, _pos2, _radius, threshold);
    drawer->draw(poly, "red");

    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (poly.contains(wm->opp.active(i)->pos)) return false;
    }

    return true;
}

void CStaticPlayOff::assignTasks(const SPlan* _plan) {
    const int &sym = _plan->execution.symmetry;
    for (size_t i = 0; i < _plan->currentSize; i++) {

        positionAgent[i].positionArg.clear();

        Q_FOREACH (playOffRobot agentPlan, _plan->execution.AgentPlan[i]) {
                SPositioningArg tempPosArg;
                tempPosArg.staticPos = agentPlan.pos;
                tempPosArg.staticAng = Vector2D::polar2vector(1, agentPlan.angle);
                tempPosArg.staticAng.assign(tempPosArg.staticAng.x, -1 * sym * tempPosArg.staticAng.y);
                tempPosArg.staticPos.assign(tempPosArg.staticPos.x, sym * tempPosArg.staticPos.y);
                tempPosArg.staticEscapeRadius = agentPlan.tolerance;

                Q_FOREACH (playOffSkill skill, agentPlan.skill) {
                        tempPosArg.leftData    = skill.data[0];
                        tempPosArg.rightData   = skill.data[1];
                        tempPosArg.staticSkill = skill.name;
                        tempPosArg.PassToId    = skill.targetAgent;
                        tempPosArg.PassToState = skill.targetIndex;

                        if (skill.name == POFFSKILL::Pass && positionAgent[i].positionArg.back().staticSkill == POFFSKILL::Move) {
                            positionAgent[i].positionArg.back().staticPos = BEHIND_BALL_POS;
                        } else if (      skill.name == POFFSKILL::ShotToGoal
                                         || skill.name == POFFSKILL::ChipToGoal
                                         || skill.name == POFFSKILL::OneTouch) {

                            if (sym < 0) tempPosArg.rightData = 1000 - tempPosArg.rightData;

                        }
                        positionAgent[i].positionArg.append(tempPosArg);
                    }
            }
    }
}

int CStaticPlayOff::findReceiver(int _passer, int _state) {
    if (_state == 0) {
        return 0;
    }
    for (int i = _state; i < positionAgent[_passer].positionArg.size(); i++) {
        if (positionAgent[_passer].getArgs(i).staticSkill == POFFSKILL::Pass) {
            SBallOwner temp{};
            temp.id = _passer;
            temp.state = i;
            ownerList.append(temp);
            findReceiver(positionAgent[_passer].getArgs(i).PassToId,
                         positionAgent[_passer].getArgs(i).PassToState);
        }
    }
}

void CStaticPlayOff::reset() {


    ROS_DEBUG("Bring yourself back online playoff");

    for (int i = 0; i < _NUM_PLAYERS; i++) {
        positionAgent[i].stateNumber = 0;
        roleAgent[i]->reset();
        positionAgent[i].zombie = false;
    }
    isBallIn = false;
    tempAgent = new CRolePlayOff();
    doAfterlife = false;
    doPass = false;
    criticalInit = true;

    setTimer = true;
    ////////////

    masterPlan = nullptr;

    playOnFlag = false;
    havePassInPlan = false;

    activeAgents.clear();

    firstPass = true;

    DBUG(QString("reset Plan"), D_MAHI);
    ROS_INFO("reset Plan");
}

void CStaticPlayOff::init(const QList<Agent*>& _agents) {
    agents = _agents;
    lastBallPos = wm->ball->pos;
    lastTime = ros::Time::now().sec;
    assignTasks(masterPlan);
    ROS_INFO_STREAM("task assigned: " << agents.size());
}

bool CStaticPlayOff::firstKickFailed() {
    return (lastBallPos.dist(wm->ball->pos) > 0.25 && wm->ball->vel.length() < 0.1);
}

/*!
*   \brief check if ball get distance from robot,
*
*          in case that it's direct shoot
*          also check that ball is in right direction or not.
*
*
*/
bool CStaticPlayOff::isKickDone(CRolePlayOff * _roleAgent) {

    if (Circle2D(_roleAgent->getAgent()->pos(), 0.2).contains(wm->ball->pos)) {
        _roleAgent->setBallIsNear(true);
        return false;
    } else if (!Circle2D(lastBallPos, 0.6).contains(wm->ball->pos) && _roleAgent->getBallIsNear()) {
        _roleAgent->setBallIsNear(false);
        return true;
    }

}

bool CStaticPlayOff::isReceiveDone(const CRolePlayOff * _roleAgent) {
    return Circle2D(_roleAgent->getAgent()->pos(), 0.3).contains(wm->ball->pos) && wm->ball->vel.length() < 0.5;
}

bool CStaticPlayOff::isOneTouchDone(CRolePlayOff * _roleAgent) {

    if (Circle2D(_roleAgent->getAgent()->pos(), 0.4).contains(wm->ball->pos)) {
        ROS_INFO("playoff one-touch BallIsNear");
        _roleAgent->setBallIsNear(true);
    } else if (!Circle2D(_roleAgent->getAgent()->pos(), 0.6).contains(wm->ball->pos)
               && _roleAgent->getBallIsNear()) {
        _roleAgent->setBallIsNear(false);

        if (_roleAgent->getChip()) {
            DBUG("[playoff] chip Done", D_MAHI);
            ROS_INFO("playofff chip one-touch is done");
            return true;
        } else {
            // check ball direction
            Vector2D sol1, sol2;
            if (wm->ball->vel.length() > 0.6 && Circle2D(_roleAgent->getTarget(), 0.75).intersection(
                    Ray2D(wm->ball->pos, wm->ball->pos + wm->ball->vel), &sol1, &sol2)) {
                DBUG("[playoff] direction is correct", D_MAHI);
                ROS_INFO("playofff one-touch is now done");
                return true;
            }

        }
    }

    return false;
}

bool CStaticPlayOff::isMoveDone(const CRolePlayOff * _roleAgent) {

    if (_roleAgent->getFirstMove() && _roleAgent->getTarget() != BEHIND_BALL_POS) {
        return true ;
    }

    if (_roleAgent->getTimeBased()) {
        DBUG(QString("EL : %1").arg(_roleAgent->getElapsed()), D_HOSSEIN);
        DBUG(QString("GT : %1").arg(_roleAgent->getTime()), D_HOSSEIN);
        if (_roleAgent->getElapsed() > _roleAgent->getTime()) {
            DBUG("D------------------", D_HOSSEIN);
            return true;
        }
    } else {
        // TODO : vartypes this
        if (_roleAgent->getAgent()->pos().dist(_roleAgent->getTarget()) < max(0.3, _roleAgent->getEventDist() / 100)) {
            return true;
        }
    }
    return false;
}

QPair<int, int> CStaticPlayOff::findTheLastShoot(const SExecution &_plan) {
    QPair<int, int> last;
    last.first = last.second = -1;

    QList<POFFSKILL> finalSkills;
    finalSkills.append(POFFSKILL::ShotToGoal);
    finalSkills.append(POFFSKILL::ChipToGoal);
    finalSkills.append(POFFSKILL::OneTouch);

    int counter = 0;
    Q_FOREACH (QList<playOffRobot> agent, _plan.AgentPlan) {
            int counter2 = 0;
            Q_FOREACH (playOffRobot node, agent) {
                    Q_FOREACH (playOffSkill skill, node.skill) {
                            if (finalSkills.contains(skill.name)) {
                                last.first  = counter;
                                last.second = counter2;
                            }
                            counter2++;
                        }
                }
            counter++;
        }
    return last;
}

void CStaticPlayOff::analyseShoot() {
    if (masterPlan != nullptr) {
        QPair<int, int> last;
        last = findTheLastShoot(masterPlan->execution);
        masterPlan->execution.theLastAgent = last.first;
        masterPlan->execution.theLastState = last.second;
        havePassInPlan = (last.first != -1  && last.second != -1);
    }
}

void CStaticPlayOff::analysePass() {
    // TODO : need edit for mulitiple pass
    if (masterPlan != nullptr) {
        // first : passer second : receiver
        QList<AgentPair> tPass = findThePasserandReciver(masterPlan->execution);
        havePassInPlan = !tPass.empty();
        masterPlan->execution.passCount = tPass.size();
        if (havePassInPlan) {
            for (int i = 0; i < tPass.size(); i++) {
                if(i == 0) {
                    AgentPoint p;
                    p.id = masterPlan->matchedID.value(tPass.at(i).first.id);
                    p.state = tPass.at(i).first.state;
                    masterPlan->execution.passer.append(p);
                } else{
                    masterPlan->execution.passer.append(tPass.at(i).first);
                }

                masterPlan->execution.receiver.append(tPass.at(i).second);
            }
        }
    }

}

void CStaticPlayOff::criticalPlay() {

    ROS_INFO("critical");
    if (criticalInit) {
        criticalInit = false;
        criticalKick->setTarget(wm->field->oppGoal());
        criticalKick->setChip(wm->ball->pos.x < 1);
        criticalKick->setDontkick(false);
        criticalKick->setPassprofiler(false);
        criticalKick->setKickspeed(6.5);
        criticalKick->setChipdist(4);
        criticalKick->setTolerance(0.5);
    }

    double minDist = 100;
    Agent* nearest = nullptr;
    Q_FOREACH(auto& agent, agents) {
            if(agent->pos().dist(wm->ball->pos) < minDist) {
                nearest = agent;
                minDist = agent->pos().dist(wm->ball->pos);
            }
        }

    if(nearest != nullptr){
        nearest->action = criticalKick;
    } else {
        ROS_ERROR("THERE'S NO AGENT FOR CRITICAL PLAY");
    }
}

QList<AgentPair> CStaticPlayOff::findThePasserandReciver(const SExecution & _plan) {

    QList<AgentPoint> passer;
    for (int i = 0; i < _plan.AgentPlan.size(); i++) {
        const QList<playOffRobot> & agent = _plan.AgentPlan.at(i);
        for (int j = 0; j < agent.size(); j++) {
            const playOffRobot& node = agent.at(j);
            for (const auto &k : node.skill) {
                const POFFSKILL& skill = k.name;
                if (skill == POFFSKILL::Pass) {
                    passer.append(AgentPoint(i, j));
                }
            }

        }
    }

    ROS_INFO_STREAM("Pass count: " << passer.size());
    QList<AgentPair> pairList;
    for (auto i : passer) {
        const int &id = i.id;
        const int &st = i.state;

        int si = (_plan.AgentPlan[id][st].skill[1].name == POFFSKILL::Pass) ? 1 : 2;
        int rid = _plan.AgentPlan[id][st].skill[si].targetAgent;
        int rs  = _plan.AgentPlan[id][st].skill[si].targetIndex;
        DBUG(QString("PASS : %1, %2, %3, %4").arg(id).arg(st).arg(rid).arg(rs), D_MAHI);
        AgentPoint tempReciver;
        tempReciver.id    = rid;
        tempReciver.state = rs;

        AgentPair ap;
        ap.first  = i;
        ap.second = tempReciver;
        pairList.append(ap);
    }
    return pairList;

}

Polygon2D CStaticPlayOff::getPathPolygon(Vector2D _pos1, Vector2D _pos2, double _radius, double treshold) {
    Vector2D sol1, sol2, sol3, sol4;
    Line2D _path{_pos1, _pos2};
    Circle2D(_pos2, _radius + treshold).intersection(_path.perpendicular(_pos2), &sol1, &sol2);
    Circle2D(_pos1, Robot::robot_radius_new + treshold).intersection(_path.perpendicular(_pos1), &sol3, &sol4);
    Polygon2D _poly;
    _poly.addVertex(sol1);
    _poly.addVertex(sol2);
    _poly.addVertex(sol4);
    _poly.addVertex(sol3);
    _poly.addVertex(sol1);
    return _poly;
}

int CStaticPlayOff::getIndex(int _planID) {
    return masterPlan->matchedID.value(_planID);
}

Agent *CStaticPlayOff::getAgent(int _planID) {
    return agents[getIndex(_planID)];
}

SPlan* CStaticPlayOff::planMsgToSPlan(const parsian_msgs::parsian_plan& _plan, int _currSize) {
    auto *plan = new SPlan();

    plan->currentSize = _currSize;
    plan->execution.symmetry = (_plan.symmetry) ? -1 : 1;

    plan->lastDist   = _plan.lastDist;

    plan->initPos.ball.x = _plan.ballInitPos.x;
    plan->initPos.ball.y = _plan.ballInitPos.y;

    for (int j = 0; j < _plan.agentSize; j++) {
        plan->initPos.agents.push_back(_plan.agentInitPos[j]);
    }

    QList< QList<playOffRobot> > agpln;
    for (unsigned int i = 0; i < _plan.agentSize; i++) {
        ROS_INFO_STREAM("agent " << i << " pos " << _plan.agents.at(i).posSize);
        QList<playOffRobot>  ag;
        ag.clear();
        for (unsigned int j = 0; j < _plan.agents.at(i).posSize; j++) {

            ROS_INFO_STREAM("agent " << i << " pos " << _plan.agents.at(i).posSize << "-> " << j);
            auto* po = new playOffRobot();

            po->pos.x = _plan.agents.at(i).positions.at(j).pos.x;
            po->pos.y = _plan.agents.at(i).positions.at(j).pos.y;
            po->angle = (AngleDeg)_plan.agents.at(i).positions.at(j).angel;
            po->tolerance = _plan.agents.at(i).positions.at(j).tolerance;



            QList<playOffSkill> sk;
            sk.clear();
            for (unsigned int k = 0; k < _plan.agents.at(i).positions.at(j).skillSize; k++) {
                auto *p = new playOffSkill();
                p->data[0] = static_cast<int>(_plan.agents.at(i).positions.at(j).skills.at(k).primary);
                p->data[1] = static_cast<int>(_plan.agents.at(i).positions.at(j).skills.at(k).secondry);
                p->targetAgent = static_cast<int>(_plan.agents.at(i).positions.at(j).skills.at(k).agent);
                p->targetIndex = static_cast<int>(_plan.agents.at(i).positions.at(j).skills.at(k).index);
                p->name = strToEnum(_plan.agents.at(i).positions.at(j).skills.at(k).name);
                sk.append(*p);
            }
            po->skill = sk;
            ag.append(*po);
        }
        ROS_INFO_STREAM("msg: plan agent" << i << " : " << ag.size());
        agpln.append(ag);

    }
    plan->execution.AgentPlan = agpln;

    return plan;
}

POFFSKILL CStaticPlayOff::strToEnum(const std::string& _str) {
    if (_str == "NoSkill") {
        return POFFSKILL::None;
    } else if (_str == "Mark") {
        return POFFSKILL::Mark;
    } else if (_str == "Goalie") {
        return POFFSKILL::Goalie;
    } else if (_str == "Support") {
        return POFFSKILL::Support;
    } else if (_str == "Defense") {
        return POFFSKILL::Defense;
    } else if (_str == "Position") {
        return POFFSKILL::Position;
    } else if (_str == "MoveSkill") {
        return POFFSKILL::Move;
    } else if (_str == "PassSkill") {
        return POFFSKILL::Pass;
    } else if (_str == "OneTouchSkill") {
        return POFFSKILL::OneTouch;
    } else if (_str == "ChipToGoalSkill") {
        return POFFSKILL::ChipToGoal;
    } else if (_str == "ShotToGoalSkill") {
        return POFFSKILL::ShotToGoal;
    } else if (_str == "ReceivePassSkill") {
        return POFFSKILL::ReceivePass;
    } else if (_str == "ReceivePassIASkill") {
        return POFFSKILL::ReceivePassIA;
    } else {
        return POFFSKILL::None;
    }
}

void CStaticPlayOff::matchPlan(SPlan *_plan, const QList<Agent*>& _ourplayers) {

    MWBM matcher;
    matcher.create(_plan->currentSize - 1, _ourplayers.size() - 1);

    int matchedIndex = 0;
    double weight = 0;
    double minweight = 10000000;

    for (int j = 0; j < _ourplayers.size(); j++) {
        weight = _ourplayers.at(j)->pos().dist(wm->ball->pos);
        if (weight < minweight) {
            minweight = weight;
            matchedIndex = j;
        }
    }
    _plan->matchedID.insert(0, matchedIndex);

    QList<int> othersmatch;
    for (int i = 1; i < _plan->currentSize; i++) {
        int k = 0;
        for (int j = 0; j < _ourplayers.size(); j++) {
            if (j != matchedIndex) {
                weight = _plan->initPos.agents.at(i).dist(_ourplayers.at(j)->pos());
                matcher.setWeight(i - 1, k, -(weight));
                othersmatch.append(j);
                k++;
            }
        }
    }

    int nmatchedID;
    ROS_INFO_STREAM("matched plan with : " << matcher.findMatching());
    for (int i = 1; i < _plan->currentSize; i++) {
        nmatchedID = matcher.getMatch(i - 1);
        nmatchedID = othersmatch.at(nmatchedID);
        _plan->matchedID.insert(i, nmatchedID);

    }
}

void CStaticPlayOff::checkGUItoRefineMatch(SPlan *_plan, const QList<Agent*>& _ourplayers) {
    QList<int> ids;
    for (const auto& a: _ourplayers) ids.append(a->id());

    if (conf.IDBasePasser && ids.contains(conf.PasserID)) {
        int temp = _plan->matchedID.value(0);
        _plan->matchedID[0] = ids.indexOf(conf.PasserID);
        for (int i = 1; i < _plan->matchedID.size(); i++) {
            if (_plan->matchedID[i] == ids.indexOf(conf.PasserID)) {
                _plan->matchedID[i] = temp;
                break;
            }
        }
    }

    if (conf.IDBaseOneToucher
        && ids.contains(conf.OneToucherID)) {
        int temp = _plan ->  matchedID.value(1);
        _plan -> matchedID[1] = ids.indexOf(conf.OneToucherID);
        for (int i = 2; i < _plan->matchedID.size(); i++) {
            if (_plan->matchedID[i] == ids.indexOf(conf.OneToucherID)) {
                _plan->matchedID[i] = temp;
                break;
            }
        }
    }
}

void CStaticPlayOff::parsePlan(const parsian_msgs::parsian_plan &_plan) {
    masterPlan = planMsgToSPlan(_plan, agents.size());
    analyseShoot(); // should call after setmasterplan
    analysePass();  // should call after setmasterplan

    matchPlan(masterPlan, agents); //Match The Plan
    checkGUItoRefineMatch(masterPlan, agents);

}
