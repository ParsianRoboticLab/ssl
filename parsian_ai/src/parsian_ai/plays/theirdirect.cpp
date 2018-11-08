#include "parsian_ai/plays/theirdirect.h"

CTheirDirect::CTheirDirect() {
}

CTheirDirect::~CTheirDirect() {

}

void CTheirDirect::reset() {
    executedCycles = 0;

}

void CTheirDirect::init(QList<Agent*>& _agents) {
    agents = _agents;
    initMaster();
}

void CTheirDirect::execute_x() {
    if (agents.empty()) {
        return;
    }
    executedCycles++;
    chooseBlocker();
    if (agents.size() > 1) {
        appendRemainingsAgents(markAgents);
    }
}
