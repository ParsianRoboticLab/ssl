#include <parsian_ai/soccer.h>
#include <parsian_ai/plays/masterplay.h>

//PositioningPlan CMasterPlay::positioningPlan;
//CMarkPlan CMasterPlay::markPlan;
//DefensePlan CMasterPlay::defensePlan;
//CRolePlayMake CMasterPlay::playMakeRole{nullptr};
//CRoleBlock CMasterPlay::blockRole{nullptr};

CMasterPlay::CMasterPlay() : playMakeRole(nullptr), blockRole(nullptr) {
    markAgents.clear();
    executedCycles = 0;
    lockAgents = false;
    //defenseN = 2;
}

CMasterPlay::~CMasterPlay() = default;

void CMasterPlay::initMaster() {
    blockAgent = nullptr;
    playMakeAgent = nullptr;
    positionAgents.clear();
//    if (gameState->isStart()) {
//        markAgents.clear();
//    }
    stopAgents.clear();
    masterStaticPoints.clear();
    staticInited = false;
    //defenseN = 2;
    formationName = "";


}

bool CMasterPlay::canScore() {
    QList<int> ourRelaxed, theirRelaxed;
    theirRelaxed.clear();
    ourRelaxed.clear();
    for (int i = 0 ; i < wm->our.activeAgentsCount() ; i++) {
        ourRelaxed.append(wm->our.activeAgentID(i));
    }
    return  getOpenness(wm->ball->pos, wm->field->oppGoalL(), wm->field->oppGoalR(), ourRelaxed, theirRelaxed) > .2;
}


void CMasterPlay::setAgentsID(const QList<Agent*>& _agentsID) {
    agents.clear();
    agents.append(_agentsID);
}

void CMasterPlay::setFormation(QString _formationName) {
    formationName = std::move(_formationName);
}

void CMasterPlay::setStaticPoints(QList< holdingPoints > _staticPoints) {
    staticInited = true;
    masterStaticPoints = std::move(_staticPoints);
}

void CMasterPlay::resetPositioning() {
    positioningPlan.reset();
}

void CMasterPlay::resetPlayMaker() {
    playMakeRole.resetPlayMake();
}


// This function is critical, any manipulation would
// result in bad bugs!
void CMasterPlay::appendRemainingsAgents(QList<Agent *> &_list) {

    QList <Agent*> remainings;

    for (auto i : agents) {
        remainings.append(i);
    }

    if (blockAgent) {
        remainings.removeOne(blockAgent);
    }
    if (playMakeAgent) {
        remainings.removeOne(playMakeAgent);
    }

    for (auto markAgent : markAgents) {
        remainings.removeOne(markAgent);
    }

    for (auto positionAgent : positionAgents) {
        remainings.removeOne(positionAgent);
    }

    for (auto remaining : remainings) {
        _list.append(remaining);
    }
}

void CMasterPlay::choosePlayMaker() {
    int playMakeID = -1;
    QList <Agent *> playAgents;
    playAgents.clear();
    for (auto i : agents) {
        playAgents.append(i);
    }


//    for( int i=0 ; i<knowledge->agentsWithIntention.size() ; i++ ){
//        int id = knowledge->agentsWithIntention.at(i);
//        if( agents.contains(id) ){
//            Agent *agnt = knowledge->getAgent(id);
//            if( agnt->intention->M_type == "playmake" ){
//                playMakeID = id;
//            }
//        }
//    }

    if (playMakeID == -1) {
//        playMakeID = knowledge->newFastestSelector(playAgents);
//        if( playMakeID != -1 ){
//            Agent *agnt = knowledge->getAgent(playMakeID);
//            if( agnt != NULL ){
//                agnt->playMakeIntent.assign(playMakeID , knowledge->frameCount);
//                agnt->intention = &agnt->playMakeIntent;
//            }
//        }
    }

    ///////////////////////////////////////

    ///////////////////////////////////////

    if (playMakeID != -1) {
        playMakeAgent = soccer->agents[playMakeID];
    }
}

void CMasterPlay::chooseBlocker() {

    int blockID = -1;
    QList <Agent *> playAgents;
    playAgents.clear();
    for (auto i : agents) {
        playAgents.append(i);
    }

//    for( int i=0 ; i<knowledge->agentsWithIntention.size() ; i++ ){
//        int id = knowledge->agentsWithIntention.at(i);
//        if( agents.contains(id) ){
//            Agent *agnt = knowledge->getAgent(id);
//            if( agnt->intention->M_type == "block" ){
//                blockID = id;
//            }
//        }
//    }

    if (blockID == -1) {
//        blockID = knowledge->newFastestSelector(playAgents);
//        if( blockID != -1 ){
//            Agent *agnt = knowledge->getAgent(blockID);
//            agnt->blockIntent.assign(blockID , knowledge->frameCount);
//            agnt->intention = &agnt->blockIntent;
//        }
    }

    if (blockID != -1) {
        blockAgent = soccer->agents[blockID];
    }
}


