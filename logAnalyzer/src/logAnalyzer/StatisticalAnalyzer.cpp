//
// Created by rebinnaf on 9/13/18.
//

#include <logAnalyzer/StatisticalAnalyzer.h>
#include <ncvalues.h>
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
//    ROS_INFO_STREAM("REEEF"<<ref->command<<"__");
//    ROS_INFO_STREAM("REEEF"<<ref->us.name<<"__"<<ref->them.name);

}

void StatisticalAnalyzer::wmCb(const parsian_msgs::parsian_world_modelConstPtr &_wm) {
    wm = _wm;
    updatewm();

    if(isPlayingTime())
        preprocess();

}
void StatisticalAnalyzer::preprocess(){

    if(validPossession()) {
        if(BP==BallPossession ::theirs|| BP ==BallPossession::ours)
            BPsaved=BP;

    }
    ROS_INFO_STREAM("possession"<<(int)BP<<(int)BP<<(int)BP<<(int)BP<<(int)BP<<"__"<<(int)BPsaved<<(int)BPsaved<<(int)BPsaved<<(int)BPsaved<<(int)BPsaved<<"___");

    writeToPossession();

    int shotOrPass=validShotOrPass();

    if(shotOrPass==0)
        writeToShot();
    else if(shotOrPass==2)
        writeToPass();


}


int StatisticalAnalyzer::validShotOrPass(){


//update shotter and shotDir if pass or shot hasn't occurd
    if((!shottedFlag || !passFlag) && BPsaved==BallPossession::theirs) {
        shotterRobot.x = wm->opp.at(oppBPID).pos.x;
        shotterRobot.y = wm->opp.at(oppBPID).pos.y;
        shotDir.x = wm->opp.at(oppBPID).dir.x;
        shotDir.y = wm->opp.at(oppBPID).dir.y;
        shotTarget = Line2D(shotterRobot, shotterRobot + shotDir).intersection(
                Line2D(field.ourCornerL(), field.ourCornerR()));
    }

    Vector2D ballTarget=Line2D(ballPos,ballPos+ballvel).intersection(Line2D(field.ourCornerL(),field.ourCornerR()));
    //robot points to Goal
    if(((shotTarget.y<1.2
         && shotTarget.y>-1.2
         && sign(shotDir.x)==-1) ||
        (ballvel.length()>1 && ballTarget.y<1 && ballTarget.y>-1 && field.ourPenaltyRect().contains(ballPos)))
       && !shottedFlag
       && !passFlag
       && field.fieldRect().contains(ballPos)
       && shotterRobot.x <0
       && BPsaved==BallPossession::theirs) {
        isShottingFlag = 1;
//                ROS_INFO_STREAM("is shotting"<<shotterRobot.x<<"___"<<sign(shotDir.x));
    } else if(sign(shotDir.x)==sign(ballvel.x) && BPsaved==BallPossession::theirs){
        isShottingFlag=2;
//        ROS_INFO_STREAM("is Passing");
    }
    else {
        isShottingFlag=0;
//        ROS_INFO_STREAM("is ooooooing");
    }
    //ball points to Goal ** shot T  pass F
    if(shotterRobot.dist(ballPos)>0.3
       && ballvel.length() >3
       && sign(ballvel.x)== -1
       && isShottingFlag==1
       && !shottedFlag){
        shottedFlag= true;
        passFlag=false;
        shotDir=ballvel;

        ROS_INFO_STREAM("shott"<<(int)BP<<"__");


        //determine if shot is in the goal area
        shotTarget=Line2D(ballPos,ballPos+ballvel).intersection(Line2D(field.ourCornerL(),field.ourCornerR()));
        if(shotTarget.y<0.8 && shotTarget.y>-0.8
           && fabs(ballPos.x-field.ourGoal().x)<1){
            shotInGoal=true;
        } else {
            shotInGoal=false;
        }

        return 0;
    }//ball not points to goal, Passing ** pass T shot F passFinish F
    else if(shotterRobot.dist(ballPos)>0.3
            && ballvel.length() >1.5
            && isShottingFlag==2
            && !shottedFlag
            && !passFlag
            && field.fieldRect().contains(ballPos)){


        BPLast=BPsaved;
        shottedFlag= false;
        passFlag = true;
        passFnished=false;
        shotTarget=Line2D(ballPos,ballPos+ballvel).intersection(Line2D(field.ourCornerL(),field.ourCornerR()));


        ballDir.x=ballvel.x;
        ballDir.y=ballvel.y;
        shotDir=ballvel;
//        ROS_INFO_STREAM("Pass"<<(int)BP<<"__");

        return 1;


    }
    // Passing succeed  ** pass F shot F passFinish T
    if(((fabs(ballDir.x-ballvel.x)> 1 && sign(ballDir.x) ==-1*sign(ballvel.x))
        || (fabs(ballDir.y-ballvel.y)> 1 && sign(ballDir.y) ==-1*sign(ballvel.y))
        ||  !(ballPos.x>4 && ballvel.x>2)
       )
       && passFlag && !shottedFlag && !passFnished && ballDir!=Vector2D(0,0)
       && BP==BallPossession::theirs
       && BPLast==BallPossession::theirs){

        shottedFlag=false;
        passFlag= false;
        passFnished=true;
        isShottingFlag=0;
        shotInGoal=false;
        passSucceed=true;
        ballDir=Vector2D(0,0);
        receiverRobot.x = wm->opp.at(oppBPID).pos.x;
        receiverRobot.y = wm->opp.at(oppBPID).pos.y;
        ROS_INFO_STREAM("Pass Succeeed"<<(int)BP<<"__");
        return 2;

    }//Passing Fails  ** pass F shot F passFinish T
    else if(passFlag && !shottedFlag && !passFnished && ballDir!=Vector2D(0,0) && BPLast==BallPossession::theirs
            && (BP==BallPossession::ours
               || ! Rect2D(field.oppCornerL(),field.ourCornerR()).contains(ballPos))) {


        shottedFlag=false;
        passFlag= false;
        passFnished=true;
        isShottingFlag=0;
        shotInGoal=false;
        passSucceed=false;
        ballDir=Vector2D(0,0);
        ROS_INFO_STREAM("Pass Failed"<<(int)BP<<"__");
        return 2;
    }


    if(shottedFlag
       &&(((fabs(ballDir.x-ballvel.x)> 3 && sign(ballDir.x) ==-1*sign(ballvel.x))
           || (fabs(ballDir.y-ballvel.y)> 3 && sign(ballDir.y) ==-1*sign(ballvel.y)))
          && field.fieldRect().contains(ballPos))
            )
    {
//        ROS_INFO_STREAM("shot back"<<(int)BP<<"__");

        shottedFlag=false;

    }

    if(shottedFlag && !field.fieldRect().contains(ballPos)){
//        ROS_INFO_STREAM("shot out"<<passFlag<<"__");
        shottedFlag=false;
        isShottingFlag=0;
        shotInGoal=false;
        passSucceed=false;
        ballDir=Vector2D(0,0);
    }


    return 3;
}




