#include <parsian_ai/plays/dynamicattack.h>

CDynamicAttack::CDynamicAttack() {
    dribbleIntention.start();
    lastPMInitWasDribble = false;
    //isShotInPass = false;
    lastPassPosLoc = Vector2D(5000, 5000);
    guardSize = 3;
    for (int &lastGuard : lastGuards) {
        lastGuard = -1;
    }
    positioningIntentionInterval = 500;
    shotInPass = false;

    for (int i = 0; i  < 6; i++) {
        roleAgents[i] = new CRoleDynamic();
    }
    roleAgentPM = new CRoleDynamic();

    passFlag   = false;
    repeatFlag = false;
    passerID   = -1;
    counter    = 0;
    currentPlan.agentSize = 0;
    lastPasserRoleIndex  = -1;

    isBallInOurField = ballPos.x < 0;

    guards[0] = new Rect2D();
    for (size_t i = 1; i < 7; i++) {
        guards[i] = new Rect2D[i];
        guardLocations[i] = new Vector2D* [i];
        for (size_t j = 0; j < i; j++) {
            guardLocations[i][j] = new Vector2D[3];
        }
    }



    for (bool &i : goToDynamic) {
        i = false;
    }

    assignRegions();
    assignLocations();
}

CDynamicAttack::~CDynamicAttack() {
    for (auto& roleAgent : roleAgents) {
        delete roleAgent;
    }

    delete roleAgentPM;

    delete   guards[0];
    delete   guards[1];
    delete[] guards[2];
    delete[] guards[3];
    delete[] guards[4];
    delete[] guards[5];

    for (size_t i = 1; i < 6; i++) {
        for (size_t j = 0; j < 6; j++) {
            delete[] guardLocations[i][j];
        }
    }

    for (size_t i = 0; i < 4; i++) {
        delete guardLocations[i];
    }

}

void CDynamicAttack::init(const QList<Agent*>& _agents) {
    setAgentsID(_agents);
    initMaster();

//    if( knowledge->getLastPlayExecuted() != StartPlay){
//        reset();
//    }
//    knowledge->setLastPlayExecuted(StartPlay);
}

void CDynamicAttack::reset() {
    executedCycles = 0;
    DBUG(QString("Dynamic Attack Reset"), D_MAHI);
}

void CDynamicAttack::execute_x() {
    DBUG(QString("Dynamic Attack : %1").arg(agents.size()),D_MAHI);
    ROS_INFO_STREAM("Dynamic Attack : " << agents.size());
    globalExecute(agents.size());
}

void CDynamicAttack::globalExecute(int agentSize) {
    ballPos = wm->ball->pos;
    ballVel = wm->ball->vel;
    OppGoal = wm->field->oppGoal();
    ballVel = (ballVel.length() < 0.2) ? Vector2D(0, 0) : ballVel;
    ballLocation();
    dynamicPlanner(agentSize);
}

void CDynamicAttack::makePlan(int agentSize) {

    //Initialize Plan with null values
    currentPlan.mode = DynamicMode::NoMode;
    currentPlan.agentSize = agentSize;
    DBUG(QString("[dynamicAttack] ball pos : %1").arg(ballPos.x), D_MAHI);

    for (auto &positionAgent : currentPlan.positionAgents) {
        positionAgent.region = DynamicRegion::NoMatter;
        positionAgent.skill  = DynamicSkill::NoSkill;
    }

    lastPMInitWasDribble = (currentPlan.playmake.skill == DynamicSkill::Dribble);

    //// We Don't have the ball -- counter-attack, blocking, move forward
    if (wm->ball->pos.x < 0) {
        currentPlan.mode = DynamicMode::NotWeHaveBall;
        if (conf.ChipForward) {
            currentPlan.playmake.init(DynamicSkill::Chip, DynamicRegion::Forward);
        } else {
            currentPlan.playmake.init(DynamicSkill::Chip, DynamicRegion::Goal);
        }
        for (size_t i = 0; i < agentSize; i++) {
            currentPlan.positionAgents[i].region = DynamicRegion::Near;
            currentPlan.positionAgents[i].skill  = DynamicSkill::Ready;
        }
    }
    //// we have ball and
    //// shot prob is more than 50%
    else if (directShot) {
        currentPlan.mode = DynamicMode::HighProb;
        currentPlan.playmake.init(DynamicSkill::Shot, DynamicRegion::Goal);
        for (size_t i = 0; i < agentSize; i++) {
            currentPlan.positionAgents[i].region = DynamicRegion::Best;
            currentPlan.positionAgents[i].skill  = DynamicSkill::Ready;
        }
    } else if (critical) {
        currentPlan.mode = DynamicMode::Critical;
        {
            oppRob = wm->field->oppGoal();
            lastPMInitWasDribble = true;
            if (conf.DribbleInFast) {
                currentPlan.playmake.init(DynamicSkill::Dribble, DynamicRegion::Goal);
            } else {
                currentPlan.playmake.init(DynamicSkill::Shot, DynamicRegion::Goal);
            }
            for (size_t i = 0; i < agentSize; i++) {
                currentPlan.positionAgents[i].region = DynamicRegion::Best;
                currentPlan.positionAgents[i].skill  = DynamicSkill::Ready;
            }
        }

    }
    //// if Defense isn't clearing and
    //// we have ball and
    //// shot prob isn't more than 50% and
    //// there isn't a critical situation and
    //// we don't have positioning agent
    else if (agentSize == 0) {
        currentPlan.mode = DynamicMode::NoPositionAgent;
        currentPlan.playmake.init(DynamicSkill::Shot, DynamicRegion::Goal);
    }
    // we have ball and
    // shot prob isn't more than 50% and
    // there isn't a critical situation and
    // we have positioning agents
    // it's needed to be fast
    else if (fast) {
        oppRob = wm->field->oppGoal();
        currentPlan.mode = DynamicMode ::Fast;
        if (conf.DribbleInFast) {
            currentPlan.playmake.init(DynamicSkill ::Dribble, DynamicRegion ::Goal);
        } else {
            currentPlan.playmake.init(DynamicSkill ::Shot, DynamicRegion ::Goal);
        }
        for (size_t i = 0; i < agentSize; i++) {
            currentPlan.positionAgents[i].region = DynamicRegion ::Best;
            currentPlan.positionAgents[i].skill  = DynamicSkill ::Ready;
        }
    }
    // if Defense isn't clearing and
    // we have ball and
    // shot prob isn't more than 50% and
    // there isn't a critical situation and
    // we have positioning agents
    // there's no need to be fast and
    // there is no plan for this situation
    else {
        currentPlan.mode = DynamicMode::NoMode;
        currentPlan.playmake.init(DynamicSkill::Pass, DynamicRegion::Best);
        for (size_t i = 0; i < agentSize; i++) {
            currentPlan.positionAgents[i].region = DynamicRegion::Best;
            currentPlan.positionAgents[i].skill  = DynamicSkill::Ready;
        }
    }
}

Agent* CDynamicAttack::getMahiPlayMaker() {
    return mahiPlayMaker;
}

void CDynamicAttack::assignId() {

    mahiPlayMaker = nullptr;
    if (playmakeID != -1) {
        mahiPlayMaker = playmake;
    }

    int tempIndex;
    QList<int> matchedIDList;
    mahiPositionAgents.clear();
    guardIndexList.clear();

    MWBM matcher;
    int n = agents.size();
    matcher.create(n, n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matcher.setWeight(i, j, -1 * semiDynamicPosition[i].dist(agents.at(j)->pos()));
        }
    }
    matcher.findMaxMinMatching();
    //matcher.findMatching();
    for (int i = 0; i < n; i++) {
        tempIndex = matcher.getMatch(i);
        matchedIDList.append(tempIndex);
        guardIndexList.append(i);
        mahiAgentsID[i] = tempIndex;
        mahiPositionAgents.append(agents.at(tempIndex));
    }
    for (auto mahiPoisitionAgent : mahiPositionAgents) {
        DBUG(QString("1 : %2").arg(mahiPoisitionAgent->id()), D_MAHI);
    }
}

void CDynamicAttack::assignTasks() {
    if (mahiPlayMaker != nullptr) {
        playMake();
    }
    if (currentPlan.agentSize > 0) {

        positioning(semiDynamicPosition);
    }

}

