// game_state.h

#ifndef __game_state_h__
#define __game_state_h__

#include <parsian_msgs/ssl_refree_wrapper.h>
#include <parsian_msgs/ssl_refree_command.h>
#include <parsian_msgs/ssl_refree_stage.h>


enum class States {
    Halt = 0,
    Stop = 1,
    Start = 2,

    HalfTime = 3,
    PostGame = 4,

    OurBallPlacement = 5, /// start of restart & our
    OurDirectKick = 6,
    OurIndirectKick = 7,
    OurPenaltyKick = 8,
    OurPenaltyShootOut = 9,
    OurKickOff = 10,
    OurTimeOut = 11,

    TheirBallPlacement = 12,/// start of their
    TheirDirectKick = 13,
    TheirIndirectKick = 14,
    TheirPenaltyKick = 15,
    TheirPenaltyShootOut = 16,
    TheirKickOff = 17,
    TheirTimeOut = 18




};



using namespace  parsian_msgs;

class GameState {
private:
    int command_ctr;
    States state;
    int ourScore, theirScore;
    bool isReady;
public:



    GameState();
    bool ready();
    void setRefree(ssl_refree_wrapperConstPtr ref_wrapper);
    bool isStart();
    bool isStop();
    bool playOffKick();
    bool ourPlayOffKick();
    bool theirPlayOffKick();
    bool kickoff();
    bool ourKickoff();
    bool theirKickoff();
    bool penaltyKick();
    bool ourPenaltyKick();
    bool theirPenaltyKick();
    bool directKick();
    bool ourDirectKick();
    bool theirDirectKick();
    bool indirectKick();
    bool ourIndirectKick();
    bool theirIndirectKick();
    bool freeKick();
    bool ourFreeKick();
    bool theirFreeKick();
    bool canMove();
    bool allowedNearBall();
    bool canKickBall();
    int  getOurScore();
    int  getTheirScore();
    bool ballPlacement();
    bool ourBallPlacement();
    bool theirBallPlacement();
    bool halfTime();
    bool penaltyShootout();
    bool ourPenaltyShootout();
    bool theirPenaltyShootout();
    bool ourTimeOut();
    bool theirTimeOut();
    bool timeOut();
    void setReady(bool ready);
    States getState();
    void setState(const States& s, bool isReady = false);
};
extern GameState *gameState;
#endif
