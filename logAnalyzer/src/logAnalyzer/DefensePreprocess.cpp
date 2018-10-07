//
// Created by rebinnaf on 7/17/18.
//

#include <logAnalyzer/DefensePreprocess.h>
#include "parsian_util/core/knowledge.h"


DefensePreprocess::DefensePreprocess() {
//    ros::NodeHandle n("LogAnalyzer");


    wm_sub = n.subscribe("/world_model", 1000, &DefensePreprocess::wmCb, this);
    ref_sub = n.subscribe("/referee", 1000, &DefensePreprocess::refCb, this);
    myfile.setFileName("Tigers_ZJU.csv");
    if(!myfile.open(QIODevice::WriteOnly | QIODevice::Append))
        ROS_INFO_STREAM("Can't open the file to analyze");
    else
        ROS_INFO_STREAM("Analyze file opened :) \n");
    AnalyzeDS.setDevice(&myfile);
    /*   AnalyzeDS << "refcommand,";
       for(int i=0; i<8; i++) {
           AnalyzeDS << "our"<<i<<"Dist,our"<<i<<"Ang,";
       }
       AnalyzeDS << "ballDist,ballAng,";

       for(int i=0; i<8; i++) {
           AnalyzeDS << "opp"<<i<<"Dist,opp"<<i<<"Ang";
           if(i!=7)
               AnalyzeDS <<',';
       }
       AnalyzeDS<<'\n';
   */
//    myfile.open ("PREPARED.csv");
//    myfile<<"nadia";
    myfile.close();
}

DefensePreprocess::~DefensePreprocess() {

    std::cout<<"yees";
    myfile.close();
}

