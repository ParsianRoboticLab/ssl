#include "parsian_ai/plays/theirpenalty.h"
#include "parsian_ai/roles/playmake.h"
#include "parsian_ai/soccer.h"

CTheirPenalty::CTheirPenalty() {
}

CTheirPenalty::~CTheirPenalty() {

}

void CTheirPenalty::reset() {
    positioningPlan.reset();
    executedCycles = 0;
}

void CTheirPenalty::init(const QList<Agent*>& _agents) {
    setAgentsID(_agents);
    initMaster();

//  if( knowledge->getLastPlayExecuted() != TheirPenaltyPlay ){
//    reset();
//  }
//  knowledge->setLastPlayExecuted(TheirPenaltyPlay);
}

void CTheirPenalty::execute_x() {
    if (agents.empty()) {
        return;
    }
    executedCycles++;
    if (gameState->penaltyShootout()) {
        choosePlayMaker();
    }
    appendRemainingsAgents(positionAgents);
    setFormation("TheirP");

}
