//
// Created by atiyeh on 3/27/19.
//

#include "parsian_agent/moveforward.h"
#include "parsian_agent/kick.h"

CSkillMoveForward::CSkillMoveForward(Agent *_agent) : CSkill(_agent) {
    gtpAvoid = new CSkillGotoPointAvoid(_agent);
    recPass = new CSkillReceivePass(_agent);
    kick = new CSkillKick(_agent);}

CSkillMoveForward::~CSkillMoveForward(){
    delete gtpAvoid;
    delete recPass; }

void CSkillMoveForward::kickForward(double kickspeed) {
    setKickspeed(kickspeed);
    bool kickerOn;

    if(kickspeed != 0.0) {
        setSpin(static_cast<float>(0.7));
    };
}

MFMode CSkillMoveForward::decideMode() {

}


void CSkillMoveForward::execute() {
    MFMode moveForwardMode = decideMode();
    switch (moveForwardMode){
        case MFMode::MFNONE:
            break;
        case MFMode::RECEIVE:
            recPass->execute();
            break;
        case MFMode::KICKFORWARD:
            kickForward(1.0); //checkout
            break;
        case MFMode::KICK:
            kick->execute();
            break;
    };}



/*int main(int argc, char **argv){
    ros::init(argc, argv, "moveForward");
    ros::NodeHandle n;
    ros::Publisher do_kik = n.advertise<parsian_msgs::parsian_skill_kick>("kikspeed",10);
    ros::Publisher do_spin = n.advertise<parsian_msgs::parsian_robot_command>("kikspeed",10);
    ros::Rate loop_rate(10);
    int count{};
    while(ros::ok()){
        parsian_msgs::parsian_skill_kick kick;
        kick.kickSpeed = 0.1;
        parsian_msgs::parsian_robot_command command;
        command.robot_id = 0;
        command.spinner = 1;
        command.roller_speed = 1.0;
        do_kik.publish(kick);
        do_spin.publish(command);
        ros::spinOnce();
        loop_rate.sleep();
        ++count;
    }
    return 0;
}*/

