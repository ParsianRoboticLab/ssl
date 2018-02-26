#include <parsian_ai/plans/markplan.h>
#include <parsian_util/core/robot.h>
#include <algorithm>
#include <parsian_ai/util/worldmodel.h>
#include <parsian_ai/gamestate.h>
#include <parsian_ai/util/knowledge.h>
#include <parsian_ai/config.h>

using namespace std;

CMarkPlan::CMarkPlan()
{
    fastestTimeSum = 0;
    fastest = -1;
    ballInOurField = -1;
    distance = -1;
    lastFastest = -1;
    lastFastestFoundCycle = 0;
    fastestTime = 0;
    ourBallOwner = -1;
    supporter = nullptr;
    oppBallOwner = -1;
    oppFastest = -1;
    ourFastest = -1;
    supporter = nullptr;
    lastFrameInSupportMode = -1;


    /// --- HMD---///
    segmentpershoot = 0.5;
    segmentperpass = 0.1;
    markRadius = 1.5;
    markRadiusStrict = 1.43;


    for (auto &i : markGPA) {
        i = new GotopointavoidAction;
    }
    /////////////////////

}

CMarkPlan::~CMarkPlan()
{
    for (auto &i : markGPA) {
        delete i;
    }
}

#define Ball_Ours_History 25
#define Our_Fastest_History 25
#define Opp_Fastest_History 25

//////////////////////////////// AHZ //////////////////////////////////////////
bool CMarkPlan::isInIndirectArea(Vector2D aPoint){
    //// check that a point is in the circle around the ball
    //// with 50cm radius or not.

    bool localFlag = Circle2D(wm->ball->pos , 0.7).contains(aPoint);
    return localFlag;
}

Line2D CMarkPlan::getBisectorLine(Vector2D firstPoint , Vector2D originPoint , Vector2D thirdPoint){
    Line2D bisectorLine (originPoint , AngleDeg::bisect((firstPoint - originPoint).th() , (thirdPoint - originPoint).th()));
    return bisectorLine;
}
Segment2D CMarkPlan::getBisectorSegment(Vector2D firstPoint , Vector2D originPoint , Vector2D thirdPoint){
    Line2D bisectorLine (originPoint , AngleDeg::bisect((firstPoint - originPoint).th() , (thirdPoint - originPoint).th()));
    Segment2D bisectorSegment(originPoint , Segment2D (thirdPoint , firstPoint).intersection(bisectorLine));
    return bisectorSegment;
}

