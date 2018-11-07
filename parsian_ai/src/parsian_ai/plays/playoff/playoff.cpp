#include <utility>

#include <parsian_ai/plays/playoff.h>

CPlayOff::CPlayOff() : CMasterPlay() {

    ROS_INFO("Bring yourself back online playoff");

    mode = POMode::None;
    playOnFlag = false;

    staticPlayOff = new CStaticPlayOff;
    dynamicPlayoff = new CDynamicPlayOff;
    firstPlayoff = new CFirstPlayOff;

}

CPlayOff::~CPlayOff() {
    delete staticPlayOff;
    delete dynamicPlayoff;
    delete firstPlayoff;
}

void CPlayOff::reset() {

    playOnFlag = false;
    executedCycles = 0;
}

void CPlayOff::init() {
    initMaster();
    unsigned char p_mode = (gameState->getState() == States::OurKickOff) ? parsian_ai_plan_request::KICKOFF
                                                                         : parsian_ai_plan_request::INDIRECT;
    decideMode(agents.size(), p_mode);
    switch (mode) {
        case POMode::None:
            break;
        case POMode::First:
            firstPlayoff->init(agents);
            break;
        case POMode::Dynamic:
            dynamicPlayoff->init(agents);
            break;
        case POMode::Static:
            parsian_msgs::plan_service srv{};
            srv.request.plan_req = getRequest(agents.size(), p_mode);
            if (client->call(srv)) setResponse(srv.response.the_plan);
            staticPlayOff->init(agents);
            break;
    }

}

void CPlayOff::execute_x() {
    switch (mode) {
        case POMode::None:
            break;
        case POMode::First:
            firstPlayoff->execute();
            break;
        case POMode::Dynamic:
            dynamicPlayoff->execute();
            break;
        case POMode::Static:
            staticPlayOff->execute();
            break;
    }
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

void CPlayOff::decideMode(const int& _agentSize, const unsigned char& _mode) {

    switch (mode) {

        case POMode::None:break;
        case POMode::First:break;
        case POMode::Static:break;
        case POMode::Dynamic:break;

    }


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
}

POMode CPlayOff::getMode() {
    return mode;
}

void CPlayOff::setPlanClient(ros::ServiceClientPtr _client) {
    client = std::move(_client);
}
