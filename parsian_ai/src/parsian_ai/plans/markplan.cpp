#include <parsian_ai/plans/markplan.h>
#include <parsian_util/core/robot.h>
#include <algorithm>
#include <parsian_ai/util/worldmodel.h>
#include <parsian_ai/gamestate.h>
#include <parsian_ai/util/knowledge.h>
#include <parsian_ai/config.h>

using namespace std;

CMarkPlan::CMarkPlan() {
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
    segmentpershoot = 0.5;
    segmentperpass = 0.1;
    markRadius = 1.5;
    markRadiusStrict = 1.43;
    for (auto &i : markGPA) {
        i = new GotopointavoidAction;
    }
}

CMarkPlan::~CMarkPlan() {
    for (auto &i : markGPA) {
        delete i;
    }
}

bool CMarkPlan::isInIndirectArea(Vector2D aPoint) {
    //// check that a point is in the circle around the ball
    //// with 50cm radius or not.

    bool localFlag = Circle2D(wm->ball->pos , 0.7).contains(aPoint);
    return localFlag;
}

Line2D CMarkPlan::getBisectorLine(Vector2D firstPoint , Vector2D originPoint , Vector2D thirdPoint) {
    Line2D bisectorLine(originPoint , AngleDeg::bisect((firstPoint - originPoint).th() , (thirdPoint - originPoint).th()));
    return bisectorLine;
}

Segment2D CMarkPlan::getBisectorSegment(Vector2D firstPoint , Vector2D originPoint , Vector2D thirdPoint) {
    Line2D bisectorLine(originPoint , AngleDeg::bisect((firstPoint - originPoint).th() , (thirdPoint - originPoint).th()));
    Segment2D bisectorSegment(originPoint , Segment2D(thirdPoint , firstPoint).intersection(bisectorLine));
    return bisectorSegment;
}

