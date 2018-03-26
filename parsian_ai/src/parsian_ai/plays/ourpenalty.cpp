#include <parsian_ai/plays/ourpenalty.h>

COurPenalty::COurPenalty() = default;

COurPenalty::~COurPenalty() = default;

void COurPenalty::reset() {
    positioningPlan.reset();
    executedCycles = 0;
}

void COurPenalty::init(const QList<Agent*>& _agents) {
    setAgentsID(_agents);
    initMaster();

    // TODO : Last Play seems important!
//  if( knowledge->getLastPlayExecuted() != OurPenaltyPlay ){
//    reset();
//  }
//  knowledge->setLastPlayExecuted(OurPenaltyPlay);
}

void COurPenalty::penaltyKick() {
    executedCycles++;
    if (gameState->penaltyShootout()) {
        playOnFlag = false;
    }
    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    setFormation("OurP");

}

void COurPenalty::execute_x() {
    if (agents.empty()) {
        return;
    }
    penaltyKick();
}
