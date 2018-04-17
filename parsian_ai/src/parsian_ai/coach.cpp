////
//// Created by parsian-ai on 9/22/17.
////

#include <parsian_ai/coach.h>
#include <parsian_util/geom/vector_2d.h>

CCoach::CCoach(Agent**_agents)
{
    clearBallVels();
    averageVel = 0;
    goalieTrappedUnderGoalNet = false;
    inited = false;
    agents = _agents;
    lastAssignCycle = -10;
    first = true;

    ///////////////////////////////////
    goalieTimer.start();
    ////////////////////intentions
    playOnExecTime.start();
    intentionTimePossession.start();
    playMakeIntention.start();

    playOnTime = 2000;


    // Old Plays
    ourPenalty          = new COurPenalty;
    ourPenaltyShootout  = new COurPenaltyShootout;
    theirDirect         = new CTheirDirect;
    theirKickOff        = new CTheirKickOff;
    theirPenalty        = new CTheirPenalty;
    theirIndirect       = new CTheirIndirect;
    ourBallPlacement    = new COurBallPlacement;
    halftimeLineup      = new CHalftimeLineup;
    theirBallPlacement  = new CTheirBallPlacement;


    // New Plays
    ourPlayOff          = new CPlayOff;
    dynamicAttack       = new CDynamicAttack();

    //Stop
    stopPlay            = new CStopPlay();

    for (auto &stopRole : stopRoles) {
        stopRole = new CRoleStop(nullptr);
    }
    //fault
    for (auto &faultRole : faultRoles) {
        faultRole = new CRoleFault(nullptr);
    }
    lastDefenseAgents.clear();

    defenseTimeForVisionProblem[0].start();
    defenseTimeForVisionProblem[1].start();
    know->variables["transientFlag"].setValue(false);
    trasientTimeOut.start();
    translationTimeOutTime = 4000;

    //    m_planLoader = new CLoadPlayOffJson(QDir::currentPath() + QString("/playoff"));
    goalieAgent = nullptr;
    firstPlay = true;
    firstIsFinished = false;
    preferedDefenseCounts = 2;
    overDefThr = 0;
    selectedPlay = stopPlay;
    for (int &i : faultDetectionCounter) {
        i = 0;
    }
    firstTime = true;

    haltAction = new NoAction;

    gotplan = true;
}

CCoach::~CCoach() {
    delete haltAction;

    delete ourPenalty        ;
    delete theirDirect       ;
    delete theirKickOff      ;
    delete theirPenalty      ;
    delete theirIndirect     ;
    delete ourBallPlacement  ;
    delete theirBallPlacement;
    delete ourPlayOff        ;
    delete dynamicAttack     ;
    delete stopPlay          ;
    //    savePostAssignment();
    //    saveLFUReapeatData(LFUList);
}

//void CCoach::saveGoalie()
//{
//     DBUG((QString("goalie under net timer : %1").arg(goalieTimer.elapsed())), D_MHMMD);
//
//    if(goalieTimer.elapsed() > 100 )
//    {
//        if(goalieTimer.elapsed() <200)
//        {
//            knowledge->getAgent(preferedGoalieID)->setRobotAbsVel(2,0,0);
//        }
//        else if(goalieTimer.elapsed() <300)
//        {
//            knowledge->getAgent(preferedGoalieID)->setRobotAbsVel(-2,0,0);
//
//        }
//        else if(goalieTimer.elapsed() < 400)
//        {
//            knowledge->getAgent(preferedGoalieID)->setRobotAbsVel(0,2,0);
//
//        }
//        else if(goalieTimer.elapsed() < 500)
//        {
//            knowledge->getAgent(preferedGoalieID)->setRobotAbsVel(0,-2,0);
//
//        }
//        else
//        {
//            knowledge->getAgent(preferedGoalieID)->setRobotAbsVel(1,1,5);
//
//        }
//    }
//    return;
//    if (goalieTrappedUnderGoalNet && knowledge->goalie != NULL)
//    {
//        knowledge->goalie->setRobotAbsVel(2, 2, 4);
//    }
//}

bool isNan(double x) {
    return (QString("%1").arg(x).trimmed().toLower() == "nan");
}

void CCoach::checkGoalieInsight() {
    goalieTrappedUnderGoalNet = false;
    /////////////////////////////////////////////////// new method by Don Mhmmd
    if (goalieAgent != nullptr) {
        //        if (goalieAgent->isVisible())
        //        {
        //            goalieTimer.restart();
        //        }
        DBUG("inja miad", D_MHMMD);
    }
}



void CCoach::decidePreferredDefenseAgentsCount() {

    missMatchIds.clear();
    if (gameState->getState() == States::Stop || gameState->getState() == States::Halt || first) {
        if (workingIDs.size() != 0u) {
            robotsIdHist.clear();
            for (int i = 0 ; i < workingIDs.size() ; i++) {
                robotsIdHist.append(workingIDs[i]);
            }
        }
        first = false;
    }

    if (workingIDs.size() > _NUM_PLAYERS) {
        missMatchIds.clear();
        for (int i = 0 ; i < workingIDs.size() ; i++) {
            for (int k = 0 ; k < robotsIdHist.count() ; k++) {
                if (robotsIdHist.at(k) == workingIDs[i]) {
                    break;
                }
                if (k == robotsIdHist.count() - 1) {
                    missMatchIds.append(workingIDs[i]);
                }
            }
        }
    }

    int agentsCount = workingIDs.size() - missMatchIds.count();
    if (goalieAgent != nullptr) {
        if (goalieAgent->isVisible()) {
            agentsCount--;
        }
    }

    if (gameState->isStop()) {
        if (wm->ball->pos.x < 1) {
            preferedDefenseCounts = agentsCount - 1;

        } else if (wm->ball->pos.x > 1.2) {
            preferedDefenseCounts = conf.Defense;
        }
    } else if (gameState->isStart()) {
        if (know->variables["transientFlag"].toBool())
        {
            //// Add Playmake after time
            if (trasientTimeOut.elapsed() > 800 && !wm->field->ourBigPenaltyArea(1,0.1,0).contains(wm->ball->pos)) {
                preferedDefenseCounts = std::max(0, agentsCount - missMatchIds.count() - 1);

            } else {
                preferedDefenseCounts = agentsCount - missMatchIds.count();

            }
        } else { //// PLAYON

            if (agentsCount == 1) {
                preferedDefenseCounts = 0; //// just one playmake

            } else if (agentsCount == 2) {
                preferedDefenseCounts = 1; //// one playmake and one defense
            } else {
                preferedDefenseCounts = std::min(selectedPlay->defensePlan.findNeededDefense(), agentsCount - 1);
//                preferedDefenseCounts += agentsCount - 1 - preferedDefenseCounts - selectedPlay->markPlan.findNeededMark();
            }
        }
    } else if (gameState->ourPlayOffKick()) {
        if (wm->ball->pos.x < 1) {
            preferedDefenseCounts = (selectedPlay->defensePlan.findNeededDefense() == 1) ? 1 : 2;

        } else if (wm->ball->pos.x > 1.5) {
            preferedDefenseCounts = 0;
        }

    } else if (gameState->theirPlayOffKick()) {
        if (gameState->theirKickoff()) {
            preferedDefenseCounts = 2;
        } else {
            preferedDefenseCounts = std::max(agentsCount - missMatchIds.count() - 1, 0);
        }
    } else {
        DBUG("UNKNOWN STATE", D_ERROR);
    }

    if (gameState->halfTime() || gameState->timeOut()) {
        preferedGoalieID = -1;
        preferedDefenseCounts = 0;
    }

    if (gameState->penaltyShootout() || gameState->penaltyKick()) {
        preferedDefenseCounts = 0;
    }
    if(conf.StrictFormation){
        if (conf.Defense > 3){
            preferedDefenseCounts = 3;
        } else {
            preferedDefenseCounts = conf.Defense;
        }
    }
    lastPreferredDefenseCounts = preferedDefenseCounts;
}