void CMarkPlan::manToManMarkInPlayOnBlockPass(QList<Vector2D> opponentAgentsToBeMarkPossition , int ourMarkAgentsSize , double proportionOfDistance) {
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
    QList<QPair<Vector2D, double> > sortDangerAgentsToBeMarkBlockPassPlayOff;
    QList<QPair<Vector2D, double> > tempSortDangerAgentsToBeBlockPassPlayOff;
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
    for (auto i : opponentAgentsToBeMarkPossition) {
        if (!isInIndirectArea(i)) {
            tempOpponentAgentsToBeMarkedPosition.append(i);
        }
    }
    opponentAgentsToBeMarkPossition.clear();
    opponentAgentsToBeMarkPossition = tempOpponentAgentsToBeMarkedPosition;
    for (i = 0 ; i < opponentAgentsToBeMarkPossition.size(); i++) {
        opponentAgentsToBeMarkCircle.append(Circle2D(opponentAgentsToBeMarkPossition.at(i) , opponentAgentsCircleR));
        drawer->draw(opponentAgentsToBeMarkCircle.at(i) , "Cyan");
    }
    DBUG(QString("Opponent Agents to be mark count : %1").arg(opponentAgentsToBeMarkPossition.size()) , D_SEPEHR);
    ///////////////// Block Pass Plan ////////////////////////////////////
    if (opponentAgentsToBeMarkPossition.size() == ourMarkAgentsSize) {
        for (i = 0 ; i < ourMarkAgentsSize ; i++) {
            markRoles.append(QString("passBlocker"));
            //////////// Don't Enter penalty area, mark agents!!! :) ///////////
            if (wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)) , &sol7,  &sol8)) {
                if (!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))) {
                    opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                    wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2, Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                    markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol5 : sol6 , proportionOfDistance));
                } else {
                    wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                    markPoses.append(Segment2D(sol1 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol2 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol1 : sol2);
                }
            } else {
                opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2, Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4, proportionOfDistance));
            }
            markAngs.append(wm->ball->pos - markPoses.at(i));
            drawer->draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                   Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4) , QColor(Qt::red));
            drawer->draw(Circle2D(wm->ball->pos , ballCircleR), QColor(Qt::black));
            DBUG(QString("Man To Man Mark In PlayOff Mode / BlockPass / our = opp") , D_SEPEHR);
        }
    } else if (opponentAgentsToBeMarkPossition.size() < ourMarkAgentsSize) {
        if (opponentAgentsToBeMarkPossition.empty()) {
            for (i = 0 ; i < ourMarkAgentsSize ; i++) {
                if (i % 2) {
                    markPoses.append(Vector2D(-3 , i / 1.5));
                    markAngs.append(Vector2D(0, 0));
                    markRoles.append(QString("shotBlocker"));
                } else {
                    markPoses.append(Vector2D(-3, -i  / 1.5));
                    markAngs.append(Vector2D(0, 0));
                    markRoles.append(QString("shotBlocker"));
                }
            }
        } else if (opponentAgentsToBeMarkPossition.size() == 1) {
            for (i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++) {
                markRoles.append(QString("passBlocker"));
                //////////// Don't Enter penalty area, mark agents!!! :) ///////////
                if (wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)) , &sol7,  &sol8)) {
                    if (!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))) {
                        opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                        Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                        wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2, Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                        markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol5 : sol6 , proportionOfDistance));
                    } else {
                        wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Line2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                        markPoses.append(Segment2D(sol1 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol2 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol1 : sol2);
                    }
                } else {
                    opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                    markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2, Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4, proportionOfDistance));
                }
                markAngs.append(wm->ball->pos - markPoses.at(i));
                drawer->draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2, Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4), QColor(Qt::red));
                drawer->draw(Circle2D(wm->ball->pos , ballCircleR), QColor(Qt::black));
            }
            for (i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++) {
                if (!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))) {
                    opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                    wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(opponentAgentsToBeMarkPossition.at(i) , wm->field->ourGoal()), &sol3 , &sol4);
                    markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 , Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4 , conf.ShootRatioBlock / 100));
                } else {
                    wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Line2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                    markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                }
                markAngs.append(opponentAgentsToBeMarkPossition.at(i) - wm->field->ourGoal());
                markRoles.append(QString("shotBlocker"));
            }
            for (i = 0 ; i < ourMarkAgentsSize - markPoses.size() ; i++) {
                if (i % 2) {
                    markPoses.append(Vector2D(-3 , i / 1.5));
                    markAngs.append(Vector2D(0, 0));
                    markRoles.append(QString("shotBlocker"));
                } else {
                    markPoses.append(Vector2D(-3, -i  / 1.5));
                    markAngs.append(Vector2D(0, 0));
                    markRoles.append(QString("shotBlocker"));
                }
            }
            DBUG(QString("mark pos : %1").arg(markPoses.size()) , D_AHZ);
            DBUG(QString("our mark : %1").arg(ourMarkAgentsSize) , D_AHZ);
        } else {
            for (i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++) {
                markRoles.append(QString("passBlocker"));
                //////////// Don't Enter penalty area, mark agents!!! :) ///////////
                if (wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)) , &sol7,  &sol8)) {
                    if (!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))) {
                        opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                        Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                        wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2, Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                        markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol5 : sol6 , proportionOfDistance));
                    } else {
                        wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Line2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                        markPoses.append(Segment2D(sol1 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol2 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol1 : sol2);
                    }
                } else {
                    opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                    markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2, Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4, proportionOfDistance));
                }
                markAngs.append(wm->ball->pos - markPoses.at(i));
                drawer->draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2, Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4), QColor(Qt::red));
                drawer->draw(Circle2D(wm->ball->pos , ballCircleR), QColor(Qt::black));
                DBUG(QString("Man To Man Mark In PlayOn Mode / BlockPass / opp < our") , D_SEPEHR);
            }
            ////////////// With Extra mark agents ch ghalati bokonim ? :) //////////
            if (ourMarkAgentsSize - markPoses.size() == opponentAgentsToBeMarkPossition.size()) {
                for (i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++) {
                    drawer->draw(opponentAgentsToBeMarkCircle.at(i), QColor(Qt::cyan));
                }
                for (i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++) {
                    if (!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))) {
                        opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                        wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(opponentAgentsToBeMarkPossition.at(i) , wm->field->ourGoal()), &sol3 , &sol4);
                        markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 , Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4 , conf.ShootRatioBlock / 100));
                    } else {
                        wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Line2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                        markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                    }
                    markAngs.append(opponentAgentsToBeMarkPossition.at(i) - wm->field->ourGoal());
                    markRoles.append(QString("shotBlocker"));
                }
            } else if (ourMarkAgentsSize - markPoses.size() < opponentAgentsToBeMarkPossition.size()) {
                tempSortDangerAgentsToBeBlockPassPlayOff = sortdangerpassplayoff(opponentAgentsToBeMarkPossition);
                for (i = 0 ; i < tempSortDangerAgentsToBeBlockPassPlayOff.size() ; i++) {
                    tempOpponentAgentsToBeMarkedCircle.append(Circle2D(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first , opponentAgentsCircleR));
                    drawer->draw(tempOpponentAgentsToBeMarkedCircle.at(i), QColor(Qt::yellow));
                }
                for (i = 0 ; i < ourMarkAgentsSize - markPoses.size() ; i++) {
                    if (!wm->field->isInOurPenaltyArea(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first)) {
                        tempOpponentAgentsToBeMarkedCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first), &sol1 , &sol2);
                        wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first , wm->field->ourGoal()), &sol3 , &sol4);
                        markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 , Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4 , conf.ShootRatioBlock / 100));
                    } else {
                        wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Line2D(wm->field->ourGoal() , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first) , &sol1 , &sol2);
                        markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                    }
                    markAngs.append(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first - wm->field->ourGoal());
                    markRoles.append(QString("shotBlocker"));
                }
            } else if (ourMarkAgentsSize - markPoses.size() > opponentAgentsToBeMarkPossition.size()) {
                for (i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++) {
                    drawer->draw(opponentAgentsToBeMarkCircle.at(i), QColor(Qt::cyan));
                }
                for (i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++) {
                    if (!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))) {
                        opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                        wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(opponentAgentsToBeMarkPossition.at(i) , wm->field->ourGoal()), &sol3 , &sol4);
                        markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 , Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4 , conf.ShootRatioBlock / 100));
                    } else {
                        wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Line2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                        markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                    }
                    markAngs.append(opponentAgentsToBeMarkPossition.at(i) - wm->field->ourGoal());
                    markRoles.append(QString("shotBlocker"));
                }
                for (i = 0 ; i < wm->opp.activeAgentsCount() ; i++) {
                    opponentAgentsCircle.append(Circle2D(wm->opp.active(i)->pos , opponentAgentsCircleR));
                }
                for (i = 0 ; i < wm->opp.activeAgentsCount() ; i++) {
                    for (j = 0 ; j < opponentAgentsToBeMarkPossition.size() ; j++) {
                        if (opponentAgentsToBeMarkPossition.at(j) == wm->opp.active(i)->pos) {
                            opponentAgentsCircle.removeAt(i);
                        }
                    }
                }
                for (i = 0 ; i < wm->opp.activeAgentsCount() ; i++) {
                    if (Circle2D(wm->ball->pos , ballCircleR).contains(wm->opp.active(i)->pos)) {
                        opponentAgentsCircle.removeAt(i);
                    }
                }
                if (opponentAgentsToBeMarkPossition.empty()) {
                    for (i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++) {
                        if (!wm->field->isInOurPenaltyArea(opponentAgentsCircle.at(i).center())) {
                            opponentAgentsCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsCircle.at(i).center()), &sol1 , &sol2);
                            wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(opponentAgentsCircle.at(i).center() , wm->field->ourGoal()), &sol3 , &sol4);
                            markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 , Segment2D(sol3 , opponentAgentsCircle.at(i).center()).length() < Segment2D(sol4 , opponentAgentsCircle.at(i).center()).length() ? sol3 : sol4 , conf.ShootRatioBlock / 100));
                        } else {
                            wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Line2D(wm->field->ourGoal() , opponentAgentsCircle.at(i).center()) , &sol1 , &sol2);
                            markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                        }
                        markAngs.append(opponentAgentsCircle.at(i).center() - wm->field->ourGoal());
                        markRoles.append(QString("shotBlocker"));
                    }
                } else {
                    for (i = 0 ; i < ourMarkAgentsSize - markPoses.size() + 1 ; i++) {
                        if (!wm->field->isInOurPenaltyArea(opponentAgentsCircle.at(i).center())) {
                            opponentAgentsCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsCircle.at(i).center()), &sol1 , &sol2);
                            wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(opponentAgentsCircle.at(i).center() , wm->field->ourGoal()), &sol3 , &sol4);
                            markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 , Segment2D(sol3 , opponentAgentsCircle.at(i).center()).length() < Segment2D(sol4 , opponentAgentsCircle.at(i).center()).length() ? sol3 : sol4 , conf.ShootRatioBlock / 100));
                        } else {
                            wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Line2D(wm->field->ourGoal() , opponentAgentsCircle.at(i).center()) , &sol1 , &sol2);
                            markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                        }
                        markAngs.append(opponentAgentsCircle.at(i).center() - wm->field->ourGoal());
                        markRoles.append(QString("shotBlocker"));
                    }
                }
            }
        }
        if (ourMarkAgentsSize > markPoses.size()) {
            for (i = 0 ; i < ourMarkAgentsSize - markPoses.size() ; i++) {
                if (i % 2) {
                    markPoses.append(Vector2D(-3 , (i + 1) / 0.75));
                    markAngs.append(Vector2D(0, 0));
                    markRoles.append(QString("shotBlocker"));
                } else {
                    markPoses.append(Vector2D(-3, (-i - 1)  / 0.75));
                    markAngs.append(Vector2D(0, 0));
                    markRoles.append(QString("shotBlocker"));
                }
            }
        }
    } else if (opponentAgentsToBeMarkPossition.size() > ourMarkAgentsSize) {
        sortDangerAgentsToBeMarkBlockPassPlayOff = sortdangerpassplayoff(opponentAgentsToBeMarkPossition);
        for (i = 0 ; i < ourMarkAgentsSize; i++) {
            //////////// Don't Enter penalty area, mark agents!!! :) ///////////
            if (wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(wm->ball->pos , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first) , &sol7,  &sol8)) {
                if (!wm->field->isInOurPenaltyArea(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first)) {
                    Circle2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , opponentAgentsCircleR).intersection(Segment2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol3 , &sol4);
                    wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2, Segment2D(sol3 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() ? sol3 : sol4) , &sol5 , &sol6);
                    markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() < Segment2D(sol6 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() ? sol5 : sol6 , proportionOfDistance));
                } else {
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol1 , &sol2);
                    wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(Segment2D(wm->ball->pos , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first) , &sol5 , &sol6);
                    markPoses.append(know->getPointInDirection(Segment2D(sol1 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() < Segment2D(sol2 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() ? sol1 : sol2 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6 , proportionOfDistance));
                }
            } else {
                Circle2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , opponentAgentsCircleR).intersection(Segment2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol1 , &sol2);
                Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first), &sol3 , &sol4);
                markPoses.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2, Segment2D(sol3 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() ? sol3 : sol4, proportionOfDistance));
            }
            markAngs.append(wm->ball->pos - markPoses.at(i));
            markRoles.append(QString("passBlocker"));
            drawer->draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                   Segment2D(sol3 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() ? sol3 : sol4)
                         , QColor(Qt::red));
            drawer->draw(Circle2D(wm->ball->pos , ballCircleR), QColor(Qt::black));
            DBUG(QString("Man To Man Mark In PlayOn Mode / BlockPass / opp > our") , D_SEPEHR);
        }
    }
    //////////////// drawer->draw Possition of Mark Agents //////////////////////////
    for (i = 0 ; i < markPoses.size() ; i++) {
        DBUG(QString("x : %1").arg(markPoses.at(i).x) , D_AHZ);
        drawer->draw(markPoses.at(i), "white");
        drawer->draw(markRoles.at(i) , markPoses.at(i) - Vector2D(0, 0.4) , "white");
    }
}

