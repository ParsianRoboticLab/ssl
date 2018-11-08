#include "parsian_ai/plays/theirballplacement.h"

CTheirBallPlacement::CTheirBallPlacement() : CMasterPlay() {

}

CTheirBallPlacement::~CTheirBallPlacement() {

}

void CTheirBallPlacement::reset() {

}

void CTheirBallPlacement::init(QList<Agent*>& _agents) {
    agents = _agents;
    initMaster();
}


void CTheirBallPlacement::execute_x() {

}