void CCoach::calcDesiredMarkCounts(){

    int agentsCount = workingIDs.size();
    if (goalieAgent != nullptr) {
        if (goalieAgent->isVisible()) {
            agentsCount--;
        }
    }

    // used in playoff without counting goalie

    if (agentsCount == 2) {
        desiredDefCount = 0;
    } else if (agentsCount == 3) {

        if (toBeMopps.count() == 0) {
            desiredDefCount = 1;
        } else if (toBeMopps.count() <= 2) {
            desiredDefCount = 2 - toBeMopps.count();
        } else {
            desiredDefCount = 0;
        }


    } else if (agentsCount == 4) {

        if (toBeMopps.count() == 0) {
            desiredDefCount = 2;
        } else if (toBeMopps.count() <= 2) {
            desiredDefCount = 2 - toBeMopps.count();
        } else {
            desiredDefCount = 0;
        }

    } else if (agentsCount == 5) {

        if (toBeMopps.count() == 0) {
            desiredDefCount = 2;
        } else if (toBeMopps.count() <= 3) {
            desiredDefCount = 3 - toBeMopps.count();
        } else {
            desiredDefCount = 0;
        }

    }

    else if (agentsCount == 6) {
        if (toBeMopps.count() <= 1) {
            desiredDefCount = 1;
        } else if (toBeMopps.count() <= 3) {
            desiredDefCount = 3 - toBeMopps.count();
        } else {
            desiredDefCount = 0;
        }
    }

}


void CCoach::assignGoalieAgent(int goalieID) {
    goalieAgent = nullptr;
    if (workingIDs.contains(goalieID)) {
        goalieAgent = agents[goalieID];
    }
}

BallPossesion CCoach::isBallOurs() {
    BallPossesion decidePState;

    double temp = wm->ball->pos.x + wm->ball->vel.x * 1;

    if (temp > 0.5) {
        decidePState = BallPossesion::WEHAVETHEBALL;
    } else if (temp < 0.1) {
        decidePState = BallPossesion::WEDONTHAVETHEBALL;
    } else {
        decidePState = lastBallPossesionState;
    }

    if (wm->field->isInOurPenaltyArea(wm->ball->pos)
        &&  wm->ball->vel.length() < 0.1) {
        decidePState = BallPossesion::SOSOTHEIR;
    }

    if (wm->field->isInOppPenaltyArea(wm->ball->pos)
        && wm->ball->vel.length() < 0.1) {
        decidePState = BallPossesion::SOSOOUR;
    }

    lastBallPossesionState = decidePState;

    return decidePState;
}

double CCoach::timeNeeded(Agent *_agentT,const Vector2D& posT, double vMax) {

    double acc;
    double dec = 3.5;
    Vector2D tAgentVel = _agentT->vel();
    Vector2D tAgentDir = _agentT->dir();
    double dist = 0;
    QList <Vector2D> _result;
    Vector2D _target;
    double tAgentVelTanjent =  tAgentVel.length() * cos(Vector2D::angleBetween(posT - _agentT->pos() , _agentT->vel().norm()).radian());

    double vXvirtual = (posT - _agentT->pos()).x;
    double vYvirtual = (posT - _agentT->pos()).y;
    double veltanV = (vXvirtual) * cos(tAgentDir.th().radian()) + (vYvirtual) * sin(tAgentDir.th().radian());
    double velnormV = -1 * (vXvirtual) * sin(tAgentDir.th().radian()) + (vYvirtual) * cos(tAgentDir.th().radian());
    double accCoef;

    accCoef = atan(std::fabs(veltanV) / std::fabs(velnormV)) / _PI * 2;
    acc = accCoef * 4.5 + (1 - accCoef) * 3.5;
    double tDec = vMax / dec;
    double tAcc = (vMax - tAgentVelTanjent) / acc;
    dist = posT.dist(_agentT->pos());
    double dB = tDec * vMax / 2 + tAcc * (vMax + tAgentVelTanjent) / 2;

    if (dist > dB) {
        return tAcc + tDec + (dist - dB) / vMax;
    } else {
        return ((1 / dec) + (1 / acc)) * sqrt(dist * (2 * dec * acc / (acc + dec)) + (tAgentVelTanjent * tAgentVelTanjent / (2 * acc))) - (tAgentVelTanjent) / acc;
    }

}

double CCoach::kickTimeEstimation(Agent * _agent, const Vector2D& target) {
    Vector2D agentPos = _agent->pos();
    Vector2D agentDir = _agent->dir();
    Vector2D ballPos = wm->ball->pos;
    Vector2D s1,s2;
    Vector2D finalPos;
    double agentTime = 0;
    Segment2D ballPath(ballPos,ballPos + wm->ball->vel.norm()*20);

    if(wm->ball->vel.length() < 0.1)
    {
        return timeNeeded(_agent, ballPos + (ballPos - target).norm()*0.1, 4.5);
    }

    if (Circle2D(agentPos, 0.1).intersection(Segment2D(ballPos, wm->ball->getPosInFuture(0.5)), &s1, &s2)) {
        finalPos = ballPath.nearestPoint(agentPos);
        return timeNeeded(_agent, finalPos, 4.5);
    } else {
        for (double i = 0.5; i < 5; i += 0.1) {
            finalPos = wm->ball->getPosInFuture(i);
            agentTime = timeNeeded(_agent, finalPos - (finalPos-target).norm()*0.1, 4.5);
            if (agentTime < (i - (0.5))) {
                return i;
            }
        }
        return 100;
    }

}



void CCoach::assignDefenseAgents(int defenseCount) {


    if (selectedPlay != nullptr && selectedPlay->lockAgents) {
        defenseAgents.clear();
        defenseAgents.append(lastDefenseAgents);
        return;
    }

    QList<int> ids = workingIDs;
    if (goalieAgent != nullptr) {
        ids.removeOne(goalieAgent->id());
    }
    if (playmakeId != -1) {
        ids.removeOne(playmakeId);
    }

    selectedPlay->defensePlan.fillDefencePositionsTo(defenseTargets);
    double nearestDist;
    int nearestRobot = -1;

    defenseAgents.clear();
    for (int i = 0 ; i < defenseCount ; i++) {
        nearestDist = 1000000;
        for (int j = 0 ; j < ids.count() ; j++) {
            if (!agents[ids[j]]->changeIsNeeded) {
                wm->our[ids[j]]->pos;
                if (wm->our[ids[j]]->pos.dist(defenseTargets[i]) < nearestDist) {
                    nearestDist = wm->our[ids[j]]->pos.dist(defenseTargets[i]);
                    nearestRobot =  ids[j];
                }
            }
        }
        if (nearestRobot >= 0) {
            defenseAgents.append(agents[nearestRobot]);
            ids.removeOne(nearestRobot);
        }

    }

    lastDefenseAgents.clear();
    lastDefenseAgents.append(defenseAgents);

}

bool CCoach::isBallcollide(int framCount, double diffDir) {

    if(wm->ball->vel.length() <.05 && lastBallVels.size() == 0) return false;
    lastBallVels.append(std::move(wm->ball->vel));
    averageVel += wm->ball->vel.length();
    if(lastBallVels.size()>2){
        double innerproduct = wm->ball->vel.norm().innerProduct(lastBallVels.first().norm());
        ROS_INFO_STREAM("KALI inner : "<<innerproduct<<"  vel "<<wm->ball->vel.length());

        if((wm->ball->vel.length() <.1 && averageVel/lastBallVels.size() > .1)||
           (innerproduct < .1 && innerproduct >-.1)){
            ROS_INFO("khord ro zamin");
            getDefense().ballBouncePos = wm->ball->pos;
            getDefense().ballIsBounced = true;
            PDEBUGV2D("ball bounce pos",wm->ball->pos,D_ALI);
            removeLastBallVel();
            return false;
        }

        if(innerproduct > cos(diffDir*0.6*M_PI/180)){
            removeLastBallVel();
            ROS_INFO_STREAM("KALI : saff mire");
            return false;
        }

        if(innerproduct < cos(diffDir*M_PI/180)){
            ROS_INFO_STREAM("KALI : taghir jahat");
            clearBallVels();
            return true;
        }
        if(wm->ball->vel.length() < .01){
            clearBallVels();
            ROS_INFO_STREAM("KALI : istad");
            return true;
        }
    }

    removeLastBallVel(framCount);
    ROS_INFO_STREAM("KALI : hichi nashod");
    return false;
}
void CCoach::removeLastBallVel(int frameCount){
    averageVel -=  lastBallVels.first().length();
    if(lastBallVels.size() > frameCount)
        lastBallVels.removeFirst();

}

void CCoach::clearBallVels(){
    lastBallVels.clear();
    lastBallVels.reserve(6);
}