void CMarkPlan::manToManMarkInPlayOnBlockPass(QList<Vector2D> opponentAgentsToBeMarkPossition , int ourMarkAgentsSize , double proportionOfDistance){
    //// This function blocking the lines that are between ball &&
    //// opponent agents by a variable ratio along these lines.
    //// This is one of the mark plan for defending more flexible


    ////////////////////////// Variables of this function //////////////////////
    int i;
    int j;
    Vector2D sol1 , sol2;
    Vector2D sol3 , sol4;
    Vector2D sol5 , sol6;
    Vector2D sol7 , sol8;
    double opponentAgentsCircleR = 0.2;
    QList<Circle2D> opponentAgentsCircle;
    QList<Circle2D> opponentAgentsToBeMarkCircle;
    QList<Circle2D> tempOpponentAgentsToBeMarkedCircle;
    QList<Vector2D> ourMarkAgentsPossition;
    QList<Vector2D> tempOpponentAgentsToBeMarkedPosition;
    QList<QPair<Vector2D,double> > sortDangerAgentsToBeMarkBlockPassPlayOff;
    QList<QPair<Vector2D,double> > tempSortDangerAgentsToBeBlockPassPlayOff;
    //////////////////// Clear QLists for update the states ////////////////////
    //stopMode = gameState->isPlayOff();
    ourMarkAgentsPossition.clear();
    tempOpponentAgentsToBeMarkedPosition.clear();
    markPoses.clear();
    markAngs.clear();
    markRoles.clear();
    /////////////////// Intelligent mark plan ///////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    DBUG(QString("Mark Agents Count : %1").arg(ourMarkAgentsSize) , D_SEPEHR);
    ///////// Make Cirlcles around opponent agents /////////////////////////////
    DBUG(QString("Opponent Agents to be mark count : %1").arg(opponentAgentsToBeMarkPossition.size()) , D_SEPEHR);
    for (auto i : opponentAgentsToBeMarkPossition){
        if(!isInIndirectArea(i)){
            tempOpponentAgentsToBeMarkedPosition.append(i);
        }
    }
    opponentAgentsToBeMarkPossition.clear();
    opponentAgentsToBeMarkPossition = tempOpponentAgentsToBeMarkedPosition;
    for(i = 0 ; i < opponentAgentsToBeMarkPossition.size(); i++){
        opponentAgentsToBeMarkCircle.append(Circle2D(opponentAgentsToBeMarkPossition.at(i) , opponentAgentsCircleR));
        drawer->draw(opponentAgentsToBeMarkCircle.at(i) , "Cyan");
    }
    DBUG(QString("Opponent Agents to be mark count : %1").arg(opponentAgentsToBeMarkPossition.size()) , D_SEPEHR);
    ///////////////// Block Pass Plan ////////////////////////////////////
    if(opponentAgentsToBeMarkPossition.size() == ourMarkAgentsSize){
        for(i = 0 ; i < ourMarkAgentsSize ; i++){
            markRoles.append(QString("passBlocker"));
            //////////// Don't Enter penalty area, mark agents!!! :) ///////////
            if(wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)) , &sol7,  &sol8)){
                if(!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))){
                    opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                    wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                    markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 ,Segment2D(sol5 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol5:sol6 ,proportionOfDistance));
                }
                else{
                    wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                    markPoses.append(Segment2D(sol1 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol2 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol1 : sol2);
                }
            }
            else{
                opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4, proportionOfDistance));
            }
            markAngs.append(wm->ball->pos - markPoses.at(i));
            drawer->draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                   Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4) , QColor(Qt::red));
            drawer->draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
            DBUG(QString("Man To Man Mark In PlayOff Mode / BlockPass / our = opp") , D_SEPEHR);
        }
    }
    else if(opponentAgentsToBeMarkPossition.size() < ourMarkAgentsSize){
        if(opponentAgentsToBeMarkPossition.empty()){
            for(i = 0 ; i < ourMarkAgentsSize ; i++){
                if(i % 2){
                    markPoses.append(Vector2D(-3 , i / 1.5));
                    markAngs.append(Vector2D(0,0));
                    markRoles.append(QString("shotBlocker"));
                }
                else{
                    markPoses.append(Vector2D(-3,-i  / 1.5));
                    markAngs.append(Vector2D(0,0));
                    markRoles.append(QString("shotBlocker"));
                }
            }
        }
        else if(opponentAgentsToBeMarkPossition.size() == 1){
            for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                markRoles.append(QString("passBlocker"));
                //////////// Don't Enter penalty area, mark agents!!! :) ///////////
                if(wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)) , &sol7,  &sol8)){
                    if(!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))){
                        opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                        Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                        wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                        markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 ,Segment2D(sol5 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol5:sol6 ,proportionOfDistance));
                    }
                    else{
                        wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Line2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                        markPoses.append(Segment2D(sol1 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol2 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol1 : sol2);
                    }
                }
                else{
                    opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                    markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4, proportionOfDistance));
                }
                markAngs.append(wm->ball->pos - markPoses.at(i));
                drawer->draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4), QColor(Qt::red));
                drawer->draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
            }
            for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                if(!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))){
                    opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                    wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(opponentAgentsToBeMarkPossition.at(i) , wm->field->ourGoal()), &sol3 , &sol4);
                    markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 ,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3:sol4 ,conf.ShootRatioBlock / 100));
                }
                else{
                    wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Line2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                    markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                }
                markAngs.append(opponentAgentsToBeMarkPossition.at(i) - wm->field->ourGoal());
                markRoles.append(QString("shotBlocker"));
            }
            for(i = 0 ; i < ourMarkAgentsSize - markPoses.size() ; i++){
                if(i % 2){
                    markPoses.append(Vector2D(-3 , i / 1.5));
                    markAngs.append(Vector2D(0,0));
                    markRoles.append(QString("shotBlocker"));
                }
                else{
                    markPoses.append(Vector2D(-3,-i  / 1.5));
                    markAngs.append(Vector2D(0,0));
                    markRoles.append(QString("shotBlocker"));
                }
            }
            DBUG(QString("mark pos : %1").arg(markPoses.size()) , D_AHZ);
            DBUG(QString("our mark : %1").arg(ourMarkAgentsSize) , D_AHZ);
        }
        else{
            for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                markRoles.append(QString("passBlocker"));
                //////////// Don't Enter penalty area, mark agents!!! :) ///////////
                if(wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)) , &sol7,  &sol8)){
                    if(!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))){
                        opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                        Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                        wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                        markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 ,Segment2D(sol5 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol5:sol6 ,proportionOfDistance));
                    }
                    else{
                        wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Line2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                        markPoses.append(Segment2D(sol1 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol2 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol1 : sol2);
                    }
                }
                else{
                    opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                    markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4, proportionOfDistance));
                }
                markAngs.append(wm->ball->pos - markPoses.at(i));
                drawer->draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4), QColor(Qt::red));
                drawer->draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
                DBUG(QString("Man To Man Mark In PlayOn Mode / BlockPass / opp < our") , D_SEPEHR);
            }
            ////////////// With Extra mark agents ch ghalati bokonim ? :) //////////
            if(ourMarkAgentsSize - markPoses.size() == opponentAgentsToBeMarkPossition.size()){
                for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                    drawer->draw(opponentAgentsToBeMarkCircle.at(i),QColor(Qt::cyan));
                }
                for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                    if(!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))){
                        opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                        wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(opponentAgentsToBeMarkPossition.at(i) , wm->field->ourGoal()), &sol3 , &sol4);
                        markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 ,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3:sol4 ,conf.ShootRatioBlock / 100));
                    }
                    else{
                        wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Line2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                        markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                    }
                    markAngs.append(opponentAgentsToBeMarkPossition.at(i) - wm->field->ourGoal());
                    markRoles.append(QString("shotBlocker"));
                }
            }
            else if(ourMarkAgentsSize - markPoses.size() < opponentAgentsToBeMarkPossition.size()){
                tempSortDangerAgentsToBeBlockPassPlayOff = sortdangerpassplayoff(opponentAgentsToBeMarkPossition);
                for(i = 0 ; i < tempSortDangerAgentsToBeBlockPassPlayOff.size() ; i++){
                    tempOpponentAgentsToBeMarkedCircle.append(Circle2D(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first , opponentAgentsCircleR));
                    drawer->draw(tempOpponentAgentsToBeMarkedCircle.at(i),QColor(Qt::yellow));
                }
                for(i = 0 ; i < ourMarkAgentsSize - markPoses.size() ; i++){
                    if(!wm->field->isInOurPenaltyArea(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first)){
                        tempOpponentAgentsToBeMarkedCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first), &sol1 , &sol2);
                        wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first , wm->field->ourGoal()), &sol3 , &sol4);
                        markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 ,Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3:sol4 ,conf.ShootRatioBlock / 100));
                    }
                    else{
                        wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Line2D(wm->field->ourGoal() , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first) , &sol1 , &sol2);
                        markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                    }
                    markAngs.append(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first - wm->field->ourGoal());
                    markRoles.append(QString("shotBlocker"));
                }
            }
            else if(ourMarkAgentsSize - markPoses.size() > opponentAgentsToBeMarkPossition.size()){
                for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                    drawer->draw(opponentAgentsToBeMarkCircle.at(i),QColor(Qt::cyan));
                }
                for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                    if(!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))){
                        opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                        wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(opponentAgentsToBeMarkPossition.at(i) , wm->field->ourGoal()), &sol3 , &sol4);
                        markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 ,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3:sol4 ,conf.ShootRatioBlock / 100));
                    }
                    else{
                        wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Line2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                        markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                    }
                    markAngs.append(opponentAgentsToBeMarkPossition.at(i) - wm->field->ourGoal());
                    markRoles.append(QString("shotBlocker"));
                }
                for(i = 0 ; i < wm->opp.activeAgentsCount() ; i++){
                    opponentAgentsCircle.append(Circle2D(wm->opp.active(i)->pos , opponentAgentsCircleR));
                }
                for(i = 0 ; i < wm->opp.activeAgentsCount() ; i++){
                    for(j = 0 ; j < opponentAgentsToBeMarkPossition.size() ; j++){
                        if(opponentAgentsToBeMarkPossition.at(j) == wm->opp.active(i)->pos){
                            opponentAgentsCircle.removeAt(i);
                        }
                    }
                }
                for(i = 0 ; i < wm->opp.activeAgentsCount() ; i++){
                    if(Circle2D(wm->ball->pos , ballCircleR).contains(wm->opp.active(i)->pos)){
                        opponentAgentsCircle.removeAt(i);
                    }
                }
                if(opponentAgentsToBeMarkPossition.empty()){
                    for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                        if(!wm->field->isInOurPenaltyArea(opponentAgentsCircle.at(i).center())){
                            opponentAgentsCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsCircle.at(i).center()), &sol1 , &sol2);
                            wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(opponentAgentsCircle.at(i).center() , wm->field->ourGoal()), &sol3 , &sol4);
                            markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 ,Segment2D(sol3 , opponentAgentsCircle.at(i).center()).length() < Segment2D(sol4 , opponentAgentsCircle.at(i).center()).length() ? sol3:sol4 ,conf.ShootRatioBlock / 100));
                        }
                        else{
                            wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Line2D(wm->field->ourGoal() , opponentAgentsCircle.at(i).center()) , &sol1 , &sol2);
                            markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                        }
                        markAngs.append(opponentAgentsCircle.at(i).center() - wm->field->ourGoal());
                        markRoles.append(QString("shotBlocker"));
                    }
                }
                else{
                    for(i = 0 ; i < ourMarkAgentsSize - markPoses.size()+1 ; i++){
                        if(!wm->field->isInOurPenaltyArea(opponentAgentsCircle.at(i).center())){
                            opponentAgentsCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsCircle.at(i).center()), &sol1 , &sol2);
                            wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(opponentAgentsCircle.at(i).center() , wm->field->ourGoal()), &sol3 , &sol4);
                            markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 ,Segment2D(sol3 , opponentAgentsCircle.at(i).center()).length() < Segment2D(sol4 , opponentAgentsCircle.at(i).center()).length() ? sol3:sol4 ,conf.ShootRatioBlock / 100));
                        }
                        else{
                            wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Line2D(wm->field->ourGoal() , opponentAgentsCircle.at(i).center()) , &sol1 , &sol2);
                            markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                        }
                        markAngs.append(opponentAgentsCircle.at(i).center() - wm->field->ourGoal());
                        markRoles.append(QString("shotBlocker"));
                    }
                }
            }
        }
        if(ourMarkAgentsSize > markPoses.size()){
            for(i = 0 ; i < ourMarkAgentsSize - markPoses.size() ; i++){
                if(i % 2){
                    markPoses.append(Vector2D(-3 , (i+1) / 0.75));
                    markAngs.append(Vector2D(0,0));
                    markRoles.append(QString("shotBlocker"));
                }
                else{
                    markPoses.append(Vector2D(-3,(-i-1)  / 0.75));
                    markAngs.append(Vector2D(0,0));
                    markRoles.append(QString("shotBlocker"));
                }
            }
        }
    }
    else if(opponentAgentsToBeMarkPossition.size() > ourMarkAgentsSize){
        sortDangerAgentsToBeMarkBlockPassPlayOff = sortdangerpassplayoff(opponentAgentsToBeMarkPossition);
        for(i = 0 ; i < ourMarkAgentsSize; i++){
            //////////// Don't Enter penalty area, mark agents!!! :) ///////////
            if(wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(wm->ball->pos , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first) , &sol7,  &sol8)){
                if(!wm->field->isInOurPenaltyArea(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first)){
                    Circle2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , opponentAgentsCircleR).intersection(Segment2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol3 , &sol4);
                    wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() ? sol3 : sol4) , &sol5 , &sol6);
                    markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 ,Segment2D(sol5 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() < Segment2D(sol6 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() ? sol5:sol6 ,proportionOfDistance));
                }
                else{
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol1 , &sol2);
                    wm->field->ourBigPenaltyArea(1,0.3,0).intersection(Segment2D(wm->ball->pos , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first) , &sol5 , &sol6);
                    markPoses.append(know->getPointInDirection(Segment2D(sol1 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() < Segment2D(sol2 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() ? sol1 : sol2 ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 :sol6 ,proportionOfDistance));
                }
            }
            else{
                Circle2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , opponentAgentsCircleR).intersection(Segment2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol1 , &sol2);
                Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first), &sol3 , &sol4);
                markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() ? sol3 : sol4, proportionOfDistance));
            }
            markAngs.append(wm->ball->pos - markPoses.at(i));
            markRoles.append(QString("passBlocker"));
            drawer->draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                   Segment2D(sol3 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() ? sol3 : sol4)
                         , QColor(Qt::red));
            drawer->draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
            DBUG(QString("Man To Man Mark In PlayOn Mode / BlockPass / opp > our") , D_SEPEHR);
        }
    }
    //////////////// drawer->draw Possition of Mark Agents //////////////////////////
    for(i = 0 ; i < markPoses.size() ; i++){
        DBUG(QString("x : %1").arg(markPoses.at(i).x) , D_AHZ);
        drawer->draw(markPoses.at(i), "white");
        drawer->draw(markRoles.at(i) , markPoses.at(i) - Vector2D(0,0.4) , "white");
    }
}
//////////////////////////////// end of AHZ ///////////////////////////////////

