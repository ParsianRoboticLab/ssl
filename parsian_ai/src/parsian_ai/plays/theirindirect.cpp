#include "parsian_ai/plays/theirindirect.h"
#include "parsian_ai/roles/playmake.h"
#include "parsian_ai/soccer.h"

CTheirIndirect::CTheirIndirect() : CMasterPlay() {

}

CTheirIndirect::~CTheirIndirect() {

}

void CTheirIndirect::reset() {
    executedCycles = 0;
}

void CTheirIndirect::init(QList<Agent*>& _agents) {
    agents = _agents;
    initMaster();
}

void CTheirIndirect::execute_x() {
    if (agents.empty()) {
        return;
    }
    executedCycles++;
    chooseBlocker();
    if (agents.size() > 1) {
        appendRemainingsAgents(markAgents);
    }
}