void CCoach::virtualTheirPlayOffState() {
    States currentState;
    currentState = gameState->getState();
    if (lastState == States::TheirDirectKick || lastState == States::TheirIndirectKick /*|| lastState == States::TheirKickOff*/) {
        if (currentState == States::Start) {
            know->variables["transientFlag"].setValue(true);
            getDefense().ballIsBounced = false;
            getDefense().playOffStartBallPos = wm->ball->pos;
            getDefense().playOffPassDir = wm->opp[know->nearestOppToBall()]->dir;
        }
    }

    if (! know->variables["transientFlag"].toBool()) {
        trasientTimeOut.restart();
    }

    if (trasientTimeOut.elapsed() >= translationTimeOutTime) {
        know->variables["transientFlag"].setValue(false);
    }

    if (wm->ball->pos.x >= 1) {
        know->variables["transientFlag"].setValue(false);
    }
    if(know->variables["transientFlag"].toBool())
        if (isBallcollide()) {
            know->variables["transientFlag"].setValue(false);
        }

    PDEBUG("TS flag:", know->variables["transientFlag"].toBool(), D_AHZ);
    lastState  = currentState;

}

void CCoach::decideDefense(){
    assignDefenseAgents(preferedDefenseCounts);
    ROS_INFO_STREAM("SD: " << preferedDefenseCounts << " : " << defenseAgents.size());
    if (gameState->theirPenaltyKick()) {
        defenseAgents.clear();
        selectedPlay->defensePlan.initGoalKeeper(goalieAgent);
        selectedPlay->defensePlan.initDefense(defenseAgents);
        selectedPlay->defensePlan.execute();
    } else {
        selectedPlay->defensePlan.initGoalKeeper(goalieAgent);
        selectedPlay->defensePlan.initDefense(defenseAgents);
        selectedPlay->defensePlan.execute();
        selectedPlay->defensePlan.debugAgents("Defense");
    }
}


double CCoach::findMostPossible(Vector2D agentPos) {
    QList<int> tempObstacles;
    QList <Circle2D> obstacles;
    obstacles.clear();
    for (int i = 0 ; i < wm->opp.activeAgentsCount() ; i++) {
        obstacles.append(Circle2D(wm->opp.active(i)->pos, 0.1));
    }

    for (int i = 0 ; i < workingIDs.size() ; i++) {
        if (workingIDs[i] != playmakeId) {
            obstacles.append(Circle2D(wm->our.active(i)->pos, 0.1));
        }
    }
    double prob, angle, biggestAngle;

    CKnowledge::getEmptyAngle(*wm->field, agentPos - (wm->field->oppGoal() - agentPos).norm() * 0.15, wm->field->oppGoalL(),
                              wm->field->oppGoalR(), obstacles, prob, angle, biggestAngle);


    return prob;
}

void CCoach::updateAttackState() {
    ourAttackState = SAFE;
    return;
}

void CCoach::choosePlaymakeAndSupporter(bool defenseFirst){

    if (!gameState->isStart()) {
        playmakeId = -1;
        playMakeIntention.restart();
        return;
    }
    QList<int> ourPlayers = wm->our.data->activeAgents;
    if(ourPlayers.contains(preferedGoalieID)) {
        ourPlayers.removeOne(preferedGoalieID);
    }
    if (defenseFirst) {
        for (auto& d : defenseAgents) {
            if (ourPlayers.contains(d->id())) ourPlayers.removeOne(d->id());
        }
    }

    if (ourPlayers.empty()) {
        playmakeId = -1;
        lastPlayMake = -1;
        supporterId = -1;
        lastSupporterId = -1;
        return;
    }

    ROS_INFO_STREAM("MAHIs INTENTION: " << playMakeIntention.elapsed());
    ////////////////////first we choose our playmake
    double ballVel = wm->ball->vel.length();
    Vector2D ballPos = wm->ball->pos;
    ROS_INFO("!0");

    if (ballVel < 0.4) {
        double maxD = -10000000.1;
        for (const auto& player : ourPlayers) {
            double o = -1 * agents[player]->pos().dist(ballPos) ;
            if (player == lastPlayMake) {
                o += conf.playMakeStopThr;
            }
            if (o > maxD) {
                maxD = o;
                playmakeId = player;
            }
        }

    } else {

        double maxD = -10000000.1;
        for (const auto& player : ourPlayers) {
            double o = -1 * kickTimeEstimation(agents[player], wm->field->oppGoal());
            if (player == lastPlayMake) {
                o += conf.playMakeMoveThr;
            }
            if (o > maxD) {
                maxD = o;
                playmakeId = player;
            }

        }

    }

    if (lastPlayMake != playmakeId || isBallcollide(3, 30)) {
        playMakeIntention.restart();

    } else {
        if (playMakeIntention.elapsed() < conf.playMakeIntention) {
            playmakeId = lastPlayMake;
            return;
        }
    }


    lastPlayMake = playmakeId;
}

void CCoach::decideAttack() {
    // find unused agents!

    QList<int> ourPlayersID = workingIDs;
    if (goalieAgent != nullptr) {
        ourPlayersID.removeOne(goalieAgent->id());
    }
    for (auto defenseAgent : defenseAgents) {
        if (ourPlayersID.contains(defenseAgent->id())) {
            ourPlayersID.removeOne(defenseAgent->id());
        }
    }

    switch (gameState->getState()) { // GAMESTATE

        case States::Halt:
            decideHalt(ourPlayersID);
            return;
            break;
        case States::Stop:
            ourBallPlacement->first = true;
            decideStop(ourPlayersID);
            return;
            break;

        case States::OurKickOff:
            decideOurKickOff(ourPlayersID);
            break;

        case States::TheirKickOff:
            decideTheirKickOff(ourPlayersID);
            break;

        case States::OurDirectKick:
            decideOurDirect(ourPlayersID);
            break;

        case States::TheirDirectKick:
            decideTheirDirect(ourPlayersID);
            break;

        case States::OurIndirectKick:
            decideOurIndirect(ourPlayersID);
            break;

        case States::TheirIndirectKick:
            decideTheirIndirect(ourPlayersID);
            break;

        case States::OurPenaltyKick:
            decideOurPenalty(ourPlayersID);
            break;

        case States::OurPenaltyShootOut:
            decideOurPenaltyshootout(ourPlayersID);
            break;

        case States::TheirPenaltyShootOut:
            decideTheirPenaltyshootout(ourPlayersID);
            break;

        case States::TheirPenaltyKick:
            decideTheirPenalty(ourPlayersID);
            break;
        case States::Start:
            decideStart(ourPlayersID);
            break;
        case States::OurBallPlacement:
            decideOurBallPlacement(ourPlayersID);
            break;
        case States::TheirBallPlacement:
            decideStop(ourPlayersID);
            break;
        case States::HalfTime:
            decideHalfTimeLineUp(ourPlayersID);
            break;
        default:
            decideNull(ourPlayersID);
            return;
            break;
    }

    QList<Agent*> ourAgents;
    for (auto& ourPlayer : ourPlayersID) {
        ourAgents.append(agents[ourPlayer]);
    }

    selectedPlay->init(ourAgents);
    selectedPlay->execute();

    lastPlayers.clear();
    lastPlayers.append(ourPlayersID);
}