/**
 * @brief CDynamicAttack::dynamicPlanner
 * @param agentSize number of positioning Agents
 */
void CDynamicAttack::dynamicPlanner(int agentSize) {

    for (size_t i = 0; i < 6; i++) {
        mahiAgentsID[i] = -1;
    }

    makePlan(agentSize);

    for (int i = 0; i < agents.size(); i++) {
        if (agents[i]->isVisible() && agents[i]->action != nullptr) {
            Action *mahi = agents[i]->action;
            ROS_INFO_STREAM(i << ": " << mahi->getActionName().toStdString().c_str());
        }
    }
    if (agentSize > 0) {
        chooseBestPositons();
    }
    assignId();
    if (agentSize > 0) {
        //    chooseMarkPos();
        chooseBestPosForPass(semiDynamicPosition);
    }

    assignTasks();

    DBUG(QString("MODE : %1").arg(getString(currentPlan.mode)),D_MAHI);
    ROS_INFO_STREAM("MODE : " << getString(currentPlan.mode).toStdString());
    DBUG(QString("BALL : %1").arg(isBallInOurField),D_MAHI);
    for(size_t i = 0;i < agentSize;i++) {
        if(mahiAgentsID[i] >= 0) {
            roleAgents[i]->execute();
        } else {
            DBUG(QString("[dynamicAttack - %1] mahiAgentID buged").arg(__LINE__), D_MAHI);
        }
    }
    if (playmakeID != -1) {
        roleAgentPM->execute();
    }
    for (auto i : semiDynamicPosition) {
        drawer->draw(i, QColor(Qt::black));
    }

    for (auto i : dynamicPosition) {
        drawer->draw(Circle2D(i, 0.2), QColor(Qt::red), false);
    }

    showRegions(static_cast<unsigned int>(currentPlan.agentSize), QColor(Qt::gray));
    showLocations(static_cast<unsigned int>(currentPlan.agentSize), QColor(Qt::red));

    // TODO : remove this
    if (isPlayMakeChanged()) {
        for (bool &i : goToDynamic) {
            i = false;
        }
    }

}

void CDynamicAttack::playMake() {

    drawer->draw(Circle2D(mahiPlayMaker->pos(), 0.1), QColor(Qt::red), true);
    if (teamConfig.color == teamConfig.BLUE) {
        drawer->draw(Circle2D(mahiPlayMaker->pos() + mahiPlayMaker->dir() * 0.08, 0.06), QColor(Qt::blue), true);
    } else {
        drawer->draw(Circle2D(mahiPlayMaker->pos() + mahiPlayMaker->dir() * 0.08, 0.06), QColor(Qt::yellow), true);
    }

    roleAgentPM->setAgent(mahiPlayMaker);
    roleAgentPM->setAgentID(mahiAgentsID[0]);
    roleAgentPM->setAvoidPenaltyArea(true);

    Vector2D og = wm->ball->pos - wm->field->ourGoal();
    switch (currentPlan.playmake.skill) {
    case DynamicSkill ::Dribble:
        ROS_INFO_STREAM("drrible");//<< currentPlan.playmake.skill);
        roleAgentPM -> setTargetDir(currentPlan.passPos);
        roleAgentPM -> setTarget(oppRob);
        roleAgentPM -> setChip(false);
        roleAgentPM -> setNoKick(true);
        roleAgentPM -> setSelectedSkill(DynamicSkill ::Dribble); // skill Dribble
        break;
    case DynamicSkill ::Pass:
        ROS_INFO_STREAM("pass");
        roleAgentPM -> setChip(chipOrNot(currentPlan.passPos, 0.5, 0.1));
        roleAgentPM -> setTarget(currentPlan.passPos);
        roleAgentPM -> setEmptySpot(false);
        roleAgentPM -> setNoKick(false);
        if (roleAgentPM->getChip()) {
            roleAgentPM->setKickRealSpeed(appropriateChipSpeed());
        } else {
            roleAgentPM->setKickRealSpeed(appropriatePassSpeed());
        }

        roleAgentPM -> setSelectedSkill(DynamicSkill ::Pass);// Skill Kick

/////////////////////////////// I HATE PARSA
//        for(int i = 0; i < wm->opp.activeAgentsCount(); i++)
//            if(ballPos.dist(wm->field->oppGoal()) < 0.7 && Circle2D(wm->opp[wm->opp.activeAgentID(i)]->pos + wm->opp[wm->opp.activeAgentID(i)]->dir.norm()*0.08,0.07).contains(ballPos))
//                roleAgentPM -> setSelectedSkill(DynamicEnums::Dribble); // skill Dribble
//            else
//                roleAgentPM -> setSelectedSkill(DynamicEnums::Pass);// Skill Kick
        // TODO : fix this dastan
        /*if(isRightTimeToPass()) {
            roleAgentPM->setNoKick(false); //TEST
        } else {
            roleAgentPM->setNoKick(true);
        }*/
        break;

    case DynamicSkill ::Chip:
        ROS_INFO_STREAM("chip");
        roleAgentPM->setNoKick(false);
        if (currentPlan.playmake.region == DynamicRegion ::Goal) {
            roleAgentPM ->setTarget(wm->field->oppGoal());
            if (wm->ball->pos.x < -2) {
                roleAgentPM ->setKickRealSpeed(conf.HighSpeedChip);
            } else {
                roleAgentPM ->setKickRealSpeed(conf.MediumSpeedChip);

            }
        } else if (currentPlan.playmake.region == DynamicRegion ::Forward) {
            roleAgentPM->setTarget(Vector2D(1000, 0));
            roleAgentPM->setKickRealSpeed(conf.LowSpeedChip);
        } else {
            roleAgentPM->setTarget(wm->field->oppGoal());
            roleAgentPM->setKickRealSpeed(conf.LowSpeedChip);
        }
        roleAgentPM->setChip(true);
        roleAgentPM->setSelectedSkill(DynamicSkill ::Chip);// Skill Chip
        break;
    case DynamicSkill ::Shot: {
        roleAgentPM->setEmptySpot(true);
        roleAgentPM->setChip(false);
        roleAgentPM->setNoKick(false);
        roleAgentPM->setTarget(wm->field->oppGoal());
        roleAgentPM->setKickSpeed(1023); // TODO : 8m/s by profiller
        roleAgentPM->setSelectedSkill(DynamicSkill ::Shot); // Skill Kick
        break;
    }
    default:
        ROS_INFO_STREAM("default");
        roleAgentPM->setEmptySpot(true);
        roleAgentPM->setChip(false);
        roleAgentPM->setNoKick(false);
        roleAgentPM->setTarget(wm->field->oppGoal());
        // Parsa : ino hamintory avaz kardam kar kard...
        roleAgentPM->setKickSpeed(8); // TODO : 8m/s by profiller
        roleAgentPM->setSelectedSkill(DynamicSkill ::Shot); // Skill Kick
        break;
    }
}