bool CMasterPlay::canOneTouch(QList<Agent *> posAgents, Agent *playMake) {
    for (auto posAgent : posAgents) {
        QList<int> ourRelaxIDS, oppRelaxIDS;
        int positioner = posAgent->id();
        ourRelaxIDS.push_back(positioner);
        Vector2D pos = posAgent->pos() + posAgent->dir().setLengthVector(Robot::center_from_kicker_new);
        if (getOpenness(pos, wm->field->oppGoalL(), wm->field->oppGoalR(), ourRelaxIDS, oppRelaxIDS) > 0.8) {
            return true;
        }
    }
    return false;
}

double CMasterPlay::getOpenness(Vector2D from, Vector2D p1, Vector2D p2, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs) {
    std::priority_queue < QPair< edgeMode , double > , std::vector< QPair< edgeMode , double > > , Comparar > blockedLines;
    double least = (p1 - from).th().degree();
    double most  = (p2 - from).th().degree();
    if (least > most) {
        std::swap(least, most);
    }

    for (int i = 0; i < wm->our.activeAgentsCount(); i++) {
        if (!ourRelaxedIDs.contains(wm->our.active(i)->id)) {
            double obsDeg = (wm->our.active(i)->pos - from).th().degree();
            double obsDist = wm->our.active(i)->pos.dist(from);
            double obsAng = asin(Robot::robot_radius_new / obsDist) * _RAD2DEG;
            double lowerbound = obsDeg - obsAng;
            double upperbound = obsDeg + obsAng;

            if (lowerbound > most || upperbound < least) {
                continue;
            }

            if (upperbound > most) {
                upperbound = most;
            }

            if (lowerbound < least) {
                lowerbound = least;
            }

            blockedLines.push(qMakePair(edgeMode::TOP, upperbound));
            blockedLines.push(qMakePair(edgeMode::BOT, lowerbound));
        }
    }
    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (!oppRelaxedIDs.contains(wm->opp.active(i)->id)) {
            double obsDeg = (wm->opp.active(i)->pos - from).th().degree();
            double obsDist = wm->opp.active(i)->pos.dist(from);
            double obsAng = asin(Robot::robot_radius_new / obsDist) * _RAD2DEG;
            double lowerbound = obsDeg - obsAng;
            double upperbound = obsDeg + obsAng;

            if (lowerbound > most || upperbound < least) {
                continue;
            }

            if (upperbound > most) {
                upperbound = most;
            }

            if (lowerbound < least) {
                lowerbound = least;
            }

            blockedLines.push(qMakePair(edgeMode::TOP, upperbound));
            blockedLines.push(qMakePair(edgeMode::BOT, lowerbound));
        }
    }

    return 1.0 - coveredArea(blockedLines) / (most - least);
}

double CMasterPlay::coveredArea(std::priority_queue < QPair< edgeMode , double > , std::vector< QPair< edgeMode , double > > , Comparar >& obstacles) {
    if (obstacles.size() <= 1) {
        return 0.0;
    }

    QPair< edgeMode , double > lastest = obstacles.top();
    obstacles.pop();
    QPair< edgeMode , double > second_lastest = obstacles.top();

    if (lastest.first == edgeMode::TOP && second_lastest.first == edgeMode::TOP) {
        return (lastest.second - second_lastest.second) + coveredArea(obstacles);
    }

    else if (lastest.first == edgeMode::BOT && second_lastest.first == edgeMode::BOT) {
        return (lastest.second - second_lastest.second) + coveredArea(obstacles);
    }

    else if (lastest.first == edgeMode::TOP && second_lastest.first == edgeMode::BOT) {
        return (lastest.second - second_lastest.second) + coveredArea(obstacles);
    }

    else {
        return coveredArea(obstacles);
    }
}

void CMasterPlay::execute() {

    execute_x();
    for (int i = 0; i < agents.size(); i++) {
        if (agents[i]->isVisible() && agents[i]->action != nullptr) {
            Action *mahi = agents[i]->action;
            ROS_INFO_STREAM(i << ": " << mahi->getActionName().toStdString().c_str());
        }
    }
    execPlay();
    DBUG(QString("MasterPlay size: %1").arg(agents.count()) , D_ERROR);
}

void CMasterPlay::execPlay() {

    if (blockAgent) {
        blockRole.parse();
        blockRole.assign(blockAgent);
        blockRole.setStop(true);
    }

    if (playMakeAgent) {
        if (! gameState->ourDirectKick() && ! gameState->ourIndirectKick()) {
            playMakeRole.assign(playMakeAgent);
        }
    }


    if (!markAgents.empty()) {
        markPlan.init(markAgents);
        markPlan.execute();
    }

    if (gameState->isStop()) {
        if (!positionAgents.empty()) {
            ///////// added to prevent Segmentatino fault! //////////
            DBUG(QString("Invalid Formation Name: %1").arg(formationName) , D_ERROR);
            formationName = "Stop6";
            /////////////////////////////////////////////////////////
            if (staticInited) {
                positioningPlan.staticInit(masterStaticPoints);
            }
            positioningPlan.execute();
        }
    }
}