//-------//mhsh - HMD//----------//


QList<Vector2D> CMarkPlan::ZoneMark(QList<QPair<Vector2D, double> > Posdanger , int marknum)
{
    //input is sorted Dangerious Weigth
    double zoneradius = .2;
    double limitradus = 1.5;
    double manTomanRadius = 1.0;
    Vector2D sol1,sol2,sol;
    Line2D tempLine(Vector2D(0,0), Vector2D(1,1));
    Segment2D tempseg;
    Circle2D circlemilde;
    Circle2D circlelimit;
    Circle2D manTomanArea(wm->ball->pos, manTomanRadius);
    Vector2D midle;
    QList<Vector2D> output;
    output.clear();
    segmentpershoot = conf.ShootRatioBlock/100;
    if (marknum == 1)
    {
        //  Vector2D::angleOf(Posdanger[0].first, wm->field->ourGoal(), posdanger[1]).degree();
        if(Posdanger.size() == 1)
        {
            //Shoot block
            tempseg.assign(Posdanger[0].first, wm->field->ourGoal());
            markPoses.append(Posdanger[0].first + (wm->field->ourGoal() - Posdanger[0].first) * segmentpershoot);
            markAngs.append(Posdanger[0].first - wm->field->ourGoal());

        }
        if(Posdanger.size() > 1)
        {
            /* real Zone

            tempseg.assign(Posdanger[0].first, Posdanger[1].first);

            drawer->draw(tempseg,QColor(Qt::green));
            midle = tempseg.intersection(tempseg.perpendicularBisector());
            tempseg.assign(wm->field->ourGoal(), midle);
            tempLine.assign(wm->field->ourGoal(),midle);
            drawer->draw(tempseg, QColor(Qt::green));

            circlemilde.assign(midle, zoneradius);
            circlemilde.intersection(tempLine, &sol1, &sol2);
            circlelimit.assign(wm->field->ourGoal(), limitradus);


            //cheking which on is nearest
            if((wm->field->ourGoal() - sol1).length() < (wm->field->ourGoal() - sol2).length())
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            if(!circlelimit.contains(sol))
            {
                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }
            else
            {

                circlelimit.intersection(tempLine, &sol1, &sol2);
                if(sol1.x > -1.0 * _FIELD_WIDTH / 2)
                {
                    sol = sol1;
                }
                else
                {
                    sol = sol2;
                }


                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }


*/


            // man to man
            markPoses.append(ShootBlockRatio(segmentpershoot, Posdanger[0].first).first());
            markAngs.append(ShootBlockRatio(segmentpershoot, Posdanger[0].first).last());

        }
    }

    else if(marknum == 2)
    {
        if(Posdanger.size() == 1)
        {
            //Shoot block
            markPoses.append(ShootBlockRatio(segmentpershoot, Posdanger[0].first).first());
            markAngs.append(ShootBlockRatio(segmentpershoot, Posdanger[0].first).last());

            //block pass
            tempseg.assign(wm->ball->pos, Posdanger[0].first);
            markPoses.append(tempseg.intersection(tempseg.perpendicularBisector()));
            markAngs.append(Posdanger[0].first - markPoses.last());




        }
        if(Posdanger.size() == 2)
        {

            //shoot block
            markPoses.append(ShootBlockRatio(segmentpershoot, Posdanger[0].first).first());
            markAngs.append(ShootBlockRatio(segmentpershoot, Posdanger[0].first).last());

            markPoses.append(ShootBlockRatio(segmentpershoot, Posdanger[1].first).first());
            markAngs.append(ShootBlockRatio(segmentpershoot, Posdanger[1].first).last());



        }
        if(Posdanger.size() > 2)
        {
            /* zone mark
            ////fisrt one
            tempseg.assign(Posdanger[0].first, Posdanger[1].first);

            drawer->draw(tempseg,QColor(Qt::green));
            midle = tempseg.intersection(tempseg.perpendicularBisector());
            tempseg.assign(wm->field->ourGoal(), midle);
            tempLine.assign(wm->field->ourGoal(),midle);
            drawer->draw(tempseg, QColor(Qt::green));

            circlemilde.assign(midle, zoneradius);
            circlemilde.intersection(tempLine, &sol1, &sol2);
            circlelimit.assign(wm->field->ourGoal(), limitradus);


            //cheking which on is nearest
            if((wm->field->ourGoal() - sol1).length() < (wm->field->ourGoal() - sol2).length())
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            if(!circlelimit.contains(sol))
            {
                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }
            else
            {
                debug(QString("Going to penalty Area"), D_MAHI);
                debug(QString("sol1:x:%1y:%2 sol2:x:%3y:%4").arg(sol1.x).arg(sol1.y).arg(sol2.x).arg(sol2.y),D_MAHI);
                circlelimit.intersection(tempLine, &sol1, &sol2);
                if(sol1.x > -1.0 * _FIELD_WIDTH / 2)
                {
                    sol = sol1;
                }
                else
                {
                    sol = sol2;
                }



                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }



            ///second one

            tempseg.assign(Posdanger[1].first, Posdanger[2].first);

            drawer->draw(tempseg,QColor(Qt::green));
            midle = tempseg.intersection(tempseg.perpendicularBisector());
            tempseg.assign(wm->field->ourGoal(), midle);
            tempLine.assign(wm->field->ourGoal(),midle);
            drawer->draw(tempseg, QColor(Qt::green));

            circlemilde.assign(midle, zoneradius);
            circlemilde.intersection(tempLine, &sol1, &sol2);
            circlelimit.assign(wm->field->ourGoal(), limitradus);


            //cheking which on is nearest
            if((wm->field->ourGoal() - sol1).length() < (wm->field->ourGoal() - sol2).length())
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            if(!circlelimit.contains(sol))
            {
                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }
            else
            {
                debug(QString("Going to penalty Area"), D_MAHI);
                debug(QString("sol1:x:%1y:%2 sol2:x:%3y:%4").arg(sol1.x).arg(sol1.y).arg(sol2.x).arg(sol2.y),D_MAHI);

                circlelimit.intersection(tempLine, &sol1, &sol2);
                if(sol1.x > -1.0 * _FIELD_WIDTH / 2)
                {
                    sol = sol1;
                }
                else
                {
                    sol = sol2;
                }



                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }


*/
            // man to man

            //shoot block
            markPoses.append(ShootBlockRatio(segmentpershoot, Posdanger[0].first).first());
            markAngs.append(ShootBlockRatio(segmentpershoot, Posdanger[0].first).last());

            markPoses.append(ShootBlockRatio(segmentpershoot, Posdanger[1].first).first());
            markAngs.append(ShootBlockRatio(segmentpershoot, Posdanger[1].first).last());


        }
    }
    else if(marknum == 3)
    {
        if(Posdanger.size() > 3)
        {
            tempseg.assign(Posdanger[0].first, Posdanger[1].first);

            drawer->draw(tempseg,QColor(Qt::green));
            midle = tempseg.intersection(tempseg.perpendicularBisector());
            tempseg.assign(wm->field->ourGoal(), midle);
            tempLine.assign(wm->field->ourGoal(),midle);
            drawer->draw(tempseg, QColor(Qt::green));

            circlemilde.assign(midle, zoneradius);
            circlemilde.intersection(tempLine, &sol1, &sol2);
            circlelimit.assign(wm->field->ourGoal(), limitradus);


            //cheking which on is nearest
            if((wm->field->ourGoal() - sol1).length() < (wm->field->ourGoal() - sol2).length())
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            if(!circlelimit.contains(sol))
            {
                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }
            else
            {
                circlelimit.intersection(tempLine, &sol1, &sol2);
                if(sol1.x > -1.0 * wm->field->_FIELD_WIDTH / 2)
                {
                    sol = sol1;
                }
                else
                {
                    sol = sol2;
                }



                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }


            ///second one

            tempseg.assign(Posdanger[1].first, Posdanger[2].first);

            drawer->draw(tempseg,QColor(Qt::green));
            midle = tempseg.intersection(tempseg.perpendicularBisector());
            tempseg.assign(wm->field->ourGoal(), midle);
            tempLine.assign(wm->field->ourGoal(),midle);
            drawer->draw(tempseg, QColor(Qt::green));

            circlemilde.assign(midle, zoneradius);
            circlemilde.intersection(tempLine, &sol1, &sol2);
            circlelimit.assign(wm->field->ourGoal(), limitradus);


            //cheking which on is nearest
            if((wm->field->ourGoal() - sol1).length() < (wm->field->ourGoal() - sol2).length())
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            if(!circlelimit.contains(sol))
            {
                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }
            else
            {
                circlelimit.intersection(tempLine, &sol1, &sol2);
                if(sol1.x > -1.0 * wm->field->_FIELD_WIDTH / 2)
                {
                    sol = sol1;
                }
                else
                {
                    sol = sol2;
                }



                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }


            //third one


            tempseg.assign(Posdanger[2].first, Posdanger[3].first);

            drawer->draw(tempseg,QColor(Qt::green));
            midle = tempseg.intersection(tempseg.perpendicularBisector());
            tempseg.assign(wm->field->ourGoal(), midle);
            tempLine.assign(wm->field->ourGoal(),midle);
            drawer->draw(tempseg, QColor(Qt::green));

            circlemilde.assign(midle, zoneradius);
            circlemilde.intersection(tempLine, &sol1, &sol2);
            circlelimit.assign(wm->field->ourGoal(), limitradus);


            //cheking which on is nearest
            if((wm->field->ourGoal() - sol1).length() < (wm->field->ourGoal() - sol2).length())
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            if(!circlelimit.contains(sol))
            {
                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }
            else
            {
                circlelimit.intersection(tempLine, &sol1, &sol2);
                if(sol1.x > -1.0 * wm->field->_FIELD_WIDTH / 2)
                {
                    sol = sol1;
                }
                else
                {
                    sol = sol2;
                }



                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }


        }

    }
    return output;


}

