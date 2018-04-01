// auto-generated don't edit !

#include <parsian_util/action/autogenerate/kickaction.h>

KickAction::KickAction() {
       tolerance = 0.0;
       chip = false;
       iskickchargetime = false;
       kickchargetime = 0.0;
       kickSpeed = 0.0;
       chipDist = 0.0;
       spin = 0;
       slow = false;
       avoidPenaltyArea = false;
       avoidOppPenaltyArea = false;
       interceptMode = false;
       dontKick = false;
       sagMode = false;
       penaltyKick = false;
       shotEmptySpot = false;
       passProfiler = false;
       veryFine = false;
       goalieMode = false;
       kickAngTol = 0.0;
       kickWithCenterOfDribbler = false;
       playMakeMode = false;
       isPlayoff = false;
       isKhafan = false;
}

void KickAction::setMessage(const void* _msg) {
    parsian_msgs::parsian_skill_kick msg = *((parsian_msgs::parsian_skill_kick*)_msg);
        tolerance = msg.tolerance;
        chip = msg.chip;
        iskickchargetime = msg.iskickchargetime;
        kickchargetime = msg.kickchargetime;
        kickSpeed = msg.kickSpeed;
        chipDist = msg.chipDist;
        spin = msg.spin;
        slow = msg.slow;
        avoidPenaltyArea = msg.avoidPenaltyArea;
        avoidOppPenaltyArea = msg.avoidOppPenaltyArea;
        interceptMode = msg.interceptMode;
        dontKick = msg.dontKick;
        sagMode = msg.sagMode;
        penaltyKick = msg.penaltyKick;
        shotEmptySpot = msg.shotEmptySpot;
        passProfiler = msg.passProfiler;
        veryFine = msg.veryFine;
        goalieMode = msg.goalieMode;
        kickAngTol = msg.kickAngTol;
        kickWithCenterOfDribbler = msg.kickWithCenterOfDribbler;
        playMakeMode = msg.playMakeMode;
        isPlayoff = msg.isPlayoff;
        isKhafan = msg.isKhafan;
        target = msg.target;

}

void* KickAction::getMessage() {
    parsian_msgs::parsian_skill_kick* _msg = new parsian_msgs::parsian_skill_kick;
    _msg->tolerance = tolerance;
    _msg->chip = chip;
    _msg->iskickchargetime = iskickchargetime;
    _msg->kickchargetime = kickchargetime;
    _msg->kickSpeed = kickSpeed;
    _msg->chipDist = chipDist;
    _msg->spin = spin;
    _msg->slow = slow;
    _msg->avoidPenaltyArea = avoidPenaltyArea;
    _msg->avoidOppPenaltyArea = avoidOppPenaltyArea;
    _msg->interceptMode = interceptMode;
    _msg->dontKick = dontKick;
    _msg->sagMode = sagMode;
    _msg->penaltyKick = penaltyKick;
    _msg->shotEmptySpot = shotEmptySpot;
    _msg->passProfiler = passProfiler;
    _msg->veryFine = veryFine;
    _msg->goalieMode = goalieMode;
    _msg->kickAngTol = kickAngTol;
    _msg->kickWithCenterOfDribbler = kickWithCenterOfDribbler;
    _msg->playMakeMode = playMakeMode;
    _msg->isPlayoff = isPlayoff;
    _msg->isKhafan = isKhafan;
    _msg->target = target.toParsianMessage();
    return _msg;

}


QString KickAction::getActionName(){
    return SActionName();
}

QString KickAction::SActionName(){
    return QString{"KickAction"};
}

