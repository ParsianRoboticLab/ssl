#include "parsian_ai/plays/theirkickoff.h"
#include "parsian_ai/roles/playmake.h"
#include "parsian_ai/soccer.h"

CTheirKickOff::CTheirKickOff() = default;

CTheirKickOff::~CTheirKickOff() = default;

void CTheirKickOff::reset() {
    executedCycles = 0;
}

void CTheirKickOff::init(const QList<Agent*>& _agents) {
    setAgentsID(_agents);
    initMaster();

//  if( knowledge->getLastPlayExecuted() != TheirKickOffPlay ){
//    reset();
//  }
//  knowledge->setLastPlayExecuted(TheirKickOffPlay);
}

void CTheirKickOff::execute_x() {
    if (agents.empty()) {
        return;
    }
    executedCycles++;
    chooseBlocker();
    if (agents.size() > 1) {
        appendRemainingsAgents(markAgents);
    }
}