void CCoach::decidePlayOff(QList<int>& _ourPlayers, POMODE _mode) {

    //Decide Plan
    ROS_INFO_STREAM("playoff: " << firstTime);
    firstIsFinished = ourPlayOff->isFirstFinished();

    if (firstTime) {
        NGameOff::EMode tempMode;
        selectPlayOffMode(_ourPlayers.size(), tempMode);
        initPlayOffMode(tempMode, _mode, _ourPlayers);
        if(!gotplan){
            return;
        }
        ourPlayOff->setMasterMode(tempMode);
        if (tempMode == NGameOff::FirstPlay) {
            if (firstPlay && !firstIsFinished) {
                firstTime = true;

            } else if (firstPlay && firstIsFinished) {
                firstTime = true;
                firstPlay = false;

            } else if (!firstPlay && firstIsFinished) {
                firstTime = false;
                firstPlay = true;
                firstIsFinished = false;
                preferedShotSpot = ourPlayOff->getShotSpot();
                ourPlayOff->resetFirstPlayFinishedFlag();
            }

        } else {
            firstTime = false;

        }

    } else {

        setPlayOff(ourPlayOff->getMasterMode());
    }
}
void CCoach::decidePlayOn(QList<int>& ourPlayers, QList<int>& lastPlayers) {
    ballPState = isBallOurs();
    updateAttackState(); //// Too Bad Conditions will be Handle here

    if (0 <= playmakeId && playmakeId <= 11) {
        dynamicAttack->setPlayMake(agents[playmakeId]);
        ourPlayers.removeOne(playmakeId);
    }

    dynamicAttack->setDefenseClear(false); // TODO : fix

    if (playmakeId != -1 && wm->our[playmakeId] != nullptr) {
        double mostPossible = findMostPossible(wm->our[playmakeId]->pos);

        Rect2D pushingPenalty;
        pushingPenalty.setLength(wm->field->oppPenaltyRect().size().length() + conf.penaltyMargin*2);
        pushingPenalty.setWidth(wm->field->oppPenaltyRect().size().width() + conf.penaltyMargin);
        pushingPenalty.setTopLeft(wm->field->oppPenaltyRect().topLeft() + Vector2D(-conf.penaltyMargin, conf.penaltyMargin));

        if (pushingPenalty.contains(wm->ball->pos)) {
            dynamicAttack->setDirectShot(true);
        } else if (mostPossible > (conf.DirectTrsh - shotToGoalthr)) { // TODO : Fix This
            dynamicAttack->setDirectShot(true);
            shotToGoalthr = std::max(0.0, conf.DirectTrsh - 0.4);
        } else {
            dynamicAttack->setDirectShot(false);
            shotToGoalthr = 0;
        }
    }

    dynamicAttack->setWeHaveBall(ballPState   == BallPossesion::WEHAVETHEBALL);
    dynamicAttack->setFast(ourAttackState     == FAST);
    dynamicAttack->setCritical(ourAttackState == CRITICAL);

    //////////////////////////////////////////////assign agents
    int MarkNum = 0;
    switch (ballPState) {
        case BallPossesion::WEHAVETHEBALL:
            MarkNum = 0; // TODO : Check ERFORCE Game
            break;
        case BallPossesion::WEDONTHAVETHEBALL:
            MarkNum = std::max(selectedPlay->markPlan.findNeededMark(), ourPlayers.count() - 1);
            break;
        case BallPossesion::SOSOOUR:
            MarkNum = std::max(selectedPlay->markPlan.findNeededMark(), ourPlayers.count() - 2);
            break;
        case BallPossesion::SOSOTHEIR:
            MarkNum = std::max(selectedPlay->markPlan.findNeededMark(), ourPlayers.count() - 1);
            break;
    }
    MarkNum = std::min(MarkNum, ourPlayers.count());

    selectedPlay->markAgents.clear();
    if(wm->ball->pos.x >= 0
       && selectedPlay->lockAgents
       && lastPlayers.count() == ourPlayers.count()) {
        ourPlayers.clear();
        ourPlayers = lastPlayers;

    } else {        
        // TODO : matching is based on ID, It should be Goal-Oriented -- optimal -- base of position
        qSort(ourPlayers.begin(), ourPlayers.end());
        for (int i = 0; i < MarkNum; i++) {
            PDEBUG("marknum =" , MarkNum , D_AHZ);
            selectedPlay->markAgents.append(agents[ourPlayers.front()]);
            ourPlayers.removeFirst();
        }
        PDEBUG("mark agents =" , selectedPlay->markAgents.size() , D_AHZ);
    }

    lastBallPossesionState = ballPState;
}

void CCoach::selectPlayOffMode(int agentSize, NGameOff::EMode &_mode) {
    ROS_INFO_STREAM("HSHM: agentSize: " << agentSize);
    if (agentSize < 2) {
        _mode = NGameOff::DynamicPlay;

    } else if (isFastPlay() && false) { // TODO : fastPlay should be completed!
        _mode = NGameOff::FastPlay;

    } else if (gameState->ourKickoff() && !gameState->canKickBall()) {
        _mode = NGameOff::FirstPlay;

    } else if ((wm->ball->pos.x < 1 && !gameState->ourKickoff())|| !gotplan) {
        _mode = NGameOff::DynamicPlay;

    } else if (!firstIsFinished && conf.UseFirstPlay) {
        _mode = NGameOff::FirstPlay;

    } else if (wm->ball->pos.x > -1) {
        _mode = NGameOff::StaticPlay;

    } else {
        _mode = NGameOff::DynamicPlay;

    }
}

void CCoach::initPlayOffMode(const NGameOff::EMode _mode,
                             const POMODE _gameMode,
                             const QList<int>& _ourplayers) {
    switch (_mode) {


        case NGameOff::StaticPlay:
            initStaticPlay(_gameMode, _ourplayers);
            ROS_INFO("initPlayOffMode: StaticPlay");
            break;
        case NGameOff::DynamicPlay:
            ROS_INFO("initPlayOffMode: DynamicPlay");
            initDynamicPlay(_ourplayers);
            break;
        case NGameOff::FastPlay:
            ROS_INFO("initPlayOffMode: initFastPlay");
            initFastPlay(_ourplayers);
            break;
        case NGameOff::FirstPlay:
            ROS_INFO("initPlayOffMode: initFirstPlay");
            initFirstPlay(_ourplayers);
            break;
        default:
            ROS_INFO("initPlayOffMode: initStaticPlay");
            initStaticPlay(_gameMode, _ourplayers);
    }
}

void CCoach::setPlayOff(NGameOff::EMode _mode) {
    switch (_mode) {
        case NGameOff::StaticPlay:
            setStaticPlay();
            break;
        case NGameOff::DynamicPlay:
            setDynamicPlay();
            break;
        case NGameOff::FastPlay:
            setFastPlay();
            break;
        case NGameOff::FirstPlay:
            setFirstPlay();
            break;
        default:
            setStaticPlay();
    }
}

void CCoach::initDynamicPlay(const QList<int> &_ourplayers) {

    for (int i = 0; i < _NUM_PLAYERS; i++) {
        if (i >= _ourplayers.size()) {
            ourPlayOff->dynamicMatch[i] = -1;
        } else {
            ourPlayOff->dynamicMatch[i] = _ourplayers.at(i);
        }
    }
    if (_ourplayers.size() < 2) {
        ourPlayOff->dynamicSelect = CHIP;
    } else if (!gotplan){
        gotplan = true;
        ourPlayOff->dynamicSelect = KICK;
    } else {
        ourPlayOff->dynamicSelect = KHAFAN;
    }


    double dis = 1000000;
    int id = 0;
    int swapID = 0;
    for (int i = 0; i < _ourplayers.size(); i++) {
        double tempDis = agents[_ourplayers.at(i)]->pos().dist(wm->ball->pos) ;
        if (tempDis < dis) {
            dis = tempDis;
            id = _ourplayers.at(i);
            swapID = i;
        }
    }

    int tempID = ourPlayOff->dynamicMatch[0];
    ourPlayOff->dynamicMatch[0] = id;
    ourPlayOff->dynamicMatch[swapID] = tempID;


    ourPlayOff->setInitial(true);
    ourPlayOff->lockAgents = true;

}

void CCoach::initFastPlay(const QList<int> &_ourplayers) {
    // TODO : Initial Fast Play
}

void CCoach::initFirstPlay(const QList<int> &_ourplayers) {

    if(gameState->ourKickoff()){
        ourPlayOff->kickoffPositioning(_ourplayers.size());
    }

    double minDist = wm->field->_MAX_DIST;
    int minID = -1;
    int minOwner = 0;
    for (int i = 0; i < _ourplayers.size(); i++) {

        int tempID = _ourplayers.at(i);
        double tempDist = agents[tempID]->pos().dist(wm->ball->pos);
        if (tempDist < minDist) {
            minDist = tempDist;
            minID = tempID;
            minOwner = i;
        }

        if (i >= _ourplayers.size()) {
            ourPlayOff->dynamicMatch[i] = -1;
        } else {
            ourPlayOff->dynamicMatch[i] = _ourplayers.at(i);
        }
    }
    // fix passer :)
    int tempID = ourPlayOff -> dynamicMatch[0];
    ourPlayOff -> dynamicMatch[0] = minID;
    ourPlayOff -> dynamicMatch[minOwner] = tempID;

    ourPlayOff -> setInitial(true);
    ourPlayOff -> lockAgents = true;

    // TODO : Initial First Play
}

void CCoach::setStaticPlay() {
    // TODO : Complete staticPlay checker
    ourPlayOff->setInitial(false);
}

void CCoach::setDynamicPlay() {
    // TODO : Write Dynamic Play checker
    ourPlayOff->setInitial(false);
}

void CCoach::setFirstPlay() {
    // TODO : Write First Play checker
    ourPlayOff->setInitial(false);
    firstIsFinished = false;
}