QList<QPair<Vector2D, double> > CMarkPlan::sortdangershoot(double _radius, double treshold) {

    Vector2D sol1,sol2,sol3;
    Vector2D _pos1 = wm->ball->pos;
    Vector2D _pos2 = wm->ball->pos + 5.0  * wm->ball->vel;
    Line2D _path(_pos1,_pos2);
    Polygon2D _poly;
    Circle2D(_pos2,_radius + treshold).
            intersection(_path.perpendicular(_pos2),&sol1,&sol2);


    _poly.addVertex(sol1);
    sol3 = sol1;
    _poly.addVertex(sol2);
    Circle2D(_pos1,Robot::robot_radius_new + treshold).
            intersection(_path.perpendicular(_pos1),&sol1,&sol2);

    _poly.addVertex(sol2);
    _poly.addVertex(sol1);
    _poly.addVertex(sol3);

    drawer->draw(_poly,"blue");


    QPair<Vector2D, double> temp;
    QList<QPair<Vector2D, double> > output;
    output.clear();


    double KA = 1; //Angle parameter
    double KDB = 1; //distancetoball
    double KDI = 1; //distancetointersect

    double Angle;
    double distanceToBallProjection;
    double distanceToIntersect;
    double danger;

    double RangeofAngle = 90;
    double RangeofdistanceToBallProjection = Segment2D(Vector2D(-1.0 * wm->field->_FIELD_WIDTH / 2, -1.0 * wm->field->_FIELD_HEIGHT /2 ), Vector2D(wm->field->_FIELD_WIDTH / 2 , wm->field->_FIELD_HEIGHT / 2)).length();
    double RangeofdistanceToIntersect =  _radius;


    for(int i = 0;i < oppAgentsToMarkPos.count();i++) {
        if(_poly.contains(oppAgentsToMarkPos[i]))
        {
            Angle = Vector2D::angleOf( oppAgentsToMarkPos[i], wm->ball->pos, _path.projection(oppAgentsToMarkPos[i]) ).degree();
            temp.first = oppAgentsToMarkPos[i];


            distanceToIntersect = _path.dist(oppAgentsToMarkPos[i]);
            distanceToBallProjection = _path.projection(oppAgentsToMarkPos[i]).length();
            danger = KA * ( 1 - Angle/RangeofAngle) + KDB * (1 - distanceToBallProjection/RangeofdistanceToBallProjection ) + KDI * (1 - distanceToIntersect / RangeofdistanceToIntersect);
            temp.second = danger;
            output.append(temp);
        }

    }
    if(!output.isEmpty())
    {

        Segment2D tempsegment;
        tempsegment.assign(output.first().first, wm->field->ourGoal());

        drawer->draw(tempsegment, QColor(Qt::blue));
    }





    ///////---------sorting------------//////
    for(int i=0; i<output.count(); i++)
    {

        for(int j=0; j<output.count() - 1; j++)
        {
            if(output[j].second < output[j+1].second)
                output.swap(j,j+1);
        }

    }
    // this is for test
    output.clear();

    return output;
}