void CDynamicAttack::positioning(QList<Vector2D> _points) {
    bool check = false;
    for (int i = 0 ; i < currentPlan.agentSize; i++) {
        if (mahiAgentsID[i] >= 0) {
            roleAgents[i]->setAgentID(mahiAgentsID[i]);
            roleAgents[i]->setAgent(mahiPositionAgents.at(i));
            roleAgents[i]->setAvoidPenaltyArea(true);
            if (i < _points.size()) {

                switch (currentPlan.positionAgents[i].skill) {
                case DynamicSkill ::Ready: // Ready For Pass

                    roleAgents[i]->setTarget(_points.at(i));
                    roleAgents[i]->setReceiveRadius(
                        std::max(0.5, 2 - roleAgents[i]->getAgent()->pos()
                                 .dist(roleAgents[i]->getTarget())));
                    roleAgents[i]->setSelectedSkill(DynamicSkill ::Ready);// Receive Skill

                    break;
                case DynamicSkill ::OneTouch: // OneTouch Reflects

                    roleAgents[i]->setWaitPos(_points.at(i));
                    roleAgents[i]->setReceiveRadius(
                        std::max(0.5, 2 - roleAgents[i]->getAgent()->pos()
                                 .dist(roleAgents[i]->getTarget())));

                    // TODO : fix the target
                    roleAgents[i]->setTarget(wm->field->oppGoal());
                    roleAgents[i]->setSelectedSkill(DynamicSkill ::OneTouch);// Receive Skill

                    break;
                case DynamicSkill ::Move:


                    roleAgents[i]->setReceiveRadius(
                        std::max(0.5, 2 - roleAgents[i]->getAgent()->pos()
                                 .dist(roleAgents[i]->getTarget())));
                    roleAgents[i]->setTarget(_points.at(i));
                    roleAgents[i]->setTargetDir(ballPos - roleAgents[i]->getAgent()->pos());
                    roleAgents[i]->setSelectedSkill(DynamicSkill ::Move);

                    break;
                case DynamicSkill ::NoSkill:
                    roleAgents[i]->setSelectedSkill(DynamicSkill ::Ready);// Receive Skill

                    break;
                }

                if (roleAgents[i]->getTarget() == currentPlan.passPos) {
                    check = true;
                }
                //debug(QString("pos positions : %1 %2").arg(roleAgents[i]->getTarget().x).arg(roleAgents[i]->getTarget().y), D_PARSA);
            }
        } else {
            DBUG("[dynamicAttack] mahiagent ha eshtebahe chera ?", D_MAHI);
        }
    }
    //assert(check);
}


inline bool CDynamicAttack::chipOrNot(Vector2D target,
                                      double _radius, double _treshold) {
    return !isPathClear(ballPos, target, _radius, _treshold);
}

bool CDynamicAttack::keepOrNot() {
    return true;
}

bool CDynamicAttack::isPathClear(Vector2D _pos1, Vector2D _pos2,
                                 double _radius, double treshold) {
    Vector2D sol1, sol2, sol3;
    Line2D _path(_pos1, _pos2);
    Polygon2D _poly;
    Circle2D(_pos2, _radius + treshold).
    intersection(_path.perpendicular(_pos2), &sol1, &sol2);

    _poly.addVertex(sol1);
    sol3 = sol1;
    _poly.addVertex(sol2);
    Circle2D(_pos1, Robot::robot_radius_new + treshold).
    intersection(_path.perpendicular(_pos1), &sol1, &sol2);

    _poly.addVertex(sol2);
    _poly.addVertex(sol1);
    _poly.addVertex(sol3);

    drawer->draw(_poly, "red");

    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (_poly.contains(wm->opp.active(i)->pos)) {
            return false;
        }
    }
    return true;
}

bool CDynamicAttack::isPlayMakeChanged() {

    if (mahiPlayMaker != nullptr) {
        if (mahiPlayMaker->id() != lastPlayMakerId) {
            lastPlayMakerId = mahiPlayMaker->id();
            return true;
        }
    }
    return false;
}

int CDynamicAttack::appropriatePassSpeed() {

    double tempDistance = 0;
    double speed = 0;
    if (mahiPlayMaker != nullptr) {
        tempDistance = mahiPlayMaker->pos().dist(currentPlan.passPos);

        if (false) { // dynamic pass Speed // FALSED IN ROS
//            if (tempDistance < 2) {
//                speed = knowledge->getProfile(mahiPlayMaker->id(), tempDistance) + policy()->DynamicPlay_LowSpeedPass();
//
//            } else if(tempDistance > 4) {
//                speed = knowledge->getProfile(mahiPlayMaker->id(), tempDistance) + policy()->DynamicPlay_HighSpeedPass();
//
//            } else {
//                speed = knowledge->getProfile(mahiPlayMaker->id(), tempDistance) + policy()->DynamicPlay_MediumSpeedPass();
//
//            }

        } else {
            if (tempDistance < 2) {
                speed = conf.LowSpeedPass;

            } else if (tempDistance > 4) {
                speed = conf.HighSpeedPass;

            } else {
                speed = conf.MediumSpeedPass;

            }

        }
    } else {
        speed = conf.MediumSpeedPass;
    }
    return static_cast<int>(speed);
}


int CDynamicAttack::appropriateChipSpeed() {

    double tempDistance = 0;
    double speed = 0;
    if (mahiPlayMaker != nullptr) {
        tempDistance = mahiPlayMaker->pos().dist(currentPlan.passPos);

        if (false) { // dynamic chip Speed
//            if (tempDistance < 2) {
//                speed = knowledge->getProfile(mahiPlayMaker->id(), tempDistance, false) + policy()->DynamicPlay_LowSpeedChip();
//
//            } else if(tempDistance > 4) {
//                speed = knowledge->getProfile(mahiPlayMaker->id(), tempDistance, false) + policy()->DynamicPlay_HighSpeedChip();
//
//            } else {
//                speed = knowledge->getProfile(mahiPlayMaker->id(), tempDistance, false) + policy()->DynamicPlay_MediumSpeedChip();
//
//            }

        } else {
            if (tempDistance < 2) {
                speed = conf.LowSpeedChip;

            } else if (tempDistance > 4) {
                speed = conf.HighSpeedChip;

            } else {
                speed = conf.MediumSpeedChip;

            }

        }
    } else {
        speed = conf.MediumSpeedPass;
    }
    return static_cast<int>(speed);
}