void CCoach::checkTransitionToForceStart() {
    Vector2D lastPos;
    ballHist.append(wm->ball->pos);
    if (ballHist.count() > 100) {
        ballHist.removeAt(0);
    }
    if (ballHist.size() > 10) {
        lastPos = ballHist.at(ballHist.size() - 10);
    } else {
        if (!ballHist.isEmpty()) {
            lastPos = ballHist.first();
        } else {
            lastPos = wm->ball->pos;
        }
    }

    double ballChangedPosDist = wm->ball->pos.dist(lastPos);

    if (!gameState->isStart()) {
        if (cyclesWaitAfterballMoved == 0 && ballChangedPosDist > 0.05) {
            cyclesWaitAfterballMoved = 1;
        } else if (cyclesWaitAfterballMoved != 0) {
            cyclesWaitAfterballMoved++;
        }
    }
    ///////////////////////////////////// by DON
    if (gameState->ourPlayOffKick()) {
        //transition to game on
        ROS_INFO_STREAM("MAHIS: " << cyclesWaitAfterballMoved << " + " << selectedPlay->playOnFlag);
        if (cyclesWaitAfterballMoved > 6 && selectedPlay->playOnFlag) {
            gameState->setState(States::Start);
        }
    }

    if (gameState->theirPlayOffKick()) {
        //transition to game on
        if (cyclesWaitAfterballMoved > 0) {
            gameState->setState(States::Start);
        }
    }
}

void CCoach::generateWorkingRobotIds()
{
    workingIDs.clear();
    workingIDs = wm->our.data->activeAgents;
    for(int i{}; i < _MAX_NUM_PLAYERS; i++)
    {
        if(agents[i] != nullptr)
        {
            if(agents[i]->fault && agents[i]->faultstate == Agent::FaultState::DESTROYED)
            {
                if(workingIDs.contains(agents[i]->id()))
                {
                    workingIDs.removeOne(agents[i]->id());
                }
            }
            if(gameState->isStop() && agents[i]->fault && agents[i]->faultstate == Agent::FaultState::DAMEGED)
            {
                if(workingIDs.contains(agents[i]->id()))
                {
                    workingIDs.removeOne(agents[i]->id());
                }
            }
        }
    }
}

void CCoach::replacefaultedrobots()
{
    //faulted robots replacement
    QList<int> ourPlayers = wm->our.data->activeAgents;
    QList<int> faultPlayers;
    for(int i{}; i < _MAX_NUM_PLAYERS; i++)
    {
        if(agents[i] != nullptr)
        {
            if(agents[i]->fault && agents[i]->faultstate == Agent::FaultState::DAMEGED)
            {
                if(ourPlayers.contains(agents[i]->id()))
                {
                    faultPlayers.push_back(agents[i]->id());
                    //ROS_INFO_STREAM("kian:assign to faultPlayers " << agents[i]->id());
                }
            }
        }
    }

    for (int i = 0; i < faultPlayers.size(); i++) {
        faultRoles[i]->assign(agents[faultPlayers.at(i)]);
    }
    for (auto &faultRole : faultRoles) {
        if (faultRole->agent != nullptr) {
            faultRole->execute();
        }
    }
}

void CCoach::resetnonVisibleAgents()
{
    for(int i{}; i < _MAX_NUM_PLAYERS; i++)
    {
        if(agents[i] != nullptr)
        {
            bool isvisible{false};
            for(int j{}; j < wm->our.activeAgentsCount(); j++)
                if(agents[i]->id() == wm->our.activeAgentID(j))
                    isvisible = true;
            if(!isvisible)
            {
                ROS_INFO_STREAM("kian: reset: " << agents[i]->id());
                agents[i]->fault = false;
                agents[i]->faultstate = Agent::FaultState::HEALTHY;
            }
        }
    }
}

void CCoach::execute()
{
    resetnonVisibleAgents();
    generateWorkingRobotIds();
    if(gameState->isStop()) replacefaultedrobots();
    findGoalie();
    decidePreferredDefenseAgentsCount();

    // choose playmake agent
    bool defenseFirst = wm->ball->vel.length() > 1
                        && Segment2D(wm->field->oppGoalR(), wm->field->oppGoalL()).
            intersection(Segment2D(wm->ball->pos, wm->ball->pos + wm->ball->dir.norm() * 10)).isValid();
    playmakeId = -1;
    if (defenseFirst) {
        decideDefense();
        choosePlaymakeAndSupporter(true);
    } else {
        choosePlaymakeAndSupporter(false);
        decideDefense();
    }


    // devcide the whole strategy for defense agents, including Goalie, defense and Mark

    checkTransitionToForceStart();
    virtualTheirPlayOffState();
    CRoleStop::info()->reset();
    CRoleFault::info()->reset();

    for (auto &stopRole : stopRoles) {
        stopRole->assign(nullptr);
    }
    decideAttack();
    for (auto &stopRole : stopRoles) {
        if (stopRole->agent != nullptr) {
            stopRole->execute();
        }
    }

}

void CCoach::setFastPlay() {
    // TODO : Write Fast Play checker

}

void CCoach::checkRoleAssignments() {
    //    knowledge->roleAssignments.clear();


    //////////////// Matching for marker agents to mark better! /////////////////
    //    CRoleMarkInfo *markInfo = (CRoleMarkInfo*) CSkills::getInfo("mark");
    //    markInfo->matching();
    /////////////////////////////////////////////////////////////////////////////
}

DefensePlan& CCoach::getDefense() {
    return selectedPlay->defensePlan;
}

void CCoach::decideHalt(QList<int>& _ourPlayers) {
    firstTime = true;
    cyclesWaitAfterballMoved = 0;
    _ourPlayers.clear();
    _ourPlayers.append(wm->our.data->activeAgents);
    for (int i = 0 ; i < _ourPlayers.count() ; i++) {
        //        if (timePassed) {
        //
        //        }
        agents[_ourPlayers[i]]->action = haltAction;
    }

    if (!ourPlayOff->deleted) {
        ourPlayOff->reset();
        ourPlayOff->deleted = true;
    }

}

void CCoach::decideStop(QList<int> & _ourPlayers) {
    firstTime = true;
    firstPlay = true;
    cyclesWaitAfterballMoved = 0;
    lastPlayMake = -1;
    if (!ourPlayOff->deleted) {
        ourPlayOff->reset();
        ourPlayOff->deleted = true;
    }

    getBadsAndGoods(_ourPlayers);

    if(goodshooters.size() > 0) {
        double minweight = 200, weight = 200;
        int minID = -1;

        for (int goodshooter: goodshooters) {
            if(_ourPlayers.contains(goodshooter)) {
                int j = _ourPlayers.indexOf(goodshooter);
                weight = agents[goodshooter]->pos().dist(wm->ball->pos);
                if (weight < minweight) {
                    minweight = weight;
                    minID = j;
                }
            }
        }
        if (minID != -1) {
            CRoleStop::info()->setAgentBehindBall(_ourPlayers.at(minID));
        } else {
            CRoleStop::info()->setAgentBehindBall(goodshooters.at(0));
        }
    } else if(badshooters.size() >= _ourPlayers.size()) {
        CRoleStop::info()->setAgentBehindBall(-1);
    } else {
        for (int i = 0; i < _ourPlayers.size(); i++) {
            if (!badshooters.contains(i)) {
                CRoleStop::info()->setAgentBehindBall(i);
                break;
            }
        }
    }

    for (int i = 0; i < _ourPlayers.size(); i++) {
        if(!agents[_ourPlayers.at(i)]->fault)
            stopRoles[i]->assign(agents[_ourPlayers.at(i)]);
    }
    _ourPlayers.clear();
}

void CCoach::decideOurKickOff(QList<int> &_ourPlayers) {
    if (ourPlayOff->deleted) {
        ourPlayOff->deleted = false;
    }
    selectedPlay = ourPlayOff;
    decidePlayOff(_ourPlayers, KICKOFF);
    PDEBUG("ourplayers", _ourPlayers.size(), D_MAHI);

}

void CCoach::decideTheirKickOff(QList<int> &_ourPlayers) {
    selectedPlay = theirKickOff;
    firstTime = true;
}

void CCoach::decideOurDirect(QList<int> &_ourPlayers) {
    ROS_INFO_STREAM("inja kharab shod!");
    if (ourPlayOff->deleted) {
        ourPlayOff->deleted = false;
    }
    selectedPlay = ourPlayOff;
    decidePlayOff(_ourPlayers, DIRECT);
    PDEBUG("ourplayers", _ourPlayers.size(), D_MAHI);

}

void CCoach::decideTheirDirect(QList<int> &_ourPlayers) {
    selectedPlay = theirDirect;
    firstTime = true;
}

void CCoach::decideOurIndirect(QList<int> &_ourPlayers) {
    if (ourPlayOff->deleted) {
        ourPlayOff->deleted = false;
    }
    selectedPlay = ourPlayOff;
    decidePlayOff(_ourPlayers, INDIRECT);
    PDEBUG("ourplayers", _ourPlayers.size(), D_MAHI);

}

void CCoach::decideTheirIndirect(QList<int> &_ourPlayers) {
    selectedPlay = theirIndirect;
    firstTime = true;
}