QList<CRobot*> CMarkPlan::sortdanger(const QList<CRobot*> oppagent)
{
    const QList<CRobot*> &sorted = oppagent;
    for(int i=0; i<sorted.count(); i++)
    {

        for(int k = 0; k<sorted.count() - 1; k++)
        {

            //                if(sorted[k]->danger < sorted[k+1]->danger) //todo : crobot
            //                {
            //                    sorted.swap(k, k+1);
            //                }

        }

    }
    return sorted;

}

QList<QPair<Vector2D, double> > CMarkPlan::sortdangerpassplayoff(QList<Vector2D> oppposdanger) {



    ///////////////
    double KA=1; //Angle Coefficient
    double KDB=1;  //Distance To Ball
    double KDG=1;  //Distnce To Goal
    double RangeofAngle = Vector2D::angleOf(wm->field->ourGoalR(),Vector2D(-1.0 * (wm->field->_FIELD_WIDTH / 2 - wm->field->_PENALTY_DEPTH), 0), wm->field->ourGoalL()).degree();
    //drawer->draw(Vector2D(-1.0 * (wm->field->_FIELD_WIDTH - _GOAL_WIDTH), 0), QColor(Qt::red));
    // double RangeofAngle2 = Vector2D::angleOf(wm->field->ou,Vector2D(0, -1.0 * (wm->field->_FIELD_WIDTH - _GOAL_WIDTH)), wm->field->ourGoalL()).degree();

    double RangeofDistancetoBall = fabs(Segment2D(Vector2D(wm->field->_FIELD_WIDTH/2,wm->field->_FIELD_HEIGHT /2), Vector2D(-1.0 * wm->field->_FIELD_WIDTH/2,-1.0 * wm->field->_FIELD_HEIGHT /2)).length());

    double RangeofDistancetoGoal = fabs(Segment2D(Vector2D(wm->field->_FIELD_WIDTH/2,wm->field->_FIELD_HEIGHT /2), wm->field->ourGoal()).length());

    //double RangeofTempDis = 2;
    double angle, distancetoball, distancetogoal,danger;


    QPair<Vector2D, double> temp;
    QList<QPair<Vector2D, double> > output;
    for(int i = 0; i<oppposdanger.count(); i++) {
        temp.first = oppposdanger[i];


        angle = Vector2D::angleOf(wm->field->ourGoalR(), oppposdanger[i], wm->field->ourGoalL() ).degree();
        distancetoball =  (oppposdanger[i] - wm->ball->pos).length();
        distancetogoal =  (oppposdanger[i] - wm->field->ourGoal()).length();
        danger = (KA * fabs(angle) / RangeofAngle) + ( KDB * 1 - (distancetoball / RangeofDistancetoBall) ) + (KDG * 1 -(distancetogoal / RangeofDistancetoGoal));
        temp.second = danger;
        output.append(temp);
        // finding nearest to intersect
        Segment2D tempsegment;
        tempsegment.assign(oppposdanger[i],wm->field->ourGoal());

        double mintempdis = 0.0;
        if(wm->our.activeAgentsCount() != 0)
            mintempdis = tempsegment.dist(wm->our.active(0)->pos);

        for(int j=0; j<wm->our.activeAgentsCount(); j++)
        {
            if(tempsegment.dist(wm->our.active(j)->pos) < mintempdis)
            {
                mintempdis = tempsegment.dist(wm->our.active(j)->pos);
            }

        }
    }

    ///sorting the Qlist
    for(int i = 0; i< output.count(); i++)
    {
        for(int j = 0; j< output.count() - 1; j++ )
        {
            if(output[j].second < output[j + 1].second)
                output.swap(j, j+1);
        }
    }

    return output;
}

Vector2D CMarkPlan::findcenterdefence()
{

    Segment2D tempsegment;

    // TODO : QList<Agent*> defenseagents should be acquired
    //    if(knowledge->defenseAgents.count() == 0)
    //    {
    //        return Vector2D(-2, 0);
    //
    //    }
    //
    //    else if (knowledge->defenseAgents.count() == 1)
    //    {
    //        return knowledge->defenseAgents[0]->pos();
    //
    //    }
    //
    //
    //
    //    else if(knowledge->defenseAgents.count() == 2)
    //    {
    //        tempsegment.assign(knowledge->defenseAgents[0]->pos(),knowledge->defenseAgents[1]->pos());
    //        return tempsegment.intersection(tempsegment.perpendicularBisector());
    //    }
    //
    //    else
    //    {
    //        return Vector2D(2,0);
    //    }

}