void CDynamicAttack::chooseBestPositons() {
    //it has three code of choosing best position that only one of them must be uncommented
    int agentSize = agents.size();
    int cntD = 0;
    Vector2D ans;
    bool haveSupporter = false;
    Vector2D passPos;
    guardIndexList.clear();
    for (int i = 0; i < currentPlan.agentSize; i++) {
        guardIndexList.append(i);
    }

    semiDynamicPosition.clear();

    if (mahiPlayMaker != nullptr && ballPos.dist(mahiPlayMaker->pos()) < 0.15) {
        passPos        = currentPlan.passPos;
        lastPassPosLoc = currentPlan.passPos;
    } else {
        passPos = lastPassPosLoc;
    }
    //first type of choosing and probably the best one
    //this choose the points with maximum x that has a good one touch angle
    for (int i = 0; i < agentSize; i++) {
        //if it wants to get a dribble position:
        if (currentPlan.positionAgents[i].region == DynamicRegion ::Supporter ||
                (i <= 1 && dribbleIntention.elapsed() < 3000)) {
            cntD++;
            double x, y;
            if (cntD == 1) {
                if (ballPos.x < 3.3 - thrshDribble) {
                    thrshDribble = 0;
                    if (lastYDrib == 10) {
                        if (ballPos.y > 0) {
                            y = ballPos.y - 1.5;
                        } else {
                            y = ballPos.y + 1.5;
                        }
                    } else if (ballPos.y > 0.2) {
                        y = ballPos.y - 1.5;
                    } else if (ballPos.y < -0.2) {
                        y = ballPos.y + 1.5;
                    } else {
                        y = lastYDrib;
                    }
                    x = min(ballPos.x - 0.1, 3.3);
                } else {
                    thrshDribble = 0.2;
                    if (lastYDrib == 10) {
                        if (ballPos.y > 0) {
                            y = -1.35;
                        } else {
                            y = 1.35;
                        }
                    } else if (ballPos.y > 0.2) {
                        y = -1.35;
                    } else if (ballPos.y < -0.2) {
                        y = 1.35;
                    } else {
                        y = lastYDrib;
                    }
                    x = 3.65;
                }
                lastYDrib = y;
            }
            if (cntD == 2) {
                x = ballPos.x - 1;
                y = ballPos.y;
            }
            semiDynamicPosition.append(Vector2D(x, y));
            if (i == 1) {
                haveSupporter = true;
                i = -1;
                agentSize -= 2;
                continue;
            }
        }

        //if it wants to get the BEST position :
        //first if we can have supporter :
        if (!haveSupporter && conf.SupportPriority <= agentSize) {
            haveSupporter = true;
            semiDynamicPosition.append(Vector2D(ballPos.x - 1, ballPos.y));
            i--;
            agentSize--;
            continue;
        }
        //second if we have only one positioning
        if (agentSize == 1) {
            if (ballPos.y > 0.3) {
                semiDynamicPosition.append(guardLocations[agentSize][0][2]);
                lastGuards[0] = 2;
            } else if (ballPos.y < -0.3) {
                semiDynamicPosition.append(guardLocations[agentSize][0][0]);
                lastGuards[0] = 0;
            } else {
                semiDynamicPosition.append(guardLocations[agentSize][0][lastGuards[0]]);
            }
        }
        //third if we have more than one positioning :
        else {
            lastYDrib = 10;
            int best = -1;
            Vector2D  points  [guardSize];
            double tempAngle  [guardSize];
            bool tooNearToBall[guardSize];
            bool farChoice = false;
            bool tooFarToBall [guardSize];
            double ballVelCoef = 0.3;
            double maxAng = -1;

            for (int j = 0; j < guardSize; j++) {
                points[j]    = guardLocations[agentSize][i][j];
                tooNearToBall[j] = ((ballPos + ballVel * ballVelCoef).
                                    dist(points[j]) <= 1.2);
                tooFarToBall[j] = ((ballPos + ballVel * ballVelCoef).
                                   dist(points[j]) > 4.7);
                tempAngle[j] = Vector2D::angleOf(wm->field->oppGoal(),
                                                 points[j], ballPos + 0.2 * ballVel).degree();
                //            debug(QString("angle   is  : %1").arg(tempAngle    [j]), D_PARSA);
                //            debug(QString("toofar  is  : %1").arg(tooFarToBall [j]), D_PARSA);
                //            debug(QString("toonear is  : %1").arg(tooNearToBall[j]), D_PARSA);
                maxAng = max(maxAng, tempAngle[j]);
            }
            for (int j = 0; j < guardSize; j++)
                if (tempAngle[j] < 70 || (j == guardSize - 1 && tempAngle[j] < 80))
                    if (best == -1 || points[best].x < points[j].x)
                        if (!tooNearToBall[j])
                            if (!tooFarToBall[j] || agentSize != 3) { //exception for 3 positionings
                                best = j;
                            }
            //        debug(QString("best is : %1").arg(best), D_PARSA);
            for (int j = 0; j < guardSize; j++)
                if (points[j].dist(passPos) < 0.3) {
                    best = j;
                }
            double   coef = 1.3;
            Vector2D nextPos;
            do {
                coef -= 0.1;
                nextPos = ballPos + ballVel * coef;
            } while (!(wm->field->isInField(nextPos)) &&
                     (wm->field->isInField(ballPos)));
            if (guards[currentPlan.agentSize][i]
                    .contains(nextPos)) {
                best = farGuardFromPoint(i, nextPos);
                farChoice = true;
            }
            //        debug(QString("best is : %1").arg(best), D_PARSA);
            if (agentSize == 3 && best == guardSize - 1 && i == 1
                    && points[best].x > wm->ball->pos.x + 1) { //exception for 3 positionings
                Line2D l = Line2D(wm->ball->pos, points[best]);
                Vector2D dummy;
                Segment2D s = Segment2D(wm->field->oppGoalR() - Vector2D(0, 0.3),
                                        wm->field->oppGoalL() + Vector2D(0, 0.3));
                dummy = s.intersection(l);
                if (dummy.x < 10) {
                    if (wm->ball->pos.y < 0) {
                        ans = points[best] + Vector2D(0, 0.6);
                    } else {
                        ans = points[best] - Vector2D(0, 0.6);
                    }
                    best = -2;
                }
            }
            if (best == -2);
            else if (best == -1) {
                if ((i == guardSize - 1 || i == 0) && (agentSize == 3)) {
                    if (ballPos.dist(points[guardSize / 2]) > 0.6) {
                        ans = points[guardSize / 2];
                    } else if (ballPos.dist(points[0]) > 0.6) {
                        ans = points[0];
                    } else {
                        ans = Vector2D(0, points[0].y);
                    }
                } else {
                    ans = Vector2D(0, points[0].y);
                }
            } else if (i < currentPlan.agentSize) {
                ans = points[best];
            } else if (currentPlan.mode == DynamicMode ::DefenseClear) {
                ans = Vector2D(0, 0);
            } else
                ans = guardLocations[currentPlan.agentSize]
                      [currentPlan.agentSize - 1]
                      [best];

            int matchId = -1;
            for (int j = 0; j < guardSize; j++)
                if (points[j] == ans) {
                    matchId = j;
                }
            //        debug(QString("ans is matchId: %1 %2 %3").arg(ans.x).arg(ans.y).arg(matchId), D_PARSA);
            //        debug(QString("last guard is : %1").arg(lastGuards[i]), D_PARSA);
            //        debug(QString("%1").arg(positioningIntention.msec()), D_PARSA);
            if (matchId == -1) {
                semiDynamicPosition.append(ans);
                positioningIntention.restart();
                lastGuards[i] = -1;
            } else if (matchId != lastGuards[i]) {
                if (lastGuards[i] == -1 || farChoice ||
                        positioningIntention.msec() > positioningIntentionInterval) {
                    semiDynamicPosition.append(ans);
                    positioningIntention.restart();
                    lastGuards[i] = matchId;
                } else {
                    semiDynamicPosition.append(points[lastGuards[i]]);
                }
            } else {
                semiDynamicPosition.append(points[matchId]);
                positioningIntention.restart();
            }
            //        debug(QString(""), D_PARSA);
        }
    }
    /*second type of choosing and very new but not good and is not tested well
     maybe it cause segment error
     and now it does not work as defenition of some of its variables omitted
     to fix that and run this code add this lines to the private part of
     class CDynamicAttack :


    Vector2D lastBallPos;
    And add this to the constructor of the class:

    lastBallPos    = Vector2D(5000, 5000);
    */


    /*if(mahiPlayMaker != NULL && ballPos.dist(mahiPlayMaker->pos()) > 0.15)
    {
        Vector2D passPos = currentPlan.passPos;
        passPos = lastPassPosLoc;
    }
    if(lastBallPos.dist(ballPos) > 0.2)
        passPos = currentPlan.passPos;

    for(int i = 0; i < agentSize; i++)
    {
        debug(QString("region %1").arg(i), D_PARSA);
        bool     canNot[guardSize];
        double   score [guardSize];
        Vector2D points[guardSize];
        Vector2D nextBall = ballPos + ballVel * ballVelCoef;

        double minPassDist = 1000;
        double ballVelCoef = 0.4;
        int    minPassId   = -1;

        for(int j = 0; j < guardSize; j++)
        {
            canNot[j] = 0;
            score[j]  = 0;
            points[j] = guardLocations[agentSize][i][j];
        }

        for(int j = 0; j < guardSize; j++)
        {

            score[j] += j * 0.05;
            double temp = 1 / max(0.5, wm->opp[knowledge->getNearestOppToPoint
                    (points[j])]->pos.dist(points[j]));
            score[j] -= temp;
            debug(QString("negative score : %1").arg(temp), D_PARSA);
            if(nextBall.dist(points[j]) < 0.3)
                canNot[j] = 1;
            Line2D ballToPoint = Line2D(ballPos, points[j]);
            Sement2D goalLine;
            goalLine = Segment2D(wm->field->oppGoalL(), wm->field->oppGoalR());
            if(goalLine.intersection(ballToPoint).x <= _FIELD_WIDTH / 2)
                if(points[j].x > ballPos.x)
                    canNot[j] = 1;
            if(passPos.dist(points[j]) < minPassDist)
            {
                minPassDist = passPos.dist(points[j]);
                minPassId   = j;
            }
            if(guards[agentSize][i].contains(nextBall))
            {
                double temp = sqrt(points[j].dist(nextBall)) / 4;
                score[j] += temp;
                debug(QString("points for faring from ball : %1").arg(temp), D_PARSA);
            }
            score[j] -= nextBall
        }
        if(lastGuards[i] != -1 && lastBallPos.dist(ballPos) < 0.2)
        {
            score[lastGuards[i]] += 0.2;
            debug(QString("last guard : %1").arg(lastGuards[i]), D_PARSA);
        }
        if(guards[agentSize][i].contains(passPos))
            if(Line2D(ballPos, ballPos + ballVel * ballVelCoef).dist(points[minPassId]) < 0.6)
            {
                debug(QString("minPassId : %1").arg(minPassId), D_PARSA);
                score[minPassId] += 2;
            }
        int minId = -1;
        for(int j = 0; j < guardSize; j++)
            if(minId == -1 || score[j] > score[minId])
                if(!canNot[j] || (guards[agentSize][i].contains(passPos) && j == minPassId))
                    minId = j;

        if(minId == -1)
        {
            semiDynamicPosition.append(Vector2D(0, points[0].y));
            lastGuards[i] = -1;
        }
        else if(i < currentPlan.agentSize)
        {
            semiDynamicPosition.append(points[minId]);
            lastGuards[i] = minId;
        }
        else if(currentPlan.mode == DynamicEnums::DefenseClear)
            semiDynamicPosition.append(Vector2D(0, 0));
        else
            semiDynamicPosition.append(guardLocations[currentPlan.agentSize]
                                                     [currentPlan.agentSize - 1]
                                                     [minId]);

        if(mahiPlayMaker != NULL && ballPos.dist(mahiPlayMaker->pos()) <= 0.15)
            lastPassPosLoc = passPos;

        /*debug(QString("passPos : %1 %2").arg(passPos.x).arg(passPos.y), D_PARSA);

        for(int j = 0; j < guardSize; j++)
        {
            debug(QString("score and canNot of point %1 %2 are %3 %4")
                  .arg(points[j].x).arg(points[j].y).arg(score[j])
                  .arg(canNot[j]), D_PARSA);
        }
        debug(QString(""), D_PARSA);
    }*/



    //third type of choosing and the old one

    /*draw(Segment2D(ballPos,wm->field->oppGoalL()), QColor(Qt::black));
    draw(Segment2D(ballPos,wm->field->oppGoalR()), QColor(Qt::black));
    int tempIndex = 0;
    double tempDist, minDist;

    guardIndexList.clear();
    for(int i = 0; i < currentPlan.agentSize; i++)
        guardIndexList.append(i);

    semiDynamicPosition.clear();
    for (size_t i = 0;i < currentPlan.agentSize;i++) {
        for (size_t j = 0;j < 3;j++) {
            tempDist = currentPlan.passPos
                      .dist(guardLocations[currentPlan.agentSize][i][j]);
            if (tempDist < minDist) {
                minDist = tempDist;
                tempIndex = j;
            }
        }
        if(guards[currentPlan.agentSize][i]
           .contains(ballPos + ballVel)) {
            tempIndex = farGuardFromPoint(i, ballPos + ballVel);
        }
        if(i < currentPlan.agentSize) {
            semiDynamicPosition.append(guardLocations[currentPlan.agentSize]
                    [i]
                    [tempIndex]);
        }
        else if(currentPlan.mode == DynamicEnums::DefenseClear) {
            semiDynamicPosition.append(Vector2D(0, 0));
        }
        else {
            semiDynamicPosition.append(guardLocations[currentPlan.agentSize]
                    [currentPlan.agentSize - 1]
                    [tempIndex]);
        }
    }*/

    //    Vector2D poses[5], bp = ballPos;
    //    double -x = 1, -y = 0.8;
    //    if(bp.y + y <= _FIELD_HEIGHT / 2 || )
    //        y * = 1;
}

