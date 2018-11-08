#include "parsian_ai/plays/theirkickoff.h"
#include "parsian_ai/roles/playmake.h"
#include "parsian_ai/soccer.h"

CTheirKickOff::CTheirKickOff() = default;

CTheirKickOff::~CTheirKickOff() = default;

void CTheirKickOff::reset() {
    executedCycles = 0;
}

void CTheirKickOff::init(QList<Agent*>& _agents) {
    agents = _agents;
    initMaster();
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