QList<QPair<Vector2D, double> > CMarkPlan::sortdangerpassplayon(QList<Vector2D> oppposdanger) {

    double KA=1; //Angle Coefficient
    double KDB=0;  //Distance To Ball
    double KDG=1;  //Distnce To Goal
    double KAD=2;  // angle of difense
    double RangeofAngle = Vector2D::angleOf(wm->field->ourGoalR(),Vector2D(-1.0 * (wm->field->_FIELD_WIDTH / 2 - wm->field->_PENALTY_DEPTH), 0), wm->field->ourGoalL()).degree();
    double RangeofDistancetoBall = fabs(Segment2D(Vector2D(wm->field->_FIELD_WIDTH/2,wm->field->_FIELD_HEIGHT /2), Vector2D(-1.0 * wm->field->_FIELD_WIDTH/2,-1.0 * wm->field->_FIELD_HEIGHT /2)).length());

    double RangeofDistancetoGoal = fabs(Segment2D(Vector2D(wm->field->_FIELD_WIDTH/2,wm->field->_FIELD_HEIGHT /2), wm->field->ourGoal()).length());

    double RangeofTempDis = 2;


    double RangeofAngleDef = 180;

    double angle, angleDef, distancetoball, distancetogoal, danger;

    ///finding center of defense


    QPair<Vector2D, double> temp;
    QList<QPair<Vector2D, double> > output;
    for(int i = 0; i<oppposdanger.count(); i++) {
        temp.first = oppposdanger[i];
        angle = Vector2D::angleOf(wm->field->ourGoalR(), oppposdanger[i], wm->field->ourGoalL() ).degree();
        distancetoball =  (oppposdanger[i] - wm->ball->pos).length();
        distancetogoal =  (oppposdanger[i] - wm->field->ourGoal()).length();

        angleDef = Vector2D::angleOf(oppposdanger[i], wm->field->ourGoal(), findcenterdefence()).degree();
        danger = (KA * fabs(angle) / RangeofAngle) + ( KDB * (1 - (distancetoball / RangeofDistancetoBall)) ) + (KDG * (1 -(distancetogoal / RangeofDistancetoGoal))) + (KAD * angleDef / RangeofAngleDef);
        temp.second = danger;
        output.append(temp);
    }

    ///sorting the Qlist
    for(int i = 0; i< output.count(); i++)
    {
        for(int j = 0; j< output.count() - 1; j++ )
        {
            if(output[j].second < output[j + 1].second)
                output.swap(j, j+1);
        }
    }
    return output;
}


bool CMarkPlan::sortBy(const Vector2D &robot1,const Vector2D &robot2)
{
    double dist2BallW=-1;
    double dist2GoalW=1;
    double thetaW=2;
    AngleDeg thetaMin=15;

    double ballW1 = (dist2BallW/(robot1.dist(wm->ball->pos)));
    double ballW2 = (dist2BallW/(robot2.dist(wm->ball->pos)));
    double goalW1 = (dist2GoalW/(robot1.dist(wm->field->ourGoal())));
    double goalW2 = (dist2GoalW/(robot2.dist(wm->field->ourGoal())));
    double theta1 = (robot1 - wm->field->ourGoal()).th().degree();
    if(fabs(theta1) < thetaMin.degree())
        theta1 = thetaMin.degree();
    double theta2 = (robot2 - wm->field->ourGoal()).th().degree();
    if(fabs(theta2) < thetaMin.degree())
        theta2 = thetaMin.degree();


    return ballW1  + goalW1 + (thetaW/theta1) > ballW2  + goalW2 + (thetaW/theta2);

}

Vector2D CMarkPlan::posvel(CRobot* opp){
    if(opp->vel.length() > 0.5 && opp->vel.x < 0)
        return opp->pos + 0.5 * opp->vel;
    else
        return opp->pos;
}
void CMarkPlan::findOppAgentsToMark()
{
    oppAgentsToMark.clear();
    oppAgentsMarkedByDef.clear();
    ignoredOppAgents.clear();

    for(int i=0; i<wm->opp.activeAgentsCount();i++){
        oppAgentsToMark.append(wm->opp.active(i));
        if(wm->field->isInOppPenaltyArea(oppAgentsToMark.last()->pos))
            oppAgentsToMark.removeOne(oppAgentsToMark.last());   // detection of the goali
    }

    oppAgentsToMarkPos.clear();

    if(gameState->isStart())
    {
        for(int i = 0; i < oppAgentsToMark.count(); i++)
        {
            if(oppAgentsToMark[i]->pos.x > conf.OppOmitLimitPlayon){
                oppAgentsToMark.removeOne(oppAgentsToMark[i]);
                i--;
            }
        }

        if(conf.OmmitNearestToBallPlayon)
        {
            DBUG(QString("Ommit nearest to ball"),D_MAHI);
            int nearestToBall = -1;
            double nearestToBallDist = 100000;

            for(int i = 0 ; i < oppAgentsToMark.count() ; i++)
            {
                if((oppAgentsToMark[i]->pos).dist(wm->ball->pos) < nearestToBallDist)
                {
                    nearestToBall = i;
                    nearestToBallDist = oppAgentsToMark[i]->pos.dist(wm->ball->pos);
                    DBUG(QString("the nearest id is:%1").arg(oppAgentsToMark[i]->id),D_MAHI);
                    drawer->draw(oppAgentsToMark[i]->pos + oppAgentsToMark[i]->vel);
                }
            }
            if(nearestToBall != -1)
                oppAgentsToMark.removeOne(oppAgentsToMark[nearestToBall]);
        }
    }
    else if(gameState->theirKickoff())
    {
        for(int i = 0; i < oppAgentsToMark.count(); i++)
        {
            if(posvel(oppAgentsToMark[i]).x > conf.OppOmitLimitKickOff){
                oppAgentsToMark.removeOne(oppAgentsToMark[i]);
                i--;
            }
        }

        //ommiting nearest to ball
        DBUG(QString("Ommit nearest to ball kick off"),D_MAHI);
        int nearestToBall = -1;
        double nearestToBallDist = 100000;

        for(int i = 0 ; i < oppAgentsToMark.count() ; i++)
        {
            if((oppAgentsToMark[i]->pos/*+ oppAgentsToMark[i]->vel*/).dist(wm->ball->pos /*+  wm->ball->vel*/) < nearestToBallDist)
            {
                nearestToBall = i;
                nearestToBallDist = oppAgentsToMark[i]->pos.dist(wm->ball->pos);
                DBUG(QString("the nearest id is:%1").arg(oppAgentsToMark[i]->id),D_MAHI);
                drawer->draw(oppAgentsToMark[i]->pos + oppAgentsToMark[i]->vel);
            }
        }
        if(nearestToBall != -1)
            oppAgentsToMark.removeOne(oppAgentsToMark[nearestToBall]);

    }

    //setting the positions
    for(int i=0; i < oppAgentsToMark.count(); i++)
    {
        if(gameState->isStart())
        {
            oppAgentsToMarkPos.append(oppAgentsToMark[i]->pos);
        }
        else if(gameState->theirKickoff())
        {
            oppAgentsToMarkPos.append(posvel(oppAgentsToMark[i]));
        }
    }


}