bool StatisticalAnalyzer::validPossession(){
    //ball speed limit
    double vel=(lastBallPos-ballPos).length()/16*1000;
    lastBallPos=ballPos;
    if(vel<0.3 && vel>0){
        ROS_INFO_STREAM(vel<<"++++++");
        return true;
    }
    else return false;
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
        shottedFlag=false;
        passFlag= false;
        isShottingFlag=false;
        shotInGoal=false;
        passSucceed=false;
        ballDir=Vector2D(0,0);
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

BallPossession StatisticalAnalyzer::getPossession() {
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
    if(ourdist+oppdist<0.4 && ourdist<0.20 && oppdist<0.20)
        return BallPossession::draw;
    if(ourdist<oppdist && ourdist < 0.3)
        return BallPossession::ours;
    else if(oppdist < 0.3)
        return BallPossession::theirs;
    else if(ballvel.length()< 0.1 && ourdist<oppdist && ourdist < 1)
        return BallPossession::ours;
    else if(ballvel.length()< 0.1 && oppdist < 1)
        return BallPossession::theirs;
    else
        return BallPossession::noOne;

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
    if(!shotFile.open(QIODevice::WriteOnly | QIODevice::Append))
        ROS_INFO_STREAM("Can't open the file to analyze");
//    else
//        ROS_INFO_STREAM("Shoooot Analyze) \n");
    shotDS.setDevice(&shotFile);
    shotDS << shotterRobot.x <<',' << shotterRobot.y<<','<< shotTarget.y<<','<< shotInGoal;
    shotDS<<'\n';

    shotFile.close();

}


void StatisticalAnalyzer::writeToPass(){

    if(!passFile.open(QIODevice::WriteOnly | QIODevice::Append))
        ROS_INFO_STREAM("Can't open the file to analyze");
//    else
//        ROS_INFO_STREAM("Passss Analyze) \n");
    passDS.setDevice(&passFile);
    passDS << shotterRobot.x <<',' << shotterRobot.y<<','<< receiverRobot.x<<','<<receiverRobot.y<<','<< passSucceed;
    passDS <<'\n';

    passFile.close();
}


void StatisticalAnalyzer::writeToPossession(){

    if(!possessionFile.open(QIODevice::WriteOnly | QIODevice::Append))
        ROS_INFO_STREAM("Can't open the file to analyze");
//    else
//        ROS_INFO_STREAM(".) \n");
    possessionDS.setDevice(&possessionFile);
    possessionDS << (int)BP <<(int)BPsaved <<',' <<ballPos.x<<','<<ballPos.y;
    possessionDS<<'\n';

    possessionFile.close();

}


int main(int argc, char **argv){
    ros::init(argc, argv, "StatisticalAnalyzer");
    StatisticalAnalyzer *tt=new StatisticalAnalyzer();
    ros::spin();

    return 0;
}