QList<QPair<Vector2D, double> > CMarkPlan::sortdangershoot(double _radius, double treshold) {

    Vector2D sol1, sol2, sol3;
    Vector2D _pos1 = wm->ball->pos;
    Vector2D _pos2 = wm->ball->pos + 5.0  * wm->ball->vel;
    Line2D _path(_pos1, _pos2);
    Polygon2D _poly;
    Circle2D(_pos2, _radius + treshold).
            intersection(_path.perpendicular(_pos2), &sol1, &sol2);


    _poly.addVertex(sol1);
    sol3 = sol1;
    _poly.addVertex(sol2);
    Circle2D(_pos1, Robot::robot_radius_new + treshold).
            intersection(_path.perpendicular(_pos1), &sol1, &sol2);

    _poly.addVertex(sol2);
    _poly.addVertex(sol1);
    _poly.addVertex(sol3);

    drawer->draw(_poly, "blue");


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
    double RangeofdistanceToBallProjection = Segment2D(Vector2D(-1.0 * wm->field->_FIELD_WIDTH / 2, -1.0 * wm->field->_FIELD_HEIGHT / 2), Vector2D(wm->field->_FIELD_WIDTH / 2 , wm->field->_FIELD_HEIGHT / 2)).length();
    double RangeofdistanceToIntersect =  _radius;


    for (int i = 0; i < oppAgentsToMarkPos.count(); i++) {
        if (_poly.contains(oppAgentsToMarkPos[i])) {
            Angle = Vector2D::angleOf(oppAgentsToMarkPos[i], wm->ball->pos, _path.projection(oppAgentsToMarkPos[i])).degree();
            temp.first = oppAgentsToMarkPos[i];


            distanceToIntersect = _path.dist(oppAgentsToMarkPos[i]);
            distanceToBallProjection = _path.projection(oppAgentsToMarkPos[i]).length();
            danger = KA * (1 - Angle / RangeofAngle) + KDB * (1 - distanceToBallProjection / RangeofdistanceToBallProjection) + KDI * (1 - distanceToIntersect / RangeofdistanceToIntersect);
            temp.second = danger;
            output.append(temp);
        }

    }
    if (!output.isEmpty()) {

        Segment2D tempsegment;
        tempsegment.assign(output.first().first, wm->field->ourGoal());

        drawer->draw(tempsegment, QColor(Qt::blue));
    }





    ///////---------sorting------------//////
    for (int i = 0; i < output.count(); i++) {

        for (int j = 0; j < output.count() - 1; j++) {
            if (output[j].second < output[j + 1].second) {
                output.swap(j, j + 1);
            }
        }

    }
    // this is for test
    output.clear();

    return output;
}