void CMarkPlan::markPosesRefinePlayon()
{
    Vector2D sol1, sol2, sol;
    Line2D tempLine(Vector2D(0,0), Vector2D(1,0));
    Circle2D _markAreaRestricted1(wm->field->ourGoal() + Vector2D(-1.0 * 0.2, 0), markRadiusStrict);
    Circle2D _markAreaRestricted2(wm->field->oppGoal() + Vector2D(0.2, 0), markRadiusStrict);
    for(int i = 0; i < markPoses.count(); i++)
    {
        tempLine.assign(wm->field->ourGoal(), markPoses[i]);
        if((wm->field->ourGoal() - markPoses[i]).length() < markRadiusStrict)
        {
            _markAreaRestricted1.intersection(tempLine, &sol1, &sol2);

            if(sol1.x > -1.0 * wm->field->_FIELD_WIDTH / 2)
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            markPoses[i] = sol;
        }

        tempLine.assign( markPoses[i], wm->field->oppGoal());

        if((markPoses[i] - wm->field->oppGoal()).length() < markRadiusStrict)
        {
            _markAreaRestricted2.intersection(tempLine, &sol1, &sol2);

            if(sol1.x < 1.0 * wm->field->_FIELD_WIDTH / 2)
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            markPoses[i] = sol;
        }
    }

    ///refinning kick off
    double xKickoff  = -.08;
    Circle2D indirect;
    //Vector2D sol1,sol2,sol;
    indirect.assign(Vector2D(0,0),.5);
    Segment2D tempMarkSeg;
    if(/* knowledge->getGameState() == CKnowledge::TheirKickOff */
            gameState->theirKickoff())
    {
        // Not positioning the Center circle
        for(int i=0; i<markPoses.count();i++)
        {

            if(indirect.contains(markPoses[i]))
            {
                tempMarkSeg.assign(markPoses[i], wm->field->ourGoal());
                indirect.intersection(tempMarkSeg,&sol1,&sol2);
                if(sol1.x<sol2.x)
                    sol = sol1;
                else
                    sol = sol2;

                markPoses[i] = sol;
            }
        }
        // not invading the opponent field
        for(int i=0; i < markPoses.count(); i++)
        {
            if(markPoses[i].x > xKickoff)
                markPoses[i].x = xKickoff;
        }
    }
}

QList<Vector2D> CMarkPlan::ShootBlockRatio(double ratio, Vector2D opp){
    Segment2D tempMarkSeg;
    QList<Vector2D> tempQlist;
    Circle2D indirect;
    Vector2D sol1,sol2,sol;
    Segment2D sag;
    sag.assign(Vector2D(0,-3) , Vector2D(0,3));
    indirect.assign(Vector2D(0,0),.7);
    tempQlist.clear();
    tempMarkSeg.assign(opp, wm->field->ourGoal());

    if(gameState->theirKickoff() ){
        if((opp + (wm->field->ourGoal() - opp) * ratio).x < 0 )
        {
            if(indirect.contains(opp + (wm->field->ourGoal() - opp) * ratio))
            {
                indirect.intersection(tempMarkSeg,&sol1,&sol2);
                if(sol1.x < sol2.x)
                    sol = sol1;
                else
                    sol = sol2;

                tempQlist.append(sol);
                tempQlist.append(opp - wm->field->ourGoal());
            }
            else{
                tempQlist.append(opp + (wm->field->ourGoal() - opp) * ratio);
                tempQlist.append(opp - wm->field->ourGoal());

            }
        }

        else{
            if(indirect.contains(opp + (wm->field->ourGoal() - opp) * ratio))
            {
                indirect.intersection(tempMarkSeg,&sol1,&sol2);
                if(sol1.x < sol2.x)
                    sol = sol1;
                else
                    sol = sol2;

                tempQlist.append(sol);
                tempQlist.append(opp - wm->field->ourGoal());
            }
            else{
                tempQlist.append(tempMarkSeg.intersection(sag));
                tempQlist.append(opp - wm->field->ourGoal());
            }

        }
    }
    else{
        tempQlist.append(opp + (wm->field->ourGoal() - opp) * ratio);
        tempQlist.append(opp - wm->field->ourGoal());
    }

    return tempQlist;
}

QList<Vector2D> CMarkPlan::PassBlockRatio(double ratio, Vector2D opp){
    Segment2D tempSeg;
    Segment2D isInPenaltyArea;
    Vector2D sol;
    Vector2D sol1[2];
    for(int i = 0 ; i < 2 ; i++){
        sol1[i].invalidate();
    }
    QList<Vector2D> tempQlist;
    tempQlist.clear();
    tempSeg.assign(wm->ball->pos, wm->ball->pos + (opp - wm->ball->pos) * 20);
    Vector2D pos = wm->ball->pos + (opp - wm->ball->pos) * ratio;
    isInPenaltyArea.assign(opp, wm->ball->pos);
    QList<Vector2D> tempVec;
    tempVec.clear();
    if(wm->field->ourBigPenaltyArea(1,0.3,0).intersection(isInPenaltyArea ,&sol1[0] , &sol1[1])){
        for(int i = 0 ; i < 2 ; i++){
            if(sol1[i].isValid()){
                tempVec.append(sol1[i]);
            }
        }
        if(tempVec.size() == 1)
        {
            tempQlist.append(tempVec.first());
        }
        else if(tempVec.size() == 2)
        {
            if((tempVec.first() - wm->ball->pos).length() > (tempVec.last() - wm->ball->pos).length())
            {
                sol = tempVec.first();
            }
            else if ((tempVec.last() - wm->ball->pos).length() > (tempVec.first() - wm->ball->pos).length())
            {
                sol = tempVec.last();
            }
            tempQlist.append(sol);
        }

        tempQlist.append( wm->ball->pos - opp);


        drawer->draw(tempSeg, "red");
        DBUG(QString("this is in the penalty area, Block pass Mode"), D_HAMED);
    }
    else
    {
        tempQlist.clear();
        tempQlist.append(pos);
        tempQlist.append( wm->ball->pos - opp);
        DBUG(QString("sag"), D_HAMED);
        drawer->draw(tempSeg, "blue");
    }
    return tempQlist;

}