void DefensePreprocess::wmCb(const parsian_msgs::parsian_world_modelConstPtr &_wm) {
    wm = _wm;
    preprocess();

}
void DefensePreprocess::preprocess(){


    Vector2D apos;

    for(int i=0 ; i < wm->our.size() ; i++)
    {
        apos.x=wm->our.at(i).pos.x;
        apos.y=wm->our.at(i).pos.y;
        ourdistances.append(apos.dist(field.oppGoal()));
        ourangles.append(sign(apos.y)*apos.angleOf(apos,field.oppGoal(),field.center()).degree());
        anglemeasure=ourangles.at(i);
        distmeasure=ourdistances.at(i);
        int index=i;

        for(int k=0;k<ourangles.size();k++){
//            //ROS_INFO_STREAM("dists:"<<k<<"__"<<ourangles.at(k));
        }

//        //ROS_INFO_STREAM("measure:"<<anglemeasure);


        if(ourImeasure.size()==0) {
            ourImeasure.append(anglemeasure);
            ourSIndex.append(0);
        }
        else {
            for (int j = 0; j < ourImeasure.size(); j++) {
//                //ROS_INFO_STREAM("loop:"<<j);
                if (fabs(anglemeasure - ourImeasure.at(j))<15) {
//                    //ROS_INFO_STREAM("if15Y");

                    if (fabs(distmeasure - ourdistances.at(ourSIndex.at(j))) > 0.6) {
                        if (distmeasure < ourdistances.at(ourSIndex.at(j))) {
                            ourImeasure.insert(j, anglemeasure);
                            ourSIndex.insert(j, index);
                            break;
                        }
                        else if (j == ourImeasure.size() - 1) {
//                    //ROS_INFO_STREAM("thirdif");
                            ourImeasure.append(anglemeasure);
                            ourSIndex.append(index);
                            break;
                        }
                        else{
                            ourImeasure.insert(j+1, anglemeasure);
                            ourSIndex.insert(j+1, index);
                            break;
                        }
                    }
                    else if(anglemeasure < ourImeasure.at(j)){
//                        //ROS_INFO_STREAM("2if");
                        ourImeasure.insert(j, anglemeasure);
                        ourSIndex.insert(j, index);
                        break;
                    }
                    else if (j == ourImeasure.size() - 1) {
//                    //ROS_INFO_STREAM("thirdif");
                        ourImeasure.append(anglemeasure);
                        ourSIndex.append(index);
                        break;
                    }

                }
                else if(anglemeasure < ourImeasure.at(j)){
//                    //ROS_INFO_STREAM("secondif");
                    ourImeasure.insert(j, anglemeasure);
                    ourSIndex.insert(j, index);
                    break;
                }

                else if (j == ourImeasure.size() - 1) {
//                    //ROS_INFO_STREAM("thirdif");
                    ourImeasure.append(anglemeasure);
                    ourSIndex.append(index);
                    break;
                }




            }

//
//            for(int k=0;k<ourSIndex.size();k++){
//                int id=wm->our.at(ourSIndex.at(k)).id;
//                //ROS_INFO_STREAM("_RebinI:"<<k<<"__"<<id);
//            }

        }
    }







    for(int i=0 ; i < wm->opp.size() ; i++) {
        apos.x = wm->opp.at(i).pos.x;
        apos.y = wm->opp.at(i).pos.y;
        oppdistances.append(apos.dist(field.oppGoal()));
        oppangles.append(sign(apos.y) * apos.angleOf(apos, field.oppGoal(), field.center()).degree());
        anglemeasure=oppangles.at(i);
        distmeasure=oppdistances.at(i);
        int index=i;

        for(int k=0;k<oppangles.size();k++){
            //ROS_INFO_STREAM("dists:"<<k<<"__"<<oppangles.at(k));
        }

        //ROS_INFO_STREAM("measure:"<<anglemeasure);


        if(oppImeasure.size()==0) {
            oppImeasure.append(anglemeasure);
            oppSIndex.append(0);
        }
        else {
            for (int j = 0; j < oppImeasure.size(); j++) {
                //ROS_INFO_STREAM("loop:"<<j);
                if (fabs(anglemeasure - oppImeasure.at(j))<12) {
                    //ROS_INFO_STREAM("if15Y");

                    if (fabs(distmeasure - oppdistances.at(oppSIndex.at(j))) > 0.6) {
                        if (distmeasure < oppdistances.at(oppSIndex.at(j))) {
                            oppImeasure.insert(j, anglemeasure);
                            oppSIndex.insert(j, index);
                            //ROS_INFO_STREAM("naha"<<oppSIndex.at(j));
                            break;
                        }
                        else if (j == oppImeasure.size() - 1) {
                            //ROS_INFO_STREAM("thirdif");
                            oppImeasure.append(anglemeasure);
                            oppSIndex.append(index);
                            break;
                        }
                        else{
                            oppImeasure.insert(j+1, anglemeasure);
                            oppSIndex.insert(j+1, index);
                            //ROS_INFO_STREAM("aha"<<oppSIndex.at(j+1));
                            break;
                        }
                    }
                    else if(anglemeasure < oppImeasure.at(j)){
                        //ROS_INFO_STREAM("2if");
                        oppImeasure.insert(j, anglemeasure);
                        oppSIndex.insert(j, index);
                        break;
                    }
                    else if (j == oppImeasure.size() - 1) {
                        //ROS_INFO_STREAM("thirdif");
                        oppImeasure.append(anglemeasure);
                        oppSIndex.append(index);
                        break;
                    }

                }
                else if(anglemeasure < oppImeasure.at(j)){
                    //ROS_INFO_STREAM("secondif");
                    oppImeasure.insert(j, anglemeasure);
                    oppSIndex.insert(j, index);
                    break;
                }
                if (j == oppImeasure.size() - 1) {
                    //ROS_INFO_STREAM("thirdif");
                    oppImeasure.append(anglemeasure);
                    oppSIndex.append(index);
                    break;
                }



            }


            for(int k=0;k<oppSIndex.size();k++){
                int id=wm->opp.at(oppSIndex.at(k)).id;
                //ROS_INFO_STREAM("opp_RebinI:"<<k<<"__"<<id);
            }

        }


    }



    apos.x=wm->ball.pos.x;
    apos.y=wm->ball.pos.y;
    balldistance=apos.dist(field.oppGoal());
    ballangle=sign(apos.y) * apos.angleOf(apos, field.oppGoal(), field.center()).degree();

//    //ROS_INFO_STREAM("Rebin:"<<balldistance<<"___"<<ballangle<<"__M__:"<<ballangle*balldistance);
    updateBP();

    if(isPlayingTime()) {
        writeData();
//        ROS_INFO_STREAM("BP:" << 0);
    }
    clearLists();
//    std::cout<<"nadiaaa";



}