// TODO : remove this without kindness
void CDynamicAttack::chooseMarkPos() {
    markPositions.clear();
    for (int i = 0; i < 3; i++) {
        markPositions.append((Vector2D(5000, 5000)));
    }
    if (roleAgentPM->getAgent() == nullptr) {
        return;
    }
    Vector2D reflectPos[3];
    Vector2D temp[4];
    Circle2D oppCircle(Vector2D(wm->field->_FIELD_WIDTH / 2, 0) + Vector2D(0.5, 0), 1.5);
    drawer->draw(oppCircle, QColor(Qt::black));
    //third marker
    Vector2D good = Vector2D(wm->field->_FIELD_WIDTH / 2, 0);
    if (roleAgentPM->getAgent() != nullptr && roleAgentPM->getAgent()->pos().y <= 0) {
        good = wm->field->oppGoalR();
    } else {
        good = wm->field->oppGoalL();
    }
    oppCircle.intersection(Segment2D(wm->ball->pos, good), &temp[2], &temp[3]);
//    if(roleAgentPM->getAgent()!= nullptr) // TODO : reflect pos
//        reflectPos[2] = knowledge->getReflectPos((Vector2D(wm->field->_FIELD_WIDTH/2,0) + good)/2, max(1.5, min(2.0, roleAgentPM->getAgent()->pos().dist(Vector2D(wm->field->_FIELD_WIDTH/2,0)) - 0.9)));
    int l = 3;
    if (roleAgentPM->getAgent() != nullptr) {
        l = 0;
        std::vector <Vector2D> vt;
        for (int i = 0; i < 3; i++)
            if (reflectPos[i].x < 20) {
                l++;
                vt.push_back(reflectPos[i]);
            }
        for (int i = 0; i < l; i++)
            for (int j = 0; j < l - 1; j++)
                if (roleAgentPM->getAgent()->pos().dist(vt[j]) < roleAgentPM->getAgent()->pos().dist(vt[j + 1])) {
                    std::swap(vt[j], vt[j + 1]);
                }
        /*for(int i = 0; i < 3; i++)
        {
            reflectPos[i].x = reflectPos[i].y = 5000;
        }*/
        for (int i = 0; i < l; i++) {
            reflectPos[i] = vt[i];
        }
    }
    /*reflectPos[0] = knowledge->getReflectPos(wm->field->oppGoal());
    reflectPos[1] = knowledge->getReflectPos(wm->field->oppGoalL());
    reflectPos[2] = knowledge->getReflectPos(wm->field->oppGoalR());*/

    markPositions.clear();

    for (auto reflectPo : reflectPos) {
        drawer->draw(reflectPo, QColor(Qt::cyan));
    }



    MWBM matcher;
    int n = mahiPositionAgents.size();
    matcher.create(n, 3);
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < 3; j++) {
            matcher.setWeight(i,
                              j,
                              (-1) *
                              (mahiPositionAgents[i] ->
                               pos().dist(reflectPos[j]) - (reflectPos[i].x < 30 ? 5 * roleAgentPM->getAgent()->pos().dist(reflectPos[j]) : 0)));

        }
    }
    matcher.findMatching();
    for (int i = 0; i < l; ++i) {
        //debug(QString("%1").arg(matcher.getMatch(i)),D_ALI);
        markPositions.append(reflectPos[matcher.getMatch(i)]);
        drawer->draw(markPositions.at(i), QColor(Qt::black));
    }

}


void CDynamicAttack::chooseBestPosForPass(QList<Vector2D> _points) {
    //the new one:
    QList <Vector2D> temp;
    int ans = 0;
    double points[10] = {};

    for (auto _point : _points) {
        temp.append(_point);
    }

//    debug(QString("DIntention %3").arg(dribbleIntention.elapsed()), D_PARSA);
    //if we are dribbling
    int a = temp.size();
    ROS_INFO("build shode 2");
    if (dribbleIntention.elapsed() < 3000) {
        currentPlan.passPos =  temp[0];
        return;
    }
    //else

    for (int i = 0; i < temp.size(); i++) {
        if (lastPassPosLoc == temp[i]) {
            points[i] += 2;
        }
        DBUG(QString("%1 %2 point is %3").arg(temp.at(i).x).arg(temp.at(i).y).arg(points[i]), D_PARSA);
        double M = 100;
        if (mahiPlayMaker != nullptr) {
            for (int j = 0; j < wm->opp.activeAgentsCount(); j++) {
                M = min(M, wm->opp.active(j)->pos.dist(mahiPlayMaker->pos()));
                M = min(M, (wm->opp.active(j)->pos +
                            wm->opp.active(j)->vel * 0.5).dist(mahiPlayMaker->pos()));
            }
            //if(M < 2)
            {
                double e = mahiPlayMaker->dir().angleOf(temp[i], mahiPlayMaker->pos(),
                                                        mahiPlayMaker->dir().norm() * 1 + mahiPlayMaker->pos()).degree();
                double p = (e / 30) / (M + 0.001);
                points[i] -= p;
                DBUG(QString("angle is %1").arg(mahiPlayMaker->dir().angleOf(temp[i], mahiPlayMaker->pos(),
                                                mahiPlayMaker->dir().norm() * 1 + mahiPlayMaker->pos()).degree()), D_PARSA);
            }
        }
        DBUG(QString("%1 %2 near opp %3").arg(temp.at(i).x).arg(temp.at(i).y).arg(points[i]), D_PARSA);
        M = 100;
        for (int j = 0; j < wm->opp.activeAgentsCount(); j++) {
            M = min(M, wm->opp.active(j)->pos.dist(temp[i]));
        }
        if (M < 2) {
            points[i] += M;
        } else {
            points[i] += 4;
        }
        DBUG(QString("%1 %2 opptotemp %3").arg(temp.at(i).x).arg(temp.at(i).y).arg(points[i]), D_PARSA);
//        points[i] -= ballPos.dist(temp[i]) / 2;
        M = 100;
        for (int j = 0; j < wm->our.activeAgentsCount(); j++) {
            M = min(M, wm->our.active(j)->pos.dist(temp[i]));
        }
        if (M > 1) {
            points[i] -= 5;
        }
        if (points[i] > points[ans]) {
            ans = i;
        }
        DBUG(QString("%1 %2 ourtopoint %3").arg(temp.at(i).x).arg(temp.at(i).y).arg(points[i]), D_PARSA);
        DBUG(QString("end"), D_PARSA);
    }
    currentPlan.passPos = temp[ans];
    lastPassPosLoc = currentPlan.passPos;
    DBUG(QString("pass Pos %1 %2").arg(currentPlan.passPos.x).arg(currentPlan.passPos.y), D_PARSA);
}

