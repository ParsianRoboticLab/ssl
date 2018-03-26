#include "parsian_ai/plays/theirballplacement.h"

CTheirBallPlacement::CTheirBallPlacement() : CMasterPlay() {

}

CTheirBallPlacement::~CTheirBallPlacement() {

}

void CTheirBallPlacement::reset() {

}

void CTheirBallPlacement::init(const QList<Agent*>& _agents) {
    setAgentsID(_agents);
    initMaster();

//    if( knowledge->getLastPlayExecuted() != TheirBallPlacement ){
//        reset();
//    }
//    knowledge->setLastPlayExecuted(TheirBallPlacement);
}


void CTheirBallPlacement::execute_x() {

}