bool DefensePreprocess::isPlayingTime() {
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




void DefensePreprocess::refCb(const parsian_msgs::ssl_refree_wrapperConstPtr & _ref) {
    ref=_ref;
    refcommand=ref->command.command;
    ROS_INFO_STREAM("REEEF"<<ref->command<<"__");
//    ROS_INFO_STREAM("REEEF"<<ref->us.name<<"__"<<ref->them.name);

}

void DefensePreprocess::updateBP() {
    Vector2D ballvel;
    ballvel.x=wm->ball.vel.x;
    ballvel.y=wm->ball.vel.y;


    if(refcommand==ref->command.DIRECT_FREE_US
       || refcommand==ref->command.INDIRECT_FREE_US
       || refcommand==ref->command.PREPARE_KICKOFF_US){
        BP= BallPossession::ours;
        return;
    }
    else if(refcommand==ref->command.DIRECT_FREE_THEM
            || refcommand==ref->command.INDIRECT_FREE_THEM
            || refcommand==ref->command.PREPARE_KICKOFF_THEM){
        BP= BallPossession::theirs;
        return;
    }


    double temp = wm->ball.pos.x ;
    if(temp > 0)
        BP= BallPossession::ours;
    else if(isoppNearest()==0 && temp >-2)
        BP= BallPossession::ours;
    else
        BP= BallPossession::theirs;
//
//    if(ballvel.length()<1.5){
//
////        ROS_INFO_STREAM("vel<1.5__"<<isoppNearest());
//        if(getPossession()==0)
//            BP= BallPossession::ours;
//
//        else if(getPossession()==1)
//            BP= BallPossession::theirs;
//    }



    Vector2D x;


}

int DefensePreprocess::isoppNearest() {
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
        return 2;
    if(ourdist<oppdist)
        return 0;
    else return 1;

}
void DefensePreprocess::getNearestRobotToPoint(Vector2D _point) {
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




void DefensePreprocess::clearLists() {
    ourImeasure.clear();
    ourSIndex.clear();
    ourdistances.clear();
    ourangles.clear();
    oppImeasure.clear();
    oppSIndex.clear();
    oppdistances.clear();
    oppangles.clear();
}

void DefensePreprocess::writeData(){

    if(!myfile.open(QIODevice::WriteOnly | QIODevice::Append))
        ROS_INFO_STREAM("Can't open the file to analyze");
    else
        ROS_INFO_STREAM("Analyze file opened :) \n");
    AnalyzeDS.setDevice(&myfile);
    AnalyzeDS << refcommand <<',';
    for(int i=0 ; i < 8 ; i++){
        if(i<ourSIndex.size()) {
            AnalyzeDS << ourdistances.at(ourSIndex.at(i)) << ',';
            AnalyzeDS << ourangles.at(ourSIndex.at(i)) << ',';

        } else {
            AnalyzeDS << -1.0 << ',' << -1.0 << ',';
        }

    }

    AnalyzeDS<<balldistance<<','<<ballangle<<',';
    AnalyzeDS<<wm->opp.size()<<',';
//    for(int k=0;k<ourSIndex.size();k++){
//        int id=wm->our.at(ourSIndex.at(k)).id;
//        //ROS_INFO_STREAM("Index:"<<k<<"__"<<id);
//    }

    for(int k=0;k<oppSIndex.size();k++){
        int id=wm->opp.at(oppSIndex.at(k)).id;
//ROS_INFO_STREAM("Index:"<<k<<"__"<<id);
    }


    for(int i=0 ; i < 8 ; i++){
        if(i<oppSIndex.size()) {
            AnalyzeDS << oppdistances.at(oppSIndex.at(i)) << ',';
            AnalyzeDS << oppangles.at(oppSIndex.at(i));

        } else {
            AnalyzeDS << -1.0 << ',' << -1.0 ;
        }

        if(i!=7)
            AnalyzeDS<<',';

    }
    AnalyzeDS<<'\n';









    AnalyzeDS << refcommand <<',';
    for(int i=0 ; i < 8 ; i++){
        if(i<ourSIndex.size()) {
            AnalyzeDS << ourdistances.at(ourSIndex.at(i)) << ',';
            AnalyzeDS << -1 * ourangles.at(ourSIndex.at(i)) << ',';

        } else {
            AnalyzeDS << -1.0 << ',' << -1.0 << ',';
        }

    }

    AnalyzeDS<<balldistance<<','<<-1*ballangle<<',';
    AnalyzeDS<<wm->opp.size()<<',';
//    for(int k=0;k<ourSIndex.size();k++){
//        int id=wm->our.at(ourSIndex.at(k)).id;
//        //ROS_INFO_STREAM("Index:"<<k<<"__"<<id);
//    }

    for(int k=0;k<oppSIndex.size();k++){
        int id=wm->opp.at(oppSIndex.at(k)).id;
//ROS_INFO_STREAM("Index:"<<k<<"__"<<id);
    }


    for(int i=0 ; i < 8 ; i++){
        if(i<oppSIndex.size()) {
            AnalyzeDS << oppdistances.at(oppSIndex.at(i)) << ',';
            AnalyzeDS << -1 * oppangles.at(oppSIndex.at(i));


        } else {
            AnalyzeDS << -1.0 << ',' << -1.0 ;
        }

        if(i!=7)
            AnalyzeDS<<',';

    }
    AnalyzeDS<<'\n';



    myfile.close();



}


//BallPossession CCoach::isBallOurs() {
//    BallPossession decidePState;
//
//    double temp = wm->ball->pos.x + wm->ball->vel.x * 1;
//
//    if (temp > 0.5) {
//        decidePState = BallPossession::WEHAVETHEBALL;
//    } else if (temp < 0.1) {
//        decidePState = BallPossession::WEDONTHAVETHEBALL;
//    } else {
//        decidePState = lastBallPossesionState;
//    }
//
//    if (wm->field->isInOurPenaltyArea(wm->ball->pos)
//        &&  wm->ball->vel.length() < 0.1) {
//        decidePState = BallPossession::SOSOTHEIR;
//    }
//
//    if (wm->field->isInOppPenaltyArea(wm->ball->pos)
//        && wm->ball->vel.length() < 0.1) {
//        decidePState = BallPossession::SOSOOUR;
//    }
//
//    lastBallPossesionState = decidePState;
//
//    return decidePState;
//}

int main(int argc, char **argv){
    ros::init(argc, argv, "Analyzer");
    DefensePreprocess *tt=new DefensePreprocess();
    ros::spin();

    return 0;
}

