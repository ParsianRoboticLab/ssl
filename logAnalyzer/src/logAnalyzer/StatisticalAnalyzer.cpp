//
// Created by rebinnaf on 9/13/18.
//

#include <logAnalyzer/StatisticalAnalyzer.h>
#include "parsian_util/core/knowledge.h"

StatisticalAnalyzer::StatisticalAnalyzer() {

    wm_sub = n.subscribe("/world_model", 1000, &StatisticalAnalyzer::wmCb, this);
    ref_sub = n.subscribe("/referee", 1000, &StatisticalAnalyzer::refCb, this);
//    ros::NodeHandle n("LogAnalyzer");


    possessionFile.setFileName("Possession.csv");
    if(!possessionFile.open(QIODevice::WriteOnly | QIODevice::Append))
        ROS_INFO_STREAM("Can't open the file to possessionFile");
    else
        ROS_INFO_STREAM("possessionFile file opened :) \n");
    possessionDS.setDevice(&possessionFile);



    shotFile.setFileName("Shot.csv");
    if(!shotFile.open(QIODevice::WriteOnly | QIODevice::Append))
        ROS_INFO_STREAM("Can't open the file to shotFile");
    else
        ROS_INFO_STREAM("shotFile file opened :) \n");
    shotDS.setDevice(&shotFile);


    passFile.setFileName("Pass.csv");
    if(!passFile.open(QIODevice::WriteOnly | QIODevice::Append))
        ROS_INFO_STREAM("Can't open the file to passFile");
    else
        ROS_INFO_STREAM("passFile file opened :) \n");
    passDS.setDevice(&passFile);

    possessionFile.close();
    shotFile.close();
    passFile.close();
}
StatisticalAnalyzer::~StatisticalAnalyzer() {

}


void StatisticalAnalyzer::refCb(const parsian_msgs::ssl_refree_wrapperConstPtr & _ref) {
    ref=_ref;
    refcommand=ref->command.command;
    ROS_INFO_STREAM("REEEF"<<ref->command<<"__");
//    ROS_INFO_STREAM("REEEF"<<ref->us.name<<"__"<<ref->them.name);

}

void StatisticalAnalyzer::wmCb(const parsian_msgs::parsian_world_modelConstPtr &_wm) {
    wm = _wm;
    updatewm();
    if(isPlayingTime())
        preprocess();

}
void StatisticalAnalyzer::preprocess(){
    if(validShot())
        writeToShot();
    else if(validPass())
        writeToPass();

    if(validPossession())
        writeToPossession();
}


bool StatisticalAnalyzer::validShot(){

    if(!shotFlag || ballvel.length()<0.5) {
        shotterRobot.x = wm->opp.at(oppBPID).pos.x;
        shotterRobot.y = wm->opp.at(oppBPID).pos.y;
        shotDir.x = wm->opp.at(oppBPID).dir.x;
        shotDir.y = wm->opp.at(oppBPID).dir.y;
        shotTarget = Line2D(shotterRobot, shotterRobot + shotDir).intersection(
                Line2D(field.ourCornerL(), field.ourCornerR()));
    }
    //robot points to Goal
    if(shotTarget.y<1.2
       && shotTarget.y>-1.2
       && sign(shotDir.x)==sign(field.ourGoal().x)) {
        shotFlag = true;
    } else{
        shotFlag=false;
    }
    //ball points to Goal
    if(ballvel.length()>2
       && sign(ballvel.x)==sign(field.ourGoal().x)
       && shotFlag){


        //determine if shot is in the goal area
        Vector2D target=Line2D(ballPos,ballPos+ballvel).intersection(Line2D(field.ourCornerL(),field.ourCornerR()));
        if(target.y<0.8 && target.y>-0.8
                && fabs(ballPos.x-field.ourGoal().x)<1){
            shotInGoal=true;
        } else {
            shotInGoal=false;
        }

        return true;
    }
    return false;
}