void CCoach::decideOurPenalty(QList<int> &_ourPlayers) {
    ROS_INFO_STREAM("penalty: decideourpenalty");
    selectedPlay = ourPenalty;
    if (0 <= playmakeId && playmakeId <= 11) {
        ourPenalty->setPlaymake(agents[playmakeId]);
        _ourPlayers.removeOne(playmakeId);
    }
    if(!gameState->ready())
        ourPenalty->setState(PenaltyState::Positioning);

    else if(gameState->ready())
    {
        ROS_INFO_STREAM("kian: normal start -> penalty");
        ourPenalty->setState(PenaltyState::Kicking);
    }
    DBUG("penalty", D_MHMMD);
    firstTime = true;
}

void CCoach::decideTheirPenalty(QList<int> &_ourPlayers) {
    ROS_INFO_STREAM("penalty: decideourpenalty");
    selectedPlay = theirPenalty;
    firstTime = true;
}

void CCoach::decideOurPenaltyshootout(QList<int>& _ourPlayers)
{
    ROS_INFO_STREAM("shootout: decideourpenalty");
    selectedPlay = ourPenaltyShootout;
    if (0 <= playmakeId && playmakeId <= 11) {
        ourPenaltyShootout->setPlaymake(agents[playmakeId]);
        _ourPlayers.removeOne(playmakeId);
    }
    if(!gameState->ready())
        ourPenaltyShootout->setState(PenaltyShootoutState::Positioning);


    else if(gameState->ready())
    {
        ROS_INFO_STREAM("shootout: normal start -> penalty");
        ourPenaltyShootout->setState(PenaltyShootoutState::Goaling);
    }
    DBUG("penalty", D_MHMMD);
    firstTime = true;
}

void CCoach::decideTheirPenaltyshootout(QList<int> &)
{
    ROS_INFO_STREAM("penalty: decideourpenalty");
    selectedPlay = theirPenalty;
    firstTime = true;
}

void CCoach::decideStart(QList<int> &_ourPlayers) {
    ROS_INFO_STREAM("kian: in start mode");
    if (gameState->theirPenaltyShootout()) {
        selectedPlay = theirPenalty;
        return;
    }
    selectedPlay = dynamicAttack;
    decidePlayOn(_ourPlayers, lastPlayers);
}

void CCoach::decideOurBallPlacement(QList<int> &_ourPlayers) {
    selectedPlay = ourBallPlacement;
}

void CCoach::decideTheirBallPlacement(QList<int> &_ourPlayers) {
    selectedPlay = theirBallPlacement;
}

void CCoach::decideHalfTimeLineUp(QList<int> &_ourPlayers) {
    ROS_INFO("MAHI MAHI");
    selectedPlay = halftimeLineup;
    ROS_INFO("MAHI2MAHI");
}


void CCoach::decideNull(QList<int> &_ourPlayers) {
    selectedPlay->markAgents.clear();
    firstTime = true;
    if (!ourPlayOff->deleted) {
        ourPlayOff->reset();
        ourPlayOff->deleted = true;
    }
}

void CCoach::checkSensorShootFault() {
    QList<int> ourPlayers = wm->our.data->activeAgents;
    for (int i = 0; i < _NUM_PLAYERS; i++) {
        if (ourPlayers.contains(i) != nullptr) {
            Agent* tempAgent = agents[i];
            if (tempAgent->shootSensor
                &&  wm->ball->pos.dist(tempAgent->pos() + tempAgent->dir().norm() * 0.08) > 0.2) {
                faultDetectionCounter[i]++;

            } else {
                faultDetectionCounter[i] = 0;
            }
        } else {
            faultDetectionCounter[i] = 0;
        }
    }

    for (size_t i = 0; i < _NUM_PLAYERS; i++) {
        if (faultDetectionCounter[i] > 300 || agents[i]->changeIsNeeded) {
            agents[i]->changeIsNeeded = true;
        }
    }
    QString s;
    for (int ourPlayer : ourPlayers) {
        s += QString(" %1 ").arg(ourPlayer);
        if (agents[ourPlayer]->changeIsNeeded) {
            DBUG(QString("[ROBOT FAULT] %1").arg(ourPlayer), D_ERROR);

        }

    }

}

void CCoach::initStaticPlay(const POMODE _mode, const QList<int>& _ourplayers) {

    ROS_INFO("initStaticPlay: request");

    switch (_mode) {
        case POMODE::INDIRECT:
            planRequest.plan_req.gameMode = planRequest.plan_req.INDIRECT;
            break;
        case POMODE::DIRECT:
            planRequest.plan_req.gameMode = planRequest.plan_req.INDIRECT;
            break;
        case POMODE::KICKOFF:
            planRequest.plan_req.gameMode = planRequest.plan_req.KICKOFF;
            break;
    }

    planRequest.plan_req.ballPos.x = wm->ball->pos.x;
    planRequest.plan_req.ballPos.y = wm->ball->pos.y;

    planRequest.plan_req.playersNum = static_cast<unsigned char>(_ourplayers.size());

    //    planRequest.plan_req.hint.clear();
    //    for (int i = 0; i < hint->size(); ++i) {
    //        planRequest.plan_req.hint.push_back(hint[i]);
    //    }

    parsian_msgs::plan_service req{};
    req.request = planRequest;

    ROS_INFO_STREAM("--------------------------COACH: calling request");
    if (plan_client.call(req)) {
        std::string str = req.response.the_plan.planFile;
        receivedPlan = req.response;

        NGameOff::SPlan *thePlan = planMsgToSPlan(receivedPlan, _ourplayers.size());


        ourPlayOff->setMasterPlan(thePlan);
        ourPlayOff->analyseShoot(); // should call after setmasterplan
        ourPlayOff->analysePass();  // should call after setmasterplan
        checkGUItoRefineMatch(thePlan, _ourplayers);

        matchPlan(thePlan, _ourplayers); //Match The Plan
        ourPlayOff->setInitial(true);
        ourPlayOff->lockAgents = true;
        //        lastPlan = thePlan;
        //        debug(QString("chosen plan is %1").arg(lastPlan->gui.index[3]), D_MAHI);

        gotplan = true;

        ROS_INFO_STREAM("initStaticPlay: Done :) response: %s" << str);
        return;

    }

    gotplan = false;

}




void CCoach::checkGUItoRefineMatch(SPlan *_plan, const QList<int>& _ourplayers) {
    if (conf.IDBasePasser && _ourplayers.contains(conf.PasserID)) {
        int temp = _plan->matching.common->matchedID.value(0);
        _plan->matching.common->matchedID[0] = conf.PasserID;
        for (int i = 1; i < _plan->matching.common->matchedID.size(); i++) {
            if (_plan->matching.common->matchedID[i] == conf.PasserID) {
                _plan->matching.common->matchedID[i] = temp;
                break;
            }
        }
    }

    if (conf.IDBaseOneToucher
        && _ourplayers.contains(conf.OneToucherID)) {
        int temp = _plan -> matching.common -> matchedID.value(1);
        _plan -> matching.common -> matchedID[1] = conf.OneToucherID;
        for (int i = 2; i < _plan->matching.common->matchedID.size(); i++) {
            if (_plan->matching.common->matchedID[i] == conf.OneToucherID) {
                _plan->matching.common->matchedID[i] = temp;
                break;
            }
        }
    }

    qDebug() << "[coach] final Match : " << _plan->matching.common->matchedID;
}


