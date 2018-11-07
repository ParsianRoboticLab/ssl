#include <parsian_ai/plays/playoff.h>

CPlayOff::CPlayOff() : CMasterPlay() {

    ROS_INFO("Bring yourself back online playoff");

    mode = POMode::None;
    playOnFlag = false;

}

CPlayOff::~CPlayOff() {

}

void CPlayOff::reset() {

    playOnFlag = false;
    executedCycles = 0;
}

void CPlayOff::init(const QList<Agent*>& _agents) {
    setAgents(_agents);
    initMaster();

}

void CPlayOff::execute_x() {
}

parsian_ai_plan_request CPlayOff::getRequest(const int &_agentSize, const unsigned char &mode) {
    parsian_ai_plan_request req{};
    req.gameMode = mode;
    req.ballPos.x = wm->ball->pos.x;
    req.ballPos.y = wm->ball->pos.y;
    req.playersNum = static_cast<unsigned char>(_agentSize);
    return req;
}

void CPlayOff::setResponse(const parsian_plan &_plan) {
    staticPlayOff->parsePlan(_plan);
    lockAgents = true;
}

POMode CPlayOff::decideMode(const int& _agentSize, const unsigned char& _mode) {
    if (_agentSize < 2) {
        mode = POMode::Dynamic;
    } else if (gameState->ourKickoff() && !gameState->canKickBall()) {
        mode = POMode::First;

    } else if ((wm->ball->pos.x < 1 && !gameState->ourKickoff())|| !gotPlan) {
        mode = POMode::Dynamic;

    } else if (!firstIsFinished && conf.UseFirstPlay) {
        mode = POMode::First;

    } else if (wm->ball->pos.x > -1) {
        mode = POMode::Static;

    } else {
        mode = POMode::Dynamic;
    }
    return mode;
}