double CDynamicAttack::getDynamicValue(const Vector2D &_dynamicPos) const {
    double defMoveAngle, openAngle;
    defMoveAngle = Vector2D::angleOf(ballPos, OppGoal, _dynamicPos).degree();
    return defMoveAngle;
}

Vector2D CDynamicAttack::neaerstGuardToPoint(const Vector2D &startVec) const {
    int regionIndex = 0;
    double tempDist, minDist = 10000;
    const int positionAgentCnt = currentPlan.agentSize - 1;
    int index = 0;
    for (int i = 0; i < positionAgentCnt; i++) {
        if (guards[positionAgentCnt][i].contains(startVec)) {
            regionIndex = i;
            break;
        }
    }

    for (int i = 0; i < 3; i++) {
        tempDist = guardLocations[positionAgentCnt][regionIndex][i].dist(startVec);
        if (tempDist < minDist) {
            minDist = tempDist;
            index = i;
        }
    }

    return guardLocations[positionAgentCnt][regionIndex][index];
}

bool CDynamicAttack::isRightTimeToPass() {
    double minDist = 99999, tempDist;
    int tempDefIndex = 0;

    for (int i = 0; i < mahiPositionAgents.size(); i++) {
        tempDist = mahiPositionAgents.at(i)->pos().dist(currentPlan.passPos);
        if (tempDist < minDist) {
            minDist      = tempDist;
            tempDefIndex = i;
        }
    }
    if (semiDynamicPosition.size() > tempDefIndex) {
        if (mahiPositionAgents.at(tempDefIndex)->pos()
                .dist(semiDynamicPosition.at(tempDefIndex)) < conf.Area) {
            return true;
        }
    }
    return false;
}

void CDynamicAttack::managePasser() {

    //    Vector2D sol1,sol2;

    //    if(!repeatFlag) {
    //        if(currentPlan.agents[0].role == DynamicEnums::PlayMaker) {
    //            if(currentPlan.agents[0].skill == DynamicEnums::Pass) {
    //                if(Circle2D(mahiPlayMaker->pos() + mahiPlayMaker->dir()*0.08,
    //                            0.08).contains(ballPos)) {
    //                    passFlag = true;
    //                }
    //                else if(!Circle2D(mahiPlayMaker->pos(),0.3).contains(ballPos)) {
    //                    if(Circle2D(currentPlan.passPos,1)
    //                            .intersection(Ray2D(ballPos,
    //                                                ballPos + ballVel),
    //                                          &sol1, &sol2)) {
    //                        if(passFlag) {
    //                            passFlag   = false;
    //                            repeatFlag = true;
    //                            passerID   = mahiPlayMaker->id();
    //                        }
    //                    }
    //                }
    //            }
    //        }
    //    }        /////////ISPASSNOTDONE///////////
    //    if(repeatFlag && (counter < 100)) {
    //        counter++;
    //        repeatFlag = true;
    //        for(size_t i = 0; i  < agents.size();i++) {
    //            if(roleAgents[i]->getAgent()->id() == passerID)
    //                lastPasserRoleIndex = i;
    //        }

    //        roleAgents[lastPasserRoleIndex]->setAgent(knowledge->getAgent(passerID));
    //        roleAgents[lastPasserRoleIndex]->setAgentID(passerID);
    //        roleAgents[lastPasserRoleIndex]->setAvoidPenaltyArea(true);
    //        roleAgents[lastPasserRoleIndex]->setTarget(Vector2D(0,0));
    //        roleAgents[lastPasserRoleIndex]->setReceiveRadius(0.1);
    //        roleAgents[lastPasserRoleIndex]->setSelectedSkill(DynamicEnums::Mark);

    ////        counter    = 0;
    ////        repeatFlag = false;
    ////        passerID   = -1;
    //    }
    //    else {
    //        passerID = -1;
    //    }

    //    debug(QString("PasserID %1 , Counter : %2").arg(passerID).arg(counter),D_MAHI);
    //    debug(QString("playMake %1 , skill : %2"  ).arg(mahiPlayMaker->id())
    //          .arg(roleAgentPM->getSelectedSkill()),D_MAHI);
    //    //    debug(QString("LPRI %1 , skill : %2").arg(mahiPlayMaker->id()).arg(roleAgentPM->getSelectedSkill()),D_MAHI);

}

int CDynamicAttack::farGuardFromPoint(const int &_guardIndex,
                                      const Vector2D &_point) {
    double tempDist, tempMax = 0;
    int tempIndex = -1;
    for (int i = 0; i < 3; i++) {
        tempDist = guardLocations[currentPlan.agentSize]
                   [guardIndexList.at(_guardIndex)]
                   [i].dist(_point);
        if (i == lastGuards[_guardIndex]) {
            tempDist += 0.3;
        }
        if (tempDist  > tempMax) {
            tempMax = tempDist;
            tempIndex = i;
        }
    }
    return tempIndex;
}

void CDynamicAttack::checkPoints(QList<Vector2D>& _points) {

    Rect2D validField(-wm->field->_FIELD_WIDTH / 2,
                      wm->field->_FIELD_HEIGHT / 2,
                      wm->field->_FIELD_WIDTH,
                      wm->field->_FIELD_HEIGHT);

    for (size_t i = 0; i < _points.size() - 1; i++) {
        if (!validField.contains(_points.at(i))) {
            _points.removeAt(i);
        }
    }
}

int CDynamicAttack::minHorizontalDistID(const QList<Vector2D> &_points) {
    double tempDist, minDist = 100000;
    int tempIndex = -1;

    for (int i = 0; i < _points.size(); i++) {
        tempDist = fabs(ballPos.y - _points.at(i).y);
        if (lastPassPos == i) {
            tempDist -= 2;
        }
        if (tempDist < minDist && fabs(ballPos.y - _points.at(i).y) > 0.2) {
            minDist = tempDist;
            tempIndex = i;
        }
    }
    lastPassPos = tempIndex;
    return tempIndex;
}

int CDynamicAttack::maxHorizontalDistID(const QList<Vector2D> &_points) {
    double tempDist, maxDist = -1;
    int tempIndex = -1;

    for (int i = 0; i < _points.size(); i++) {
        tempDist = fabs(ballPos.y - _points.at(i).y);
        if (lastPassPos == i) {
            tempDist += 2;
        }
        if (tempDist > maxDist && fabs(ballPos.y - _points.at(i).y) > 0.2) {
            maxDist = tempDist;
            tempIndex = i;
        }
    }
    lastPassPos = tempIndex;
    return tempIndex;
}

void CDynamicAttack::assignLocations() {
    // order of call function is important
    assignLocations_0();
    assignLocations_1();
    assignLocations_2();
    assignLocations_3();
    assignLocations_4();
    assignLocations_5();
    assignLocations_6();
    ////////////////////////////////////////

}