NGameOff::SPlan* CCoach::planMsgToSPlan(parsian_msgs::plan_serviceResponse planMsg, int _currSize) {
    auto *plan = new NGameOff::SPlan();

    //    for (int i = 0; i < planMsg.the_plan.tags.size(); i++) {
    //        plan->common.tags.push_back((planMsg.the_plan.tags.at(i)).c_str());
    //    }

    plan->common.currentSize = _currSize;
    plan->execution.symmetry = (planMsg.the_plan.symmetry) ? -1 : 1;

    //    plan->execution.AgentPlan
    if (planMsg.the_plan.planMode == "INDIRECT") {
        plan->common.planMode = POMODE::INDIRECT;
    } else if (planMsg.the_plan.planMode == "DIRECT") {
        plan->common.planMode = POMODE::DIRECT;
    } else if (planMsg.the_plan.planMode == "KICKOFF") {
        plan->common.planMode = POMODE::KICKOFF;
    }

    plan->common.succesRate = planMsg.the_plan.successRate;
    plan->common.agentSize  = planMsg.the_plan.agentSize;
    plan->common.lastDist   = planMsg.the_plan.lastDist;
    plan->common.chance     = planMsg.the_plan.chance;

    plan->matching.initPos.ball.x = planMsg.the_plan.ballInitPos.x;
    plan->matching.initPos.ball.y = planMsg.the_plan.ballInitPos.y;

    for (int j = 0; j < planMsg.the_plan.agentSize; j++) {
        plan->matching.initPos.agents.push_back(planMsg.the_plan.agentInitPos[j]);
    }

    QList< QList<playOffRobot> > agpln;
    for (unsigned int i = 0; i < planMsg.the_plan.agentSize; i++) {
        ROS_INFO_STREAM("agent " << i << " pos " << planMsg.the_plan.agents.at(i).posSize);
        QList<playOffRobot>  ag;
        ag.clear();
        for (unsigned int j = 0; j < planMsg.the_plan.agents.at(i).posSize; j++) {

            ROS_INFO_STREAM("agent " << i << " pos " << planMsg.the_plan.agents.at(i).posSize << "-> " << j);
            playOffRobot* po = new playOffRobot();

            po->pos.x = planMsg.the_plan.agents.at(i).positions.at(j).pos.x;
            po->pos.y = planMsg.the_plan.agents.at(i).positions.at(j).pos.y;
            po->angle = (AngleDeg)planMsg.the_plan.agents.at(i).positions.at(j).angel;
            po->tolerance = planMsg.the_plan.agents.at(i).positions.at(j).tolerance;



            QList<playOffSkill> sk;
            sk.clear();
            for (unsigned int k = 0; k < planMsg.the_plan.agents.at(i).positions.at(j).skillSize; k++) {
                playOffSkill *p = new playOffSkill();
                p->data[0] = planMsg.the_plan.agents.at(i).positions.at(j).skills.at(k).primary;
                p->data[1] = planMsg.the_plan.agents.at(i).positions.at(j).skills.at(k).secondry;
                p->targetAgent = planMsg.the_plan.agents.at(i).positions.at(j).skills.at(k).agent;
                p->targetIndex = planMsg.the_plan.agents.at(i).positions.at(j).skills.at(k).index;
                p->name = strToEnum(planMsg.the_plan.agents.at(i).positions.at(j).skills.at(k).name);
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

POffSkills CCoach::strToEnum(const std::string& _str) {
    if (_str == "NoSkill") {
        return NoSkill;
    } else if (_str == "Mark") {
        return Mark;
    } else if (_str == "Goalie") {
        return Goalie;
    } else if (_str == "Support") {
        return Support;
    } else if (_str == "Defense") {
        return Defense;
    } else if (_str == "Position") {
        return Position;
    } else if (_str == "MoveSkill") {
        return MoveSkill;
    } else if (_str == "PassSkill") {
        return PassSkill;
    } else if (_str == "OneTouchSkill") {
        return OneTouchSkill;
    } else if (_str == "ChipToGoalSkill") {
        return ChipToGoalSkill;
    } else if (_str == "ShotToGoalSkill") {
        return ShotToGoalSkill;
    } else if (_str == "ReceivePassSkill") {
        return ReceivePassSkill;
    } else if (_str == "ReceivePassIASkill") {
        return ReceivePassIASkill;
    } else {
        return NoSkill;
    }
}

void CCoach::getBadsAndGoods(const QList<int>& _ourplayers) {
    const int max_robot_num = 12;

    bool goods[max_robot_num]
            = {conf.good_0, conf.good_1, conf.good_2, conf.good_3, conf.good_4, conf.good_5,
               conf.good_6, conf.good_7, conf.good_8, conf.good_9};

    bool bads[max_robot_num]
            = {conf.bad_0, conf.bad_1, conf.bad_2, conf.bad_3, conf.bad_4, conf.bad_5,
               conf.bad_6, conf.bad_7, conf.bad_8, conf.bad_9};

    badshooters.clear();
    goodshooters.clear();

    for (int i = 0; i < max_robot_num; i++) {
        if (_ourplayers.contains(i)) {
            if (bads[i]) {
                badshooters.append(i);
            } else if (goods[i]) {
                goodshooters.append(i);
            }
        }
    }
}

void CCoach::matchPlan(NGameOff::SPlan *_plan, const QList<int>& _ourplayers) {

    MWBM matcher;
    int passcount = 0;
    passcount = min(_plan->execution.passCount + 1, _ourplayers.size());
    matcher.create(_plan->common.currentSize - passcount, _ourplayers.size() - passcount);

    int matchedID = -1, secMatchedID = -1, thirdMatchedID = -1;
    double weight = 0;
    double minweight = 100, secMinweight = 100, thirdMinweight = 100;
    getBadsAndGoods(_ourplayers);
    goodshooters.clear();
    badshooters.clear();

    for (int i = 0; i < passcount; i++) {

        if (goodshooters.size() > 0) {

            for (int goodshooter: goodshooters) {
                if (_ourplayers.contains(goodshooter)) {
                    int j = _ourplayers.indexOf(goodshooter);
                    if (i == 0) {
                        weight = agents[_ourplayers.at(j)]->pos().dist(wm->ball->pos);
                        if (weight < minweight) {
                            minweight = weight;
                            matchedID = j;
                        }
                    } else if (i == 1 && j != matchedID) {
                        weight = agents[_ourplayers.at(j)]->pos().dist(_plan->matching.initPos.agents.at(i));
                        if (weight < secMinweight) {
                            secMinweight = weight;
                            secMatchedID = j;
                        }
                    } else if (i == 2 && j != matchedID && j != secMatchedID) {
                        weight = agents[_ourplayers.at(j)]->pos().dist(_plan->matching.initPos.agents.at(i));
                        if (weight < secMinweight) {
                            thirdMinweight = weight;
                            thirdMatchedID = j;
                        }
                    }
                }


            }
            if (matchedID != -1)
                goodshooters.removeOne(_ourplayers[matchedID]);
            if (secMatchedID != -1)
                goodshooters.removeOne(_ourplayers[secMatchedID]);
            if (thirdMatchedID != -1)
                goodshooters.removeOne(_ourplayers[thirdMatchedID]);
        } else {

            for (int j = 0; j < _ourplayers.size(); j++) {
                if (badshooters.size() < _ourplayers.size() - i) {
                    if (badshooters.contains(agents[_ourplayers.at(j)]->id()))
                        continue;
                }
                if (i == 0) {
                    weight = agents[_ourplayers.at(j)]->pos().dist(wm->ball->pos);
                    if (weight < minweight) {
                        minweight = weight;
                        matchedID = j;
                    }
                } else if (i == 1 && j != matchedID) {
                    weight = agents[_ourplayers.at(j)]->pos().dist(_plan->matching.initPos.agents.at(i));
                    if (weight < secMinweight) {
                        secMinweight = weight;
                        secMatchedID = j;
                    }
                } else if (i == 2 && j != matchedID && j != secMatchedID) {

                    weight = agents[_ourplayers.at(j)]->pos().dist(_plan->matching.initPos.agents.at(i));
                    if (weight < thirdMinweight) {
                        thirdMinweight = weight;
                        thirdMatchedID = j;
                    }
                }
            }
        }

    }


    if (matchedID != -1) {
        _plan->common.matchedID.insert(0, _ourplayers.at(matchedID));
    }
    if (secMatchedID != -1) {
        _plan->common.matchedID.insert(1, _ourplayers.at(secMatchedID));
    }
    if (thirdMatchedID != -1) {
        _plan->common.matchedID.insert(2, _ourplayers.at(thirdMatchedID));
    }

    QList<int> othersmatch;

    othersmatch.clear();
    for (int i = passcount; i < _plan->common.currentSize; i++) {
        int k = 0;
        for (int j = 0; j < _ourplayers.size(); j++) {
            if (j != matchedID && j != secMatchedID && j != thirdMatchedID) {
                weight = _plan->matching.initPos.agents.at(i).dist(agents[_ourplayers.at(j)]->pos());
                matcher.setWeight(i - passcount, k, -(weight));
                othersmatch.append(j);
                k++;
            }
        }
    }
    int nmatchedID = -1;
    qDebug() << "[Coach] matched plan with : " << matcher.findMatching();
    for (size_t i = passcount; i < _plan->common.currentSize; i++) {
        nmatchedID = matcher.getMatch(i - passcount);
        nmatchedID = othersmatch.at(nmatchedID);

        _plan->common.matchedID.insert(i, _ourplayers.at(nmatchedID));

    }
    qDebug() << "[Coach] matched by" << _plan->common.matchedID;

}


void CCoach::setPlanClient(const ros::ServiceClient& _plan_client) {
    plan_client = _plan_client;
}

void CCoach::setBehaviorPublisher(ros::Publisher& _behavior_publisher) {
    ai_status_pub = &_behavior_publisher;
}

parsian_msgs::plan_serviceResponse CCoach::getLastPlan() {
    return receivedPlan;
}

void CCoach::updateBehavior(const parsian_msgs::parsian_behaviorConstPtr _behav) {
    m_behavior = _behav;
    //    if (_behav->name == "mahi") {
    //        selectedBehavior = behaviorMahi;
    //    } else {
    //        selectedBehavior = nullptr;
    //    }
}

int CCoach::findGoalie() {
    if (conf.useGoalieInPlayoff
        && gameState->ourPlayOffKick()
        && wm->ball->pos.x > 1
        && !gameState->penaltyKick()
        && !gameState->penaltyShootout())
    {
        preferedGoalieID = -1;
        ROS_INFO_STREAM("check goaliID first : " << preferedGoalieID);

    } else {
        if (conf.GoalieFromGUI) {
            preferedGoalieID = conf.Goalie;
        } else if (wm->our.data->activeAgents.contains(wm->our.data->goalieID)){
            preferedGoalieID = wm->our.data->goalieID;
            ROS_INFO_STREAM("check goaliID from wm : " << preferedGoalieID);

        } else {
            preferedGoalieID = -1;
        }
    }
    if (gameState->timeOut() || gameState->halfTime()) {
        preferedGoalieID = -1;
        ROS_INFO_STREAM("check goaliID timeout : " << preferedGoalieID);

    }
    assignGoalieAgent(preferedGoalieID);
    return preferedGoalieID;
}

void CCoach::sendBehaviorStatus() {

    ai_status_pub->publish(fillAIStatus());
}

parsian_msgs::parsian_ai_statusPtr CCoach::fillAIStatus()
{

    parsian_msgs::parsian_ai_statusPtr ai_status{new parsian_msgs::parsian_ai_status};
    ai_status->GK = preferedGoalieID;
    ai_status->playmake_id = playmakeId;
    ai_status->supporter_id = supporterId;

    int _max{conf.numberOfDefenseEval > 0 ? preferedDefenseCounts + conf.numberOfDefenseEval : preferedDefenseCounts},
            _min{conf.numberOfDefenseEval <= 0 ? preferedDefenseCounts + conf.numberOfDefenseEval : preferedDefenseCounts};


    for (int i {_min}; i < _max+ 1; i++)
    {
        parsian_msgs::parsian_pair_roles prs;
        ai_status->states.push_back(prs);
        for (int j = 0; j < i; j++)
        {
            parsian_msgs::parsian_pair_role pr;
            pr.id = defenseMatched[0][i][j].first;
            pr.task = defenseMatched[0][i][j].second;
            pr.role = parsian_msgs::parsian_pair_role::DEFENSE;
            ai_status->states[i - _min].roles.push_back(pr);
        }
    }
    return ai_status;
}

void CCoach::findDefneders(const int& max_number, const int& min_number) {
    defenseMatched[0] = new QPair<int, parsian_msgs::parsian_robot_task>* [max_number + 1];
    for (int i{ min_number }; i < max_number +1; i++) defenseMatched[0][i] = new QPair<int, parsian_msgs::parsian_robot_task>[i];
    for (int i{min_number}; i < max_number + 1; i++) {
        assignDefenseAgents(i);
        assignGoalieAgent(preferedGoalieID);
        selectedPlay->defensePlan.initGoalKeeper(goalieAgent);
        selectedPlay->defensePlan.initDefense(defenseAgents);
        selectedPlay->defensePlan.execute();
        for (int j = 0; j < defenseAgents.size(); j++ )
        {
            defenseMatched[0][i][j].first = defenseAgents[j]->id();

            parsian_msgs::parsian_robot_task task;
            auto task_name = defenseAgents[j]->action->getActionName().toStdString();

            if (task_name == "GotopointavoidAction")
            {
                task.select = task.GOTOPOINTAVOID;
                task.gotoPointAvoidTask = *reinterpret_cast<parsian_msgs::parsian_skill_gotoPointAvoid*>(defenseAgents[j]->action->getMessage());
                defenseMatched[0][i][j].second = task;
            }
            else if(task_name == "GotopointAction")
            {
                task.select = task.GOTOPOINT;
                task.gotoPointTask = *reinterpret_cast<parsian_msgs::parsian_skill_gotoPoint*>(defenseAgents[j]->action->getMessage());
                defenseMatched[0][i][j].second = task;
            }

            else if(task_name == "OnetouchAction")
            {
                task.select = task.ONETOUCH;
                task.oneTouchTask = *reinterpret_cast<parsian_msgs::parsian_skill_oneTouch*>(defenseAgents[j]->action->getMessage());
                defenseMatched[0][i][j].second = task;
            }

            else if(task_name == "KickAction")
            {
                task.select = task.KICK;
                task.kickTask = *reinterpret_cast<parsian_msgs::parsian_skill_kick*>(defenseAgents[j]->action->getMessage());
                defenseMatched[0][i][j].second = task;
            }

            else if(task_name == "ReceivepassAction")
            {
                task.select = task.RECIVEPASS;
                task.receivePassTask = *reinterpret_cast<parsian_msgs::parsian_skill_receivePass*>(defenseAgents[j]->action->getMessage());
                defenseMatched[0][i][j].second = task;
            }


            else if(task_name == "NoAction")
            {
                task.select = task.NOTASK;
                task.noTask = *reinterpret_cast<parsian_msgs::parsian_skill_no*>(defenseAgents[j]->action->getMessage());
                defenseMatched[0][i][j].second = task;
            }

        }
    }


    //    defenseMatched[1] = new QPair<int, parsian_msgs::parsian_robot_task>* [max_number + 1];
    //    for (int i = min_number; i <= max_number; i++) defenseMatched[0][i] = new QPair<int, parsian_msgs::parsian_robot_task>[i];

    //    for (int i = min_number; i <= max_number; i++) {
    //        assignDefenseAgents(i);
    //        assignGoalieAgent(-1);
    //        selectedPlay->defensePlan.initGoalKeeper(nullptr);
    //        selectedPlay->defensePlan.initDefense(defenseAgents);
    //        selectedPlay->defensePlan.execute();
    //        for (int j = 0; j < defenseAgents.size(); j++ )
    //        {
    //            defenseMatched[0][i][j].first = defenseAgents[j]->id();

    //            parsian_msgs::parsian_robot_task task;
    //            auto task_name = defenseAgents[j]->action->getActionName().toStdString();

    //            if (task_name == "GotopointavoidAction")
    //            {
    //                task.select = task.GOTOPOINTAVOID;
    //                task.gotoPointAvoidTask = *reinterpret_cast<parsian_msgs::parsian_skill_gotoPointAvoid*>(defenseAgents[j]->action->getMessage());
    //                defenseMatched[0][i][j].second = task;
    //            }
    //            else if(task_name == "GotopointAction")
    //            {
    //                task.select = task.GOTOPOINT;
    //                task.gotoPointTask = *reinterpret_cast<parsian_msgs::parsian_skill_gotoPoint*>(defenseAgents[j]->action->getMessage());
    //                defenseMatched[0][i][j].second = task;
    //            }

    //            else if(task_name == "OnetouchAction")
    //            {
    //                task.select = task.ONETOUCH;
    //                task.oneTouchTask = *reinterpret_cast<parsian_msgs::parsian_skill_oneTouch*>(defenseAgents[j]->action->getMessage());
    //                defenseMatched[0][i][j].second = task;
    //            }

    //            else if(task_name == "KickAction")
    //            {
    //                task.select = task.KICK;
    //                task.kickTask = *reinterpret_cast<parsian_msgs::parsian_skill_kick*>(defenseAgents[j]->action->getMessage());
    //                defenseMatched[0][i][j].second = task;
    //            }

    //            else if(task_name == "ReceivepassAction")
    //            {
    //                task.select = task.RECIVEPASS;
    //                task.receivePassTask = *reinterpret_cast<parsian_msgs::parsian_skill_receivePass*>(defenseAgents[j]->action->getMessage());
    //                defenseMatched[0][i][j].second = task;
    //            }


    //            else if(task_name == "NoAction")
    //            {
    //                task.select = task.NOTASK;
    //                task.noTask = *reinterpret_cast<parsian_msgs::parsian_skill_no*>(defenseAgents[j]->action->getMessage());
    //                defenseMatched[0][i][j].second = task;
    //            }

    //        }
    //    }

}

bool CCoach::isFastPlay() {
    if (conf.UseFastPlay) {
        return true; // TODO : fix this by considering that opp agents
    }
}