void CMarkPlan::execute()
{
    DBUG(QString(" Mark play on Execute is runninng "), D_MAHI);
    findOppAgentsToMark();
    //sortdangerpass(oppAgentsToMarkPos);

    Circle2D MarkArea(wm->field->ourGoal(),markRadius);
    oppmarkedpos.clear();
    markPoses.clear();
    drawer->draw(MarkArea,QColor(Qt::blue));
    markAngs.clear();
    segmentpershoot = conf.ShootRatioBlock / 100;
    segmentperpass = (100 - conf.PassRatioBlock) / 100;
    QList<int> matchPoints;

    //////----------HMD Play on Mark-------------////


    if(gameState->isStart() )
    {
        if(conf.PlayOnManToMan){
            manToManMarkInPlayOnBlockPass(oppAgentsToMarkPos , agents.count() , conf.PassRatioBlock / 100);
        }
        else{
            markPoses.clear();
            markAngs.clear();
            if(agents.count() == oppAgentsToMarkPos.count())
            {
                for(int i = 0; i < oppAgentsToMarkPos.count(); i++)
                {
                    markPoses.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).first());
                    markAngs.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).last());
                }
            }

            else if(agents.count() < oppAgentsToMarkPos.count())
            {
                QList<QPair<Vector2D, double> >tempQlistQpair = sortdangerpassplayon(oppAgentsToMarkPos);
                for(int i=0; i<agents.count(); i++)
                {
                    markPoses.append(ShootBlockRatio(segmentpershoot, tempQlistQpair[i].first).first());
                    markAngs.append(ShootBlockRatio(segmentpershoot, tempQlistQpair[i].first).last());
                }
                DBUG(QString("size of temp qlist pair %1").arg(tempQlistQpair.size()), D_HAMED);
            }

            else if(agents.count() > oppAgentsToMarkPos.count())
            {
                QList<QPair<Vector2D, double> >tempQlistQpair = sortdangerpassplayon(oppAgentsToMarkPos);

                for(int i=0; i<oppAgentsToMarkPos.count(); i++)
                {
                    markPoses.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).first());
                    markAngs.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).last());
                }

                for(int i=0; i < min(agents.count() - oppAgentsToMarkPos.count(), oppAgentsToMarkPos.count()); i++)
                {

                    markPoses.append(PassBlockRatio(segmentperpass, tempQlistQpair[i].first).first());
                    markAngs.append(PassBlockRatio(segmentperpass, tempQlistQpair[i].first).last());

                    DBUG(QString("size of temp qlist pair %1").arg(tempQlistQpair.size()), D_HAMED);

                    /*markPoses.append(Vector2D(-1,-1));
                    markAngs.append(Vector2D(0,1));
                    */
                }

            }


            for(int i=0; i < agents.count(); i++)
            {
                drawer->draw(QString("I am Mark"), agents[i]->pos(),QColor(Qt::black));
            }


            drawer->draw(QString("Number of Mark %1").arg(agents.count()), Vector2D(-3,3));
            int count = 0;
            for(int i = 0; i < agents.count();i++)
            {
                if(markPoses.size() < agents.count())
                {
                    DBUG("EXTRA PLAY ON", D_HAMED);
                    markPoses.append(Vector2D(0,count));
                    markAngs.append(Vector2D(1,0));
                    count ++;
                }
            }
        }


        //Checking Not Going To Penalty Area
        markPosesRefinePlayon();

        matchPoints.clear();
        matchPoints.append(0);
        matchPoints.append(1);
        matchPoints.append(2);
        matchPoints.append(3);

        know->Matching(agents,markPoses,matchPoints); // todo : knowledge
        if(agents.count() == markPoses.count())
        {
            DBUG(QString("agnet count %1").arg(agents.count()) , D_AHZ);
            DBUG(QString("markpos %1").arg(markPoses.count()) , D_AHZ);
            DBUG(QString("match point %1").arg(matchPoints.size()) , D_AHZ);
            for(int i =0; i< markPoses.count(); i++){
                if(i < matchPoints.size()){

                    //markGPA[i]->init(markPoses[matchPoints[i]],markAngs[matchPoints[i]]);

                    markGPA[i]->setTargetpos(markPoses[matchPoints[i]]); //HINT : gpa->init
                    markGPA[i]->setTargetdir(markAngs[matchPoints[i]]);

                    markGPA[i]->setAvoidpenaltyarea(true);
                    //markGPA[i]->execute();
                    agents[i]->action = markGPA[i];
                }
            }

        }




    }
    else if(/*knowledge->getGameState() == CKnowledge::TheirKickOff */
            gameState->theirKickoff())
    {
        segmentpershoot = 0.3;
        markPoses.clear();
        markAngs.clear();
        DBUG(QString("Its TheirKickoff"),D_MAHI);
        findOppAgentsToMark();
        Segment2D tempsegLine, tempsegLine2;
        tempsegLine.assign(Vector2D(-2, wm->field->_FIELD_HEIGHT / 2), Vector2D(-2, -1.0 * wm->field->_FIELD_HEIGHT / 2 ));
        tempsegLine2.assign(Vector2D(-2, wm->field->_FIELD_HEIGHT / 2), Vector2D(-2, -1.0 * wm->field->_FIELD_HEIGHT / 2 ));

        if(agents.count() == oppAgentsToMarkPos.count())
        {
            for(int i = 0; i<oppAgentsToMarkPos.count(); i++)
            {
                markPoses.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).first());
                markAngs.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).last());
            }
        }

        else if(agents.count() < oppAgentsToMarkPos.count())
        {
            QList<QPair<Vector2D, double> >tempQlistQpair = sortdangerpassplayon(oppAgentsToMarkPos);
            for(int i=0; i<agents.count(); i++)
            {
                markPoses.append(ShootBlockRatio(segmentpershoot, tempQlistQpair[i].first).first());
                markAngs.append(ShootBlockRatio(segmentpershoot, tempQlistQpair[i].first).last());
            }


        }

        else if(agents.count() > oppAgentsToMarkPos.count())
        {

            for(int i=0; i<oppAgentsToMarkPos.count(); i++)
            {
                markPoses.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).first());
                markAngs.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).last());
            }
            for(int i=0; i< agents.count() - oppAgentsToMarkPos.count(); i++)
            {
                markPoses.append(Vector2D(-1,i));
                markAngs.append(Vector2D(-1,0));
                drawer->draw(Circle2D(markPoses.last(), 0.2),QColor(Qt::blue) );
            }

        }

        markPosesRefinePlayon();

        matchPoints.clear();
        matchPoints.append(0);
        matchPoints.append(1);
        matchPoints.append(2);
        matchPoints.append(3);

        know->Matching(agents, markPoses, matchPoints);
        if(agents.count() == markPoses.count())
        {
            for(int i =0; i<markPoses.count(); i++)
            {
                if(i < matchPoints.size()) {

                    markGPA[i]->setTargetpos(markPoses[matchPoints[i]]); //HINT : gpa->init
                    markGPA[i]->setTargetdir(markAngs[matchPoints[i]]);
                    //markGPA[i]->init(markPoses[matchPoints[i]],markAngs[matchPoints[i]]);
                    markGPA[i]->setAvoidpenaltyarea(true);
                    markGPA[i]->setAvoidcentercircle(true);
                    agents[i]->action = markGPA[i];

                }
            }

        }


    }

}