void CDynamicAttack::assignRegions() {
    // order of call function is important
    assignRegion_0();
    assignRegion_1();
    assignRegion_2();
    assignRegion_3();
    assignRegion_4();
    assignRegion_5();
    assignRegion_6();
    ////////////////////////////////////////
}

void CDynamicAttack::assignRegion_0() {
    //Opp Field
    guards[0]->assign(-wm->field->_CENTER_CIRCLE_RAD,                 //top_X
                      wm->field->_FIELD_HEIGHT / 2,                   //top_Y
                      wm->field->_FIELD_WIDTH / 2 + wm->field->_CENTER_CIRCLE_RAD, //X_Length
                      wm->field->_FIELD_HEIGHT);                      //Y_Length
}


void CDynamicAttack::assignRegion_1() {
    //Opp Field
    guards[1][0].assign(-wm->field->_CENTER_CIRCLE_RAD,                 //top_X
                        wm->field->_FIELD_HEIGHT / 2,                   //top_Y
                        wm->field->_FIELD_WIDTH / 2 + wm->field->_CENTER_CIRCLE_RAD, //X_Length
                        wm->field->_FIELD_HEIGHT);                      //Y_Length

}


void CDynamicAttack::assignRegion_2() {
    //Top Opp Half
    guards[2][0].assign(-wm->field->_CENTER_CIRCLE_RAD,                //top_X
                        wm->field->_FIELD_HEIGHT / 2,                  //top_Y
                        wm->field->_FIELD_WIDTH / 2 + wm->field->_CENTER_CIRCLE_RAD, //X_Length
                        wm->field->_FIELD_HEIGHT / 2);                 //Y_Length

    //Bottom Opp Half
    guards[2][1].assign(-wm->field->_CENTER_CIRCLE_RAD,                 //top_X
                        0,                                   //top_Y
                        wm->field->_FIELD_WIDTH / 2 + wm->field->_CENTER_CIRCLE_RAD, //X_Length
                        wm->field->_FIELD_HEIGHT / 2);                  //Y_Length

}


void CDynamicAttack::assignRegion_3() {
    //Top Opp Tertium
    guards[3][0].assign(-wm->field->_CENTER_CIRCLE_RAD,                 //top_X
                        wm->field->_FIELD_HEIGHT / 2,                   //top_Y
                        wm->field->_FIELD_WIDTH / 2 + wm->field->_CENTER_CIRCLE_RAD, //X_Length
                        wm->field->_FIELD_HEIGHT / 3);                  //Y_Length

    //Middle Opp Tertium
    guards[3][1].assign(-wm->field->_CENTER_CIRCLE_RAD,                 //top_X
                        wm->field->_FIELD_HEIGHT / 2 - wm->field->_FIELD_HEIGHT / 3, //top_Y
                        wm->field->_FIELD_WIDTH / 2 + wm->field->_CENTER_CIRCLE_RAD, //X_Length
                        wm->field->_FIELD_HEIGHT / 3);                  //Y_Length

    //Bottom Opp Tertium
    guards[3][2].assign(-wm->field->_CENTER_CIRCLE_RAD,                 //top_X
                        wm->field->_FIELD_HEIGHT / 2 - 2 * wm->field->_FIELD_HEIGHT / 3, //top_Y
                        wm->field->_FIELD_WIDTH / 2 + wm->field->_CENTER_CIRCLE_RAD, //X_Length
                        wm->field->_FIELD_HEIGHT / 3);                  //Y_Length

}


void CDynamicAttack::assignRegion_4() {
    //Top Opp 1/4
    guards[4][0].assign(-wm->field->_CENTER_CIRCLE_RAD,                 //top_X
                        wm->field->_FIELD_HEIGHT / 2,                   //top_Y
                        wm->field->_FIELD_WIDTH / 2 + wm->field->_CENTER_CIRCLE_RAD, //X_Length
                        wm->field->_FIELD_HEIGHT / 4);                  //Y_Length

    //Mid-Top Opp 1/4
    guards[4][1].assign(-wm->field->_CENTER_CIRCLE_RAD,                 //top_X
                        wm->field->_FIELD_HEIGHT / 4,                   //top_Y
                        wm->field->_FIELD_WIDTH / 2 + wm->field->_CENTER_CIRCLE_RAD, //X_Length
                        wm->field->_FIELD_HEIGHT / 4);                  //Y_Length

    //Mid-Bottom Opp 1/4
    guards[4][2].assign(-wm->field->_CENTER_CIRCLE_RAD,                 //top_X
                        0,                                   //top_Y
                        wm->field->_FIELD_WIDTH / 2 + wm->field->_CENTER_CIRCLE_RAD, //X_Length
                        wm->field->_FIELD_HEIGHT / 4);                  //Y_Length                    //Y_Length
    //Bottom Opp 1/4
    guards[4][3].assign(-wm->field->_CENTER_CIRCLE_RAD,                 //top_X
                        -wm->field->_FIELD_HEIGHT / 4,                  //top_Y
                        wm->field->_FIELD_WIDTH / 2 + wm->field->_CENTER_CIRCLE_RAD, //X_Length
                        wm->field->_FIELD_HEIGHT / 4);                  //Y_Length

}


void CDynamicAttack::assignRegion_5() {
    for (int i = 0; i < 4; i++) {
        guards[5][i].assign(guards[4][i].topLeft(), guards[4][i].size());
    }

    //BackUp Line
    guards[5][4].assign(-wm->field->_CENTER_CIRCLE_RAD - 1,             //top_X
                        wm->field->_FIELD_HEIGHT / 2,                   //top_Y
                        1,                                   //X_Length
                        wm->field->_FIELD_HEIGHT);                      //Y_Length

}


void CDynamicAttack::assignRegion_6() {
    for (int i = 0; i < 5; i++) {
        guards[6][i].assign(guards[5][i].topLeft(), guards[5][i].size());
    }

    //BackUp Line
    guards[6][5].assign(-wm->field->_FIELD_WIDTH / 2 + wm->field->_FIELD_PENALTY_POINT,      //top_X
                        wm->field->_PENALTY_WIDTH / 2,                      //top_Y
                        wm->field->_FIELD_WIDTH / 2 - 1 - wm->field->_CENTER_CIRCLE_RAD, //X_Length
                        wm->field->_PENALTY_WIDTH);                         //Y_Length

}

void CDynamicAttack::showRegions(unsigned int agentSize,
                                 QColor color) {

    for (size_t i = 0; i < agentSize; i++) {
        drawer->draw(guards[agentSize][i], color);
    }
}

void CDynamicAttack::showLocations(unsigned int agentSize,
                                   QColor color) {
    for (size_t i = 0; i < agentSize; i++) { //RegionsCount
        for (size_t j = 0; j < 3; j++) {
            drawer->draw(guardLocations[agentSize][i][j], color);
        }
    }
}

void CDynamicAttack::assignLocations_0() {
    //    guardLocations[0][0][0].assign(0,0);
    //    guardLocations[0][0][1].assign(0,0);
    //    guardLocations[0][0][2].assign(0,0);
}


void CDynamicAttack::assignLocations_1() {
    //Opp Feild
    guardLocations[1][0][0].assign(wm->field->_FIELD_WIDTH / 2 - 1,  wm->field->_FIELD_HEIGHT / 4 + 0.5);
    guardLocations[1][0][1].assign(wm->field->_FIELD_WIDTH / 4, 0);
    guardLocations[1][0][2].assign(wm->field->_FIELD_WIDTH / 2 - 1, -wm->field->_FIELD_HEIGHT / 4 - 0.5);
}

void CDynamicAttack::assignLocations_2() {
    //Top Opp Half
    const int offset = 1;
    guardLocations[2][0][0].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 1 + offset, wm->field->_FIELD_HEIGHT / 4);
    guardLocations[2][0][1].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 2 + offset, wm->field->_FIELD_HEIGHT / 4);
    guardLocations[2][0][2].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 3 + offset, wm->field->_FIELD_HEIGHT / 4);

    //Bottom Opp Half
    guardLocations[2][1][0].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 1 + offset, -wm->field->_FIELD_HEIGHT / 4);
    guardLocations[2][1][1].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 2 + offset, -wm->field->_FIELD_HEIGHT / 4);
    guardLocations[2][1][2].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 3 + offset, -wm->field->_FIELD_HEIGHT / 4);

}