bool StatisticalAnalyzer::validPass(){

    passerRobot.x=wm->opp.at(oppBPID).pos.x;
    passerRobot.y=wm->opp.at(oppBPID).pos.y;
    passDir.x=wm->opp.at(oppBPID).dir.x;
    passDir.y=wm->opp.at(oppBPID).dir.y;
    return false;

}


bool StatisticalAnalyzer::validPossession(){
    //ball speed limit
    if(ballvel.length()>2){
        return false;
    }
    else return true;
}


bool StatisticalAnalyzer::isPlayingTime() {
    if(refcommand==ref->command.STOP
       ||refcommand==ref->command.HALT
       || refcommand==ref->command.BALL_PLACEMENT_THEM
       || refcommand==ref->command.BALL_PLACEMENT_US
       || refcommand==ref->command.PREPARE_PENALTY_US
       || refcommand==ref->command.PREPARE_PENALTY_THEM
       || refcommand==ref->command.TIMEOUT_US
       || refcommand==ref->command.TIMEOUT_THEM){
        return false;
    }
    else return true;
}

void StatisticalAnalyzer::updatewm() {

//    Vector2D passerRobot,receiverRobot,
//            shotterRobot,shotTarget;
    ballvel.x=wm->ball.vel.x;
    ballvel.y=wm->ball.vel.y;

    ballPos.x=wm->ball.pos.x;
    ballPos.y=wm->ball.pos.y;



    BP= getPossession();



}

BallPossesion StatisticalAnalyzer::getPossession() {
    Vector2D ballpos,apos;
    double ourdist,oppdist;
    ballpos.x = wm->ball.pos.x;
    ballpos.y = wm->ball.pos.y;
    getNearestRobotToPoint(ballpos);
    apos.x = wm->our.at(ourBPID).pos.x;
    apos.y = wm->our.at(ourBPID).pos.y;
    ourdist=apos.dist(ballpos);
    apos.x = wm->opp.at(oppBPID).pos.x;
    apos.y = wm->opp.at(oppBPID).pos.y;
    oppdist=apos.dist(ballpos);
//    ROS_INFO_STREAM(oppBPID<<"___"<<ourBPID);
//    ROS_INFO_STREAM("opp:"<<oppdist<<"__our:"<<ourdist);
    if(ourdist+oppdist<0.4)
        return BallPossesion::draw;
    if(ourdist<oppdist)
        return BallPossesion ::ours;
    else return BallPossesion::theirs;

}
void StatisticalAnalyzer::getNearestRobotToPoint(Vector2D _point) {
    double minDist = 1.0e13;
    ourBPID = -1;
    Vector2D apos;
    for (int i = 0; i < wm->our.size(); i++) {
        apos.x = wm->our.at(i).pos.x;
        apos.y = wm->our.at(i).pos.y;
        double dist = (apos - _point).length();
        if (dist < minDist) {
            minDist = dist;
            ourBPID = i;
        }
    }

    minDist = 1.0e13;
    oppBPID = -1;

    for (int i = 0; i < wm->opp.size(); i++) {
        apos.x = wm->opp.at(i).pos.x;
        apos.y = wm->opp.at(i).pos.y;
        double dist = (apos - _point).length();
        if (dist < minDist) {
            minDist = dist;
            oppBPID = i;
        }
    }
}




void StatisticalAnalyzer::writeToShot(){

}


void StatisticalAnalyzer::writeToPass(){

}


void StatisticalAnalyzer::writeToPossession(){

    if(!possessionFile.open(QIODevice::WriteOnly | QIODevice::Append))
        ROS_INFO_STREAM("Can't open the file to analyze");
    else
        ROS_INFO_STREAM("Analyze file opened :) \n");
    possessionDS.setDevice(&possessionFile);
    possessionDS << (int)BP <<',' <<ballPos.x<<','<<ballPos.y;
    possessionDS<<'\n';

    possessionFile.close();

}


int main(int argc, char **argv){
    ros::init(argc, argv, "StatisticalAnalyzer");
    StatisticalAnalyzer *tt=new StatisticalAnalyzer();
    ros::spin();

    return 0;
}



