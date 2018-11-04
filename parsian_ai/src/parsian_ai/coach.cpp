////
//// Created by parsian-ai on 9/22/17.
////

#include <parsian_ai/coach.h>
#include <parsian_util/geom/vector_2d.h>

CCoach::CCoach(Agent**_agents)
{
    clearBallVels();
    averageVel = 0;
    agents = _agents;
    first = true;

    ///////////////////////////////////
    goalieTimer.start();
    ////////////////////intentions
    playOnExecTime.start();
    intentionTimePossession.start();
    playMakeIntention.start();

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
    delete stopPlay          ;
    delete ourPlayOff        ;
    delete dynamicAttack     ;
}

void CCoach::decidePreferredDefenseAgentsCount() {

    missMatchIds.clear();
    if (gameState->getState() == States::Stop || gameState->getState() == States::Halt || first) {
        if (workingIDs.size() != 0u) {
            robotsIdHist.clear();
            for (int workingID : workingIDs) {
                robotsIdHist.append(workingID);
            }
        }
        first = false;
    }

    if (workingIDs.size() > _NUM_PLAYERS) {
        missMatchIds.clear();
        for (int workingID : workingIDs) {
            for (int k = 0 ; k < robotsIdHist.count() ; k++) {
                if (robotsIdHist.at(k) == workingID) {
                    break;
                }
                if (k == robotsIdHist.count() - 1) {
                    missMatchIds.append(workingID);
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

void CCoach::decidePlayOff(const QList<int>& _ourPlayers, POMODE _mode) {

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
            ourPlayOff->dynamicMatch[i] = i;
        }
    }
    if (_ourplayers.size() < 2) {
        ourPlayOff->dynamicSelect = DynamicSelect::Chip;
    } else if (!gotplan){
        gotplan = true;
        ourPlayOff->dynamicSelect = DynamicSelect::Kick;
    } else {
        ourPlayOff->dynamicSelect = DynamicSelect::Khafan;
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
            ourPlayOff->dynamicMatch[i] = i;
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


    // decide the whole strategy for defense agents, including Goalie, defense and Mark

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

DefensePlan& CCoach::getDefense() {
    return selectedPlay->defensePlan;
}

void CCoach::decideHalt(QList<int>& _ourPlayers) {
    firstTime = true;
    cyclesWaitAfterballMoved = 0;
    _ourPlayers.clear();
    _ourPlayers.append(wm->our.data->activeAgents);
    for (int i = 0 ; i < _ourPlayers.count() ; i++) {
        agents[_ourPlayers[i]]->action = haltAction;
    }

    if (!ourPlayOff->deleted) {
        ourPlayOff->reset();
        ourPlayOff->deleted = true;
    }

}

void CCoach::decideStop(const QList<int> & _ourPlayers) {
    firstTime = true;
    firstPlay = true;
    cyclesWaitAfterballMoved = 0;
    lastPlayMake = -1;
    if (!ourPlayOff->deleted) {
        ourPlayOff->reset();
        ourPlayOff->deleted = true;
    }

    for (int i = 0; i < _ourPlayers.size(); i++) {
        if(!agents[_ourPlayers.at(i)]->fault)
            stopRoles[i]->assign(agents[_ourPlayers.at(i)]);
    }
//    _ourPlayers.clear(); TODO: CHECK THIS
}

void CCoach::decideOurKickOff(const QList<int> &_ourPlayers) {
    if (ourPlayOff->deleted) {
        ourPlayOff->deleted = false;
    }
    selectedPlay = ourPlayOff;
    decidePlayOff(_ourPlayers, POMODE::Kickoff);
    PDEBUG("ourplayers", _ourPlayers.size(), D_MAHI);

}

void CCoach::decideTheirKickOff(const QList<int> &_ourPlayers) {
    selectedPlay = theirKickOff;
    firstTime = true;
}

void CCoach::decideOurDirect(const QList<int> &_ourPlayers) {
    ROS_INFO_STREAM("inja kharab shod!");
    if (ourPlayOff->deleted) {
        ourPlayOff->deleted = false;
    }
    selectedPlay = ourPlayOff;
    decidePlayOff(_ourPlayers, POMODE::Direct);
    PDEBUG("ourplayers", _ourPlayers.size(), D_MAHI);

}

void CCoach::decideTheirDirect(const QList<int> &_ourPlayers) {
    selectedPlay = theirDirect;
    firstTime = true;
}

void CCoach::decideOurIndirect(const QList<int> &_ourPlayers) {
    if (ourPlayOff->deleted) {
        ourPlayOff->deleted = false;
    }
    selectedPlay = ourPlayOff;
    decidePlayOff(_ourPlayers, POMODE::Indirect);
    PDEBUG("ourplayers", _ourPlayers.size(), D_MAHI);

}

void CCoach::decideTheirIndirect(const QList<int> &_ourPlayers) {
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

void CCoach::decideTheirPenalty(const QList<int> &_ourPlayers) {
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

void CCoach::decideTheirPenaltyshootout(const QList<int> &)
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

void CCoach::decideOurBallPlacement(const QList<int> &_ourPlayers) {
    selectedPlay = ourBallPlacement;
}

void CCoach::decideTheirBallPlacement(const QList<int> &_ourPlayers) {
    selectedPlay = theirBallPlacement;
}

void CCoach::decideHalfTimeLineUp(const QList<int> &_ourPlayers) {
    ROS_INFO("MAHI MAHI");
    selectedPlay = halftimeLineup;
    ROS_INFO("MAHI2MAHI");
}


void CCoach::decideNull(const QList<int> &_ourPlayers) {
    selectedPlay->markAgents.clear();
    firstTime = true;
    if (!ourPlayOff->deleted) {
        ourPlayOff->reset();
        ourPlayOff->deleted = true;
    }
}

void CCoach::initStaticPlay(const POMODE _mode, const QList<int>& _ourplayers) {

    ROS_INFO("initStaticPlay: request");

    switch (_mode) {
        case POMODE::Indirect:
            planRequest.plan_req.gameMode = planRequest.plan_req.INDIRECT;
            break;
        case POMODE::Direct:
            planRequest.plan_req.gameMode = planRequest.plan_req.INDIRECT;
            break;
        case POMODE::Kickoff:
            planRequest.plan_req.gameMode = planRequest.plan_req.KICKOFF;
            break;
        case POMODE::None:break;
    }

    planRequest.plan_req.ballPos.x = wm->ball->pos.x;
    planRequest.plan_req.ballPos.y = wm->ball->pos.y;

    planRequest.plan_req.playersNum = static_cast<unsigned char>(_ourplayers.size());

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

    plan->common.currentSize = _currSize;
    plan->execution.symmetry = (planMsg.the_plan.symmetry) ? -1 : 1;

    plan->common.lastDist   = planMsg.the_plan.lastDist;

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
            auto* po = new playOffRobot();

            po->pos.x = planMsg.the_plan.agents.at(i).positions.at(j).pos.x;
            po->pos.y = planMsg.the_plan.agents.at(i).positions.at(j).pos.y;
            po->angle = (AngleDeg)planMsg.the_plan.agents.at(i).positions.at(j).angel;
            po->tolerance = planMsg.the_plan.agents.at(i).positions.at(j).tolerance;



            QList<playOffSkill> sk;
            sk.clear();
            for (unsigned int k = 0; k < planMsg.the_plan.agents.at(i).positions.at(j).skillSize; k++) {
                auto *p = new playOffSkill();
                p->data[0] = static_cast<int>(planMsg.the_plan.agents.at(i).positions.at(j).skills.at(k).primary);
                p->data[1] = static_cast<int>(planMsg.the_plan.agents.at(i).positions.at(j).skills.at(k).secondry);
                p->targetAgent = static_cast<int>(planMsg.the_plan.agents.at(i).positions.at(j).skills.at(k).agent);
                p->targetIndex = static_cast<int>(planMsg.the_plan.agents.at(i).positions.at(j).skills.at(k).index);
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

POFFSKILL CCoach::strToEnum(const std::string& _str) {
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

void CCoach::matchPlan(NGameOff::SPlan *_plan, const QList<int>& _ourplayers) {

    MWBM matcher;
    matcher.create(_plan->common.currentSize - 1, _ourplayers.size() - 1);

    int matchedID = -1;
    double weight = 0;
    double minweight = 10000;

    for (int j = 0; j < _ourplayers.size(); j++) {
        weight = agents[_ourplayers.at(j)]->pos().dist(wm->ball->pos);
        if (weight < minweight) {
            minweight = weight;
            matchedID = j;
        }
    }

    if (matchedID != -1) {
        _plan->common.matchedID.insert(0, _ourplayers.at(matchedID));
    }

    QList<int> othersmatch;
    for (int i = 1; i < _plan->common.currentSize; i++) {
        int k = 0;
        for (int j = 0; j < _ourplayers.size(); j++) {
            if (j != matchedID) {
                weight = _plan->matching.initPos.agents.at(i).dist(agents[_ourplayers.at(j)]->pos());
                matcher.setWeight(i - 1, k, -(weight));
                othersmatch.append(j);
                k++;
            }
        }
    }

    int nmatchedID = -1;
    qDebug() << "[Coach] matched plan with : " << matcher.findMatching();
    for (int i = 1; i < _plan->common.currentSize; i++) {
        nmatchedID = matcher.getMatch(i - 1);
        nmatchedID = othersmatch.at(nmatchedID);
        _plan->common.matchedID.insert(i, nmatchedID);

    }
    qDebug() << "[Coach] matched by" << _plan->common.matchedID;

}


void CCoach::setPlanClient(const ros::ServiceClient& _plan_client) {
    plan_client = _plan_client;
}


parsian_msgs::plan_serviceResponse CCoach::getLastPlan() {
    return receivedPlan;
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

bool CCoach::isFastPlay() {
    if (conf.UseFastPlay) {
        return true; // TODO : fix this by considering that opp agents
    }
}

