////
//// Created by parsian-ai on 9/22/17.
////

#include <parsian_ai/coach.h>


CCoach::CCoach(Agent**_agents)
{
    clearBallVels();
    averageVel = 0;    
    goalieTrappedUnderGoalNet = false;
    inited = false;
    agents = _agents;
    lastAssignCycle = -10;
    lastBallVelPM = Vector2D(0, 0);
    lastBallPos = Vector2D(0, 0);
    passPos = Vector2D(0, 0);
    passPlayMake = false;
    first = true;

    ///////////////////////////////////
    goalieTimer.start();
    ////////////////////intentions
    playOnExecTime.start();
    intentionTimePossession.start();
    playMakeIntention.start();

    possessionIntentionInterval = 200;
    playOnTime = 2000;
    playMakeIntentionInterval = 300;
    playMakeTh = 0.3;


    // Old Plays
    ourPenalty          = new COurPenalty;
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

    lastDefenseAgents.clear();

    defenseTimeForVisionProblem[0].start();
    defenseTimeForVisionProblem[1].start();
    know->variables["transientFlag"].setValue(false);
    trasientTimeOut.start();
    translationTimeOutTime = 4000;
    exeptionPlayMake = nullptr;
    exeptionPlayMakeThr = 0;

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
        if (wm->our.activeAgentsCount() != 0u) {
            robotsIdHist.clear();
            for (int i = 0 ; i < wm->our.activeAgentsCount() ; i++) {
                robotsIdHist.append(wm->our.active(i)->id);
            }
        }
        first = false;
    }

    if (wm->our.activeAgentsCount() > _NUM_PLAYERS) {
        missMatchIds.clear();
        for (int i = 0 ; i < wm->our.activeAgentsCount() ; i++) {
            for (int k = 0 ; k < robotsIdHist.count() ; k++) {
                if (robotsIdHist.at(k) == wm->our.active(i)->id) {
                    break;
                }
                if (k == robotsIdHist.count() - 1) {
                    missMatchIds.append(wm->our.active(i)->id);
                }
            }
        }
    }

    int agentsCount = wm->our.data->activeAgents.count() - missMatchIds.count();
    if (goalieAgent != nullptr) {
        if (goalieAgent->isVisible()) {
            agentsCount--;
        }
    }

    // TODO : Fix lock agent for unexpected robot fall
    if (selectedPlay != nullptr && selectedPlay->lockAgents) {
        preferedDefenseCounts = lastPreferredDefenseCounts;
        return;
    }


    if (gameState->isStop()) {
        if (wm->ball->pos.x < 0) {
            preferedDefenseCounts = agentsCount - 1;

        } else if (wm->ball->pos.x > 1) {
            preferedDefenseCounts = conf.Defense;
        }
    } else if (gameState->isStart()) {
        if (know->variables["transientFlag"].toBool()) {
            //// Add Playmake after time
            if (trasientTimeOut.elapsed() > 800 && !wm->field->isInOurPenaltyArea(wm->ball->pos)) {
                preferedDefenseCounts = std::max(0, agentsCount - missMatchIds.count() - 1);

            } else {
                preferedDefenseCounts = agentsCount - missMatchIds.count();

            }
        } else { //// PLAYON
            bool oppsAttack = false;
            for (int i = 0 ; i < wm->opp.activeAgentsCount() ; i++) {
                if (wm->opp.active(i)->pos.x <= 0) {
                    oppsAttack = true;
                }
            }

            if (agentsCount == 1) {
                preferedDefenseCounts = 0; //// just one playmake

            } else if (agentsCount == 2) {
                preferedDefenseCounts = 1; //// one playmake and one defense
            } else {
                if (oppsAttack) {
                    preferedDefenseCounts = 2;
                } else {
                    preferedDefenseCounts = 1;
                }

            }
        }
    } else if (gameState->ourPlayOffKick()) {
        if (wm->ball->pos.x < -1) {
            preferedDefenseCounts = (selectedPlay->defensePlan.findNeededDefense() == 1) ? 1 : 2;

        } else if (wm->ball->pos.x > 0.5) {
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

    if (gameState->penaltyShootout()) {
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

    int agentsCount = wm->our.data->activeAgents.count();
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
    QList<int> ids = wm->our.data->activeAgents;
    goalieAgent = nullptr;
    if (ids.contains(goalieID)) {
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


void CCoach::assignDefenseAgents(int defenseCount) {


    if (selectedPlay != nullptr && selectedPlay->lockAgents) {
        defenseAgents.clear();
        defenseAgents.append(lastDefenseAgents);
        return;
    }

    QList<int> ids = wm->our.data->activeAgents;
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

bool CCoach::isBallcollide() {

    if(wm->ball->vel.length() <.05 && lastBallVels.size() == 0){
        ROS_INFO_STREAM("KALI : hanooz harkat nakarde");
        return false;
    }
    lastBallVels.append(std::move(wm->ball->vel));
    averageVel += wm->ball->vel.length();
    if(lastBallVels.size()>2){
        float innerproduct = wm->ball->vel.norm().innerProduct(lastBallVels.first().norm());
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

        if(innerproduct > .985){  // <10 degree
            removeLastBallVel();
            ROS_INFO_STREAM("KALI : saff mire");
            return false;
        }

        if(innerproduct < .966){// 15 degree
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

    removeLastBallVel();
    ROS_INFO_STREAM("KALI : hichi nashod");
    return false;
}
void CCoach::removeLastBallVel(){
    averageVel -=  lastBallVels.first().length();
    if(lastBallVels.size() > 5)
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

    for (int i = 0 ; i < wm->our.activeAgentsCount() ; i++) {
        if (wm->our.active(i)->id != playmakeId) {
            obstacles.append(Circle2D(wm->our.active(i)->pos, 0.1));
        }
    }
    double prob, angle, biggestAngle;

    CKnowledge::getEmptyAngle(*wm->field, agentPos - (wm->field->oppGoal() - agentPos).norm() * 0.15, wm->field->oppGoalL(), wm->field->oppGoalR(), obstacles, prob, angle, biggestAngle);


    return prob;
}

void CCoach::updateAttackState() {
    ourAttackState = SAFE;
    return;
}

void CCoach::choosePlaymakeAndSupporter()
{
    playmakeId = -1;
    QList<int> ourPlayers = wm->our.data->activeAgents;
    if(ourPlayers.contains(preferedGoalieID)) {
        ourPlayers.removeOne(preferedGoalieID);
    }

    if (ourPlayers.empty()) {
        playmakeId = -1;
        lastPlayMake = -1;
        return;
    }


    ////////////////////first we choose our playmake
    double ballVel = wm->ball->vel.length();
    Vector2D ballPos = wm->ball->pos;
    if (ballVel < 0.3) {
        double maxD = -1000.1;
        for (int ourPlayer : ourPlayers) {
            if(selectedPlay->playoff_badPasserID != -1){
                if(ourPlayer == selectedPlay->playoff_badPasserID && ourPlayers.size() > 1){
                    ROS_INFO_STREAM("playofff: skipped: "<<ourPlayer);
                    continue;
                }
            }
            double o = -1 * agents[ourPlayer]->pos().dist(ballPos) ;
            if (ourPlayer == lastPlayMake) {
                o += conf.playMakeStopThr;
            }
            if (o > maxD) {
                maxD = o;
                playmakeId = ourPlayer;
            }
        }
        ROS_INFO_STREAM("op :" << ballPos.x << "  " << agents[ourPlayers[0]]->pos().x);
        lastPlayMake = playmakeId;
    } else {
        if (playMakeIntention.elapsed() < playMakeIntentionInterval) { // TODO : fix config
            playmakeId = lastPlayMake;
            return;

            playMakeIntention.restart();
            double nearest[10] = {};
            for (int ourPlayer : ourPlayers) {
                nearest[ourPlayer] = CKnowledge::kickTimeEstimation(agents[ourPlayer], wm->field->oppGoal(), *wm->ball,
                                                                    4, 3, 2,
                                                                    2); // TODO : read from common config agents
            }
            if (lastPlayMake >= 0 && lastPlayMake <= 9) {
                nearest[lastPlayMake] -= conf.playMakeMoveThr;
            }
            double minT = 1e8; // 10 ^ 8
            for (int ourPlayer : ourPlayers) {
                if (nearest[ourPlayer] < minT) {
                    minT = nearest[ourPlayer];
                    playmakeId = ourPlayer;
                }
            }
            lastPlayMake = playmakeId;
        }
    }
}

void CCoach::decideAttack() {
    // find unused agents!
    QList<int> ourPlayersID = wm->our.data->activeAgents;
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
        if(dynamicAttack->getPMfromCaoch())
        {
            dynamicAttack->setPlayMake(agents[playmakeId]);
            ourPlayers.removeOne(playmakeId);
            ROS_INFO_STREAM("PMfromCaoch true");
        } else
        {
            dynamicAttack->setPlayMake(agents[dynamicAttack->getReceiverID()]);
            ourPlayers.removeOne(dynamicAttack->getReceiverID());
            ROS_INFO_STREAM("PMfromCaoch false");
        }
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
            shotToGoalthr = std::max(0.0, conf.DirectTrsh - 0.2);
        } else {
            dynamicAttack->setDirectShot(false);
            shotToGoalthr = 0;
        }
    }

    dynamicAttack->setWeHaveBall(ballPState   == BallPossesion::WEHAVETHEBALL);
    dynamicAttack->setFast(ourAttackState     == FAST);
    dynamicAttack->setCritical(ourAttackState == CRITICAL);

    //////////////////////////////////////////////assign agents
    bool overdef =  (selectedPlay->defensePlan.findNeededDefense() == 1) ? true : false;
    int MarkNum = 0;
    switch (ballPState) {
        case BallPossesion::WEHAVETHEBALL:
            MarkNum = 0;
            break;
        case BallPossesion::WEDONTHAVETHEBALL:
            MarkNum = (overdef) ? 4 : 3;
            break;
        case BallPossesion::SOSOOUR:
            MarkNum = 2;
            break;
        case BallPossesion::SOSOTHEIR:
            MarkNum = 3;
            break;
    }
    MarkNum = std::min(MarkNum, ourPlayers.count());
    MarkNum = 0;
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
            selectedPlay->markAgents.append(agents[ourPlayers.front()]);
            ourPlayers.removeFirst();
        }
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

    } else if (wm->ball->pos.x < -1) {
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
        break;
    case NGameOff::DynamicPlay:
        initDynamicPlay(_ourplayers);
        break;
    case NGameOff::FastPlay:
        initFastPlay(_ourplayers);
        break;
    case NGameOff::FirstPlay:
        initFirstPlay(_ourplayers);
        break;
    default:
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

void CCoach::execute()
{
    findGoalie();
    choosePlaymakeAndSupporter();
    decidePreferredDefenseAgentsCount();
    decideDefense();

    checkTransitionToForceStart();

    virtualTheirPlayOffState();
    ROS_INFO_STREAM("PM :" << playmakeId);
    ROS_INFO_STREAM("GAME STATE : " << static_cast<int>(gameState->getState()));
    ////////////////////////////////////////////
    //// Handle Roles Here
    CRoleStop::info()->reset();
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

    for (int i = 0; i < _ourPlayers.size(); i++) {
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
    selectedPlay = ourPenalty;
    DBUG("penalty", D_MHMMD);
    firstTime = true;
}

void CCoach::decideTheirPenalty(QList<int> &_ourPlayers) {
    selectedPlay = theirPenalty;
    firstTime = true;
}

void CCoach::decideStart(QList<int> &_ourPlayers) {
    if (gameState->penaltyShootout()) {
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
            planRequest.plan_req.gameMode = planRequest.plan_req.DIRECT;
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

        matchPlan(thePlan, _ourplayers); //Match The Plan

        checkGUItoRefineMatch(thePlan, _ourplayers);
        ourPlayOff->setMasterPlan(thePlan);
        ourPlayOff->analyseShoot(); // should call after setmasterplan
        ourPlayOff->analysePass();  // should call after setmasterplan
        ourPlayOff->setInitial(true);
        ourPlayOff->lockAgents = true;
        //        lastPlan = thePlan;
        //        debug(QString("chosen plan is %1").arg(lastPlan->gui.index[3]), D_MAHI);


        ROS_INFO_STREAM("initStaticPlay: Done :) response: %s" << str);
    } else {
        ROS_INFO("initStaticPlay: ERROR");
    }

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


void CCoach::matchPlan(NGameOff::SPlan *_plan, const QList<int>& _ourplayers) {
    MWBM matcher;
    matcher.create(_plan->common.currentSize-1, _ourplayers.size()-1);

    int matchedID=-1;
    double weight=0;
    double minweight=100;

    if(_plan->common.currentSize>2) {
        for (int j = 0; j < _ourplayers.size(); j++) {

            weight = agents[_ourplayers.at(j)]->pos().dist(wm->ball->pos);
            if(weight < minweight) {
                minweight = weight;
                matchedID = j;
            }
        }
        ROS_INFO_STREAM("nanapasser:"<<_ourplayers.at(matchedID)<<"__"<<matchedID);
        if(matchedID!=-1) {
            _plan->common.matchedID.insert(0, _ourplayers.at(matchedID));
        }
        for (int i = 1; i < _plan->common.currentSize; i++) {
            for (int j = 0; j < _ourplayers.size(); j++) {
                if (j != matchedID) {
                    double weight;
                    weight = _plan->matching.initPos.agents.at(i).dist(agents[_ourplayers.at(j)]->pos());
                    matcher.setWeight(i-1, j, -(weight));
                    ROS_INFO_STREAM("nanaa:"<<j);

                }
            }
        }
        int nmatchedID=-1;
        qDebug() << "[Coach] matched plan with : " << matcher.findMatching();
        for (size_t i = 1; i < _plan->common.currentSize; i++) {
            nmatchedID = matcher.getMatch(i-1);
            if(nmatchedID>=matchedID)
                nmatchedID++;
            _plan->common.matchedID.insert(i, _ourplayers.at(nmatchedID));
            ROS_INFO_STREAM("nana:"<<_ourplayers.at(nmatchedID)<<"__"<<i<<"__"<<nmatchedID);

        }
        qDebug() << "[Coach] matched by" << _plan->common.matchedID;
    }
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
    if (conf.useGoalieInPlayoff && gameState->ourPlayOffKick() && wm->ball->pos.x > 1) {
        preferedGoalieID = -1;

    } else {
        if (conf.GoalieFromGUI) {
            preferedGoalieID = conf.Goalie;
        } else {
            preferedGoalieID = wm->our.data->goalieID;
        }
    }
    if (gameState->timeOut() || gameState->halfTime()) preferedGoalieID = -1;
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
    //    ROS_INFO_STREAM("hamid: finaldef: " << max_number << ", "<< min_number);
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
            //            ROS_INFO_STREAM("hamid: r " << task_name);

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
