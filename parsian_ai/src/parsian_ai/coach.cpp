#include <utility>

////
//// Created by parsian-ai on 9/22/17.
////
#include <parsian_ai/coach.h>

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


    ourPlayOff->setPlanClient(plan_client);

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
    preferredDefenseCounts = 2;
    selectedPlay = stopPlay;
    for (int &i : faultDetectionCounter) {
        i = 0;
    }
    firstTime = true;

    haltAction = new NoAction;

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
            preferredDefenseCounts = agentsCount - 1;

        } else if (wm->ball->pos.x > 1.2) {
            preferredDefenseCounts = conf.Defense;
        }
    } else if (gameState->isStart()) {
        if (know->variables["transientFlag"].toBool())
        {
            //// Add Playmake after time
            if (trasientTimeOut.elapsed() > 800 && !wm->field->ourBigPenaltyArea(1,0.1,0).contains(wm->ball->pos)) {
                preferredDefenseCounts = std::max(0, agentsCount - missMatchIds.count() - 1);

            } else {
                preferredDefenseCounts = agentsCount - missMatchIds.count();

            }
        } else { //// PLAYON

            if (agentsCount == 1) {
                preferredDefenseCounts = 0; //// just one playmake

            } else if (agentsCount == 2) {
                preferredDefenseCounts = 1; //// one playmake and one defense
            } else {
                preferredDefenseCounts = std::min(selectedPlay->defensePlan.findNeededDefense(), agentsCount - 1);
//                preferredDefenseCounts += agentsCount - 1 - preferredDefenseCounts - selectedPlay->markPlan.findNeededMark();
            }
        }
    } else if (gameState->ourPlayOffKick()) {
        if (wm->ball->pos.x < 1) {
            preferredDefenseCounts = (selectedPlay->defensePlan.findNeededDefense() == 1) ? 1 : 2;

        } else if (wm->ball->pos.x > 1.5) {
            preferredDefenseCounts = 0;
        }

    } else if (gameState->theirPlayOffKick()) {
        if (gameState->theirKickoff()) {
            preferredDefenseCounts = 2;
        } else {
            preferredDefenseCounts = std::max(agentsCount - missMatchIds.count() - 1, 0);
        }
    } else {
        DBUG("UNKNOWN STATE", D_ERROR);
    }

    if (gameState->halfTime() || gameState->timeOut()) {
        preferredDefenseCounts = 0;
    }

    if (gameState->penaltyShootout() || gameState->penaltyKick()) {
        preferredDefenseCounts = 0;
    }
    if(conf.StrictFormation){
        if (conf.Defense > 3){
            preferredDefenseCounts = 3;
        } else {
            preferredDefenseCounts = conf.Defense;
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

    if(wm->ball->vel.length() <.05 && lastBallVels.empty()) return false;
    lastBallVels.append(wm->ball->vel);
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
    if (lastState == States::TheirDirectKick || lastState == States::TheirIndirectKick) {
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
    assignDefenseAgents(preferredDefenseCounts);
    ROS_INFO_STREAM("SD: " << preferredDefenseCounts << " : " << defenseAgents.size());
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
}

int CCoach::choosePlayMake(const QList<int> &_agentsID){
    double maxD = -10000000.1;
    int playmake = -1;
    for (const auto& player : _agentsID) {
        double value;
        if (wm->ball->vel.length() < 0.4) value = agents[player]->pos().dist(wm->ball->pos);
        else value = kickTimeEstimation(agents[player], wm->field->oppGoal());
        value *= -1;

        if (player == lastPlayMake) {
            value += conf.playMakeStopThr;
        }

        if (value > maxD) {
            maxD = value;
            playmake = player;
        }
    }
    return playmake;
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
        case States::Stop:
            ourBallPlacement->first = true;
            decideStop(ourPlayersID);
            return;
        case States::OurKickOff:
        case States::OurDirectKick:
        case States::OurIndirectKick:
            decideOurFreeKick(ourPlayersID);
            break;
        case States::TheirKickOff:
            decideTheirKickOff(ourPlayersID);
            break;
        case States::TheirDirectKick:
            decideTheirDirect(ourPlayersID);
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
    if (firstTime) {
        selectedPlay->init();
        firstTime = false;
    }
    selectedPlay->execute(ourAgents);

    lastPlayers.clear();
    lastPlayers.append(ourPlayersID);
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
            PDEBUG("mark num =" , MarkNum , D_AHZ);
            selectedPlay->markAgents.append(agents[ourPlayers.front()]);
            ourPlayers.removeFirst();
        }
        PDEBUG("mark agents =" , selectedPlay->markAgents.size() , D_AHZ);
    }

    lastBallPossesionState = ballPState;
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

void CCoach::resetNonVisibleAgents()
{
    for(int i{}; i < _MAX_NUM_PLAYERS; i++) {
        if(agents[i] != nullptr) {
            bool isvisible{false};
            for(int j{}; j < wm->our.activeAgentsCount(); j++) {
                if (agents[i]->id() == wm->our.activeAgentID(j)) isvisible = true;
            }

            if(!isvisible) {
                ROS_INFO_STREAM("kian: reset: " << agents[i]->id());
                agents[i]->fault = false;
                agents[i]->faultstate = Agent::FaultState::HEALTHY;
            }
        }
    }
}

void CCoach::execute()
{
    resetNonVisibleAgents();
    generateWorkingRobotIds();
    if(gameState->isStop()) replacefaultedrobots();
    int goalie = findGoalie();
    assignGoalieAgent(goalie);
    decidePreferredDefenseAgentsCount();

    // choose playmake agent
    bool defenseFirst = wm->ball->vel.length() > 1
                        && wm->field->ourGoalLine().intersection(wm->ball->seg()).isValid();
    playmakeId = -1;
    defenseAgents.clear();
    if (defenseFirst) {
        decideDefense();
        handlePlayMake(remainingAgent());
    } else {
        handlePlayMake(remainingAgent());
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
    if (!firstTime) { // TODO: Reset Other plays here
        ourPlayOff->reset();
        firstTime = true;
    }
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

void CCoach::decideOurFreeKick(const QList<int> &_ourPlayers) {
    if (ourPlayOff->deleted) ourPlayOff->deleted = false;
    selectedPlay = ourPlayOff;
}

void CCoach::decideTheirKickOff(const QList<int> &_ourPlayers) {
    selectedPlay = theirKickOff;
}

void CCoach::decideTheirDirect(const QList<int> &_ourPlayers) {
    selectedPlay = theirDirect;
}

void CCoach::decideTheirIndirect(const QList<int> &_ourPlayers) {
    selectedPlay = theirIndirect;
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
}

void CCoach::decideTheirPenalty(const QList<int> &_ourPlayers) {
    ROS_INFO_STREAM("penalty: decideourpenalty");
    selectedPlay = theirPenalty;
}

void CCoach::decideOurPenaltyshootout(QList<int>& _ourPlayers) {
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
}

void CCoach::decideTheirPenaltyshootout(const QList<int> &) {
    ROS_INFO_STREAM("penalty: decideourpenalty");
    selectedPlay = theirPenalty;
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
    if (!ourPlayOff->deleted) {
        ourPlayOff->reset();
        ourPlayOff->deleted = true;
    }
}

void CCoach::setPlanClient(ros::ServiceClientPtr _plan_client) {
    plan_client = std::move(_plan_client);
}

int CCoach::findGoalie() {
    int preferredGoalieID = -1;
    int validID = (conf.GoalieFromGUI) ? conf.Goalie : wm->our.data->goalieID;

    if (!useGoalieInPlayOff()
    && !gameState->timeOut()
    && !gameState->halfTime()
    && wm->our.data->activeAgents.contains(validID)) {
        preferredGoalieID = validID;
    }
    ROS_INFO_STREAM("Goalie ID: " << preferredGoalieID);
    return preferredGoalieID;
}

bool CCoach::useGoalieInPlayOff() {
    return conf.useGoalieInPlayoff
           && (gameState->ourDirectKick() || gameState->ourIndirectKick())
           && wm->ball->pos.x > 1;
}

QList<int> CCoach::remainingAgent() {
    QList<int> ourPlayers = wm->our.data->activeAgents;
    if(ourPlayers.contains(goalieAgent->id())) {
        ourPlayers.removeOne(goalieAgent->id());
    }
    for (auto& d : defenseAgents) {
        if (ourPlayers.contains(d->id())) ourPlayers.removeOne(d->id());
    }
    return ourPlayers;
}

void CCoach::handlePlayMake(const QList<int> &_agentsID) {
    if (!gameState->isStart() || _agentsID.empty()) {

        playmakeId = -1;
        lastPlayMake = -1;
        playMakeIntention.restart();

    } else if (playMakeIntention.elapsed() < conf.playMakeIntention) {

        playmakeId = lastPlayMake;

    } else {

        if (lastPlayMake != playmakeId || isBallcollide(3, 30)) playMakeIntention.restart();
        playmakeId = choosePlayMake(_agentsID);

    }
    lastPlayMake = playmakeId;
}