void CDynamicAttack::assignLocations_3() {
    //Top Opp Tertium
    const int offset = 1;
    guardLocations[3][0][0].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 1 + offset, wm->field->_FIELD_HEIGHT / 3);
    guardLocations[3][0][1].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 2 + offset, wm->field->_FIELD_HEIGHT / 3);
    guardLocations[3][0][2].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 3 + offset, wm->field->_FIELD_HEIGHT / 3);
    //Middle Opp Tertium
    guardLocations[3][1][0].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 1 + offset, 0);
    guardLocations[3][1][1].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 2 + offset, 0);
    guardLocations[3][1][2].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 3 + offset, 0);
    //Bottom Opp Tertium
    guardLocations[3][2][0].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 1 + offset, -wm->field->_FIELD_HEIGHT / 3);
    guardLocations[3][2][1].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 2 + offset, -wm->field->_FIELD_HEIGHT / 3);
    guardLocations[3][2][2].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 3 + offset, -wm->field->_FIELD_HEIGHT / 3);
}

void CDynamicAttack::assignLocations_4() {
    const int offset = 1;
    // Top Opp 1/4
    guardLocations[4][0][0].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 1 + offset, 3 * wm->field->_FIELD_HEIGHT / 8);
    guardLocations[4][0][1].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 2 + offset, 3 * wm->field->_FIELD_HEIGHT / 8);
    guardLocations[4][0][2].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 3 + offset, 3 * wm->field->_FIELD_HEIGHT / 8);
    // Mid-Top Opp 1/4

    guardLocations[4][1][0].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 1 + offset, wm->field->_FIELD_HEIGHT / 8);
    guardLocations[4][1][1].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 2 + offset, wm->field->_FIELD_HEIGHT / 8);
    guardLocations[4][1][2].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 3 + offset, wm->field->_FIELD_HEIGHT / 8);
    // Mid-Bottom Opp 1/4

    guardLocations[4][2][0].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 1 + offset, -wm->field->_FIELD_HEIGHT / 8);
    guardLocations[4][2][1].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 2 + offset, -wm->field->_FIELD_HEIGHT / 8);
    guardLocations[4][2][2].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 3 + offset, -wm->field->_FIELD_HEIGHT / 8);

    // Bottom Opp 1/4
    guardLocations[4][3][0].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 1 + offset, -3 * wm->field->_FIELD_HEIGHT / 8);
    guardLocations[4][3][1].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 2 + offset, -3 * wm->field->_FIELD_HEIGHT / 8);
    guardLocations[4][3][2].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 3 + offset, -3 * wm->field->_FIELD_HEIGHT / 8);
}

void CDynamicAttack::assignLocations_5() {
    const int offset = 1;
    // Top Opp 1/4
    guardLocations[5][0][0].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 1 + offset, 3 * wm->field->_FIELD_HEIGHT / 8);
    guardLocations[5][0][1].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 2 + offset, 3 * wm->field->_FIELD_HEIGHT / 8);
    guardLocations[5][0][2].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 3 + offset, 3 * wm->field->_FIELD_HEIGHT / 8);
    // Mid-Top Opp 1/4

    guardLocations[5][1][0].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 1 + offset, wm->field->_FIELD_HEIGHT / 8);
    guardLocations[5][1][1].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 2 + offset, wm->field->_FIELD_HEIGHT / 8);
    guardLocations[5][1][2].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 3 + offset, wm->field->_FIELD_HEIGHT / 8);
    // Mid-Bottom Opp 1/4

    guardLocations[5][2][0].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 1 + offset, -wm->field->_FIELD_HEIGHT / 8);
    guardLocations[5][2][1].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 2 + offset, -wm->field->_FIELD_HEIGHT / 8);
    guardLocations[5][2][2].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 3 + offset, -wm->field->_FIELD_HEIGHT / 8);

    // Bottom Opp 1/4
    guardLocations[5][3][0].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 1 + offset, -3 * wm->field->_FIELD_HEIGHT / 8);
    guardLocations[5][3][1].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 2 + offset, -3 * wm->field->_FIELD_HEIGHT / 8);
    guardLocations[5][3][2].assign((wm->field->_FIELD_WIDTH - offset) / 3 * 3 + offset, -3 * wm->field->_FIELD_HEIGHT / 8);

    // BackUp-Line
    guardLocations[5][4][0].assign(0, +wm->field->_FIELD_HEIGHT / 3);
    guardLocations[5][4][1].assign(0, 0);
    guardLocations[5][4][2].assign(0, -wm->field->_FIELD_HEIGHT / 3);

}

void CDynamicAttack::assignLocations_6() {
    // Top Opp 1/4
    guardLocations[6][0][0].assign(1  , 2.5);
    guardLocations[6][0][1].assign(2.5, 2.5);
    guardLocations[6][0][2].assign(3.5, 2.5);
    // Mid-Top Opp 1/4

    guardLocations[6][1][0].assign(1  , 1);
    guardLocations[6][1][1].assign(2.5, 1);
    guardLocations[6][1][2].assign(3.5, 1);
    // Mid-Bottom Opp 1/4

    guardLocations[6][2][0].assign(1  , -1);
    guardLocations[6][2][1].assign(2.5, -1);
    guardLocations[6][2][2].assign(3.5, -1);

    // Bottom Opp 1/4
    guardLocations[6][3][0].assign(1  , -2.5);
    guardLocations[6][3][1].assign(2.5, -2.5);
    guardLocations[6][3][2].assign(3.5, -2.5);

    // BackUp-Line
    guardLocations[6][4][0].assign(-1 ,  2);
    guardLocations[6][4][1].assign(-1 ,  0);
    guardLocations[6][4][2].assign(-1 , -2);

    // Goal Area
    guardLocations[6][5][0].assign(-3.5 ,  0);
    guardLocations[6][5][1].assign(-2.5 ,  0);
    guardLocations[6][5][2].assign(-1.5 ,  0);


}

QString CDynamicAttack::getString(const DynamicMode &_mode) const {
    switch (_mode) {
    default:
    case DynamicMode::NoMode:
        return QString("NoMode");
    case DynamicMode::DefenseClear:
        return QString("DefenseClear");
    case DynamicMode::NotWeHaveBall:
        return QString("NotWeHaveBall");
    case DynamicMode::HighProb:
        return QString("HighProb");
    case DynamicMode::Fast:
        return QString("Fast");
    case DynamicMode::Critical:
        return QString("Critical");
    case DynamicMode::Plan:
        return QString("NewPlan");
    case DynamicMode::BallInOurField:
        return QString("Ball In Our Field");
    case DynamicMode::NoPositionAgent:
        return QString("No Agent");
    }
}

void CDynamicAttack::ballLocation() {
    if (ballPos.x < 0) {
        isBallInOurField = true;
    } else if (ballPos.x > 0) {
        isBallInOurField = false;
    }
}

void CDynamicAttack::setDefenseClear(bool _isDefenseClearing) {
    isDefenseClearing = _isDefenseClearing;
}

void CDynamicAttack::setDirectShot(bool _directShot) {
    directShot = _directShot;
}

void CDynamicAttack::setPositions(QList<int> _positioningRegion) {
    regionsList.clear();
    dynamicPosition.clear();
    for (int i : _positioningRegion) {
        regionsList.append(i);
//        dynamicPosition.append(knowledge->getStaticPoses
//                               (_positioningRegion.at(i))); // TODO : Static Pos
    }
}

void CDynamicAttack::setPlayMake(Agent* _playMake) {
    playmakeID = _playMake->id();
    playmake = _playMake;
}

void CDynamicAttack::setWeHaveBall(bool _ballPoss) {
    isWeHaveBall = _ballPoss;
}

void CDynamicAttack::setNoPlanException(bool _noPlanException) {
    noPlanException = _noPlanException;
}

void CDynamicAttack::setCritical(bool _critical) {
    critical = _critical;
}

void CDynamicAttack::setBallInOppJaw(bool _ballInOppJaw) {
    ballInOppJaw = _ballInOppJaw;
}

void CDynamicAttack::setFast(bool _fast) {
    fast = _fast;
}