QList<CRobot*> CMarkPlan::sortdanger(const QList<CRobot*> oppagent) {
    const QList<CRobot*> &sorted = oppagent;
    for (int i = 0; i < sorted.count(); i++) {

        for (int k = 0; k < sorted.count() - 1; k++) {

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
    double KA = 1; //Angle Coefficient
    double KDB = 1; //Distance To Ball
    double KDG = 1; //Distnce To Goal
    double RangeofAngle = Vector2D::angleOf(wm->field->ourGoalR(), Vector2D(-1.0 * (wm->field->_FIELD_WIDTH / 2 - wm->field->_PENALTY_DEPTH), 0), wm->field->ourGoalL()).degree();
    //drawer->draw(Vector2D(-1.0 * (wm->field->_FIELD_WIDTH - _GOAL_WIDTH), 0), QColor(Qt::red));
    // double RangeofAngle2 = Vector2D::angleOf(wm->field->ou,Vector2D(0, -1.0 * (wm->field->_FIELD_WIDTH - _GOAL_WIDTH)), wm->field->ourGoalL()).degree();

    double RangeofDistancetoBall = fabs(Segment2D(Vector2D(wm->field->_FIELD_WIDTH / 2, wm->field->_FIELD_HEIGHT / 2), Vector2D(-1.0 * wm->field->_FIELD_WIDTH / 2, -1.0 * wm->field->_FIELD_HEIGHT / 2)).length());

    double RangeofDistancetoGoal = fabs(Segment2D(Vector2D(wm->field->_FIELD_WIDTH / 2, wm->field->_FIELD_HEIGHT / 2), wm->field->ourGoal()).length());

    //double RangeofTempDis = 2;
    double angle, distancetoball, distancetogoal, danger;


    QPair<Vector2D, double> temp;
    QList<QPair<Vector2D, double> > output;
    for (int i = 0; i < oppposdanger.count(); i++) {
        temp.first = oppposdanger[i];


        angle = Vector2D::angleOf(wm->field->ourGoalR(), oppposdanger[i], wm->field->ourGoalL()).degree();
        distancetoball = (oppposdanger[i] - wm->ball->pos).length();
        distancetogoal = (oppposdanger[i] - wm->field->ourGoal()).length();
        danger = (KA * fabs(angle) / RangeofAngle) + (KDB * 1 - (distancetoball / RangeofDistancetoBall)) + (KDG * 1 - (distancetogoal / RangeofDistancetoGoal));
        temp.second = danger;
        output.append(temp);
        // finding nearest to intersect
        Segment2D tempsegment;
        tempsegment.assign(oppposdanger[i], wm->field->ourGoal());

        double mintempdis = 0.0;
        if (wm->our.activeAgentsCount() != 0) {
            mintempdis = tempsegment.dist(wm->our.active(0)->pos);
        }

        for (int j = 0; j < wm->our.activeAgentsCount(); j++) {
            if (tempsegment.dist(wm->our.active(j)->pos) < mintempdis) {
                mintempdis = tempsegment.dist(wm->our.active(j)->pos);
            }

        }
    }

    ///sorting the Qlist
    for (int i = 0; i < output.count(); i++) {
        for (int j = 0; j < output.count() - 1; j++) {
            if (output[j].second < output[j + 1].second) {
                output.swap(j, j + 1);
            }
        }
    }

    return output;
}

Vector2D CMarkPlan::findcenterdefence() {

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

    double KA = 1; //Angle Coefficient
    double KDB = 0; //Distance To Ball
    double KDG = 1; //Distnce To Goal
    double KAD = 2; // angle of difense
    double RangeofAngle = Vector2D::angleOf(wm->field->ourGoalR(), Vector2D(-1.0 * (wm->field->_FIELD_WIDTH / 2 - wm->field->_PENALTY_DEPTH), 0), wm->field->ourGoalL()).degree();
    double RangeofDistancetoBall = fabs(Segment2D(Vector2D(wm->field->_FIELD_WIDTH / 2, wm->field->_FIELD_HEIGHT / 2), Vector2D(-1.0 * wm->field->_FIELD_WIDTH / 2, -1.0 * wm->field->_FIELD_HEIGHT / 2)).length());

    double RangeofDistancetoGoal = fabs(Segment2D(Vector2D(wm->field->_FIELD_WIDTH / 2, wm->field->_FIELD_HEIGHT / 2), wm->field->ourGoal()).length());

    double RangeofTempDis = 2;


    double RangeofAngleDef = 180;

    double angle, angleDef, distancetoball, distancetogoal, danger;

    ///finding center of defense


    QPair<Vector2D, double> temp;
    QList<QPair<Vector2D, double> > output;
    for (int i = 0; i < oppposdanger.count(); i++) {
        temp.first = oppposdanger[i];
        angle = Vector2D::angleOf(wm->field->ourGoalR(), oppposdanger[i], wm->field->ourGoalL()).degree();
        distancetoball = (oppposdanger[i] - wm->ball->pos).length();
        distancetogoal = (oppposdanger[i] - wm->field->ourGoal()).length();

        angleDef = Vector2D::angleOf(oppposdanger[i], wm->field->ourGoal(), findcenterdefence()).degree();
        danger = (KA * fabs(angle) / RangeofAngle) + (KDB * (1 - (distancetoball / RangeofDistancetoBall))) + (KDG * (1 - (distancetogoal / RangeofDistancetoGoal))) + (KAD * angleDef / RangeofAngleDef);
        temp.second = danger;
        output.append(temp);
    }

    ///sorting the Qlist
    for (int i = 0; i < output.count(); i++) {
        for (int j = 0; j < output.count() - 1; j++) {
            if (output[j].second < output[j + 1].second) {
                output.swap(j, j + 1);
            }
        }
    }
    return output;
}

Vector2D CMarkPlan::posvel(CRobot* opp) {
    if (opp->vel.length() > 0.3 && opp->vel.x < 0) {
        return opp->pos + (conf.VelReliability * opp->vel);
    }
    else {
        return opp->pos;
    }
}

void CMarkPlan::findOppAgentsToMark() {
    oppAgentsToMark.clear();
    oppAgentsMarkedByDef.clear();
    ignoredOppAgents.clear();

    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        oppAgentsToMark.append(wm->opp.active(i));
        if (wm->field->isInOppPenaltyArea(oppAgentsToMark.last()->pos)) {
            oppAgentsToMark.removeOne(oppAgentsToMark.last());    // detection of the goali
        }
    }

    oppAgentsToMarkPos.clear();

    if (gameState->isStart()) {
        for (int i = 0; i < oppAgentsToMark.count(); i++) {
            if (oppAgentsToMark[i]->pos.x > conf.OppOmitLimitPlayon) {
                oppAgentsToMark.removeOne(oppAgentsToMark[i]);
                i--;
            }
        }
        if(conf.OmmitNearestToBallPlayon){
            int nearestToBall = -1;
            double nearestToBallDist = 100000;
            for (int i = 0 ; i < oppAgentsToMark.count() ; i++) {
                if ((oppAgentsToMark[i]->pos).dist(wm->ball->pos) < nearestToBallDist) {
                    nearestToBall = i;
                    nearestToBallDist = oppAgentsToMark[i]->pos.dist(wm->ball->pos);
                }
            }
            if (nearestToBall != -1) {
                oppAgentsToMark.removeOne(oppAgentsToMark[nearestToBall]);
            }
        }
    }
    else if(gameState->theirKickoff()){
        for (int i = 0; i < oppAgentsToMark.count(); i++) {
            if (posvel(oppAgentsToMark[i]).x > conf.OppOmitLimitKickOff) {
                oppAgentsToMark.removeOne(oppAgentsToMark[i]);
                i--;
            }
        }
        int nearestToBall = -1;
        double nearestToBallDist = 100000;
        for (int i = 0 ; i < oppAgentsToMark.count() ; i++) {
            if((oppAgentsToMark[i]->pos).dist(wm->ball->pos) < nearestToBallDist) {
                nearestToBall = i;
                nearestToBallDist = oppAgentsToMark[i]->pos.dist(wm->ball->pos);
            }
        }
        if (nearestToBall != -1) {
            oppAgentsToMark.removeOne(oppAgentsToMark[nearestToBall]);
        }
    }

    //setting the positions
    for(int i = 0; i < oppAgentsToMark.count(); i++){
        if(gameState->isStart() || gameState->theirKickoff()){
            oppAgentsToMarkPos.append(posvel(oppAgentsToMark[i]));
        }
    }

}

void CMarkPlan::markPosesRefinePlayon() {
    Vector2D sol1, sol2, sol;
    Line2D tempLine(Vector2D(0, 0), Vector2D(1, 0));
    Circle2D _markAreaRestricted1(wm->field->ourGoal() + Vector2D(-1.0 * 0.2, 0), markRadiusStrict);
    Circle2D _markAreaRestricted2(wm->field->oppGoal() + Vector2D(0.2, 0), markRadiusStrict);
    for (int i = 0; i < markPoses.count(); i++) {
        tempLine.assign(wm->field->ourGoal(), markPoses[i]);
        if ((wm->field->ourGoal() - markPoses[i]).length() < markRadiusStrict) {
            _markAreaRestricted1.intersection(tempLine, &sol1, &sol2);

            if (sol1.x > -1.0 * wm->field->_FIELD_WIDTH / 2) {
                sol = sol1;
            } else {
                sol = sol2;
            }

            markPoses[i] = sol;
        }

        tempLine.assign(markPoses[i], wm->field->oppGoal());

        if ((markPoses[i] - wm->field->oppGoal()).length() < markRadiusStrict) {
            _markAreaRestricted2.intersection(tempLine, &sol1, &sol2);

            if (sol1.x < 1.0 * wm->field->_FIELD_WIDTH / 2) {
                sol = sol1;
            } else {
                sol = sol2;
            }

            markPoses[i] = sol;
        }
    }

    ///refinning kick off
    double xKickoff  = -.08;
    Circle2D indirect;
    //Vector2D sol1,sol2,sol;
    indirect.assign(Vector2D(0, 0), .5);
    Segment2D tempMarkSeg;
    if (gameState->theirKickoff()) {
        // Not positioning the Center circle
        for (int i = 0; i < markPoses.count(); i++) {

            if (indirect.contains(markPoses[i])) {
                tempMarkSeg.assign(markPoses[i], wm->field->ourGoal());
                indirect.intersection(tempMarkSeg, &sol1, &sol2);
                if (sol1.x < sol2.x) {
                    sol = sol1;
                } else {
                    sol = sol2;
                }

                markPoses[i] = sol;
            }
        }
        // not invading the opponent field
        for (int i = 0; i < markPoses.count(); i++) {
            if (markPoses[i].x > xKickoff) {
                markPoses[i].x = xKickoff;
            }
        }
    }
}

QList<Vector2D> CMarkPlan::ShootBlockRatio(double ratio, Vector2D opp) {
    //// This function produces a point between opponent agents to be mark &&
    //// ourGoal to block the shot path. Also checks if this point is in the
    //// penalty area, produces a point that is intersection of penalty area with
    //// shot path.
    ///
    QList<Vector2D> tempQlist;
    Circle2D indirect;
    Vector2D sol1, sol2, sol;
    Vector2D solutions[2];
    Segment2D centerLineOfField;
    Vector2D intersectionWithOurGoalLine;
    Vector2D intersectionWithOppGoalLine;
    centerLineOfField.assign(Vector2D(0, -4.5) , Vector2D(0, 4.5));
    indirect.assign(Vector2D(0, 0), 0.7);
    tempQlist.clear();
    Line2D tempLine = getBisectorLine(wm->field->ourGoalL() , opp , wm->field->ourGoalR());
    intersectionWithOurGoalLine = Segment2D(wm->field->ourGoalL() , wm->field->ourGoalR()).intersection(tempLine);
    intersectionWithOppGoalLine = Segment2D(wm->field->oppGoalL() , wm->field->oppGoalR()).intersection(tempLine);
    Vector2D markPos = know->getPointInDirection(intersectionWithOurGoalLine , opp , ratio);
    if(gameState->theirKickoff()){
        if(indirect.contains(markPos)){
            indirect.intersection(tempLine, &sol1, &sol2);
            if(sol1.x < sol2.x){
                sol = sol1;
            }
            else{
                sol = sol2;
            }
            tempQlist.append(sol);
            tempQlist.append(opp - intersectionWithOurGoalLine);
        }
        else{
            if(wm->field->isInOurPenaltyArea(opp)){
                wm->field->ourBigPenaltyArea(1,Robot::robot_radius_new,0).intersection(tempLine, &solutions[0] , &solutions[1]);
                tempQlist.append(solutions[0].isValid() && solutions[0].dist(opp) < solutions[1].dist(opp) ? solutions[0] : solutions[1]);
                tempQlist.append(tempQlist.first() - intersectionWithOurGoalLine);
            }
            else if(wm->field->isInOppPenaltyArea(opp)){
                wm->field->oppBigPenaltyArea(1,Robot::robot_radius_new,0).intersection(tempLine, &solutions[0] , &solutions[1]);
                tempQlist.append(solutions[0].isValid() && solutions[0].dist(opp) < solutions[1].dist(opp) ? solutions[0] : solutions[1]);
                tempQlist.append(intersectionWithOppGoalLine - tempQlist.first());
            }
            else{
                if(wm->field->isInOurPenaltyArea(markPos)){
                    wm->field->ourBigPenaltyArea(1, Robot::robot_radius_new, 0).intersection(tempLine, &solutions[0] , &solutions[1]);
                    tempQlist.append(solutions[0].isValid() && solutions[0].dist(opp) < solutions[1].dist(opp) ? solutions[0] : solutions[1]);
                    tempQlist.append(tempQlist.first() - intersectionWithOurGoalLine);
                }
                else if(wm->field->isInOppPenaltyArea(markPos)){
                    wm->field->oppBigPenaltyArea(1, Robot::robot_radius_new, 0).intersection(tempLine, &solutions[0] , &solutions[1]);
                    tempQlist.append(solutions[0].isValid() && solutions[0].dist(opp) < solutions[1].dist(opp) ? solutions[0] : solutions[1]);
                    tempQlist.append(intersectionWithOppGoalLine - tempQlist.first());
                }
                else{
                    tempQlist.append(markPos);
                    tempQlist.append(opp - intersectionWithOurGoalLine);
                }
            }
        }
    }
    else{
        if(wm->field->isInOurPenaltyArea(opp)){
            wm->field->ourBigPenaltyArea(1,Robot::robot_radius_new,0).intersection(tempLine, &solutions[0] , &solutions[1]);
            tempQlist.append(solutions[0].isValid() && solutions[0].dist(opp) < solutions[1].dist(opp) ? solutions[0] : solutions[1]);
            tempQlist.append(tempQlist.first() - intersectionWithOurGoalLine);
        }
        else if(wm->field->isInOppPenaltyArea(opp)){
            wm->field->oppBigPenaltyArea(1,Robot::robot_radius_new,0).intersection(tempLine, &solutions[0] , &solutions[1]);
            tempQlist.append(solutions[0].isValid() && solutions[0].dist(opp) < solutions[1].dist(opp) ? solutions[0] : solutions[1]);
            tempQlist.append(intersectionWithOppGoalLine - tempQlist.first());
        }
        else{
            if(wm->field->isInOurPenaltyArea(markPos)){
                wm->field->ourBigPenaltyArea(1, Robot::robot_radius_new, 0).intersection(tempLine, &solutions[0] , &solutions[1]);
                tempQlist.append(solutions[0].isValid() && solutions[0].dist(opp) < solutions[1].dist(opp) ? solutions[0] : solutions[1]);
                tempQlist.append(tempQlist.first() - intersectionWithOurGoalLine);
            }
            else if(wm->field->isInOppPenaltyArea(markPos)){
                wm->field->oppBigPenaltyArea(1, Robot::robot_radius_new, 0).intersection(tempLine, &solutions[0] , &solutions[1]);
                tempQlist.append(solutions[0].isValid() && solutions[0].dist(opp) < solutions[1].dist(opp) ? solutions[0] : solutions[1]);
                tempQlist.append(intersectionWithOppGoalLine - tempQlist.first());
            }
            else{
                tempQlist.append(markPos);
                tempQlist.append(opp - intersectionWithOurGoalLine);
            }
        }
    }

    return tempQlist;
}

QList<Vector2D> CMarkPlan::PassBlockRatio(double ratio, Vector2D opp) {
    //// This function produces a point that block the pass path.Also if the
    //// resulted point is in the penalty area, this function generates a suitable
    //// point.
    ///
    QList<Vector2D> passBlocker;
    Vector2D sol1,sol2,sol3,sol4,sol5,sol6,sol7,sol8;
    double opponentAgentsCircleR = 0.2;
    Circle2D opponentAgentsToBeMarkCircle = Circle2D(opp , opponentAgentsCircleR);
    if(wm->field->ourBigPenaltyArea(1, Robot::robot_radius_new, 0).intersection(Segment2D(wm->ball->pos , opp) , &sol7,  &sol8)){
        if(!wm->field->isInOurPenaltyArea(opp)){
            opponentAgentsToBeMarkCircle.intersection(Segment2D(wm->ball->pos , opp), &sol1 , &sol2);
            Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opp), &sol3 , &sol4);
            wm->field->ourBigPenaltyArea(1, Robot::robot_radius_new, 0).intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2, Segment2D(sol3 , opp).length() < Segment2D(sol4 , opp).length() ? sol3 : sol4) , &sol5 , &sol6);
            passBlocker.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opp).length() < Segment2D(sol6 , opp).length() ? sol5 : sol6 , ratio));
        }
        else{
            wm->field->ourBigPenaltyArea(1, Robot::robot_radius_new, 0).intersection(Line2D(wm->field->ourGoal() , opp) , &sol1 , &sol2);
            passBlocker.append(Segment2D(sol1 , opp).length() < Segment2D(sol2 , opp).length() ? sol1 : sol2);
        }
    }
    else if(wm->field->oppBigPenaltyArea(1, Robot::robot_radius_new, 0).intersection(Segment2D(wm->ball->pos , opp) , &sol7,  &sol8)){
        if(!wm->field->isInOppPenaltyArea(opp)){
            opponentAgentsToBeMarkCircle.intersection(Segment2D(wm->ball->pos , opp), &sol1 , &sol2);
            Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opp), &sol3 , &sol4);
            wm->field->oppBigPenaltyArea(1, Robot::robot_radius_new, 0).intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2, Segment2D(sol3 , opp).length() < Segment2D(sol4 , opp).length() ? sol3 : sol4) , &sol5 , &sol6);
            passBlocker.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opp).length() < Segment2D(sol6 , opp).length() ? sol5 : sol6 , ratio));
        }
        else{
            wm->field->oppBigPenaltyArea(1, Robot::robot_radius_new, 0).intersection(Line2D(wm->field->oppGoal() , opp) , &sol1 , &sol2);
            passBlocker.append(Segment2D(sol1 , opp).length() < Segment2D(sol2 , opp).length() ? sol1 : sol2);
        }
    }
    else{
        opponentAgentsToBeMarkCircle.intersection(Segment2D(wm->ball->pos , opp), &sol1 , &sol2);
        Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opp), &sol3 , &sol4);
        passBlocker.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2, Segment2D(sol3 , opp).length() < Segment2D(sol4 , opp).length() ? sol3 : sol4, ratio));
    }
    passBlocker.append(wm->ball->pos - passBlocker.first());
    return passBlocker;

}

