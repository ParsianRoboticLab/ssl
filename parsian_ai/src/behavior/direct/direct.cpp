#include <behavior/direct/direct.h>

BehaviorDirect::BehaviorDirect()
: Behavior("Direct", "describe me") {
    kicker = nullptr;
    kick = new KickAction;
    for (auto& g : gpa) g = new GotopointavoidAction;
}

BehaviorDirect::~BehaviorDirect() {

}

int BehaviorDirect::execute() {
    if (agents.empty()) return 0;
    findKicker();
    generateFormation();
    getFormation();
    assignKick(kicker);


    return 0;
}

void BehaviorDirect::findKicker() {
    for (auto& agent : agents){
        if (agent->id() == 0) kicker = agent;
    }
    return;
    for (auto& role : msg->matching) {
        if (role.role == role.PLAYMAKE) {
            int id = role.id;
            for (auto& agent : agents){
                if (agent->id() == id) kicker = agent;
            }
        }
    }
}

double BehaviorDirect::eval(parsian_msgs::parsian_behaviorPtr _behav) {
    _behav = nullptr;
    return 0.0;
}

void BehaviorDirect::assignKick(Agent* const _kicker) {
    if (_kicker != nullptr) {
        kick->setKickspeed(650); // TODO : Change it to 6.5 after profiler merge
        kick->setChip(false);
        kick->setAvoidpenaltyarea(true);
        kick->setAvoidopppenaltyarea(true);
        kick->setTarget(Vector2D(0,0)); //// just for run not a real target
        kick->setShotemptyspot(true); // TODO : Can find a better target
        kicker->action = kick;
    }
}

//// Find Best Position to place Positionin
void BehaviorDirect::generateFormation() {

    for (int i = 0; i < agents.size(); i++) {
        if (agents[i] == kicker) continue;
        gpa[i]->setAvoidpenaltyarea(true);
        Vector2D pos;
        double degree = 300 / (agents.size() - 1) * i + 40;
        pos.setPolar(3, degree);
        gpa[i]->setTargetpos(pos + kicker->pos());
        gpa[i]->setLookat(wm->field->oppGoal());
    }
    match(agents, gpa);

}

//// Static Formation
void BehaviorDirect::getFormation() {
    // TODO : READ FROM FILE
}

void BehaviorDirect::match(QList<Agent*>& _agents, GotopointavoidAction* _gpa[]) {
    MWBM matcher;
    matcher.create(_agents.size(), _agents.size());
    for (int i = 0; i < _agents.size(); i++) {
        if (_agents[i] ==  kicker) continue;
        for (int j = 0; j < _agents.size(); j++) {
            matcher.setWeight(i, j, -_agents[i]->pos().dist(_gpa[j]->getTargetpos()));
        }
    }
    matcher.findMatching();
    for (int i = 0; i < _agents.size(); i++) {
        if (_agents[i] == kicker) continue;
        _agents[i]->action = _gpa[matcher.getMatch(i)];
    }
}

