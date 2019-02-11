#include "parsian_ai/plays/substitution.h"

CSubstitution::CSubstitution() {

}

CSubstitution::~CSubstitution() {

}

void CSubstitution::reset(){
    positioningPlan.reset();
    executedCycles = 0;
}

void CSubstitution::init(QList<Agent*>& _agents) {
    agents = _agents;
    initMaster();
}


void CSubstitution::execute_x(){
    for(auto agent: agents)
        ROS_INFO_STREAM("kianf : " << agent->id());
    ROS_INFO_STREAM("kianf : ----------------------------");
}
