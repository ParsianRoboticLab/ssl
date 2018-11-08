#include <utility>

#include <parsian_ai/plays/playoff.h>

CPlayOff::CPlayOff() : CMasterPlay() {

    ROS_INFO("Bring yourself back online playoff");

    mode = POMode::None;
    playOnFlag = false;
    selectedPlayoff = nullptr;
    firstPlayoff = new CFirstPlayOff;
    staticPlayOff = new CStaticPlayOff;
    dynamicPlayoff = new CDynamicPlayOff;

}

CPlayOff::~CPlayOff() {
    delete staticPlayOff;
    delete dynamicPlayoff;
    delete firstPlayoff;
}

void CPlayOff::reset() {

    playOnFlag = false;
    executedCycles = 0;
    mode = POMode::None;
    firstPlayoff->reset();
    dynamicPlayoff->reset();
    staticPlayOff->reset();
}

void CPlayOff::init(QList<Agent*>& _agents) {
    agents.clear();
    agents.append(_agents);
    initMaster();
    decideMode(agents.size());
    reInitPlay();

}

void CPlayOff::execute_x() {
    ROS_INFO_STREAM("MODEL:" << static_cast<int>(mode));
    selectedPlayoff->execute();
    playOnFlag = selectedPlayoff->getPlayonFlag();
    decideMode(agents.size());

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

void CPlayOff::decideMode(const int& _agentSize) {

    switch (mode) {
        case POMode::None:
            if (agents.size() < 2 || wm->ball->pos.x < 0) mode = POMode::Dynamic;
            else if (conf.UseFirstPlay) mode = POMode::First;
            else mode = POMode::Static;
            break;
        case POMode::First:
            if (firstPlayoff->isFirstFinished()) {
                mode = POMode::Static;
                selectedPlayoff = staticPlayOff;
                reInitPlay();
            } else {
                selectedPlayoff = firstPlayoff;
            }
            break;
        case POMode::Static:
            if (!gotPlan) {
                mode = POMode::Dynamic;
                selectedPlayoff = dynamicPlayoff;
                reInitPlay();
            } else {
                selectedPlayoff = staticPlayOff;
            }
            break;
        case POMode::Dynamic:
            selectedPlayoff = dynamicPlayoff;
            break;
    }

    switch (mode) {
        case POMode::None:
            selectedPlayoff = nullptr;
            break;
        case POMode::First:
            selectedPlayoff = firstPlayoff;
            break;
        case POMode::Static:
            selectedPlayoff = staticPlayOff;
            break;
        case POMode::Dynamic:
            selectedPlayoff = dynamicPlayoff;
            break;
    }


}

void CPlayOff::setPlanClient(ros::ServiceClientPtr _client) {
    client = std::move(_client);
}

void CPlayOff::reInitPlay() {
    unsigned char p_mode = (gameState->getState() == States::OurKickOff) ? parsian_ai_plan_request::KICKOFF
                                                                         : parsian_ai_plan_request::INDIRECT;

    selectedPlayoff->init(agents);
    if(mode == POMode::Static) {
        parsian_msgs::plan_service srv{};
        srv.request.plan_req = getRequest(agents.size(), p_mode);
        gotPlan = true;
        if (client->call(srv)) setResponse(srv.response.the_plan);
        else gotPlan = false;
    }
}
