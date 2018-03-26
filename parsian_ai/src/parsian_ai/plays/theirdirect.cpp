#include "parsian_ai/plays/theirdirect.h"

CTheirDirect::CTheirDirect() {
}

CTheirDirect::~CTheirDirect() {

}

void CTheirDirect::reset() {
    executedCycles = 0;

}

void CTheirDirect::init(const QList<Agent*>& _agents) {
    setAgentsID(_agents);
    initMaster();

//  if( knowledge->getLastPlayExecuted() != TheirDirectPlay ){
//    reset();
//  }
//  knowledge->setLastPlayExecuted(TheirDirectPlay);
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
