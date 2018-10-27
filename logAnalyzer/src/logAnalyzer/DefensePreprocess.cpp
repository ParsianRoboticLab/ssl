//
// Created by rebinnaf on 7/17/18.
//

#include <logAnalyzer/DefensePreprocess.h>
#include "parsian_util/core/knowledge.h"


DefensePreprocess::DefensePreprocess() {
//    ros::NodeHandle n("LogAnalyzer");


    wm_sub = n.subscribe("/world_model", 1000, &DefensePreprocess::wmCb, this);
    ref_sub = n.subscribe("/referee", 1000, &DefensePreprocess::refCb, this);
    myfile.setFileName("ZJU_CM.csv");
    if(!myfile.open(QIODevice::WriteOnly | QIODevice::Append))
        ROS_INFO_STREAM("Can't open the file to analyze");
    else
        ROS_INFO_STREAM("Analyze file opened :) \n");
    AnalyzeDS.setDevice(&myfile);

//    monitor_pub = n.advertise<parsian_msgs::parsian_draw>("/draws", 1000);

//drawer=new Drawer();


    /*   AnalyzeDS << "refcommand,";
     *
     *
     *
     *
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


    updatewm();
    if(ballPos.y <-4.7 || ballPos.y>4.7 || ballPos.x<-6 || ballPos.x>6){
        outflag=true;
//        ROS_INFO_STREAM("ouuuuut"<<ballPos.x<<"___"<<ballPos.y);
    }


    if(ref== nullptr)
        return;

    if(isPlayingTime() && !outflag) {
        preprocess();
    }
}





void DefensePreprocess::coachProcess(){


    Vector2D apos;



    oppCoachDef=0;
    oppCoachMark=0;
    for(int i=0 ; i < wm->opp.size() ; i++) {
        apos.x=wm->opp.at(i).pos.x+wm->opp.at(i).vel.x;
        apos.y=wm->opp.at(i).pos.y+wm->opp.at(i).vel.y;
        Vector2D intersect=Line2D(ballPos,field.oppGoal()).intersection(Line2D(field.oppGoalL()+Vector2D(-1,0),field.oppGoalR()+Vector2D(-1,0)));

        ROS_INFO_STREAM("iii: " << intersect.y<< "__"<<field.oppGoalL().y);
//        if(intersect.y>1.2){
//
//        }
//        else if()

        double ballGoalRobotDir=(wm->opp.at(i).dir-(wm->ball.pos-field.oppGoal()).norm()).length();
        double robotGoalRobotDir=(wm->opp.at(i).dir-(apos-field.oppGoal()).norm()).length();

        getNearestRobotToPoint(apos);
        Vector2D ourNearestPos=wm->our.at(ourBPID).pos;
        if(ourNearestPos.dist(apos)<1.5){


        }
        else if(apos.dist(field.oppGoal())<3 && ballGoalRobotDir<0.5 && robotGoalRobotDir<0.5) {
            oppCoachDef++;
            int id= wm->opp.at(i).id;

            ROS_INFO_STREAM("heyy__" <<id << "__");
        }




    }

    ROS_INFO_STREAM("ooo:"<<oppCoachDef);
}




void DefensePreprocess::preprocess(){


    Vector2D apos;

//
//    drawer->draws.circles.clear();
//    drawer->draws.vectors.clear();
//
//    if(ballvel.length()>0.5)
//        drawer->draw(ballvel.norm(),QColor("red"));
//    else
//        drawer->draw(Vector2D(0,0),QColor("red"));
//
//    Vector2D vvv=wm->our.at(1).vel;
//    if(vvv.length()<0.7)
//        vvv=Vector2D(0,0);
//    drawer->draw(vvv.norm(), QColor("blue"));
//
//    monitor_pub.publish(drawer->draws);

    for(int i=0 ; i < wm->our.size() ; i++)
    {
//        apos.x=wm->our.at(i).pos.x;
//        apos.y=wm->our.at(i).pos.y;

        apos=wm->our.at(i).pos;
        ourdistances.append(apos.dist(field.oppGoal()));
        ourangles.append(sign(apos.y)*apos.angleOf(apos,field.oppGoal(),field.center()).degree());

        apos=wm->our.at(i).vel;
        if(apos.length()>0.7)
            ourVelNorm.append(apos.norm());
        else
            ourVelNorm.append(Vector2D(0,0));

        ourvellength.append(apos.length());
        ourVels.append(apos);

        anglemeasure=ourangles.at(i);
        distmeasure=ourdistances.at(i);
        int index=i;

//        for(int k=0;k<ourangles.size();k++){
////            //ROS_INFO_STREAM("dists:"<<k<<"__"<<ourangles.at(k));
//        }

//        //ROS_INFO_STREAM("measure:"<<anglemeasure);


        if(ourImeasure.size()==0) {
            ourImeasure.append(distmeasure);
            ourSIndex.append(0);
        }
        else {
            for (int j = 0; j < ourImeasure.size(); j++) {
//                //ROS_INFO_STREAM("loop:"<<j);
                if (fabs(distmeasure - ourImeasure.at(j))<0.6) {
//                    //ROS_INFO_STREAM("if15Y");


                    if(anglemeasure < ourangles.at(ourSIndex.at(j))){
//                        //ROS_INFO_STREAM("2if");
                        ourImeasure.insert(j, distmeasure);
                        ourSIndex.insert(j, index);
                        break;
                    }
                    else if (j == ourImeasure.size() - 1) {
//                    //ROS_INFO_STREAM("thirdif");
                        ourImeasure.append(distmeasure);
                        ourSIndex.append(index);
                        break;
                    }

                }
                else if(distmeasure < ourImeasure.at(j)){
//                    //ROS_INFO_STREAM("secondif");
                    ourImeasure.insert(j, distmeasure);
                    ourSIndex.insert(j, index);
                    break;
                }

                else if (j == ourImeasure.size() - 1) {
//                    //ROS_INFO_STREAM("thirdif");
                    ourImeasure.append(distmeasure);
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







    for(int i=0 ; i < wm->opp.size() ; i++)
    {
//        apos.x=wm->opp.at(i).pos.x;
//        apos.y=wm->opp.at(i).pos.y;

        apos=wm->opp.at(i).pos;
        oppdistances.append(apos.dist(field.oppGoal()));
        oppangles.append(sign(apos.y)*apos.angleOf(apos,field.oppGoal(),field.center()).degree());


        apos=wm->opp.at(i).vel;
        if(apos.length()>0.7)
            oppVelNorm.append(apos.norm());
        else
            oppVelNorm.append(Vector2D(0,0));

        oppVellength.append(apos.length());
        oppVels.append(apos);

        anglemeasure=oppangles.at(i);
        distmeasure=oppdistances.at(i);
        int index=i;

        for(int k=0;k<oppangles.size();k++){
//            //ROS_INFO_STREAM("dists:"<<k<<"__"<<oppangles.at(k));
        }

//        //ROS_INFO_STREAM("measure:"<<anglemeasure);


        if(oppImeasure.size()==0) {
            oppImeasure.append(distmeasure);
            oppSIndex.append(0);
        }
        else {
            for (int j = 0; j < oppImeasure.size(); j++) {
//                //ROS_INFO_STREAM("loop:"<<j);
                if (fabs(distmeasure - oppImeasure.at(j))<0.6) {
//                    //ROS_INFO_STREAM("if15Y");


                    if(anglemeasure < oppangles.at(oppSIndex.at(j))){
//                        //ROS_INFO_STREAM("2if");
                        oppImeasure.insert(j, distmeasure);
                        oppSIndex.insert(j, index);
                        break;
                    }
                    else if (j == oppImeasure.size() - 1) {
//                    //ROS_INFO_STREAM("thirdif");
                        oppImeasure.append(distmeasure);
                        oppSIndex.append(index);
                        break;
                    }

                }
                else if(distmeasure < oppImeasure.at(j)){
//                    //ROS_INFO_STREAM("secondif");
                    oppImeasure.insert(j, distmeasure);
                    oppSIndex.insert(j, index);
                    break;
                }

                else if (j == oppImeasure.size() - 1) {
//                    //ROS_INFO_STREAM("thirdif");
                    oppImeasure.append(distmeasure);
                    oppSIndex.append(index);
                    break;
                }




            }

//


        }
    }


//    ROS_INFO_STREAM(oppdistances.at(oppSIndex.at(0))<<")))))))))))");


    apos.x=wm->ball.pos.x;
    apos.y=wm->ball.pos.y;
    balldistance=apos.dist(field.oppGoal());
    ballangle=sign(apos.y) * apos.angleOf(apos, field.oppGoal(), field.center()).degree();


//    ROS_INFO_STREAM("Rebin sindex o:"<<ourSIndex.size()<<"_opp:"<<oppSIndex.size());
//    for(int k=0;k<oppSIndex.size();k++){
//        double id=oppdistances.at(oppSIndex.at(k));
//        ROS_INFO_STREAM("_Rebinopp:"<<k<<"__"<<id);
//    }
//
//    for(int k=0;k<ourSIndex.size();k++){
//        double id=ourdistances.at(ourSIndex.at(k));
//        ROS_INFO_STREAM("_Rebinour:"<<k<<"__"<<id);
//    }

//    coachProcess();
    writeData();
//        ROS_INFO_STREAM("BP:" << 0);
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
        outflag=false;
        return false;
    }
    else return true;
}




void DefensePreprocess::refCb(const parsian_msgs::ssl_refree_wrapperConstPtr & _ref) {
    ref=_ref;
    refcommand=ref->command.command;
//    ROS_INFO_STREAM("REEEF"<<ref->command<<"__");
//    ROS_INFO_STREAM("REEEF"<<ref->us.name<<"__"<<ref->them.name);

}
bool DefensePreprocess::validPossession(){



    if((ballvel.length()<0.5 && ballvel.length()>0)
//        || ballvel.length()==0
            ){
        return true;
    }
    else return false;
}



void DefensePreprocess::updatewm() {

//    Vector2D passerRobot,receiverRobot,
//            shotterRobot,shotTarget;
    ballvel.x=wm->ball.vel.x;
    ballvel.y=wm->ball.vel.y;

    ballPos.x=wm->ball.pos.x;
    ballPos.y=wm->ball.pos.y;



    BP= getPossession();



}

BallPossession DefensePreprocess::getPossession() {
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


    Vector2D ballNextPos;
    ballNextPos=ballPos+ballvel;
    ROS_INFO_STREAM(ballvel.length());
    double ballNextDegree=sign(ballNextPos.y)*ballNextPos.angleOf(ballNextPos,field.oppGoal(),field.center()).degree();
    double ballNextDistance=ballNextPos.dist(field.oppGoal());
    Vector2D ballvelNorm=ballvel.norm();

    if(!myfile.open(QIODevice::WriteOnly | QIODevice::Append));
//        ROS_INFO_STREAM("Can't open the file to analyze");
    else;
//        ROS_INFO_STREAM("Analyze file opened :) \n");
    AnalyzeDS.setDevice(&myfile);

    AnalyzeDS << refcommand <<',';
    for(int i=0 ; i < 8 ; i++){
        if(i<ourSIndex.size()) {
            if (ourdistances.at(ourSIndex.at(i)) < 15){
                AnalyzeDS << wm->our.at(ourSIndex.at(i)).pos.x << ',' << wm->our.at(ourSIndex.at(i)).pos.y << ',';
                AnalyzeDS << ourangles.at(ourSIndex.at(i)) << ',';
                AnalyzeDS << ourdistances.at(ourSIndex.at(i)) << ',';
                AnalyzeDS << ourangles.at(ourSIndex.at(i)) << ',';
                AnalyzeDS << ourVelNorm.at(ourSIndex.at(i)).x << ',' << ourVelNorm.at(ourSIndex.at(i)).y<< ',';
                AnalyzeDS << ourvellength.at(ourSIndex.at(i)) << ',';
                AnalyzeDS << ourVels.at(ourSIndex.at(i)).x << ',' << ourVels.at(ourSIndex.at(i)).y<< ',';
            }else {
                AnalyzeDS << -1.0 << ',' << 100.0 << ','<< 2.0 << ',' << 2.0 << ','<< 10.0 << ','<< 10.0 << ','<< 10.0 << ',';
            }
        } else {
            AnalyzeDS << -1.0 << ',' << 100.0 << ','<< 2.0 << ',' << 2.0 << ','<< 10.0 << ','<< 10.0 << ','<< 10.0 << ',';;
        }

    }

    AnalyzeDS<<balldistance<<','<<ballangle<<',';
    AnalyzeDS<<ballNextDistance<<','<<ballNextDegree<<',';

    if(ballvel.length()>0.5)
        AnalyzeDS<<ballvelNorm.x << ',' << ballvelNorm.y << ',';
    else
        AnalyzeDS<< 0.0 <<','<<0.0<<',';
    AnalyzeDS<<ballvel.length()<<',';
    AnalyzeDS<<wm->opp.size()<<',';
//    for(int k=0;k<ourSIndex.size();k++){
//        int id=wm->our.at(ourSIndex.at(k)).id;
//        //ROS_INFO_STREAM("Index:"<<k<<"__"<<id);
//    }





//    for(int k=0;k<oppSIndex.size();k++){
//        int id=wm->opp.at(oppSIndex.at(k)).id;
////ROS_INFO_STREAM("Index:"<<k<<"__"<<id);
//    }


    for(int i=0 ; i < 8 ; i++){
        if(i<oppSIndex.size()) {
            if(oppdistances.at(oppSIndex.at(i)) < 15) {
                AnalyzeDS << wm->opp.at(oppSIndex.at(i)).pos.x << ',' << wm->opp.at(oppSIndex.at(i)).pos.y << ',';
                AnalyzeDS << oppdistances.at(oppSIndex.at(i)) << ',';
                AnalyzeDS << oppangles.at(oppSIndex.at(i))<< ',';
                AnalyzeDS << oppVelNorm.at(oppSIndex.at(i)).x << ',' << oppVelNorm.at(oppSIndex.at(i)).y << ',';
                AnalyzeDS << oppVellength.at(oppSIndex.at(i))<<',';
                AnalyzeDS << oppVels.at(oppSIndex.at(i)).x << ',' << oppVels.at(oppSIndex.at(i)).y;

            }
            else {
                AnalyzeDS << -1.0 << ',' << 100.0 << ','<< 2.0 << ',' << 2.0 << ','<< 10.0 << 10.0 << ','<< 10.0 ;
            }
        } else {
            AnalyzeDS << -1.0 << ',' << 100.0 << ','<< 2.0 << ',' << 2.0 << ','<< 10.0 << 10.0 << ','<< 10.0 ;
        }

        if(i!=7)
            AnalyzeDS<<',';

    }
    AnalyzeDS<<'\n';









    AnalyzeDS << refcommand <<',';
    for(int i=0 ; i < 8 ; i++){
        if(i<ourSIndex.size()) {
            if(ourdistances.at(ourSIndex.at(i)) < 15) {
                AnalyzeDS << wm->our.at(ourSIndex.at(i)).pos.x << ',' << -1* wm->our.at(ourSIndex.at(i)).pos.y << ',';
                AnalyzeDS << ourdistances.at(ourSIndex.at(i)) << ',';
                AnalyzeDS << -1 * ourangles.at(ourSIndex.at(i)) << ',';
                AnalyzeDS << ourVelNorm.at(ourSIndex.at(i)).x << ','<< -1 * ourVelNorm.at(ourSIndex.at(i)).y << ',';
                AnalyzeDS << ourvellength.at(ourSIndex.at(i)) << ',';
                AnalyzeDS << ourVels.at(ourSIndex.at(i)).x << ','<< -1 * ourVels.at(ourSIndex.at(i)).y << ',';

            }
            else {
                AnalyzeDS << -1.0 << ',' << 100.0 << ','<< 2.0 << ',' << 2.0 << ','<< 10.0 << ',' << 10.0 << ','<< 10.0 << ',';
            }
        } else {
            AnalyzeDS << -1.0 << ',' << 100.0 << ','<< 2.0 << ',' << 2.0 << ','<< 10.0 << ',' << 10.0 << ','<< 10.0 << ',';
        }

    }


    AnalyzeDS<<balldistance<<','<<-1*ballangle<<',';
    AnalyzeDS<<ballNextDistance<<','<<-1*ballNextDegree<<',';
    if(ballvel.length()>0.5)
        AnalyzeDS<<ballvelNorm.x << ',' << -1* ballvelNorm.y << ',';
    else
        AnalyzeDS<< 0.0 <<','<<0.0<<',';
    AnalyzeDS<<ballvel.length()<<',';
    AnalyzeDS<<wm->opp.size()<<',';

//    for(int k=0;k<ourSIndex.size();k++){
//        int id=wm->our.at(ourSIndex.at(k)).id;
//        //ROS_INFO_STREAM("Index:"<<k<<"__"<<id);
//    }








    for(int i=0 ; i < 8 ; i++){
        if(i<oppSIndex.size()) {
            if(oppdistances.at(oppSIndex.at(i)) < 15) {
                AnalyzeDS << wm->opp.at(oppSIndex.at(i)).pos.x << ',' << -1* wm->opp.at(oppSIndex.at(i)).pos.y << ',';
                AnalyzeDS << oppdistances.at(oppSIndex.at(i)) << ',';
                AnalyzeDS << -1 * oppangles.at(oppSIndex.at(i))<<',';
                AnalyzeDS << oppVelNorm.at(oppSIndex.at(i)).x << ','<< -1 * oppVelNorm.at(oppSIndex.at(i)).y << ',';
                AnalyzeDS << oppVellength.at(oppSIndex.at(i)) <<',';
                AnalyzeDS << oppVels.at(oppSIndex.at(i)).x << ','<< -1 * oppVels.at(oppSIndex.at(i)).y;

            }
            else {
                AnalyzeDS << 10 << 10 << -1.0 << ',' << 100.0 << ','<< 2.0 << ',' << 2.0 << ','<< 10.0 << 10.0 << ','<< 10.0;
            }
        } else {
            AnalyzeDS << 10 << 10 << -1.0 << ',' << 100.0 << ','<< 2.0 << ',' << 2.0 << ','<< 10.0 << 10.0 << ','<< 10.0;
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