void CMarkPlan::execute(){
    findOppAgentsToMark();
    oppmarkedpos.clear();
    markPoses.clear();
    markAngs.clear();
    segmentpershoot = conf.ShootRatioBlock / 100;
    segmentperpass = conf.PassRatioBlock / 100;
    QList<int> matchPoints;
    if (gameState->isStart()){
        if(conf.PlayOnManToMan){
            manToManMarkInPlayOnBlockPass(oppAgentsToMarkPos , agents.count() , segmentperpass);
        }
        else{
            markPoses.clear();
            markAngs.clear();
            if(agents.count() == oppAgentsToMarkPos.count()){
                for (int i = 0; i < oppAgentsToMarkPos.count(); i++) {
                    markPoses.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).first());
                    markAngs.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).last());
                }
            }
            else if(agents.count() < oppAgentsToMarkPos.count()){
                QList<QPair<Vector2D, double> >tempQlistQpair = sortdangerpassplayon(oppAgentsToMarkPos);
                for (int i = 0; i < agents.count(); i++) {
                    markPoses.append(ShootBlockRatio(segmentpershoot, tempQlistQpair[i].first).first());
                    markAngs.append(ShootBlockRatio(segmentpershoot, tempQlistQpair[i].first).last());
                }
            }
            else if(agents.count() > oppAgentsToMarkPos.count()){
                QList<QPair<Vector2D, double> >tempQlistQpair = sortdangerpassplayon(oppAgentsToMarkPos);
                for(int i = 0; i < oppAgentsToMarkPos.count(); i++){
                    markPoses.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).first());
                    markAngs.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).last());
                }
                for(int i = 0; i < min(agents.count() - oppAgentsToMarkPos.count(), oppAgentsToMarkPos.count()); i++) {
                    markPoses.append(PassBlockRatio(segmentperpass, tempQlistQpair[i].first).first());
                    markAngs.append(PassBlockRatio(segmentperpass, tempQlistQpair[i].first).last());
                }
            }
            int count = 0;
            for (int i = 0; i < agents.count(); i++) {
                if (markPoses.size() < agents.count()) {
                    markPoses.append(Vector2D(0, count));
                    markAngs.append(Vector2D(1, 0));
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
        know->Matching(agents, markPoses, matchPoints); // todo : knowledge
        if(agents.count() == markPoses.count()){
            for(int i = 0; i < markPoses.count(); i++){
                if (i < matchPoints.size()) {
                    markGPA[i]->setTargetpos(markPoses[matchPoints[i]]); //HINT : gpa->init
                    markGPA[i]->setTargetdir(markAngs[matchPoints[i]]);
                    markGPA[i]->setAvoidpenaltyarea(true);
                    agents[i]->action = markGPA[i];
                }
            }
        }
    }
    else if(gameState->theirKickoff()) {
        segmentpershoot = 0.7;
        markPoses.clear();
        markAngs.clear();
        DBUG(QString("Its TheirKickoff"), D_MAHI);
        findOppAgentsToMark();
        Segment2D tempsegLine, tempsegLine2;
        tempsegLine.assign(Vector2D(-2, wm->field->_FIELD_HEIGHT / 2), Vector2D(-2, -1.0 * wm->field->_FIELD_HEIGHT / 2));
        tempsegLine2.assign(Vector2D(-2, wm->field->_FIELD_HEIGHT / 2), Vector2D(-2, -1.0 * wm->field->_FIELD_HEIGHT / 2));

        if (agents.count() == oppAgentsToMarkPos.count()) {
            for (int i = 0; i < oppAgentsToMarkPos.count(); i++) {
                markPoses.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).first());
                markAngs.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).last());
            }
        }

        else if (agents.count() < oppAgentsToMarkPos.count()) {
            QList<QPair<Vector2D, double> >tempQlistQpair = sortdangerpassplayon(oppAgentsToMarkPos);
            for (int i = 0; i < agents.count(); i++) {
                markPoses.append(ShootBlockRatio(segmentpershoot, tempQlistQpair[i].first).first());
                markAngs.append(ShootBlockRatio(segmentpershoot, tempQlistQpair[i].first).last());
            }


        }

        else if (agents.count() > oppAgentsToMarkPos.count()) {

            for (int i = 0; i < oppAgentsToMarkPos.count(); i++) {
                markPoses.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).first());
                markAngs.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).last());
            }
            for (int i = 0; i < agents.count() - oppAgentsToMarkPos.count(); i++) {
                markPoses.append(Vector2D(-1, i));
                markAngs.append(Vector2D(-1, 0));
                drawer->draw(Circle2D(markPoses.last(), 0.2), QColor(Qt::blue));
            }

        }

        markPosesRefinePlayon();

        matchPoints.clear();
        matchPoints.append(0);
        matchPoints.append(1);
        matchPoints.append(2);
        matchPoints.append(3);

        know->Matching(agents, markPoses, matchPoints);
        if (agents.count() == markPoses.count()) {
            for (int i = 0; i < markPoses.count(); i++) {
                if (i < matchPoints.size()) {

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

