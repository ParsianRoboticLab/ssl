#include <parsian_ai/gamestate.h>

GameState *gameState;

GameState::GameState()
{
    state= States::Stop;
    ourScore = 0;
    theirScore = 0;
    command_ctr = 0;
}

int GameState::getOurScore(){return ourScore;}
int GameState::getTheirScore(){ return theirScore;}

void GameState::setRefree(ssl_refree_wrapperConstPtr ref_wrapper) {


    if(ref_wrapper->command_counter == command_ctr)
        return;
    command_ctr = ref_wrapper->command_counter;
    ///////////////////// when we are ready any command means force start
    if(isReady && (state != States::Start) && (ref_wrapper->command.command != ssl_refree_command::HALT)
            && (ref_wrapper->command.command != ssl_refree_command::STOP)){
        state = States::Stop;
        isReady = false;
        return;
    }

    switch (ref_wrapper->stage.stage) {
    case ssl_refree_stage::NORMAL_HALF_TIME:
    case ssl_refree_stage::EXTRA_HALF_TIME:
        state = States::HalfTime;
        return;

    case ssl_refree_stage::POST_GAME:
        state = States::PostGame;
        return;
        default:break;
    }

    switch (ref_wrapper->command.command) {
    case ssl_refree_command::FORCE_START:
        state = States::Start;
        isReady=false;
        return;
    case ssl_refree_command::HALT:
        state = States::Halt;
        isReady=false;
        return;

    case ssl_refree_command::STOP:
        state = States::Stop;
        isReady=false;
        return;

    case ssl_refree_command::NORMAL_START:
        isReady=true;
        return;

        //####################################################
    case ssl_refree_command::BALL_PLACEMENT_THEM:
        isReady=true;
        state = States::TheirBallPlacement;
        return;
    case ssl_refree_command::BALL_PLACEMENT_US:
        isReady=true;
        state = States::OurBallPlacement;
        return;

    case ssl_refree_command::DIRECT_FREE_THEM:
        isReady=true;
        state = States::TheirDirectKick;
        return;
    case ssl_refree_command::DIRECT_FREE_US:
        isReady=true;
        state = States::OurDirectKick;
        return;

    case ssl_refree_command::GOAL_THEM:
        theirScore++;
        return;
    case ssl_refree_command::GOAL_US:
        ourScore++;
        return;

    case ssl_refree_command::INDIRECT_FREE_THEM:
        isReady=true;
        state = States::TheirIndirectKick;
        return;
    case ssl_refree_command::INDIRECT_FREE_US:
        isReady=true;
        state = States::OurIndirectKick;
        return;

    case ssl_refree_command::PREPARE_KICKOFF_THEM:
        isReady=false;
        state = States::TheirKickOff;
        return;
    case ssl_refree_command::PREPARE_KICKOFF_US:
        isReady=false;
        state = States::OurKickOff;
        return;

    case ssl_refree_command::PREPARE_PENALTY_THEM:
        isReady=false;
        state= ((ref_wrapper->stage.stage==ssl_refree_stage::PENALTY_SHOOTOUT)?
                States::TheirPenaltyShootOut : States::TheirPenaltyKick);
        return;
    case ssl_refree_command::PREPARE_PENALTY_US:
        isReady=false;
        state= (ref_wrapper->stage.stage==ssl_refree_stage::PENALTY_SHOOTOUT)?
               States::OurPenaltyShootOut : States::OurPenaltyKick;
        return;
        default:break;
    }

}

bool GameState::isStart() { return state == States::Start; }
bool GameState::isStop(){ return state == States::Stop;}
bool GameState::canMove()  { return state != States::Halt; }

bool GameState::allowedNearBall() {return isStart() || ourPlayOffKick(); }
bool GameState::canKickBall() {return isStart() || (ourPlayOffKick() && isReady); }

bool GameState::playOffKick() { return (state >= States::OurBallPlacement ); }
bool GameState::ourPlayOffKick() { return playOffKick() &&  state< States::TheirBallPlacement ;}
bool GameState::theirPlayOffKick() { return state>= States::TheirBallPlacement; }

bool GameState::kickoff() { return (ourKickoff() || theirKickoff()); }
bool GameState::ourKickoff() { return (state == States::OurKickOff); }
bool GameState::theirKickoff() { return (state == States::TheirKickOff); }

bool GameState::penaltyKick() { return ourPenaltyKick()|| theirPenaltyKick(); }
bool GameState::ourPenaltyKick() { return (state==States::OurPenaltyKick); }
bool GameState::theirPenaltyKick() { return (state==States::TheirPenaltyKick); }

bool GameState::directKick() { return (ourDirectKick() || theirDirectKick()); }
bool GameState::ourDirectKick() { return (state == States::OurDirectKick); }
bool GameState::theirDirectKick() { return (state== States::TheirDirectKick); }

bool GameState::indirectKick() { return ourIndirectKick() || theirIndirectKick(); }
bool GameState::ourIndirectKick() { return (state == States::OurIndirectKick) ;}
bool GameState::theirIndirectKick() { return (state == States::TheirIndirectKick); }

bool GameState::freeKick() { return directKick() || indirectKick(); }
bool GameState::ourFreeKick() { return ourDirectKick() || ourIndirectKick(); }
bool GameState::theirFreeKick() { return theirDirectKick() || theirIndirectKick(); }

bool GameState::ballPlacement() { return (ourBallPlacement() || theirBallPlacement()); }
bool GameState::ourBallPlacement() { return (state == States::OurBallPlacement); }
bool GameState::theirBallPlacement() { return (state == States::TheirBallPlacement); }

bool GameState::halfTimeLineUp(){return state == States::HalfTime;}

bool GameState::penaltyShootout() {return ourPenaltyShootout()|| theirPenaltyShootout();}
bool GameState::ourPenaltyShootout(){return state == States::OurPenaltyShootOut;}
bool GameState::theirPenaltyShootout(){return state == States::TheirPenaltyShootOut;}
bool GameState::ready(){
    return isReady;
}

/////
States GameState::getState(){
    return state;
}

void GameState::setState(const States &s, bool isReady) {
    state = s;
    this->isReady = isReady;
}
