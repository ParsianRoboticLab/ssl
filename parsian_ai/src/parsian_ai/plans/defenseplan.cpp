#include "parsian_ai/plans/defenseplan.h"
#include "parsian_util/tools/blackboard.h"
#include "parsian_util/geom/polygon_2d.h"

using namespace std;

#define LONG_CHIP_POWER 1023
#define RADIUS_FOR_CRITICAL_DEFENSE_AREA 1.697056275 + Robot::robot_radius_new

QPair<Vector2D , Vector2D> DefensePlan::avoidRectangularPenaltyAreaByMhmmd(Vector2D finalPosition , Vector2D agentPosition , Vector2D agentDirection , Vector2D agentVelocity){
    int skillAgent = 7;
    Rect2D penaltyArea = wm->field->ourBigPenaltyArea(1,0.1,0);
    Segment2D directPath(agentPosition,finalPosition);
    Vector2D A(-3.5,-1.3) , B(-3.5 , 1.3);
    Vector2D target , targetDir;
    Rect2D penaltyAreaK = wm->field->ourBigPenaltyArea(1,0,0);

    Vector2D sol1,sol2;
    if(penaltyAreaK.contains(agentPosition)) {
        Segment2D agentSeg (wm->field->ourGoal(), wm->field->ourGoal() + (agentPosition - wm->field->ourGoal()).norm()*10);
        penaltyArea.intersection(agentSeg,&sol1,&sol2);
        target = sol1;
        if(sol1 == wm->field->ourGoal()) {
            target = sol2;
        }
        targetDir = agentDirection;
        target = target + (agentPosition - wm->field->ourGoal()).norm()*2;
    }

    else if(penaltyArea.intersection(directPath,&sol1,&sol2) == 2){
        if((sol1.y == -1.3 && sol2.y == 1.3) || (sol2.y == -1.3 && sol1.y == 1.3)){
            if(agentPosition.dist(A) < agentDirection.dist(B) ) {
                target = A + Vector2D(0* std::min(agentVelocity.length(),1.0),0* std::min(agentVelocity.length(),1.0));
            } else {
                target = B + Vector2D(0* std::min(agentVelocity.length(),1.0),-0* std::min(agentVelocity.length(),1.0));
            }
        }
        else if(sol1.y == -1.3 || sol2.y == -1.3) {
            ROS_INFO_STREAM("ahz "<< sol1.y << sol2.y);
            target = A + Vector2D(-1.2,-0.5);
        }
        else if(sol1.y == 1.3 || sol2.y == 1.3){
            target = B + Vector2D(-1.2,0.5);
        }
        drawer->draw(sol1);
        drawer->draw(sol2);
        targetDir = target - agentPosition;
    }
    else{
        target = finalPosition;
        targetDir = target - wm->field->ourGoal();
    }
    drawer->draw(target,QColor(Qt::red));
    QPair<Vector2D , Vector2D> temp;
    temp.first = target;
    temp.second = targetDir;
    return temp;
}

double DefensePlan::timeNeeded(Agent *_agentT, Vector2D posT, double vMax, QList <int> _ourRelax, QList <int> _oppRelax , bool avoidPenalty, double ballObstacleReduce, bool _noAvoid){

    double _x3;
    double acc = 4.5;
    double dec = 3.5;
    double xSat;
    Vector2D tAgentVel = _agentT->vel();
    Vector2D tAgentDir = _agentT->dir();
    double veltan = (tAgentVel.x) * cos(tAgentDir.th().radian()) + (tAgentVel.y) * sin(tAgentDir.th().radian());
    double offset = 0;
    double velnorm = -1 * (tAgentVel.x) * sin(tAgentDir.th().radian()) + (tAgentVel.y) * cos(tAgentDir.th().radian());
    double distCoef = 1, distEffect = 1, angCoef = 0.003;
    double dist = 0;
    double rrtAngSum = 0;
    QList <Vector2D> _result;
    Vector2D _target;

    double tAgentVelTanjent =  tAgentVel.length() * cos(Vector2D::angleBetween(posT - _agentT->pos() , _agentT->vel().norm()).radian());

    double vXvirtual = (posT - _agentT->pos()).x;
    double vYvirtual = (posT - _agentT->pos()).y;
    double veltanV = (vXvirtual) * cos(tAgentDir.th().radian()) + (vYvirtual) * sin(tAgentDir.th().radian());
    double velnormV = -1 * (vXvirtual) * sin(tAgentDir.th().radian()) + (vYvirtual) * cos(tAgentDir.th().radian());
    double accCoef = 1, realAcc = 4;

    accCoef = atan(fabs(veltanV) / fabs(velnormV)) / _PI * 2;
    acc = accCoef * 4.5 + (1 - accCoef) * 3.5;

    double tDec = vMax / dec;
    double tAcc = (vMax - tAgentVelTanjent) / acc;
    dist = posT.dist(_agentT->pos());
    double dB = tDec * vMax / 2 + tAcc * (vMax + tAgentVelTanjent) / 2;

    if (dist > dB) {
        return tAcc + tDec + (dist - dB) / vMax;
    } else {
        return ((1 / dec) + (1 / acc)) * sqrt(dist * (2 * dec * acc / (acc + dec)) + (tAgentVelTanjent * tAgentVelTanjent / (2 * acc))) - (tAgentVelTanjent) / acc;
    }

}

QList<Vector2D> DefensePlan::defenseFormation(QList<Vector2D> circularPositions, QList<Vector2D> rectangularPositions){
    suitableRadius = RADIUS_FOR_CRITICAL_DEFENSE_AREA;
    Circle2D defenseArea(wm->field->ourGoal() , suitableRadius);
    if((wm->field->ourBigPenaltyArea(1,0.2,0).contains(wm->ball->pos) && defenseArea.contains(wm->ball->pos)) || defenseArea.contains(wm->ball->pos)){
        drawer->draw("oomad" , Vector2D(0,0) , 40);
        return rectangularPositions;
    }
    return circularPositions;
}

QList<Vector2D> DefensePlan::defenseFormationForCircularPositioning(int neededDefenseAgents, int allOfDefenseAgents , double downLimit , double upLimit) {
    QList<Vector2D> defensePosiotion;
    defensePosiotion.clear();
    if (neededDefenseAgents == allOfDefenseAgents) {
        if (neededDefenseAgents == 1) {
            defensePosiotion.append(oneDefenseFormationForCircularPositioning(downLimit , upLimit));
        } else if (neededDefenseAgents == 2) {
            defensePosiotion = twoDefenseFormationForCircularPositioning(downLimit , upLimit);
        }
    }
    else if (neededDefenseAgents < allOfDefenseAgents) {
        if (neededDefenseAgents == 1) {
            defensePosiotion.append(oneDefenseFormationForCircularPositioning(downLimit , upLimit));
        } else if (neededDefenseAgents == 2) {
            defensePosiotion = twoDefenseFormationForCircularPositioning(downLimit , upLimit);
        }
        for (int i = 0 ; i < allOfDefenseAgents - neededDefenseAgents ; i++) {
            defensePosiotion.append(Vector2D(0, i));
        }
    }
    else {
        if (allOfDefenseAgents == 1) {
            defensePosiotion.append(oneDefenseFormationForCircularPositioning(downLimit , upLimit));
        } else if (allOfDefenseAgents == 2) {
            defensePosiotion = twoDefenseFormationForCircularPositioning(downLimit , upLimit);
        }
    }
    return defensePosiotion;
}

Vector2D DefensePlan::oneDefenseFormationForCircularPositioning(double downLimit , double upLimit){
    Vector2D defensePosition;
    Vector2D sol[2];
    Vector2D ourGoalLeft = wm->field->ourGoalL();
    Vector2D ourGoalRight = wm->field->ourGoalR();
    Vector2D ballPosition = ballPrediction(false);
    int numberOfDefenseAgents = 1;
    Circle2D defenseArea(wm->field->ourGoal(),findBestRadiusForDefenseArea(getBestLineWithTallesForCircularPositioning(numberOfDefenseAgents , ourGoalLeft , ballPosition , ourGoalRight) , downLimit , upLimit));
    defenseArea.intersection(getBisectorLine(ourGoalLeft , ballPosition , ourGoalRight) , &sol[0] , &sol[1]);
    defensePosition = sol[0].isValid() && sol[0].dist(ballPosition) < sol[1].dist(ballPosition) ? sol[0] : sol[1];
    return defensePosition;
}

QList<Vector2D> DefensePlan::twoDefenseFormationForCircularPositioning(double downLimit , double upLimit){
    QList<Vector2D> defensePosition;
    Vector2D sol[4];
    double robotRadius = Robot::robot_radius_new;
    Vector2D ourGoalLeft = wm->field->ourGoalL();
    Vector2D ourGoalRight = wm->field->ourGoalR();
    Vector2D ballPosition = ballPrediction(false);
    Segment2D ourGoalLine = Segment2D(ourGoalLeft , ourGoalRight);
    Vector2D anIntesection = getBisectorSegment(ourGoalLeft , ballPosition , ourGoalRight).intersection(ourGoalLine);
    Vector2D anotherIntesection = getBisectorSegment(ourGoalLeft , ballPosition , ourGoalRight).intersection(ourGoalLine);
    int numberOfDefenseAgents = 2;
    Circle2D defenseArea(wm->field->ourGoal(),findBestRadiusForDefenseArea(getBestLineWithTallesForCircularPositioning(numberOfDefenseAgents , ourGoalLeft , ballPosition , ourGoalRight) , downLimit , upLimit));
    defenseArea.intersection(getBisectorLine(ourGoalLeft , ballPosition , anIntesection) , &sol[0] , &sol[1]);
    defenseArea.intersection(getBisectorLine(ourGoalRight , ballPosition , anIntesection) , &sol[2] , &sol[3]);
    defensePosition.append(sol[0].dist(ballPosition) < sol[1].dist(ballPosition) ? sol[0] : sol[1]);
    defensePosition.append(sol[2].dist(ballPosition) < sol[3].dist(ballPosition) ? sol[2] : sol[3]);
    /////////////////// Az chizaki bar sikhaki malideh im :) ///////////////////////////////
    if(getLinesOfBallTriangle().at(0).dist(defensePosition.at(0)) < getLinesOfBallTriangle().at(1).dist(defensePosition.at(0))){
        double distanceFromYalForFirstPosition = getLinesOfBallTriangle().at(0).dist(defensePosition.at(0));
        double distanceFromYalForSecondPosition = getLinesOfBallTriangle().at(1).dist(defensePosition.at(1));
        if(distanceFromYalForFirstPosition > robotRadius){
            anotherIntesection = getLinesOfBallTriangle().at(0).nearestPoint(defensePosition.at(0));
            defensePosition[0] += Vector2D(anotherIntesection - defensePosition.at(0)).norm()*(distanceFromYalForFirstPosition - robotRadius);
        }
        else if(distanceFromYalForFirstPosition <= robotRadius){
            anotherIntesection = getLinesOfBallTriangle().at(0).nearestPoint(defensePosition.at(0));
            defensePosition[0] += Vector2D(defensePosition.at(0) - anotherIntesection).norm()*(robotRadius - distanceFromYalForFirstPosition);
        }

        if(distanceFromYalForSecondPosition > robotRadius){
            anotherIntesection = getLinesOfBallTriangle().at(1).nearestPoint(defensePosition.at(1));
            defensePosition[1] += Vector2D(anotherIntesection - defensePosition.at(1)).norm()*(distanceFromYalForFirstPosition - robotRadius);
        }
        else if(distanceFromYalForSecondPosition <= robotRadius){
            anotherIntesection = getLinesOfBallTriangle().at(1).nearestPoint(defensePosition.at(1));
            defensePosition[1] += Vector2D(defensePosition.at(1) - anotherIntesection).norm()*(robotRadius - distanceFromYalForFirstPosition);
        }
    }
    else{
        double distanceFromYalForFirstPosition = getLinesOfBallTriangle().at(1).dist(defensePosition.at(0));
        double distanceFromYalForSecondPosition = getLinesOfBallTriangle().at(0).dist(defensePosition.at(1));
        if(distanceFromYalForFirstPosition > robotRadius){
            anotherIntesection = getLinesOfBallTriangle().at(1).nearestPoint(defensePosition.at(0));
            defensePosition[0] += Vector2D(anotherIntesection - defensePosition.at(0)).norm()*(distanceFromYalForFirstPosition - robotRadius);
        }
        else if(distanceFromYalForFirstPosition <= robotRadius){
            anotherIntesection = getLinesOfBallTriangle().at(1).nearestPoint(defensePosition.at(0));
            defensePosition[0] += Vector2D(defensePosition.at(0) - anotherIntesection).norm()*(robotRadius - distanceFromYalForFirstPosition);
        }

        if(distanceFromYalForSecondPosition > robotRadius){
            anotherIntesection = getLinesOfBallTriangle().at(0).nearestPoint(defensePosition.at(1));
            defensePosition[1] += Vector2D(anotherIntesection - defensePosition.at(1)).norm()*(distanceFromYalForFirstPosition - robotRadius);
        }
        else if(distanceFromYalForSecondPosition <= robotRadius){
            anotherIntesection = getLinesOfBallTriangle().at(0).nearestPoint(defensePosition.at(1));
            defensePosition[1] += Vector2D(defensePosition.at(1) - anotherIntesection).norm()*(robotRadius - distanceFromYalForFirstPosition);
        }
    }

    ROS_INFO_STREAM("AYA injas");
    return defensePosition;
}

QList<int> DefensePlan::detectOpponentPassOwners(double downEdgeLength , double upEdgeLength){
    QList<int> IDOfOpponentsInPolygon;
    Vector2D solutions[4];
    Vector2D solution;
    Polygon2D ballArea;
    Rect2D temp;
    Vector2D currentBallPosition = wm->ball->pos;
    Vector2D finalBallPosition = wm->ball->getPosInFuture(10);
    Circle2D downEdgeCircle(currentBallPosition , downEdgeLength / 2);
    Circle2D upEdgeCircle(finalBallPosition, upEdgeLength / 2);
    Line2D ballPath(currentBallPosition , finalBallPosition);
    IDOfOpponentsInPolygon.clear();
    //////////////// Make the polygon2D ////////////////////////////////
    if(wm->ball->vel.length()){
        downEdgeCircle.intersection(ballPath.perpendicular(currentBallPosition) , &solutions[0] , &solutions[1]);
        upEdgeCircle.intersection(ballPath.perpendicular(finalBallPosition) , &solutions[3] , &solutions[2]);
        solution = solutions[0];
        for(size_t i = 0 ; i < 4 ; i++){
            ballArea.addVertex(solutions[i]);
        }
        ballArea.addVertex(solution);
        for(size_t i = 0; i < 4 ; i++){
            drawer->draw(solutions[i] , QColor("red"));
        }
        drawer->draw(ballArea , QColor("black"));
    }
    ///////////////////// Calculate the reach time of each opponent agent ///////////////////////
    temp = Rect2D(currentBallPosition , finalBallPosition);
    drawer->draw(temp);
    for(size_t i = 0 ; i < wm->opp.activeAgentsCount() ; i++){
        if(temp.contains(wm->opp.active(i)->pos)){
            drawer->draw(Circle2D(wm->opp.active(i)->pos, 0.3) , "black");
            IDOfOpponentsInPolygon.append(wm->opp.activeAgentID(i));
        }
        else if(Line2D(wm->opp.active(i)->pos , wm->opp.active(i)->pos + wm->opp.active(i)->vel).
                intersection(Line2D(currentBallPosition , finalBallPosition)).valid()){
            drawer->draw(Circle2D(wm->opp.active(i)->pos, 0.3) , "black");
            IDOfOpponentsInPolygon.append(wm->opp.activeAgentID(i));
        }
    }
    ////////// Sort dangerous ///////////////////////////////
    qSort(IDOfOpponentsInPolygon.begin() , IDOfOpponentsInPolygon.end());
    return IDOfOpponentsInPolygon;
}

int DefensePlan::defenseNumber(){
    if (conf.StrictFormation){
        if (conf.Defense > 3){
            return 3;
        } else {
            return conf.Defense;
        }
    } else {
        return findNeededDefense();
    }
}

Vector2D DefensePlan::oneDefenseFormationForRecatngularPositioning(double downLimit , double upLimit) {
    Vector2D defensePosition;
    Vector2D sol[2];
    Vector2D ourGoalLeft = wm->field->ourGoalL();
    Vector2D ourGoalRight = wm->field->ourGoalR();
    Vector2D ballPosition = ballPrediction(false);
    int numberOfDefenseAgents = 1;
    wm->field->ourBigPenaltyArea(1, findBestOffsetForDefenseArea(getBestLineWithTallesForRecatngularPositioning(numberOfDefenseAgents , ourGoalLeft , ballPosition , ourGoalRight) , downLimit , upLimit), 0).intersection(
                getBisectorSegment(ourGoalLeft , ballPosition , ourGoalRight) , &sol[0] , &sol[1]);
    defensePosition = sol[0].dist(ballPosition) < sol[1].dist(ballPosition) ? sol[0] : sol[1];
    return defensePosition;
}

QList<Vector2D> DefensePlan::twoDefenseFormationForRectangularPositioning(double downLimit , double upLimit) {
    QList<Vector2D> defensePosition;
    Vector2D sol[4];
    Vector2D ourGoalLeft = wm->field->ourGoalL();
    Vector2D ourGoalRight = wm->field->ourGoalR();
    Vector2D ballPosition = ballPrediction(false);
    Segment2D ourGoalLine = Segment2D(ourGoalLeft , ourGoalRight);
    Vector2D anIntesection = getBisectorSegment(ourGoalLeft , ballPosition , ourGoalRight).intersection(ourGoalLine);
    int numberOfDefenseAgents = 2;
    wm->field->ourBigPenaltyArea(1, findBestOffsetForDefenseArea(getBestLineWithTallesForRecatngularPositioning(numberOfDefenseAgents , ourGoalLeft , ballPosition , ourGoalRight) , downLimit , upLimit), 0).intersection(
                getBisectorSegment(anIntesection , ballPosition , ourGoalLeft) , &sol[0] , &sol[1]);
    wm->field->ourBigPenaltyArea(1, findBestOffsetForDefenseArea(getBestLineWithTallesForRecatngularPositioning(numberOfDefenseAgents , ourGoalLeft , ballPosition , ourGoalRight) , downLimit , upLimit), 0).intersection(
                getBisectorSegment(anIntesection , ballPosition , ourGoalRight) , &sol[2] , &sol[3]);
    defensePosition.append(sol[0].dist(ballPosition) < sol[1].dist(ballPosition) ? sol[0] : sol[1]);
    defensePosition.append(sol[2].dist(ballPosition) < sol[3].dist(ballPosition) ? sol[2] : sol[3]);
    return defensePosition;
}

QList<Vector2D> DefensePlan::threeDefenseFormationForRecatangularPositioning(double downLimit , double upLimit) {
    QList<Vector2D> defensePosition;
    Vector2D sol[8];
    Vector2D ourGoalLeft = wm->field->ourGoalL();
    Vector2D ourGoalRight = wm->field->ourGoalR();
    Vector2D ballPosition = ballPrediction(false);
    Segment2D ourGoalLine = Segment2D(ourGoalLeft , ourGoalRight);
    Vector2D downIntersection;
    Vector2D upIntesection;
    double forwardDefenseUpLimit = upLimit + 0.3;
    double forwardDefenseBestOffset = forwardDefenseUpLimit - wm->field->_PENALTY_DEPTH;
    if (fabs(ballPosition.x) >= 6 - wm->field->_PENALTY_DEPTH) {
        wm->field->ourBigPenaltyArea(1, 0.2, 0).intersection(
                    getBisectorSegment(ourGoalLeft , ballPosition , ourGoalRight) , &sol[0] , &sol[1]);
        wm->field->ourBigPenaltyArea(1, 0.2, 0).intersection(
                    getLinesOfBallTriangle().at(0), &sol[4] , &sol[5]);
        wm->field->ourBigPenaltyArea(1, 0.2, 0).intersection(
                    getLinesOfBallTriangle().at(1) , &sol[6] , &sol[7]);
    } else {
        if (wm->field->ourBigPenaltyArea(1, forwardDefenseBestOffset, 0).contains(ballPosition)) {
            wm->field->ourBigPenaltyArea(1, 0.2, 0).intersection(
                        getBisectorSegment(ourGoalLeft , ballPosition , ourGoalRight) , &sol[0] , &sol[1]);
            wm->field->ourBigPenaltyArea(1, 0.2, 0).intersection(
                        getLinesOfBallTriangle().at(0), &sol[4] , &sol[5]);
            wm->field->ourBigPenaltyArea(1, 0.2, 0).intersection(
                        getLinesOfBallTriangle().at(1) , &sol[6] , &sol[7]);
        } else {
            wm->field->ourBigPenaltyArea(1, forwardDefenseBestOffset, 0).intersection(
                        getBisectorSegment(ourGoalLeft , ballPosition , ourGoalRight) , &sol[0] , &sol[1]);
            Circle2D forwardDefense(sol[0].dist(wm->ball->pos) < sol[1].dist(wm->ball->pos) ? sol[0] : sol[1] , Robot::robot_radius_new);
            forwardDefense.tangent(ballPosition , &sol[2] , &sol[3]);
            downIntersection = ourGoalLine.intersection(Line2D(ballPosition , sol[2])).y <= ourGoalLine.intersection(Line2D(ballPosition , sol[3])).y ?
                        ourGoalLine.intersection(Line2D(ballPosition , sol[2])) : ourGoalLine.intersection(Line2D(ballPosition , sol[3]));
            upIntesection = ourGoalLine.intersection(Line2D(ballPosition , sol[2])).y >= ourGoalLine.intersection(Line2D(ballPosition , sol[3])).y ?
                        ourGoalLine.intersection(Line2D(ballPosition , sol[2])) : ourGoalLine.intersection(Line2D(ballPosition , sol[3]));
            //            ROS_INFO(QString("down: %1").arg(downIntersection.y).toStdString().c_str());
            //            ROS_INFO(QString("up: %1").arg(upIntesection.y).toStdString().c_str());
            //            ROS_INFO(QString("sol2: %1").arg(sol[2].y).toStdString().c_str());
            //            ROS_INFO(QString("sol3: %1").arg(sol[3].y).toStdString().c_str());
            if (upIntesection.isValid()) {
                wm->field->ourBigPenaltyArea(1, findBestOffsetForDefenseArea(getBestLineWithTallesForRecatngularPositioning(1 , upIntesection , ballPosition , ourGoalLeft) , downLimit , upLimit), 0).intersection(
                            getBisectorSegment(ourGoalLeft , ballPosition , upIntesection) , &sol[4] , &sol[5]);
            } else {
                wm->field->ourBigPenaltyArea(1, findBestOffsetForDefenseArea(getBestLineWithTallesForRecatngularPositioning(1 , upIntesection , ballPosition , ourGoalLeft) , downLimit , upLimit), 0).intersection(
                            getLinesOfBallTriangle().at(0), &sol[4] , &sol[5]);
            }
            if (downIntersection.isValid()) {
                wm->field->ourBigPenaltyArea(1, findBestOffsetForDefenseArea(getBestLineWithTallesForRecatngularPositioning(1 , downIntersection , ballPosition , ourGoalRight) , downLimit , upLimit), 0).intersection(
                            getBisectorSegment(ourGoalRight , ballPosition , downIntersection) , &sol[6] , &sol[7]);
            } else {
                wm->field->ourBigPenaltyArea(1, findBestOffsetForDefenseArea(getBestLineWithTallesForRecatngularPositioning(1 , downIntersection , ballPosition , ourGoalRight) , downLimit , upLimit), 0).intersection(
                            getLinesOfBallTriangle().at(1) , &sol[6] , &sol[7]);
            }
        }
    }
    defensePosition.append(sol[0].dist(ballPosition) < sol[1].dist(ballPosition) ? sol[0] : sol[1]);
    defensePosition.append(sol[4].dist(ballPosition) < sol[5].dist(ballPosition) ? sol[4] : sol[5]);
    defensePosition.append(sol[6].dist(ballPosition) < sol[7].dist(ballPosition) ? sol[6] : sol[7]);
    return defensePosition;

}

QList<Vector2D> DefensePlan::defenseFormationForRectangularPositioning(int neededDefenseAgents, int allOfDefenseAgents , double downLimit , double upLimit) {
    QList<Vector2D> defensePosiotion;
    Vector2D ourGoalLeft = wm->field->ourGoalL();
    Vector2D ourGoalRight = wm->field->ourGoalR();
    Vector2D ballPosition = ballPrediction(false);
    defensePosiotion.clear();
    //    ROS_INFO(QString("allOfDefenseAgents: %1").arg(allOfDefenseAgents).toStdString().c_str());
    double temp = findBestOffsetForDefenseArea(getBestLineWithTallesForRecatngularPositioning(neededDefenseAgents , ourGoalLeft , ballPosition , ourGoalRight) , downLimit , upLimit);
    //    ROS_INFO(QString("Best Offset for penalty area: %1").arg(temp).toStdString().c_str());
    if (neededDefenseAgents == allOfDefenseAgents) {
        if (neededDefenseAgents == 1) {
            defensePosiotion.append(oneDefenseFormationForRecatngularPositioning(downLimit , upLimit));
        } else if (neededDefenseAgents == 2) {
            defensePosiotion = twoDefenseFormationForRectangularPositioning(downLimit , upLimit);
        } else if (neededDefenseAgents == 3) {
            defensePosiotion = threeDefenseFormationForRecatangularPositioning(downLimit , upLimit);
        }
    } else if (neededDefenseAgents < allOfDefenseAgents) {
        if (neededDefenseAgents == 1) {
            defensePosiotion.append(oneDefenseFormationForRecatngularPositioning(downLimit , upLimit));
        } else if (neededDefenseAgents == 2) {
            defensePosiotion = twoDefenseFormationForRectangularPositioning(downLimit , upLimit);
        } else if (neededDefenseAgents == 3) {
            defensePosiotion = threeDefenseFormationForRecatangularPositioning(downLimit , upLimit);
        }
        for (int i = 0 ; i < allOfDefenseAgents - neededDefenseAgents ; i++) {
            defensePosiotion.append(Vector2D(0, i));
        }
    } else {
        if (allOfDefenseAgents == 1) {
            defensePosiotion.append(oneDefenseFormationForRecatngularPositioning(downLimit , upLimit));
        } else if (allOfDefenseAgents == 2) {
            defensePosiotion = twoDefenseFormationForRectangularPositioning(downLimit , upLimit);
        } else if (allOfDefenseAgents == 3) {
            defensePosiotion = threeDefenseFormationForRecatangularPositioning(downLimit , upLimit);
        }
    }
    return defensePosiotion;
}

QList<Segment2D> DefensePlan::getLinesOfBallTriangle(){
    QList<Segment2D> linesOfBallTriangle;
    Vector2D ballPos = ballPrediction(false);
    Vector2D ourGoalL = wm->field->ourGoalL();
    Vector2D ourGoalR = wm->field->ourGoalR();
    linesOfBallTriangle.append(Segment2D(ballPos , ourGoalL));
    linesOfBallTriangle.append(Segment2D(ballPos , ourGoalR));
    return linesOfBallTriangle;
}

Line2D DefensePlan::getBestLineWithTallesForRecatngularPositioning(int defenseCount , Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint) {
    double robotDiameter = 2 * Robot::robot_radius_new;
    Segment2D ourGoalLine(firstPoint, secondPoint);
    Segment2D biggerFrontageOfTriangle;
    Segment2D smallerFrontageOfTriangle;
    Vector2D desirePoint;
    if (getLinesOfBallTriangle().at(0).length() > getLinesOfBallTriangle().at(1).length()) {
        biggerFrontageOfTriangle = getLinesOfBallTriangle().at(0);
        smallerFrontageOfTriangle = getLinesOfBallTriangle().at(1);
    } else {
        biggerFrontageOfTriangle = getLinesOfBallTriangle().at(1);
        smallerFrontageOfTriangle = getLinesOfBallTriangle().at(0);
    }
    Line2D aimLessLine(ourGoalLine.intersection(smallerFrontageOfTriangle) , biggerFrontageOfTriangle.nearestPoint(ourGoalLine.intersection(smallerFrontageOfTriangle)));
    Segment2D tempAimLessLine(ourGoalLine.intersection(smallerFrontageOfTriangle) , biggerFrontageOfTriangle.nearestPoint(ourGoalLine.intersection(smallerFrontageOfTriangle)));
    if (tempAimLessLine.length() >= robotDiameter) {
        if (originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()) >= (-6 + 1.4)) {
            aimLessLine = Line2D(Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), -4.5),
                                 Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), 4.5));
            desirePoint = smallerFrontageOfTriangle.intersection(aimLessLine);
            aimLessLine = Line2D(biggerFrontageOfTriangle.nearestPoint(smallerFrontageOfTriangle.intersection(aimLessLine)) , smallerFrontageOfTriangle.intersection(aimLessLine));
        } else {
            if (originPoint.x < -6 + 1.4) {
                aimLessLine = Line2D(Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), 4.5),
                                     Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), -4.5));
                desirePoint = smallerFrontageOfTriangle.intersection(aimLessLine);
                aimLessLine = Line2D(biggerFrontageOfTriangle.nearestPoint(smallerFrontageOfTriangle.intersection(aimLessLine)) , smallerFrontageOfTriangle.intersection(aimLessLine));
            } else {
                aimLessLine = Line2D(Vector2D(-6 + 1.4, -4.5), Vector2D(-6 + 1.4, 4.5));
                desirePoint = smallerFrontageOfTriangle.intersection(aimLessLine);
                aimLessLine = Line2D(biggerFrontageOfTriangle.nearestPoint(smallerFrontageOfTriangle.intersection(aimLessLine)) , smallerFrontageOfTriangle.intersection(aimLessLine));
            }
        }
    }
    return aimLessLine;
}

Segment2D DefensePlan::getBestSegmentWithTallesForRectangularPositioning(int defenseCount , Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint) {
    double robotDiameter = 2 * Robot::robot_radius_new;
    Segment2D ourGoalLine(firstPoint, secondPoint);
    Segment2D biggerFrontageOfTriangle;
    Segment2D smallerFrontageOfTriangle;
    Vector2D desirePoint;
    if (getLinesOfBallTriangle().at(0).length() > getLinesOfBallTriangle().at(1).length()) {
        biggerFrontageOfTriangle = getLinesOfBallTriangle().at(0);
        smallerFrontageOfTriangle = getLinesOfBallTriangle().at(1);
    } else {
        biggerFrontageOfTriangle = getLinesOfBallTriangle().at(1);
        smallerFrontageOfTriangle = getLinesOfBallTriangle().at(0);
    }

    Line2D aimLessLine(ourGoalLine.intersection(smallerFrontageOfTriangle) , biggerFrontageOfTriangle.nearestPoint(ourGoalLine.intersection(smallerFrontageOfTriangle)));
    Segment2D tempAimLessLine(ourGoalLine.intersection(smallerFrontageOfTriangle) , biggerFrontageOfTriangle.nearestPoint(ourGoalLine.intersection(smallerFrontageOfTriangle)));
    if (tempAimLessLine.length() >= robotDiameter) {
        if (originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()) > (-6 + 1.4)) {
            aimLessLine = Line2D(Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), 4.5),
                                 Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), -4.5));
            desirePoint = smallerFrontageOfTriangle.intersection(aimLessLine);
            tempAimLessLine = Segment2D(biggerFrontageOfTriangle.nearestPoint(smallerFrontageOfTriangle.intersection(aimLessLine)) , smallerFrontageOfTriangle.intersection(aimLessLine));
        } else {
            if (originPoint.x < -6 + 1.4) {
                aimLessLine = Line2D(Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), 4.5),
                                     Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), -4.5));
                desirePoint = smallerFrontageOfTriangle.intersection(aimLessLine);
                tempAimLessLine = Segment2D(biggerFrontageOfTriangle.nearestPoint(smallerFrontageOfTriangle.intersection(aimLessLine)) , smallerFrontageOfTriangle.intersection(aimLessLine));
            } else {
                aimLessLine = Line2D(Vector2D(-6 + 1.4, -4.5), Vector2D(-6 + 1.4, 4.5));
                desirePoint = smallerFrontageOfTriangle.intersection(aimLessLine);
                tempAimLessLine = Segment2D(biggerFrontageOfTriangle.nearestPoint(smallerFrontageOfTriangle.intersection(aimLessLine)) , smallerFrontageOfTriangle.intersection(aimLessLine));

            }
        }
        return tempAimLessLine;
    }
}

Line2D DefensePlan::getBestLineWithTallesForCircularPositioning(int defenseCount , Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint) {
    double robotDiameter = 2 * Robot::robot_radius_new;
    Vector2D sol[2];
    Vector2D suitablePoint;
    Segment2D ourGoalLine(firstPoint, secondPoint);
    Segment2D biggerFrontageOfTriangle;
    Segment2D smallerFrontageOfTriangle;
    if (getLinesOfBallTriangle().at(0).length() > getLinesOfBallTriangle().at(1).length()) {
        biggerFrontageOfTriangle = getLinesOfBallTriangle().at(0);
        smallerFrontageOfTriangle = getLinesOfBallTriangle().at(1);
    } else {
        biggerFrontageOfTriangle = getLinesOfBallTriangle().at(1);
        smallerFrontageOfTriangle = getLinesOfBallTriangle().at(0);
    }
    Line2D aimLessLine(ourGoalLine.intersection(smallerFrontageOfTriangle) , biggerFrontageOfTriangle.nearestPoint(ourGoalLine.intersection(smallerFrontageOfTriangle)));
    Segment2D tempAimLessLine(ourGoalLine.intersection(smallerFrontageOfTriangle) , biggerFrontageOfTriangle.nearestPoint(ourGoalLine.intersection(smallerFrontageOfTriangle)));
    if(tempAimLessLine.length() >= robotDiameter){
        if(biggerFrontageOfTriangle.intersection(Line2D(Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), -4.5),
                                                        Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), 4.5))).dist(wm->field->ourGoal()) <= RADIUS_FOR_CRITICAL_DEFENSE_AREA
                || (originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length())) < -6){
            Circle2D(wm->field->ourGoal() , RADIUS_FOR_CRITICAL_DEFENSE_AREA).intersection(biggerFrontageOfTriangle , &sol[0] , &sol[1]);
            drawer->draw("is" ,  Vector2D(-1,0) , 40);
            suitablePoint = sol[0].isValid() && sol[0].dist(wm->ball->pos) < sol[1].dist(wm->ball->pos) ? sol[0] : sol[1];
            aimLessLine = Line2D(Vector2D(suitablePoint.x, -4.5), Vector2D(suitablePoint.x, 4.5));
            //            drawer->draw(Segment2D(Vector2D(suitablePoint.x, -4.5), Vector2D(suitablePoint.x, 4.5)));
        }
        else{
            aimLessLine = Line2D(Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), -4.5),
                                 Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), 4.5));
            //            drawer->draw(Segment2D(Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), -4.5),
            //                                   Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), 4.5)));
        }
    }
    return aimLessLine;
}

Segment2D DefensePlan::getBestSegmentWithTallesForCircularPositioning(int defenseCount , Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint) {
    double robotDiameter = 2 * Robot::robot_radius_new;
    Vector2D sol[2];
    Vector2D suitablePoint;
    Segment2D ourGoalLine(firstPoint, secondPoint);
    Segment2D biggerFrontageOfTriangle;
    Segment2D smallerFrontageOfTriangle;
    if (getLinesOfBallTriangle().at(0).length() > getLinesOfBallTriangle().at(1).length()) {
        biggerFrontageOfTriangle = getLinesOfBallTriangle().at(0);
        smallerFrontageOfTriangle = getLinesOfBallTriangle().at(1);
    } else {
        biggerFrontageOfTriangle = getLinesOfBallTriangle().at(1);
        smallerFrontageOfTriangle = getLinesOfBallTriangle().at(0);
    }
    Segment2D tempAimLessLine(ourGoalLine.intersection(smallerFrontageOfTriangle) , biggerFrontageOfTriangle.nearestPoint(ourGoalLine.intersection(smallerFrontageOfTriangle)));
    if(tempAimLessLine.length() >= robotDiameter){
        if(biggerFrontageOfTriangle.intersection(Line2D(Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), -4.5),
                                                        Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), 4.5))).dist(wm->field->ourGoal()) <= RADIUS_FOR_CRITICAL_DEFENSE_AREA
                || (originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length())) < -6){
            Circle2D(wm->field->ourGoal() , RADIUS_FOR_CRITICAL_DEFENSE_AREA).intersection(biggerFrontageOfTriangle , &sol[0] , &sol[1]);
            drawer->draw("is" ,  Vector2D(-1,0) , 40);
            suitablePoint = sol[0].isValid() && sol[0].dist(wm->ball->pos) < sol[1].dist(wm->ball->pos) ? sol[0] : sol[1];
            tempAimLessLine = Segment2D(Vector2D(suitablePoint.x, -4.5), Vector2D(suitablePoint.x, 4.5));
            //            drawer->draw(Segment2D(Vector2D(suitablePoint.x, -4.5), Vector2D(suitablePoint.x, 4.5)));
        }
        else{
            tempAimLessLine = Segment2D(Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), -4.5),
                                        Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), 4.5));
            //            drawer->draw(Segment2D(Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), -4.5),
            //                                   Vector2D(originPoint.x - (defenseCount * robotDiameter * (6 - fabs(originPoint.x)) / tempAimLessLine.length()), 4.5)));
        }
    }
    return tempAimLessLine;
}

double DefensePlan::findBestOffsetForDefenseArea(Line2D bestLineWithTalles , double downLimit , double upLimit) {
    double bestOffsetForPenaltyArea = 0;
    Vector2D ballPos = ballPrediction(false);
    Segment2D biggerFrontageOfTriangle;
    Segment2D smallerFrontageOfTriangle;
    if (getLinesOfBallTriangle().at(0).length() > getLinesOfBallTriangle().at(1).length()) {
        biggerFrontageOfTriangle = getLinesOfBallTriangle().at(0);
        smallerFrontageOfTriangle = getLinesOfBallTriangle().at(1);
    } else {
        biggerFrontageOfTriangle = getLinesOfBallTriangle().at(1);
        smallerFrontageOfTriangle = getLinesOfBallTriangle().at(0);
    }
    if (wm->field->ourBigPenaltyArea(1, upLimit - wm->field->_PENALTY_DEPTH , 0).contains(ballPos)) {
        if (fabs(ballPos.x) >= 6 - wm->field->_PENALTY_DEPTH) {
            upLimit = fabs(ballPos.y) - wm->field->_PENALTY_WIDTH / 2 - 1;
        } else {
            upLimit = 6 - fabs(ballPos.x) - 1;
        }
    }
    //////////////// The position Isn't in penalty area ////////////////
    if (fabs(ballPos.x) >= 6 - wm->field->_PENALTY_DEPTH) {
        bestOffsetForPenaltyArea = fabs(biggerFrontageOfTriangle.intersection(bestLineWithTalles).y) - wm->field->_PENALTY_WIDTH / 2;
        if (bestOffsetForPenaltyArea <= downLimit - wm->field->_PENALTY_WIDTH / 2) {
            bestOffsetForPenaltyArea = downLimit - wm->field->_PENALTY_WIDTH / 2;
        }
        if (bestOffsetForPenaltyArea > upLimit - wm->field->_PENALTY_WIDTH / 2) {
            bestOffsetForPenaltyArea = upLimit - wm->field->_PENALTY_WIDTH / 2;
        }
    } else if (fabs(biggerFrontageOfTriangle.intersection(bestLineWithTalles).x) >= 6 - wm->field->_PENALTY_DEPTH) {
        bestOffsetForPenaltyArea = 0.2;
    } else {
        bestOffsetForPenaltyArea = 6 - fabs(biggerFrontageOfTriangle.intersection(bestLineWithTalles).x) - wm->field->_PENALTY_DEPTH;
        if (bestOffsetForPenaltyArea <= downLimit - wm->field->_PENALTY_DEPTH) {
            bestOffsetForPenaltyArea = downLimit - wm->field->_PENALTY_DEPTH;
        }
        if (bestOffsetForPenaltyArea > upLimit - wm->field->_PENALTY_DEPTH) {
            bestOffsetForPenaltyArea = upLimit - wm->field->_PENALTY_DEPTH;
        }
    }
    if (wm->field->ourBigPenaltyArea(1, 0, 0).contains(ballPos)) {
        bestOffsetForPenaltyArea = 0.2;
    }
    if (bestOffsetForPenaltyArea <= 0) {
        bestOffsetForPenaltyArea = 0.2;
    }
    if (wm->field->ourBigPenaltyArea(1, 0, 0).contains(wm->ball->pos)) {
        bestOffsetForPenaltyArea = 0.2;
    }
    return bestOffsetForPenaltyArea;
}

double DefensePlan::findBestRadiusForDefenseArea(Line2D bestLineWithTalles , double downLimit , double upLimit){
    double bestRadiusForDefenseArea = 0;
    Segment2D smallerFrontageOfTriangle;
    Segment2D biggerFrontageOfTriangle;
    if (getLinesOfBallTriangle().at(0).length() > getLinesOfBallTriangle().at(1).length()) {
        biggerFrontageOfTriangle = getLinesOfBallTriangle().at(0);
        smallerFrontageOfTriangle = getLinesOfBallTriangle().at(1);
    } else {
        biggerFrontageOfTriangle = getLinesOfBallTriangle().at(1);
        smallerFrontageOfTriangle = getLinesOfBallTriangle().at(0);
    }
    bestRadiusForDefenseArea = biggerFrontageOfTriangle.intersection(bestLineWithTalles).dist(wm->field->ourGoal());
    if(bestRadiusForDefenseArea <= downLimit){
        bestRadiusForDefenseArea = downLimit;
    }
    if(bestRadiusForDefenseArea >= upLimit){
        bestRadiusForDefenseArea = upLimit;
    }
    ROS_INFO_STREAM("bestRadius: " << bestRadiusForDefenseArea);
    return bestRadiusForDefenseArea;
}

int DefensePlan::findNeededDefense(){
    int neededDefense = 0;
    int numOfDefenses = 0;
    Vector2D sol[4];
    double robotDiameter = 2 * Robot::robot_radius_new;
    Vector2D ballPos = ballPrediction(false);
    Vector2D ourGoalL = wm->field->ourGoalL();
    Vector2D ourGoalR = wm->field->ourGoalR();
    Segment2D aimLessLine;
    Segment2D ourGoalLine(ourGoalL, ourGoalR);
    Segment2D biggerFrintageOfTriangle;
    Segment2D smallerFrintageOfTriangle;
    if (getLinesOfBallTriangle().at(0).length() > getLinesOfBallTriangle().at(1).length()) {
        biggerFrintageOfTriangle = getLinesOfBallTriangle().at(0);
        smallerFrintageOfTriangle = getLinesOfBallTriangle().at(1);
    } else {
        biggerFrintageOfTriangle = getLinesOfBallTriangle().at(1);
        smallerFrintageOfTriangle = getLinesOfBallTriangle().at(0);
    }
    aimLessLine = Segment2D(ourGoalLine.intersection(smallerFrintageOfTriangle) , biggerFrintageOfTriangle.nearestPoint(ourGoalLine.intersection(smallerFrintageOfTriangle)));
    //////// with itterative algorithm /////////////////////////
    //            Circle2D(wm->field->ourGoal(),findBestRadiusForDefenseArea(getBestLineWithTallesForCircularPositioning(numOfDefenses,ourGoalL,ballPos, ourGoalR),RADIUS_FOR_CRITICAL_DEFENSE_AREA,2.7)).intersection(getBestSegmentWithTallesForCircularPositioning(numOfDefenses,ourGoalL,ballPos,ourGoalR) , &sol[0],&sol[1]);
    //            Segment2D(sol[0] , sol[1]).length()
    if (aimLessLine.length() <= robotDiameter){
        neededDefense = 1;
    }
    else{
        for(numOfDefenses = 2 ; numOfDefenses < 3 ; numOfDefenses++){
            Circle2D temp = Circle2D(wm->field->ourGoal(),findBestRadiusForDefenseArea(getBestLineWithTallesForCircularPositioning(numOfDefenses,ourGoalL,ballPos, ourGoalR),RADIUS_FOR_CRITICAL_DEFENSE_AREA,2.7));
            temp.intersection(getLinesOfBallTriangle().at(0) , &sol[0] , &sol[1]);
            temp.intersection(getLinesOfBallTriangle().at(1) , &sol[2] , &sol[3]);
            drawer->draw(temp,0,360);
            drawer->draw(Segment2D(sol[0] , sol[2]) , "blue");
            if(Segment2D(sol[0] , sol[2]).length() <= ((numOfDefenses) * (robotDiameter))){
                neededDefense = numOfDefenses;
                break;
            }
        }
        if(Circle2D(wm->field->ourGoal() , RADIUS_FOR_CRITICAL_DEFENSE_AREA).contains(wm->ball->pos)){
            neededDefense = 2;
        }
        if(neededDefense == 0){
            neededDefense = 2;
        }
    }
    return neededDefense;
}

bool DefensePlan::areAgentsStuckTogether(const QList<Vector2D> &agentsPosition) {
    //// If defense agents stuck together , this function
    for (int i = 0 ; i < agentsPosition.size() ; i++) {
        for (int j = i+1 ; j < agentsPosition.size() ; j++) {
            if (agentsPosition.at(i).dist(agentsPosition.at(j)) <= 2 * Robot::robot_radius_new) {
                return true;
            }
        }
    }
    return false;
}

void DefensePlan::agentsStuckTogether(const QList<Vector2D> &agentsPosition , QList<Vector2D> &stuckPositions , QList<int> &stuckIndexs) {
    //// If defense agents stuck together , this function
    stuckPositions.clear();
    stuckIndexs.clear();
    for (int i = 0 ; i < agentsPosition.size() ; i++) {
        for (int j = 0 ; j < i ; j++) {
            if (agentsPosition.at(i).dist(agentsPosition.at(j)) <= 2 * Robot::robot_radius_new + MIN_TWO_ROBOTS_DIST) {
                stuckPositions.append(agentsPosition.at(i));
                stuckPositions.append(agentsPosition.at(j));
            }
        }
    }

    for (auto stuckPosition : stuckPositions) {
        for (int j = 0 ; j < agentsPosition.size() ; j++) {
            if (stuckPosition == agentsPosition.at(j)) {
                stuckIndexs.append(j);
            }
        }
    }
}

void DefensePlan::correctingTheAgentsAreStuckTogether(QList<Vector2D> &agentsPosition, QList<Vector2D> &stuckPositions , QList<int> &stuckIndexs) {

    QList<Segment2D> centerToCenter;
    QList<Vector2D> solvedPosition;
    QList<Vector2D> temp;
    QList<Vector2D> tempSol;
    QList<int> tempIndexs;
    QList<Vector2D> tempAgentsPosition;
    QList<Vector2D> finalSolvedPosition;
    QList<Vector2D> solvedPositionsAreNotInThePenaltyArea;
    QList<Vector2D> nonRepetitiveFinalSolvedPosition;
    Vector2D tempPoint = Vector2D(0, 0);
    QList<Vector2D> tempVectors;
    ///////////////////// Update the state /////////////////////////////////////
    solvedPosition.clear();
    tempAgentsPosition.clear();
    nonRepetitiveFinalSolvedPosition.clear();
    solvedPositionsAreNotInThePenaltyArea.clear();
    ///////////////////////////////////////////////////////////////////////////
    for (int i = 0 ; i < stuckPositions.size() ; i++) {
        if (i % 2 == 0) {
            centerToCenter.append(Segment2D(stuckPositions.at(i) , stuckPositions.at(i + 1)));
        } else {
            centerToCenter.append(Segment2D(stuckPositions.at(i) , stuckPositions.at(i - 1)));
        }
    }
    DBUG(QString("center : %1").arg(centerToCenter.size()) , D_AHZ);
    int desire_robot_dist = stuckPositions.size() == 2 ? MIN_TWO_ROBOTS_DIST:MIN_MORE_ROBOTS_DIST;
    for (int i = 0 ; i < stuckPositions.size() ; i++)
        solvedPosition.append(stuckPositions.at(i) + (1.1 * (Robot::robot_radius_new - centerToCenter.at(i).length() / 2) + desire_robot_dist) * ((centerToCenter.at(i).a() - centerToCenter.at(i).b()).norm()));
    ///////////// Check the resulted points, don't be in the PArea /////////////
    for (int i = 0 ; i < solvedPosition.size() ; i++) {
        if (wm->field->isInOurPenaltyArea(solvedPosition.at(i))) {
            DBUG(QString("stuck position is penalty area") , D_AHZ);
            solvedPosition.removeAt(i);
            tempSol = wm->field->ourPAreaIntersect(Segment2D(wm->field->ourGoal() , stuckPositions.at(i)));
            if (tempSol.size()) {
                if (tempSol.size() == 2) {
                    solvedPositionsAreNotInThePenaltyArea.append(tempSol.at(0).dist(stuckPositions.at(i)) < tempSol.at(1).dist(stuckPositions.at(i)) ? tempSol.at(0) : tempSol.at(1));
                } else if (tempSol.size() == 1) {
                    solvedPositionsAreNotInThePenaltyArea.append(tempSol.at(0));
                }
            }
        } else {
            solvedPositionsAreNotInThePenaltyArea.append(solvedPosition.at(i));
        }
    }
    ////////////////////////////////////////////////////////////////////////////
    for (int i = 0 ; i < stuckPositions.size() && i < stuckIndexs.size() && i < solvedPositionsAreNotInThePenaltyArea.size() ; i++) {
        for (int j = 0 ; j < stuckIndexs.size() ; j++) {
            if (stuckIndexs.at(i) == stuckIndexs.at(j) && i != j) {
                tempIndexs.append(j);
                DBUG(QString("temp Index : %1").arg(j) , D_AHZ);
            }
        }
        tempIndexs.append(i);
        if (tempIndexs.size() > 1) {
            tempPoint = stuckPositions.at(i);
            for (int k = 0 ; k < solvedPositionsAreNotInThePenaltyArea.size() ; k++) {
                for (int tempIndex : tempIndexs) {
                    if (tempIndex == k) {
                        temp.append(solvedPositionsAreNotInThePenaltyArea.at(k));
                    }
                }
            }
            for (auto n : temp) {
                tempVectors.append(n - tempPoint);
            }
            for (auto tempVector : tempVectors) {
                tempPoint += tempVector;
            }
            finalSolvedPosition.append(tempPoint);
        } else{
            finalSolvedPosition.append(solvedPositionsAreNotInThePenaltyArea.at(i));
        }
        tempIndexs.clear();
        tempVectors.clear();
        temp.clear();
    }
    //// Tnx for Mahi && Parsa && Mhmmd :) ////////////////////////////////////
    for (auto i : finalSolvedPosition) {
        if (!nonRepetitiveFinalSolvedPosition.contains(i)) {
            nonRepetitiveFinalSolvedPosition.append(i);
        }
    }
    ///////////////////////////////////////////////////////////////////////////
    for (auto stuckPosition : stuckPositions) {
        for (int j = 0 ; j < agentsPosition.size() ; j++) {
            if (agentsPosition.at(j) == stuckPosition) {
                agentsPosition.removeAt(j);
            }
        }
    }

    for (auto i : nonRepetitiveFinalSolvedPosition) {
        agentsPosition.append(i);
    }

    DBUG(QString("solved : %1").arg(finalSolvedPosition.size()), D_AHZ);
    DBUG(QString("match : %1").arg(agentsPosition.size()), D_AHZ);
}

float getDegree(Vector2D pos1, Vector2D origin, Vector2D pos3) {
    //// get the angle of between two vector that
    //// is made up by this 3 points.

    Vector2D v1 = pos1 - origin;
    Vector2D v2 = pos3 - origin;
    return (float)(v1.th() - v2.th()).degree();
}

bool DefensePlan::isInIndirectArea(Vector2D aPoint) {
    //// checks that a point is in the circle around the ball
    //// with 50cm radius or not.

    bool localFlag = Circle2D(wm->ball->pos , 0.7).contains(aPoint);
    return localFlag;
}

Line2D DefensePlan::getBisectorLine(Vector2D firstPoint , Vector2D originPoint , Vector2D thirdPoint) {
    //// gets the bisector line of an angle
    //// that is made up by this 3 points.

    Line2D bisectorLine(originPoint , AngleDeg::bisect((firstPoint - originPoint).th() , (thirdPoint - originPoint).th()));
    return bisectorLine;
}

Segment2D DefensePlan::getBisectorSegment(Vector2D firstPoint , Vector2D originPoint , Vector2D thirdPoint) {
    //// gets the bisector segment of an angle
    //// that is made up by this 3 points.

    Line2D bisectorLine(originPoint , AngleDeg::bisect((firstPoint - originPoint).th() , (thirdPoint - originPoint).th()));
    Segment2D bisectorSegment(originPoint , Segment2D(thirdPoint , firstPoint).intersection(bisectorLine));
    return bisectorSegment;
}

void DefensePlan::manToManMarkBlockPassInPlayOff(QList<Vector2D> opponentAgentsToBeMarkPossition , int ourMarkAgentsSize , double proportionOfDistance) {
    //// This function blocks the lines that are between ball &&
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
    stopMode = gameState->isStop();
    ourMarkAgentsPossition.clear();
    tempOpponentAgentsToBeMarkedPosition.clear();
    markPoses.clear();
    markAngs.clear();
    markRoles.clear();
    /////////////////// Intelligent mark plan ///////////////////////////////
    ///
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
    //    sortdangerpassplayoff(opponentAgentsToBeMarkPossition);
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
    //////////////// Draw Possition of Mark Agents //////////////////////////
    for (i = 0 ; i < markPoses.size() ; i++) {
        DBUG(QString("x : %1").arg(markPoses.at(i).x) , D_AHZ);
        drawer->draw(markPoses.at(i) , "white"); //Review
        drawer->draw(markRoles.at(i) , markPoses.at(i) - Vector2D(0, 0.4) , "white");
    }
}

void DefensePlan::manToManMarkBlockShotInPlayOff(int _markAgentSize) {
    //// This function blocks the lines that are between center of our goal &&
    //// opponent agents by a variable ratio along these lines.
    //// This is one of the mark plan for defending more flexible.

    bool playOn = gameState->isStart();
    bool playOff = gameState->theirDirectKick() || gameState->theirIndirectKick();
    int count;
    oppmarkedpos.clear();
    markPoses.clear();
    markRoles.clear();
    markAngs.clear();
    /////////////////// Intelligent mark plan ///////////////////////////////
    PDEBUGV2D("ALi's prediction mode",getMarkPlayoffPredictWaitPos(),D_ALI);
    //////////////////////////////////////////////////////
    if(_markAgentSize == oppAgentsToMarkPos.count()){
        for (int i = 0; i < oppAgentsToMarkPos.count(); i++) {
            markRoles.append(QString("shotBlocker"));
            if (!isInTheIndirectAreaShoot(oppAgentsToMarkPos[i])) {
                markPoses.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).first());
                markAngs.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).last());
            } else {
                markPoses.append(indirectAvoidShoot(oppAgentsToMarkPos[i]).first());
                markAngs.append(indirectAvoidShoot(oppAgentsToMarkPos[i]).last());
            }
        }
    }
    else if(_markAgentSize > oppAgentsToMarkPos.count()){
        for(int i = 0; i < oppAgentsToMarkPos.count(); i++){
            markRoles.append(QString("shotBlocker"));
            if(!isInTheIndirectAreaShoot(oppAgentsToMarkPos[i])){
                markPoses.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).first());
                markAngs.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).last());
            }
            else {
                markPoses.append(indirectAvoidShoot(oppAgentsToMarkPos[i]).first());
                markAngs.append(indirectAvoidShoot(oppAgentsToMarkPos[i]).last());
            }
        }
        QList<QPair<Vector2D, double> > tempsorted = sortdangerpassplayoff(oppAgentsToMarkPos);
        for(int i = 0; i < min(_markAgentSize - oppAgentsToMarkPos.count(), oppAgentsToMarkPos.count()); i++) {
            markRoles.append(QString("passBlocker"));
            if (!isInTheIndirectAreaPass(tempsorted[i].first)) {
                markPoses.append(PassBlockRatio(segmentperpass, tempsorted[i].first).first());
                markAngs.append(PassBlockRatio(segmentperpass, tempsorted[i].first).last());
            } else {
                markPoses.append(indirectAvoidPass(tempsorted[i].first).first());
                markAngs.append(indirectAvoidPass(tempsorted[i].first).last());
            }
        }
        //extra robot we have
        if (_markAgentSize > markPoses.count()) {
            count = 0;
            for (int i = markPoses.count() ; i < _markAgentSize; i++) {
                if (markPoses.count() < _markAgentSize) {
                    markRoles.append(QString("shotBlocker"));
                    markPoses.append(Vector2D(-1,  count *  4.5 / 6  * pow(-1, count)));
                    markAngs.append(Vector2D(1, 0));
                }
                count++;
            }
        }
    }
    else if(_markAgentSize < oppAgentsToMarkPos.count()){
        if (playOff || know->variables["transientFlag"].toBool() || gameState->isStop()) {
            QList<QPair<Vector2D, double> > tempsorted = sortdangerpassplayoff(oppAgentsToMarkPos);
            for (int i = 0; i < _markAgentSize; i++) {
                markRoles.append(QString("shotBlocker"));
                if (!isInTheIndirectAreaShoot(tempsorted[i].first)) {
                    markPoses.append(ShootBlockRatio(segmentpershoot, tempsorted[i].first).first());
                    markAngs.append(ShootBlockRatio(segmentpershoot, tempsorted[i].first).last());
                } else {
                    markPoses.append(indirectAvoidShoot(tempsorted[i].first).first());
                    markAngs.append(indirectAvoidShoot(tempsorted[i].first).last());
                }
            }
        } else if (playOn) {
            QList<QPair<Vector2D, double> > tempsorted = sortdangerpassplayon(oppAgentsToMarkPos);
            for (int i = 0; i < _markAgentSize; i++) {
                markRoles.append(QString("shotBlocker"));
                if (!isInTheIndirectAreaShoot(tempsorted[i].first)) {
                    markPoses.append(ShootBlockRatio(segmentpershoot, tempsorted[i].first).first());
                    markAngs.append(ShootBlockRatio(segmentpershoot, tempsorted[i].first).last());
                } else {
                    markPoses.append(indirectAvoidShoot(tempsorted[i].first).first());
                    markAngs.append(indirectAvoidShoot(tempsorted[i].first).last());
                }
            }
        }
    }
    for(int i = 0 ; i < markPoses.size() ; i++){
        drawer->draw(markRoles.at(i) , markPoses.at(i) - Vector2D(0, 0.4) , "white");
    }
}

void DefensePlan::setGoalKeeperState(){
    //// In this function,we determine the specific states that goalkeeper must
    //// have a logical behavior by good conditions.In other word we have some
    //// exceptions mode for goalkeeper.These modes are :
    //// 1- ball is behind the goalkeeper,
    //// 2- One touch mode(goalkeeper diving mode) ,
    //// 3- Throwing out the ball from penalty area (Clear mode) ,
    //// 4- Ball is out of the field.

    ////////////////// Variables of this function //////////////////////////////
    dangerForGoalKeeperClear = false;
    dangerForInsideOfThePenaltyArea = false;
    dangerForGoalKeeperClearByOurAgents = false;
    dangerForGoalKeeperClearByOppAgents = false;
    isCrowdedInFrontOfPenaltyAreaByOppAgents = false;
    Rect2D ourLeftPole(wm->field->ourGoalL() + Vector2D(0.3 , 0.3) , wm->field->ourGoalL() - Vector2D(0 , 0.3));
    Rect2D ourRightPole(wm->field->ourGoalR() + Vector2D(0.3 , 0.3) , wm->field->ourGoalR() - Vector2D(0 , 0.3));
    isCrowdedInFrontOfPenaltyAreaByOurAgents = false;
    playOnMode = gameState->isStart();
    ////////////////////////////////////////////////////////////////////////////
    Circle2D dangerCircle;
    Circle2D dangerCircle1;
    Vector2D sol[2];
    if (goalKeeperAgent != nullptr) {
        if (wm->field->isInField(wm->ball->pos)){
            ballIsOutOfField = false;
            QList<Vector2D> solutions;
            Segment2D ballLine(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm() * 10);
            Segment2D goalLine(wm->field->ourGoal() + Vector2D(0 , 1) , wm->field->ourGoal() - Vector2D(0 , 1));
            QList<Circle2D> defs;
            double AZBisecOpenAngle = 0, AZBigestOpenAngle = 0, AZDangerPercent = 0;
            for (int g = 0; g < defenseAgents.count(); g++) {
                defs.append(Circle2D(defenseAgents[g]->pos(), Robot::robot_radius_new));
            }
            know->getEmptyAngle(wm->ball->pos, wm->field->ourGoalL(), wm->field->ourGoalR(), defs, AZDangerPercent, AZBisecOpenAngle, AZBigestOpenAngle, false);
            /////////////////////// Added danger mode for not switching between "ball behindGoalie && danger mode /////////////
            wm->field->ourBigPenaltyArea(1,0,0).intersection(Segment2D(wm->ball->pos , wm->field->ourGoal()) , &sol[0] , &sol[1]);
            if(sol[0].isValid()){
                solutions.append(sol[0]);
            }
            if(sol[1].isValid()){
                solutions.append(sol[1]);
            }
            if (solutions.size()){
                if (solutions.size() == 1) {
                    if (wm->field->isInField(solutions.at(0))) {
                        dangerCircle = Circle2D(solutions.at(0), 0.40);
                        dangerCircle1 = Circle2D(solutions.at(0), 0.40);
                    }
                }
                else if (solutions.size() == 2) {
                    dangerCircle = Circle2D(solutions.at(0).dist(wm->ball->pos) < solutions.at(1).dist(wm->ball->pos) ? solutions.at(0) : solutions.at(1), 0.40);
                    dangerCircle1 = Circle2D(solutions.at(0).dist(wm->ball->pos) < solutions.at(1).dist(wm->ball->pos) ? solutions.at(0) : solutions.at(1), 0.40);
                }
                if(wm->our.activeAgentsCount() > 0 || wm->opp.activeAgentsCount() > 0){
                    for (int i = 0; i < wm->our.activeAgentsCount() ; i++) {
                        if (wm->our.active(i)->id != goalKeeperAgent->id()) {
                            if (dangerCircle.contains(wm->our.active(i)->pos)) {
                                isCrowdedInFrontOfPenaltyAreaByOurAgents = true;
                            }
                        }
                    }
                    for (int i = 0 ; i < wm->opp.activeAgentsCount() ; i++) {
                        if (dangerCircle.contains(wm->opp.active(i)->pos)) {
                            isCrowdedInFrontOfPenaltyAreaByOppAgents = true;
                        }
                    }
                }
                if (isCrowdedInFrontOfPenaltyAreaByOurAgents) {
                    DBUG("Crowded" , D_AHZ);
                    if (dangerCircle.contains(wm->ball->pos)) {
                        dangerForGoalKeeperClearByOurAgents = true;
                    }
                }
                if (isCrowdedInFrontOfPenaltyAreaByOppAgents) {
                    if (dangerCircle1.contains(wm->ball->pos)) {
                        dangerForGoalKeeperClearByOppAgents = true;
                    }
                }
                if (dangerForGoalKeeperClearByOurAgents || dangerForGoalKeeperClearByOppAgents) {
                    dangerForGoalKeeperClear = true;
                }
            }
            ///////////////////////////////////////////////////////////////////////
            if (playOnMode && !dangerForGoalKeeperClear) {
                if ((wm->ball->vel.length() > 1.3) && (goalLine.intersection(ballLine).valid() || oneTouchCnt < 5)) {
                    ballIsBesidePoles = false;
                    goalKeeperOneTouch = true;
                    goalKeeperClearMode = false;
                    ballIsOutOfField = false;
                    if (!goalLine.intersection(ballLine).valid()) {
                        oneTouchCnt++;
                        return;
                    }
                    oneTouchCnt = 0;
                    return;
                }
                else if (wm->field->isInOurPenaltyArea(wm->ball->pos)){
                    if(wm->ball->vel.length() < 0.1 && (ourLeftPole.contains(wm->ball->pos) || ourRightPole.contains(wm->ball->pos) ||
                                                        (wm->field->ourGoalL().y >= wm->ball->pos.y
                                                         && wm->field->ourGoalR().y < wm->ball->pos.y
                                                         && wm->field->ourGoal().x < wm->ball->pos.x
                                                         && wm->field->ourGoal().x + 0.05 > wm->ball->pos.x))){
                        ballIsBesidePoles = true;
                        goalKeeperOneTouch = false;
                        goalKeeperClearMode = false;
                        ballIsOutOfField = false;
                        return;
                    }
                    else {
                        ballIsBesidePoles = false;
                        goalKeeperOneTouch = false;
                        goalKeeperClearMode = true;
                        ballIsOutOfField = false;
                        return;
                    }
                }
                else {
                    goalKeeperClearMode = false;
                    goalKeeperOneTouch = false;
                    ballIsBesidePoles = false;
                    ballIsOutOfField = false;
                    return;
                }
            }
        }
        else {
            ballIsBesidePoles = false;
            goalKeeperOneTouch = false;
            goalKeeperClearMode = false;
            ballIsOutOfField = true;
            return;
        }
    }
    else {
        drawer->draw("GoalKeeper is gone !!!!" , Vector2D(0, 0) , "red");
    }
}

void DefensePlan::setGoalKeeperTargetPoint() {
    //// This function determine the target point that goalkeeper must go to it.
    //// For producing the target point, we certainly consider the states that
    //// is result from the "setGoalKeeperState" function.  :)

    ////////////////////////// Variables of this function //////////////////////
    Vector2D ballPos;
    Vector2D ballVel;
    Vector2D predictedBall;
    Vector2D oppPasser;
    Vector2D upBallRectanglePoint;
    Vector2D downBallRectanglePoint;
    Circle2D dangerCircle;
    Circle2D dangerCircle1;
    Vector2D goalKeeperTargetOffSet = Vector2D(0.2 , 0.0);
    QList<Vector2D> tempSol;
    QList<Vector2D> ballRectanglePoints;
    QList<Vector2D> solutions;
    Vector2D sol[2];
    dangerForGoalKeeperClear = false;
    dangerForInsideOfThePenaltyArea = false;
    dangerForGoalKeeperClearByOurAgents = false;
    dangerForGoalKeeperClearByOppAgents = false;
    isCrowdedInFrontOfPenaltyAreaByOppAgents = false;
    isCrowdedInFrontOfPenaltyAreaByOurAgents = false;
    playOffMode = gameState->theirDirectKick() || gameState->theirIndirectKick();
    playOnMode = gameState->isStart();
    stopMode = gameState->isStop();
    tempSol.clear();
    ballRectanglePoints.clear();
    ///////////////////////////////////////////////////////////////////////////
    if (goalKeeperAgent != nullptr) {
        ballPos = wm->ball->pos;
        ballVel = wm->ball->vel;
        predictedBall = ballPos + ballVel;
        wm->field->ourBigPenaltyArea(1,0,0).intersection(Segment2D(wm->ball->pos , wm->field->ourGoal()) , &sol[0] , &sol[1]);
        if(sol[0].isValid()){
            solutions.append(sol[0]);
        }
        if(sol[1].isValid()){
            solutions.append(sol[1]);
        }
        if (solutions.size()) {
            if (solutions.size() == 1){
                if (wm->field->isInField(solutions.at(0))) {
                    dangerCircle = Circle2D(solutions.at(0), 0.40);
                    dangerCircle1 = Circle2D(solutions.at(0), 0.40);
                }
            } else if (solutions.size() == 2) {
                dangerCircle = Circle2D(solutions.at(0).dist(wm->ball->pos) < solutions.at(1).dist(wm->ball->pos) ? solutions.at(0) : solutions.at(1), 0.40);
                dangerCircle1 = Circle2D(solutions.at(0).dist(wm->ball->pos) < solutions.at(1).dist(wm->ball->pos) ? solutions.at(0) : solutions.at(1), 0.40);
            }
        }
        ROS_INFO_STREAM("target");
        drawer->draw(dangerCircle , "yellow");
        drawer->draw(dangerCircle1 , "yellow");
        if (ballIsOutOfField || stopMode) {
            ROS_INFO_STREAM("1");
            lastStateForGoalKeeper = QString("noBesidePoleMode");
            dangerForGoalKeeperClear = false;
            drawer->draw(QString("Ball Is Out Of Field"), Vector2D(0, 1), "red");
            goalKeeperTarget = wm->field->ourGoal() + goalKeeperTargetOffSet;
            return;
        }
        else if(playOffMode){
            ROS_INFO_STREAM("2");
            lastStateForGoalKeeper = QString("noBesidePoleMode");
            dangerForGoalKeeperClear = false;
            goalKeeperPredictionModeInPlayOff = true;
            DBUG(QString("Their Indirect") , D_AHZ);
            oppPasser = wm->opp[know->nearestOppToBall()]->pos; //todo: move to wm
            if(gameState->theirIndirectKick()){
                goalKeeperTarget = wm->field->ourGoal() + goalKeeperTargetOffSet;
            }
            else {
                goalKeeperTarget = strictFollowBall(wm->ball->pos);
            }
            return;
        }
        else if (know->variables["transientFlag"].toBool()) {
            ROS_INFO_STREAM("3");
            lastStateForGoalKeeper = QString("noBesidePoleMode");
            dangerForGoalKeeperClear = false;
            DBUG(QString("TS Mode") , D_AHZ);
            goalKeeperPredictionModeInPlayOff = false;
            goalKeeperTarget = know->getPointInDirection(wm->field->ourGoal() , ballPrediction(true) , 0.5);
            if (!wm->field->isInOurPenaltyArea(goalKeeperTarget)) {
                wm->field->ourBigPenaltyArea(1,0,0).intersection(Segment2D(goalKeeperTarget , wm->field->ourGoal()) , &sol[0] , &sol[1]);
                if(sol[0].isValid()){
                    tempSol.append(sol[0]);
                }
                if(sol[1].isValid()){
                    tempSol.append(sol[1]);
                }
                if (tempSol.size() == 1) {
                    goalKeeperTarget = tempSol.at(0);
                } else if (tempSol.size() == 2) {
                    goalKeeperTarget = tempSol.at(0).dist(wm->ball->pos) < tempSol.at(1).dist(wm->ball->pos) ? tempSol.at(0) : tempSol.at(1);
                }
            }
            return;
        }
        else if (goalKeeperOneTouch) {
            ROS_INFO_STREAM("4");
            lastStateForGoalKeeper = QString("noBesidePoleMode");
            Segment2D ballLine(ballPos, ballPos + ballVel.norm() * 50);
            //if(wm->field->isInOurPenaltyArea(ballPos + ballVel))
            //    goalKeeperTarget = ballLine.nearestPoint(goalKeeperAgent->pos());
            //if(wm->field->isInOurPenaltyArea(ballLine.intersection(Segment2D(wm->field->ourGoalR() + Vector2D(0.1 , 0) , wm->field->ourGoalL() + Vector2D(0.1 , 0))))) {//time ham bayad check she hatma !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            /*if(goalKeeperTarget.dist(goalKeeperAgent->pos()) < 0.2)
                goalKeeperTarget = ballLine.nearestPoint(goalKeeperAgent->pos());
            else*/
                goalKeeperTarget = ballLine.intersection(Segment2D(wm->field->ourGoalR() + Vector2D(0.1 , 0), wm->field->ourGoalL() + Vector2D(0.1 , 0)));
            //Segment2D segment= Segment2D(ballLine.intersection(Segment2D(wm->field->ourGoalR(), wm->field->ourGoalL())), ballPos);
            //else{
            //    goalKeeperTarget = ballPos;
            //}
            //drawer->draw(ballPos , QColor(Qt::yellow));
            drawer->draw(ballLine.intersection(Segment2D(wm->field->ourGoalR() + Vector2D(0.1 , 0), wm->field->ourGoalL() + Vector2D(0.1 , 0))) , QColor(Qt::yellow));
            //drawer->draw(ballVel , QColor(Qt::yellow));
            drawer->draw(goalKeeperTarget , QColor(Qt::cyan));
            DBUG(QString("OneTouch To Side Point"), D_AHZ);
            return;
        }
        else if (goalKeeperClearMode) {
            ROS_INFO_STREAM("5");
            lastStateForGoalKeeper = QString("noBesidePoleMode");
            ////////////// Danger Mode for inside of the penalty area///////////
            if (wm->our.activeAgentsCount() > 0 || wm->opp.activeAgentsCount() > 0) {
                for (int i = 0; i < wm->our.activeAgentsCount() ; i++) {
                    if (wm->our.active(i)->id != goalKeeperAgent->id()) {
                        if (dangerCircle.contains(wm->our.active(i)->pos)) {
                            isCrowdedInFrontOfPenaltyAreaByOurAgents = true;
                        }
                    }
                }
                for (int i = 0 ; i < wm->opp.activeAgentsCount() ; i++) {
                    if (dangerCircle.contains(wm->opp.active(i)->pos)) {
                        isCrowdedInFrontOfPenaltyAreaByOppAgents = true;
                    }
                }
            }
            if (isCrowdedInFrontOfPenaltyAreaByOurAgents) {
                DBUG("Crowded" , D_AHZ);
                if (dangerCircle1.contains(wm->ball->pos)) {
                    dangerForGoalKeeperClearByOurAgents = true;
                }
            }
            if (isCrowdedInFrontOfPenaltyAreaByOppAgents) {
                if (dangerCircle1.contains(wm->ball->pos)) {
                    dangerForGoalKeeperClearByOppAgents = true;
                }
            }
            if (dangerForGoalKeeperClearByOurAgents || dangerForGoalKeeperClearByOppAgents) {
                dangerForGoalKeeperClear = true;
            }
            if (dangerForGoalKeeperClear) {
                dangerForInsideOfThePenaltyArea = true;
                DBUG(QString("inside : %1").arg(dangerForInsideOfThePenaltyArea) , D_AHZ);
                if (dangerForGoalKeeperClearByOppAgents) {
                    //                     goalKeeperTarget =  know->getPointInDirection(wm->ball->pos , wm->field->ourGoal() ,0.2);
                }
                else if (dangerForGoalKeeperClearByOurAgents){
                    DBUG("danger" , D_AHZ);
                    //                    penaltyArea.intersection(Line2D(wm->ball->pos , wm->field->ourGoal()),&Solutions[0] , &Solutions[1]);
                    //                    goalieTarget = Solutions[0].dist(wm->ball->pos) < Solutions[1].dist(wm->ball->pos) ? Solutions[0] : Solutions[1];
                    //                    dangerIntersection.append(wm->field->ourPAreaIntersect(Line2D(Vector2D(wm->ball->pos.x , -3) , Vector2D(wm->ball->pos.x , 3))));
                    //                    if(!dangerIntersection.isEmpty()){
                    //                        if(dangerIntersection.size() == 2){
                    //                            goalieTarget = dangerIntersection.at(0).isValid()
                    //                                    && (dangerIntersection.at(0).dist(wm->ball->pos) < dangerIntersection.at(1).dist(wm->ball->pos)) ? dangerIntersection.at(0) : dangerIntersection.at(1);
                    //                            goalieDirection = wm->ball->pos - goalieTarget;
                    //                            goalieTarget =  know->getPointInDirection(wm->ball->pos , wm->field->ourGoal() ,0.2);

                    //                        }
                    //                        else if(dangerIntersection.size() == 1){
                    //                            goalieTarget = dangerIntersection.at(0);
                    //                        }
                    //                    }
                }
            }
            /////////////// End of Danger Mode ///////////////////////////////
            else {
                drawer->draw(QString("Clear"), Vector2D(0, 1), "red");
            }
            return;
        }
        else if (ballIsBesidePoles) {
            Rect2D ballRectangle(wm->ball->pos + Vector2D(0.25 , 0.25) , wm->ball->pos + Vector2D(-0.25 , -0.25));
            drawer->draw(ballRectangle);
            if (wm->field->isInField(ballRectangle.topLeft())) {
                ballRectanglePoints.append(ballRectangle.topLeft());
            }
            if (wm->field->isInField(ballRectangle.topRight())) {
                ballRectanglePoints.append(ballRectangle.topRight());
            }
            if (wm->field->isInField(ballRectangle.bottomLeft())) {
                ballRectanglePoints.append(ballRectangle.bottomLeft());
            }
            if (wm->field->isInField(ballRectangle.bottomRight())) {
                ballRectanglePoints.append(ballRectangle.bottomRight());
            }
            if (lastStateForGoalKeeper == QString("noBesidePoleMode")) {
                ROS_INFO_STREAM("6");
                drawer->draw("injas" , Vector2D(-1,0) , 60);
                isPermissionToKick = false;
                goalKeeperTarget = ballRectanglePoints.at(0).dist(goalKeeperAgent->pos()) < ballRectanglePoints.at(1).dist(goalKeeperAgent->pos()) ? ballRectanglePoints.at(0) : ballRectanglePoints.at(1);
            }
            if(wm->field->ourGoalL().y >= wm->ball->pos.y && wm->field->ourGoalR().y < wm->ball->pos.y && wm->field->ourGoal().x < wm->ball->pos.x && wm->field->ourGoal().x + 0.05 > wm->ball->pos.x){
                goalKeeperTarget = Vector2D(wm->field->ourGoal().x + 0.6 , 0);
                ROS_INFO_STREAM("7");
                return;
            }
            if((fabs(wm->ball->pos.x - wm->field->ourGoalR().x) < 0.1 && fabs(wm->ball->pos.y - wm->field->ourGoalR().y) < 0.1) || (fabs(wm->ball->pos.x - wm->field->ourGoalL().x) < 0.1 && fabs(wm->ball->pos.y - wm->field->ourGoalL().y) < 0.1)){
                return;
            }
            if (goalKeeperAgent->pos().dist(goalKeeperTarget) < 0.05 && f == 0) {
                drawer->draw(Circle2D(goalKeeperTarget , 0.2) , QColor(Qt::red));
                f = 1;
                ROS_INFO_STREAM("8");
                if (wm->ball->pos.y > 0) {
                    if (ballRectanglePoints.at(0).y < ballRectanglePoints.at(1).y) {
                        downBallRectanglePoint = ballRectanglePoints.at(0);
                        upBallRectanglePoint = ballRectanglePoints.at(1);
                    }
                    else {
                        downBallRectanglePoint = ballRectanglePoints.at(1);
                        upBallRectanglePoint = ballRectanglePoints.at(0);
                    }
                    if(fabs(wm->ball->pos.y - wm->field->ourGoalL().y) < 0.1) {
                        goalKeeperTarget = downBallRectanglePoint - Vector2D(0.25, 0);
                    }
                    else if(wm->ball->pos.y < wm->field->ourGoalL().y)
                        goalKeeperTarget = downBallRectanglePoint;
                    else
                        goalKeeperTarget = upBallRectanglePoint;
                }
                else {
                    if (ballRectanglePoints.at(0).y < ballRectanglePoints.at(1).y) {
                        upBallRectanglePoint = ballRectanglePoints.at(1);
                        downBallRectanglePoint = ballRectanglePoints.at(0);
                    } else {
                        upBallRectanglePoint = ballRectanglePoints.at(0);
                        downBallRectanglePoint = ballRectanglePoints.at(1);
                    }
                    if (fabs(wm->ball->pos.y - wm->field->ourGoalR().y) < 0.1){
                        goalKeeperTarget = upBallRectanglePoint;
                    }
                    else if (wm->ball->pos.y < wm->field->ourGoalR().y)
                        goalKeeperTarget = downBallRectanglePoint;
                    else
                        goalKeeperTarget = upBallRectanglePoint;
                }
            }
            else if (goalKeeperAgent->pos().dist(goalKeeperTarget) < 0.05 && f == 1) {
                f= 2;
                if (wm->ball->pos.y > 0) {
                    if (ballRectanglePoints.at(0).y < ballRectanglePoints.at(1).y) {
                        downBallRectanglePoint = ballRectanglePoints.at(0);
                        upBallRectanglePoint = ballRectanglePoints.at(1);
                    }
                    else {
                        downBallRectanglePoint = ballRectanglePoints.at(1);
                        upBallRectanglePoint = ballRectanglePoints.at(0);
                    }
                    if(fabs(wm->ball->pos.y - wm->field->ourGoalL().y) < 0.1)
                        goalKeeperTarget = downBallRectanglePoint - Vector2D(0.25, 0);
                    else if(wm->ball->pos.y < wm->field->ourGoalL().y )
                        goalKeeperTarget = downBallRectanglePoint - Vector2D(0.4 , 0);
                    else
                        goalKeeperTarget = upBallRectanglePoint - Vector2D(0.4 , 0);
                }
                else {
                    if (ballRectanglePoints.at(0).y < ballRectanglePoints.at(1).y) {
                        upBallRectanglePoint = ballRectanglePoints.at(1);
                        downBallRectanglePoint = ballRectanglePoints.at(0);
                    }
                    else {
                        upBallRectanglePoint = ballRectanglePoints.at(0);
                        downBallRectanglePoint = ballRectanglePoints.at(1);
                    }
                    if(fabs(wm->ball->pos.y - wm->field->ourGoalR().y) < 0.1)
                        goalKeeperTarget = upBallRectanglePoint - Vector2D(0.25 ,0);
                    else if(wm->ball->pos.y < wm->field->ourGoalR().y)
                        goalKeeperTarget = downBallRectanglePoint - Vector2D(0.4 , 0);
                    else
                        goalKeeperTarget = upBallRectanglePoint - Vector2D(0.4 , 0);
                }
            }
            else if (goalKeeperAgent->pos().dist(goalKeeperTarget) < 0.05 && f == 2) {
                f = 3;
                if (wm->ball->pos.y > 0) {
                    if (ballRectanglePoints.at(0).y < ballRectanglePoints.at(1).y) {
                        downBallRectanglePoint = ballRectanglePoints.at(0);
                        upBallRectanglePoint = ballRectanglePoints.at(1);
                    }
                    else {
                        downBallRectanglePoint = ballRectanglePoints.at(1);
                        upBallRectanglePoint = ballRectanglePoints.at(0);
                    }
                    if(fabs(wm->ball->pos.y - wm->field->ourGoalL().y) < 0.1)
                        goalKeeperTarget = downBallRectanglePoint - Vector2D(0.25, 0);
                    else if(wm->ball->pos.y < wm->field->ourGoalL().y )
                        goalKeeperTarget = Vector2D(downBallRectanglePoint.x , 0) - Vector2D(0.4 , 0) + Vector2D(0 , wm->ball->pos.y);
                    else
                        goalKeeperTarget = Vector2D(upBallRectanglePoint.x , 0) - Vector2D(0.4 , 0) + Vector2D(0 , wm->ball->pos.y);
                }
                else {
                    if (ballRectanglePoints.at(0).y < ballRectanglePoints.at(1).y) {
                        upBallRectanglePoint = ballRectanglePoints.at(1);
                        downBallRectanglePoint = ballRectanglePoints.at(0);
                    }
                    else {
                        upBallRectanglePoint = ballRectanglePoints.at(0);
                        downBallRectanglePoint = ballRectanglePoints.at(1);
                    }
                    if(fabs(wm->ball->pos.y - wm->field->ourGoalR().y) < 0.1)
                        goalKeeperTarget = upBallRectanglePoint - Vector2D(0.25 ,0);
                    else if(wm->ball->pos.y < wm->field->ourGoalR().y)
                        goalKeeperTarget = Vector2D(downBallRectanglePoint.x , 0) - Vector2D(0.4 , 0) + Vector2D(0 , wm->ball->pos.y);
                    else
                        goalKeeperTarget = Vector2D(upBallRectanglePoint.x , 0) - Vector2D(0.4 , 0) + Vector2D(0 , wm->ball->pos.y);
                }
            }
            else if (goalKeeperAgent->pos().dist(goalKeeperTarget) < 0.05 && f >= 3) {
                f = 4;
            }
            lastStateForGoalKeeper = QString("ballIsBesidePoles");
            //            drawer->draw(upBallRectanglePoint , QColor(Qt::blue));
            //            drawer->draw(upBallRectanglePoint - Vector2D(0.5 , 0) , QColor(Qt::blue));
            //            drawer->draw(Vector2D(upBallRectanglePoint.x , 0) - Vector2D(0.5 , 0) + Vector2D(0 , wm->ball->pos.y) , QColor(Qt::blue));
            drawer->draw(goalKeeperTarget , "cyan");
            return;
        }
        else {
            lastStateForGoalKeeper = QString("noBesidePoleMode");
            ////////////// Danger Mode for out of the penalty area /////////////
            if (wm->our.activeAgentsCount() > 0 || wm->opp.activeAgentsCount() > 0) {
                for (int i = 0; i < wm->our.activeAgentsCount() ; i++) {
                    if (wm->our.active(i)->id != goalKeeperAgent->id()) {
                        if (dangerCircle.contains(wm->our.active(i)->pos)) {
                            isCrowdedInFrontOfPenaltyAreaByOurAgents = true;
                        }
                    }
                }
            }
            if (isCrowdedInFrontOfPenaltyAreaByOurAgents) {
                if (dangerCircle.contains(wm->ball->pos)) {
                    dangerForGoalKeeperClear = true;
                }
            }
            if (isCrowdedInFrontOfPenaltyAreaByOppAgents) {
                if (dangerCircle.contains(wm->ball->pos)) {
                    dangerForGoalKeeperClearByOppAgents = true;
                }
            }
            if (dangerForGoalKeeperClearByOurAgents || dangerForGoalKeeperClearByOppAgents) {
                dangerForGoalKeeperClear = true;
            }
            if (dangerForGoalKeeperClear) {
                DBUG("danger" , D_AHZ);
                ROS_INFO_STREAM("9");
                goalKeeperTarget = know->getPointInDirection(wm->ball->pos , wm->field->ourGoal() , 0.15);
                if (!wm->field->isInOurPenaltyArea(goalKeeperTarget)) {
                    solutions = wm->field->ourPAreaIntersect(Line2D(wm->ball->pos , wm->field->ourGoal()));
                    if (solutions.size()) {
                        if (solutions.size() == 1) {
                            goalKeeperTarget = solutions.at(0);
                        } else if (solutions.size() == 2) {
                            goalKeeperTarget = solutions.at(0).dist(wm->ball->pos) < solutions.at(1).dist(wm->ball->pos) ? solutions.at(0) : solutions.at(1);
                        }
                    }
                }
            }
            //////////////// End of Danger Mode ////////////////////////////////
            else {
                lastStateForGoalKeeper = QString("noBesidePoleMode");
                DBUG(QString("strict follow"), D_AHZ);
                predictedBall = ballPrediction(true);
                if (predictedBall.x - 0.02 < goalKeeperAgent->pos().x) {
                    Segment2D ball2PredictedBall(ballPos, predictedBall);
                    Line2D robotPrGoalLine(goalKeeperAgent->pos(), Vector2D(goalKeeperAgent->pos().x, (goalKeeperAgent->pos().y + 0.01)));
                    if (ball2PredictedBall.intersection(robotPrGoalLine).valid()) {
                        predictedBall = ball2PredictedBall.intersection(robotPrGoalLine);
                    }
                }
                ROS_INFO_STREAM("10");
                goalKeeperTarget = strictFollowBall(predictedBall);
                return;
            }
        }
    }
    drawer->draw(Circle2D(goalKeeperTarget , 0.2) , QColor(Qt::red));
}

void DefensePlan::assignSkill(Agent *_agent , Action *_skill) {
    //// For run any skills,for example: kick , we must initialize
    //// some main variables that are effective to run a skill.
    //// This function is called anywhere we need to run a skill.

    _agent->action = _skill;
}

void DefensePlan::initGoalKeeper(Agent *_goalieAgent) {
    //// This function determines the goalkeeper agent. Actually this function
    //// is used in Coach.cpp &&

    goalKeeperAgent = _goalieAgent;
    agents.clear();
    if (_goalieAgent) {
        agents.append(_goalieAgent);
    }
}

void DefensePlan::initDefense(QList <Agent*> _defenseAgents) {
    //// Just like the "initGoalKeeper" function , for inializing the defense
    //// agent , we have a function like this. :)

    defenseAgents.clear();
    defenseAgents.append(_defenseAgents);
    agents.append(_defenseAgents);
}

DefensePlan::DefensePlan(){
    //// Constructor function of DefensePlan class

    goalieThr = 0.0;

    thr = 0;
    isOnetouch = false;
    inPenaltyAreaFlag = false;
    noDefThr = 0;

    clearCnt = 0;

    defenseCount = defenseAgents.size();

    defExceptions.active = false;
    defExceptions.exeptionMode = NoneExep;
    defExceptions.exepAgentId = -1;

    defClearThr = 0;
    defClearFlag = false;

    overDefThr = 0;

    goalKeeperOneTouch = false;
    goalKeeperClearMode = false;
    ballIsOutOfField = false;

    ballIsBesidePoles = false;

    oneTouchCnt = 5;
    markRadius = 1.6;
    segmentpershoot = conf.ShootRatioBlock / 100.0;
    segmentperpass = conf.PassRatioBlock / 100.0;
    dir  = Vector2D(1, 0);
    MantoManAllTransientFlag =  conf.ManToManAllTransiant;
    predictThresh = 0;
    isInOneTouch = false;
    oneTouchCycleTest = 0;
    cycleCounter = 0;
    timeToReach = 0;
    doBlockPass = false;
    goalieAreaHis = 0;
    isBallGoingToOppAreaCnt = -1;
    pushBallHist = 0;
    failureAtempCnt = 0;
    goaliePassBlockCnt = -1;
    GOTThresh = 0.0;
    GOTCounter = 0;
    lastClearID = -1;
    lastTouchTheGoalie = -1;
    lastStateOffPlay = -1;
    oneToucher = 0;
    /////////// AHZ //////////////
    lastMarkRoles.append(markRoles);
    goalKeeperTarget = Vector2D(0, 0);
    dangerModeThresholdForClear = false;
    dangerModeThresholdForDanger = false;
    /////////////// For Adding TS Mode in Mark ///////////////////////////////
    xLimitForblockingPass = 0;
    manToManMarkBlockPassFlag = conf.PlayOffManToMan;
    if (manToManMarkBlockPassFlag || wm->ball->pos.x > xLimitForblockingPass) {
        know->variables["lastStateForMark"] = QString("BlockPass");
        know->variables["stateForMark"] = QString("BlockPass");
    } else {
        know->variables["lastStateForMark"] = QString("BlockShot");
        know->variables["stateForMark"] = QString("BlockShot");
    }
    ////////////////////////////////

    striker_Robot = new GotopointavoidAction;

    for (int i = 0; i < _MAX_NUM_PLAYERS; i++) {
        lastMarker[i] = -1;

        gpa[i] = new GotopointavoidAction;
        gpa[i]->setNoavoid(false);
        gpa[i]->setOnetouchmode(true);

        gps[i] = new GotopointAction;
        gps[i]->setSlowmode(false);
    }
    kickSkill = new KickAction;
    AHZSkills = nullptr;
    defenderForMark = false;
    doubleMarking = false;

}

void DefensePlan::matchingDefPos(int _defenseNum){
    //// This Function matches the points that is produced by other functions to
    //// our agents we have in defense plan. Then we run the "GotoPointAvoid"
    //// skill on the agents.

    QList <Agent*> ourAgents;
    QList <Vector2D> matchPoints;
    QList <Vector2D> ahzMatchPoints;
    QList <Vector2D> ahzMatchDirections;
    QList <Vector2D> stuckPositions;
    QList <int> stuckIndexs;
    QList <int> matchResult;
    Vector2D tempPoint;
    stopMode = gameState->isStop();
    ourAgents.clear();
    matchPoints.clear();
    ahzMatchDirections.clear();
    ahzMatchPoints.clear();
    ourAgents.append(defenseAgents);
    if (defExceptions.active) {
        if (defExceptions.exepAgentId != -1) {
            for (int i = 0; i < ourAgents.size(); i++) {
                if (i == defExceptions.exepAgentId) {
                    ourAgents.removeAt(i);
                }
            }
        }
    }
    ///////////////// Added By AHZ for segment (before MRL game) ///////////////
    if(stopMode){
        ourAgents.clear();
        ourAgents.append(defenseAgents);
    }
    //////////////////////////////////////////////////////////////////////
    for (int i = 0 ; i < AHZDefPoints.size() ; i++) {
        matchPoints.append(AHZDefPoints.at(i));
    }
    findOppAgentsToMark();
    findPos(decideNumOfMarks());
    matchPoints.append(markPoses);
    /////////////// Stucking agents ///////////////////////////////////////////
    for (int i = 0; i < 4; i++) {
        if (areAgentsStuckTogether(matchPoints)) {
            agentsStuckTogether(matchPoints, stuckPositions, stuckIndexs);
            correctingTheAgentsAreStuckTogether(matchPoints, stuckPositions, stuckIndexs);
        }
    }
    //////////////// Avoid Penalty Area ///////////////////////
    //////////////////// Added for RC 2017 /////////////////////////////////////
    Vector2D tempMatchPoints[matchPoints.size()];
    if (ourAgents.size() > matchPoints.size()) {
        for (int i = 0 ; i < ourAgents.size() - matchPoints.size() ; i++) {
            ourAgents.removeAt(i);
        }
    }
    else if (ourAgents.size() < matchPoints.size()) {
        for (int i = 0 ; i < matchPoints.size() ; i++) {
            tempMatchPoints[i] = matchPoints.at(i);
        }
        for (int i = 0 ; i < matchPoints.size() ; i++) {
            for (int j = 0 ; j < matchPoints.size() ; j++) {
                if (i != j) {
                    if (tempMatchPoints[i].x > tempMatchPoints[j].x) {
                        tempPoint = tempMatchPoints[i];
                        tempMatchPoints[i] = tempMatchPoints[j];
                        tempMatchPoints[j] = tempPoint;
                    }
                }
            }
        }
        for (int i = matchPoints.size() - ourAgents.size() - 1 ; i >= 0 ; i--) {
            matchPoints.removeOne(tempMatchPoints[i]);
        }
    }
    ////////////////////////////////////////////////////////////////////////////
    know->Matching(ourAgents, matchPoints, matchResult);
    for (int i = 0 ; i < defenseCount && i < matchPoints.size(); i++) {
        defensePoints[i] = matchPoints[i];
    }
    for (int i = 0 ; i < matchPoints.count() && i < matchResult.count() ; i++) {
        gpa[ourAgents[i]->id()]->clearOurrelax();
        gpa[ourAgents[i]->id()]->clearTheirrelax();

        for (int j = 0; j < ourAgents.size(); j++) {
            if (j != i) {
                gpa[ourAgents[i]->id()]->addOurrelax(ourAgents[j]->id());//TODO: gotopiontaction
            }
        }
        assignSkill(ourAgents[i] , gpa[ourAgents[i]->id()]);
        drawer->draw(Circle2D(matchPoints[matchResult[i]] , 0.05) , 0 , 360 , "black" , true);
//        if(wm->field->ourBigPenaltyArea(1,0.4,0).contains(matchPoints.at(matchResult.at(i)))){
            ahzMatchPoints.append(avoidRectangularPenaltyAreaByMhmmd(matchPoints.at(matchResult.at(i))
                                             , ourAgents.at(i)->pos()
                                             , ourAgents.at(i)->dir()
                                             , ourAgents.at(i)->vel()).first);
            ahzMatchDirections.append(avoidRectangularPenaltyAreaByMhmmd(matchPoints.at(matchResult.at(i))
                                             , ourAgents.at(i)->pos()
                                             , ourAgents.at(i)->dir()
                                             , ourAgents.at(i)->vel()).second);
//        }
//        else{
//            ahzMatchPoints.append(avoidCircularPenaltyAreaByArash(ourAgents.at(i) , matchPoints.at(matchResult.at(i))).first);
//            ahzMatchDirections.append(avoidCircularPenaltyAreaByArash(ourAgents.at(i) , matchPoints.at(matchResult.at(i))).second);
//        }
        if(gameState->theirIndirectKick()){
            gpa[ourAgents[i]->id()]->setNoavoid(true);
            gpa[ourAgents[i]->id()]->setSlowmode(false);
            gpa[ourAgents[i]->id()]->setAvoidpenaltyarea(false);
            gpa[ourAgents[i]->id()]->setBallobstacleradius(0.5);
        }
        else if(stopMode){
            gpa[ourAgents[i]->id()]->setNoavoid(true);
            gpa[ourAgents[i]->id()]->setSlowmode(true);
            gpa[ourAgents[i]->id()]->setDivemode(false);
            gpa[ourAgents[i]->id()]->setAvoidpenaltyarea(true);
            gpa[ourAgents[i]->id()]->setBallobstacleradius(0.5);
        }
        else{
            gpa[ourAgents[i]->id()]->setNoavoid(true);
            gpa[ourAgents[i]->id()]->setSlowmode(false);
            gpa[ourAgents[i]->id()]->setAvoidpenaltyarea(true);
            gpa[ourAgents[i]->id()]->setBallobstacleradius(0);
        }
        //////////// Go To Point Avoid for defense agents //////////////////
        if(i < _defenseNum){
            gpa[ourAgents[i]->id()]->setTargetpos(ahzMatchPoints.at(i)); //HINT : gpa->init
            gpa[ourAgents[i]->id()]->setTargetdir(ahzMatchDirections.at(i));
        }
        ///////// Go To Point Avoid for mark agents ////////////////////
        else{
            gpa[ourAgents[i]->id()]->setTargetpos(ahzMatchPoints.at(i)); //HINT : gpa->init
            gpa[ourAgents[i]->id()]->setTargetdir(ahzMatchDirections.at(i));

        }
    }
}

void DefensePlan::execute(){
    ///// All of the goalKeeper && defense functions are linked in this function.
    ///// First of all, we determine the behavior of goalKeeper.
    ///// (first in penalty mode then other mode)
    ///// Now,we identify the number of defense agents && then this number is
    ///// sent to "matchingDefPos()" function to match between the produced
    ///// points && our agents in defense plan.

    int realDefSize = 0;
    //    detectOpponentPassOwners(1 , 2);
    stopMode = gameState->isStop();
    suitableRadius = RADIUS_FOR_CRITICAL_DEFENSE_AREA;
    drawer->draw(Circle2D(wm->field->ourGoal() , suitableRadius) , 0 , 180 , "blue" , false);
    drawer->draw(getLinesOfBallTriangle().at(0));
    drawer->draw(getLinesOfBallTriangle().at(1));
    ballPosHistory.prepend(Vector2D(wm->ball->pos.x, wm->ball->pos.y));
    if (ballPosHistory.count() > 7){
        ballPosHistory.removeLast();
    }
    //////////////////////////////////////
    playOnMode = gameState->isStart();
    if(gameState->theirPenaltyKick() && !gameState->penaltyShootout()){
        if (goalKeeperAgent != nullptr) {
            drawer->draw(QString("Penalty") , Vector2D(1, 2) , "white");
//            penaltyMode();
        }
        else {
            drawer->draw(QString("No Goalie!") , Vector2D(1, 2) , "white");
        }
        return;
    }
    if(gameState->theirPenaltyKick()){
        //TO DO: add penalty goalie for penalty shootout
        penaltyShootOutMode();
        lastBallPosition = wm->ball->pos;
        return;
    }
    if(gameState->isStart() && gameState->penaltyShootout()) {
        penaltyShootOutMode();
    }
    else{
        if(goalKeeperAgent != nullptr){
            setGoalKeeperState();
            setGoalKeeperTargetPoint();
            executeGoalKeeper();
            assignSkill(goalKeeperAgent , AHZSkills);
        }
        if(!defenseAgents.empty()){
            if(wm->our.activeAgentsCount() <= _NUM_PLAYERS){
                if(playOnMode || stopMode){
                    checkDefenseExeptions();
                    if (defExceptions.active && !know->variables["transientFlag"].toBool()) {
                        runDefenseExeptions();
                        defenseCount = defenseAgents.size() - 1;
                    }
                    else {
                        defExceptions.exepAgentId = -1;
                        defExceptions.exeptionMode = NoneExep;
                        defenseCount = defenseAgents.size();
                        know->variables["defenseOneTouchMode"] = false;
                    }
                }
                else{
                    know->variables["defenseOneTouchMode"] = false;
                    defenseCount = defenseAgents.size();
                }
                if(defenseCount > 0){
                    realDefSize = defenseCount - decideNumOfMarks();
                    AHZDefPoints = defenseFormation(defenseFormationForCircularPositioning(defenseNumber() , realDefSize , RADIUS_FOR_CRITICAL_DEFENSE_AREA , 2.7),
                                                    defenseFormationForRectangularPositioning(defenseNumber() , realDefSize , 1.4 , 2.5));
                    matchingDefPos(realDefSize);
                }
            }
            else{
                drawer->draw("Vision Problem", Vector2D(0, 0), "red" , 20);
            }
        }
    }
}

bool DefensePlan::agentEffectOnBallProbability(Vector2D ballPos, Vector2D ballVel, Vector2D agentPos, Vector2D agentVel, bool isTowardOurgoal) {
    Vector2D goal;
    bool isInPenaltyRect;

    if (isTowardOurgoal) {
        goal = wm->field->ourGoal();
        isInPenaltyRect = wm->field->isInOurPenaltyArea(ballPos);
    } else {
        goal = wm->field->oppGoal();
        isInPenaltyRect = wm->field->isInOppPenaltyArea(ballPos);
    }


    if (ballVel.length() > 4 || ballPos.dist(goal) < 3) {
        agentEffectOnBallProbabilityRes = true;     // skyDive
    } else if (ballVel.length() < 2 || ballPos.dist(agentPos) > 1) {
        agentEffectOnBallProbabilityRes = false;    // ballBisector
    }

    return agentEffectOnBallProbabilityRes;
}

Vector2D DefensePlan::getGoalieShootOutTarget(bool isSkyDive) {
    Vector2D degree;
    Vector2D finalTarget;

    Line2D ballPath(wm->ball->pos , wm->ball->pos + (wm->ball->vel.norm() * 10));
    //    Line2D ballLine(lastBallPos.first(), lastBallPos.last());
    //    Line2D ballRay(wm->ball->pos, wm->ball->pos + wm->opp[knowledge->nearestOppToBall]->dir);
    Line2D oppAgentLine(wm->opp[know->nearestOppToBall()]->pos, wm->opp[know->nearestOppToBall()]->pos + wm->opp[know->nearestOppToBall()]->dir * 10);

    Vector2D a, b;
    Circle2D c1 = Circle2D(wm->field->ourGoal(), 1);

    if (!isSkyDive) {
        degree = (wm->field->ourGoalL() - (wm->ball->pos + 0.2 * wm->ball->vel)).norm()
                + (wm->field->ourGoalR() - (wm->ball->pos + 0.2 * wm->ball->vel)).norm();

        Line2D bisectorLine(wm->ball->pos + 0.2 * wm->ball->vel, wm->ball->pos + degree * 10);

        if (know->chipGoalPropability(false, goalKeeperAgent->pos()) > 0.1 || know->chipGoalPropability(false, goalKeeperAgent->pos()) < 0.05) {
            shootOutDiam = min(2 * wm->ball->pos.dist(wm->field->ourGoal()) / 5.0, 2);
        }

        DBUG(QString("ballBisector, diam:%1").arg(shootOutDiam), D_FATEME);
        Circle2D c = Circle2D(wm->field->ourGoal(), shootOutDiam);

        if (c.intersection(bisectorLine, &a, &b)) {
            finalTarget = (a.x > b.x) ? a : b;
        }

        drawer->draw(finalTarget, QColor(Qt::cyan));
    } else {
        if (wm->ball->vel.length() > 4
                || (wm->ball->pos.dist(wm->field->ourGoal()) < 2.7
                    && wm->ball->pos.dist(wm->opp[know->nearestOppToBall()]->pos) > 0.15)
                || wm->ball->pos.dist(wm->field->ourGoal()) < 1.4) {
            DBUG("skydive, ballpath", D_FATEME);
            finalTarget = ballPath.perpendicular(wm->our[goalKeeperAgent->id()]->pos).intersection(ballPath);
        } else {
            finalTarget = oppAgentLine.perpendicular(wm->our[goalKeeperAgent->id()]->pos).intersection(oppAgentLine);
            DBUG("skydive, oppAgentLine", D_FATEME);
        }

        if (!wm->field->isInOurPenaltyArea(finalTarget)) {
            c1.intersection(oppAgentLine, &a, &b);
            finalTarget = (a.x > b.x) ? a : b;
        }

        drawer->draw(finalTarget, "blue");
    }
    return finalTarget;
}

bool DefensePlan::canReachToBall(int ourAgentId, int theirAgentId) {
    if (wm->our[ourAgentId] == nullptr
            || wm->opp[theirAgentId] == nullptr) {
        return false;
    }
    Vector2D ballPosAndVel;
    ballPosAndVel = wm->ball->pos + wm->ball->vel;

    return wm->our[ourAgentId]->pos.dist(ballPosAndVel) < wm->opp[theirAgentId]->pos.dist(ballPosAndVel) - 0.3
            && wm->ball->pos.dist(wm->field->ourGoal()) > 2.5 && wm->ball->pos.dist(wm->opp[theirAgentId]->pos) > 1;
}

int DefensePlan::decideShootOutMode() {
    if (goalKeeperAgent == nullptr) {
        return 0;
    }
    int result;


    if (lastBallPosition.dist(wm->ball->pos) < 0.04) {
        DBUG("beforeTouch", D_FATEME);
        shootOutClearModeSelected = false;
        result = beforeTouch;
    } else if (canReachToBall(goalKeeperAgent->id(), know->nearestOppToBall())
               || (!Circle2D(wm->ball->pos, 0.10).contains(wm->opp[know->nearestOppToBall()]->pos)
                   && wm->ball->pos.dist(wm->field->ourGoal()) < 1.7)
               || shootOutClearModeSelected
               ) {
        DBUG("shootOutClear", D_FATEME);
        shootOutClearModeSelected = true;
        result = shootOutClear;
    } else if (!agentEffectOnBallProbability(wm->ball->pos, wm->ball->vel, wm->opp[know->nearestOppToBall()]->pos, wm->opp[know->nearestOppToBall()]->vel, true)) {
        DBUG("ballBisector", D_FATEME);
        result = ballBisector;
    } else {
        DBUG("skydive", D_FATEME);
        result = skyDive;
    }

    return result;
}

void DefensePlan::penaltyShootOutMode() {
    if (goalKeeperAgent == nullptr) {
        return;
    }
    penaltyShootoutMode = decideShootOutMode();

    Vector2D targetDir(10, 5), agentTarget;
    targetDir = wm->opp[know->nearestOppToBall()]->pos - wm->field->ourGoal();

    if (lastBallPos.count() < 15) {
        lastBallPos.append(wm->ball->pos);
    } else {
        lastBallPos.removeFirst();
    }


    switch (penaltyShootoutMode) {
    case beforeTouch:
        assignSkill(goalKeeperAgent , gpa[goalKeeperAgent->id()]);
        gpa[goalKeeperAgent->id()]->setSlowmode(false);
        gpa[goalKeeperAgent->id()]->setDivemode(true);
        gpa[goalKeeperAgent->id()]->setTargetpos(wm->field->ourGoal() + Vector2D(0.1, 0));
        gpa[goalKeeperAgent->id()]->setTargetdir(targetDir);

        break;

    case shootOutClear:
        assignSkill(goalKeeperAgent, kickSkill);
        kickSkill->setKickspeed(6.5);
        kickSkill->setTolerance(50);
        kickSkill->setDontkick(false);
        kickSkill->setSlow(false);
        kickSkill->setSpin(0);
        kickSkill->setAvoidpenaltyarea(false);
        kickSkill->setGoaliemode(false);
        kickSkill->setChip(true);
        kickSkill->setTarget(wm->field->oppGoal());
        kickSkill->setSagmode(true);

        break;

    case ballBisector:
        assignSkill(goalKeeperAgent , gpa[goalKeeperAgent->id()]);
        gpa[goalKeeperAgent->id()]->setSlowmode(false);
        gpa[goalKeeperAgent->id()]->setDivemode(false);
        gpa[goalKeeperAgent->id()]->setLookat(wm->ball->pos);
        agentTarget = getGoalieShootOutTarget(false);

        gpa[goalKeeperAgent->id()]->setTargetpos(agentTarget);
        gpa[goalKeeperAgent->id()]->setTargetdir(targetDir);

        drawer->draw(agentTarget, QColor(Qt::darkRed));
        break;

    case skyDive:
        assignSkill(goalKeeperAgent , gpa[goalKeeperAgent->id()]);
        gpa[goalKeeperAgent->id()]->setSlowmode(false);
        gpa[goalKeeperAgent->id()]->setDivemode(true);
        gpa[goalKeeperAgent->id()]->setLookat(wm->ball->pos);
        agentTarget = getGoalieShootOutTarget(true) ;

        gpa[goalKeeperAgent->id()]->setTargetpos(agentTarget);
        gpa[goalKeeperAgent->id()]->setTargetdir(targetDir);

        drawer->draw(agentTarget, QColor(Qt::darkBlue));
        break;
    default:
        break;
    }


}

void DefensePlan::penaltyMode() {
    //// By this function goalKeeper is able to move according to the direction
    //// of the opponent agents that will shot to our goal in pentalty mode.

    Vector2D ballPos = wm->ball->pos;
    const float goalLineExtra = 0.03;
    const double xDiff = 0.10;
    Line2D goalLine(wm->field->ourGoalL() + Vector2D(+xDiff, +goalLineExtra),
                    wm->field->ourGoalR() + Vector2D(+xDiff, -goalLineExtra));
    const double epsilon = 0.12;
    Vector2D target(-2.93, 0.0);

    Line2D ballRay(ballPos, ballPos + wm->opp[know->nearestOppToBall()]->dir);


    Vector2D intersectionPoint = goalLine.intersection(ballRay);
    double ang = ballRay.a() * goalLine.b() - ballRay.b() * goalLine.a();

    if (fabs(ang) > 0.01 && fabs(ang) < 0.95) {
        if (ang * intersectionPoint.y > 0) {
            intersectionPoint.y = wm->field->oppGoalR().y;
        } else if (ang * intersectionPoint.y < 0) {
            intersectionPoint.y = wm->field->oppGoalL().y;
        }
    }

    if (ang <= 0.95) {
        intersectionPoint.y *= -1;
    }

    intersectionPoint.y *= (7.0 / 10.0);

    //    if(fabs(knowledge->getAgent(goalKeeperAgent->id())->pos().y) > fabs(wm->field->ourGoalR().y))
    //        intersectionPoint.y += 1*knowledge->getAgent(goalKeeperAgent->id())->pos().dist(intersectionPoint)*knowledge->getAgent(goalKeeperAgent->id())->pos().dist(intersectionPoint)
    //                *(fabs((intersectionPoint-knowledge->getAgent(goalKeeperAgent->id())->pos()).y)/(intersectionPoint-knowledge->getAgent(goalKeeperAgent->id())->pos()).y);   // sign

    if (intersectionPoint.valid()) {
        target = intersectionPoint;
        drawer->draw(target, "red");
    } else {
        target.y = 0.0;
    }

    //    target.y = min(max(target.y, wm->field->ourGoalR().y + epsilon), wm->field->ourGoalL().y - epsilon + 0.03);
    Vector2D targetDir(10, 5);
    targetDir.setDir(AngleDeg(0));
    targetDir.setLength(1);

    drawer->draw(target, "blue");

    assignSkill(goalKeeperAgent , gpa[goalKeeperAgent->id()]);
    gpa[goalKeeperAgent->id()]->setSlowmode(false);
    gpa[goalKeeperAgent->id()]->setDivemode(true);

    gpa[goalKeeperAgent->id()]->setTargetpos(target); //HINT : gpa->init
    gpa[goalKeeperAgent->id()]->setTargetdir(targetDir);

    //gpa[goalKeeperAgent->id()]->init(target , targetDir);

}

Vector2D* DefensePlan::getIntersectWithDefenseArea(const Line2D& line, const Vector2D& blockPoint) {
    //// This function return the intersection a line with defense area (3 part).

    Vector2D* intersectionWithBottomCircle[2];
    Vector2D* intersectionWithTopCircle[2];
    for (int i = 0; i < 2; i++) {
        intersectionWithBottomCircle[i] = new Vector2D();
        intersectionWithTopCircle[i] = new Vector2D();
    }
    Vector2D intersectionWithDefenseLine = defenseAreaLine.intersection(line);
    int intersectionWithBottomCircleCount = defenseAreaBottomCircle.intersection(line, intersectionWithBottomCircle[0], intersectionWithBottomCircle[1]);
    int intersectionWithTopCircleCount = defenseAreaTopCircle.intersection(line, intersectionWithTopCircle[0], intersectionWithTopCircle[1]);
    std::vector<Vector2D*> points;
    for (int i = 0; i < intersectionWithBottomCircleCount; i++) {
        float angle = getDegree(defenseAreaBottomCircle.center() + Vector2D(1, 0), defenseAreaBottomCircle.center(), *intersectionWithBottomCircle[i]);
        if (angle <= 0 && angle >= -90) {
            points.push_back(intersectionWithBottomCircle[i]);
        } else {
            delete intersectionWithBottomCircle[i];
        }
    }
    for (int i = 0; i < intersectionWithTopCircleCount; i++) {
        float angle = getDegree(defenseAreaTopCircle.center() + Vector2D(1, 0), defenseAreaTopCircle.center(), *intersectionWithTopCircle[i]);
        if (angle >= 0 and angle <= 90) {
            points.push_back(intersectionWithTopCircle[i]);
        } else {
            delete intersectionWithTopCircle[i];
        }
    }
    double minDist = 99999;
    Vector2D* retPoint = nullptr;
    if (intersectionWithDefenseLine.valid()) {
        retPoint = new Vector2D(intersectionWithDefenseLine);
        minDist = retPoint->dist(blockPoint);
    }
    for (vector<Vector2D*>::const_iterator it = points.begin(); it != points.end(); it++) {
        double dist = (*it)->dist(blockPoint);
        if (dist < minDist) {
            retPoint = (*it);
            minDist = dist;
        }
    }
    for (auto &point : points) {
        if (point != retPoint) {
            delete point;
        }
    }
    return retPoint;
}

Vector2D* DefensePlan::getIntersectWithDefenseArea(const Segment2D& segment, const Vector2D& blockPoint) {
    //// This function gets us the intersection point between defense area && input
    //// segment. If defense area with input segment have two intersecton points,
    //// this function returns the point that is more near to the blockPoint.

    Vector2D* intersectionWithBottomCircle[2];
    Vector2D* intersectionWithTopCircle[2];
    for (int i = 0; i < 2; i++) {
        intersectionWithBottomCircle[i] = new Vector2D();
        intersectionWithTopCircle[i] = new Vector2D();
    }
    Vector2D intersectionWithDefenseLine = defenseAreaLine.intersection(segment);
    int intersectionWithBottomCircleCount = defenseAreaBottomCircle.intersection(segment, intersectionWithBottomCircle[0], intersectionWithBottomCircle[1]);
    int intersectionWithTopCircleCount = defenseAreaTopCircle.intersection(segment, intersectionWithTopCircle[0], intersectionWithTopCircle[1]);

    std::vector<Vector2D*> points;
    for (int i = 0; i < intersectionWithBottomCircleCount; i++) {
        float angle = getDegree(defenseAreaBottomCircle.center() + Vector2D(1, 0), defenseAreaBottomCircle.center(), *intersectionWithBottomCircle[i]);
        if (angle <= 0 and angle >= -90) {
            points.push_back(intersectionWithBottomCircle[i]);
        } else {
            delete intersectionWithBottomCircle[i];
        }
    }
    for (int i = 0; i < intersectionWithTopCircleCount; i++) {
        float angle = getDegree(defenseAreaTopCircle.center() + Vector2D(1, 0), defenseAreaTopCircle.center(), *intersectionWithTopCircle[i]);
        if (angle >= 0 and angle <= 90) {
            points.push_back(intersectionWithTopCircle[i]);
        } else {
            delete intersectionWithTopCircle[i];
        }
    }

    double minDist = 99999;
    Vector2D* retPoint = nullptr;
    if (intersectionWithDefenseLine.valid()) {
        retPoint = new Vector2D(intersectionWithDefenseLine);
        minDist = retPoint->dist(blockPoint);
    }
    for (vector<Vector2D*>::const_iterator it = points.begin(); it != points.end(); it++) {
        double dist = (*it)->dist(blockPoint);
        if (dist < minDist) {
            retPoint = (*it);
            minDist = dist;
        }
    }

    for (auto &point : points)
        if (point != retPoint) {
            delete point;
        }

    return retPoint;
}

void DefensePlan::executeGoalKeeper() {
    //// This Function execute the goalkeeper skills according to the
    //// target point that have been produced in the "setGoalKeeperTargetPoint"
    //// function. In this function also like the other functions for goalkeeper,
    //// we have some mode for handling the goalkeeper behavior.

    playOffMode = gameState->theirDirectKick()  || gameState->theirIndirectKick();
    playOnMode = gameState->isStart();
    stopMode = gameState->isStop();
    QList<Vector2D> tempSol;
    tempSol.clear();
    ROS_INFO_STREAM("_________________");
    ROS_INFO_STREAM(f);
    if (!goalKeeperOneTouch) {
        firstTimeGoalKeeperOneTouch = false;
    }
    if(!ballIsBesidePoles){
        FlagBesidePoles = false;
        f = 0;
        Rect2D ballRectangle(wm->ball->pos + Vector2D(0.25 , 0.25) , wm->ball->pos + Vector2D(-0.25 , -0.25));
        drawer->draw(ballRectangle);
        QList<Vector2D> ballRectanglePoints;
        if (wm->field->isInField(ballRectangle.topLeft())) {
            ballRectanglePoints.append(ballRectangle.topLeft());
        }
        if (wm->field->isInField(ballRectangle.topRight())) {
            ballRectanglePoints.append(ballRectangle.topRight());
        }
        if (wm->field->isInField(ballRectangle.bottomLeft())) {
            ballRectanglePoints.append(ballRectangle.bottomLeft());
        }
        if (wm->field->isInField(ballRectangle.bottomRight())) {
            ballRectanglePoints.append(ballRectangle.bottomRight());
        }
        //goalKeeperTarget = ballRectanglePoints.at(0).dist(goalKeeperAgent->pos()) < ballRectanglePoints.at(1).dist(goalKeeperAgent->pos()) ? ballRectanglePoints.at(0) : ballRectanglePoints.at(1);
    }
    if (!wm->field->isInOurPenaltyArea(goalKeeperTarget) && !goalKeeperOneTouch && !stopMode && !ballIsBesidePoles) {
        tempSol = wm->field->ourPAreaIntersect(Segment2D(wm->field->ourGoal() , goalKeeperTarget));
        if (tempSol.size()) {
            if (tempSol.size() == 1) {
                goalKeeperTarget = tempSol.at(0);
            } else if (tempSol.size() == 2) {
                goalKeeperTarget = tempSol.at(0).dist(wm->ball->pos) < tempSol.at(1).dist(wm->ball->pos) ? tempSol.at(0) : tempSol.at(1);
            }
        }
    }
    if(!wm->field->isInOurPenaltyArea(goalKeeperAgent->pos()) && !goalKeeperOneTouch && !stopMode && !ballIsBesidePoles) {
        tempSol.append(wm->field->ourPAreaIntersect(Segment2D(wm->field->ourGoal() , goalKeeperAgent->pos())));
        if (tempSol.size() == 1) {
            goalKeeperTarget = tempSol.at(0);
        } else if (tempSol.size() == 2) {
            goalKeeperTarget = tempSol.at(0).dist(wm->ball->pos) < tempSol.at(1).dist(wm->ball->pos) ? tempSol.at(0) : tempSol.at(1);
        }
    }
    if (goalKeeperAgent != nullptr) {
        ROS_INFO_STREAM("1");
        DBUG(QString("goalKeeper clear mode : %1").arg(know->variables["goalKeeperClearMode"].toBool()) , D_AHZ);
        DBUG(QString("goalKeeper oneTouch mode : %1").arg(know->variables["goalKeeperOneTouchMode"].toBool()) , D_AHZ);
        if (playOffMode) {
            ROS_INFO_STREAM("2");
            know->variables["goalKeeperClearMode"] = false;
            know->variables["goalKeeperOneTouchMode"] = false;
            AHZSkills = gpa[goalKeeperAgent->id()];
            DBUG("Their Indirect" , D_AHZ);
            gpa[goalKeeperAgent->id()]->setDivemode(false);
            gpa[goalKeeperAgent->id()]->setSlowmode(false);
            gpa[goalKeeperAgent->id()]->setAvoidpenaltyarea(false);
            kickSkill->setKickspeed(0);
            gpa[goalKeeperAgent->id()]->setTargetpos(goalKeeperTarget); //HINT : gpa->init
            gpa[goalKeeperAgent->id()]->setTargetdir(wm->ball->pos - wm->field->ourGoal());
            goalKeeperAgent->action = gpa[goalKeeperAgent->id()];

        }
        else if (know->variables["transientFlag"].toBool()) {
            ROS_INFO_STREAM("3");
            know->variables["goalKeeperClearMode"] = false;
            know->variables["goalKeeperOneTouchMode"] = false;
            AHZSkills = gpa[goalKeeperAgent->id()];
            DBUG("TS Mode" , D_AHZ);
            gpa[goalKeeperAgent->id()]->setDivemode(true);
            gpa[goalKeeperAgent->id()]->setSlowmode(false);
            gpa[goalKeeperAgent->id()]->setOnetouchmode(true);
            gpa[goalKeeperAgent->id()]->setNoavoid(true);
            gpa[goalKeeperAgent->id()]->setAvoidpenaltyarea(false);
            kickSkill->setKickspeed(0);
            if (goalKeeperPredictionModeInPlayOff) {
                ROS_INFO_STREAM("4");
                gpa[goalKeeperAgent->id()]->setTargetpos(goalKeeperTarget); //HINT : gpa->init
                gpa[goalKeeperAgent->id()]->setTargetdir(wm->ball->pos - goalKeeperTarget);
            } else {
                ROS_INFO_STREAM("5");
                gpa[goalKeeperAgent->id()]->setTargetpos(goalKeeperTarget); //HINT : gpa->init
                gpa[goalKeeperAgent->id()]->setTargetdir(ballPrediction(true) - wm->field->ourGoal());
            }
            goalKeeperAgent->action = gpa[goalKeeperAgent->id()];
        }
        else if(stopMode){
            ROS_INFO_STREAM("6");
            know->variables["goalKeeperClearMode"] = false;
            know->variables["goalKeeperOneTouchMode"] = false;
            AHZSkills = gpa[goalKeeperAgent->id()];
            DBUG("Stop Mode" , D_AHZ);
            gpa[goalKeeperAgent->id()]->setDivemode(false);
            gpa[goalKeeperAgent->id()]->setSlowmode(true);
            gpa[goalKeeperAgent->id()]->setAvoidpenaltyarea(false);
            kickSkill->setKickspeed(0);
            gpa[goalKeeperAgent->id()]->setTargetpos(goalKeeperTarget); //HINT : gpa->init
            gpa[goalKeeperAgent->id()]->setTargetdir(wm->ball->pos - wm->field->ourGoal());
            goalKeeperAgent->action = gpa[goalKeeperAgent->id()];
        }
        else if (ballIsOutOfField) {
            ROS_INFO_STREAM("7");
            know->variables["goalKeeperClearMode"] = false;
            know->variables["goalKeeperOneTouchMode"] = false;
            AHZSkills = gpa[goalKeeperAgent->id()];
            DBUG("Ball is out of field" , D_AHZ);
            gpa[goalKeeperAgent->id()]->setDivemode(false);
            gpa[goalKeeperAgent->id()]->setSlowmode(true);
            gpa[goalKeeperAgent->id()]->setAvoidpenaltyarea(false);
            kickSkill->setKickspeed(0);
            gpa[goalKeeperAgent->id()]->setTargetpos(goalKeeperTarget); //HINT : gpa->init
            gpa[goalKeeperAgent->id()]->setTargetdir(wm->ball->pos - wm->field->ourGoal());
            goalKeeperAgent->action = gpa[goalKeeperAgent->id()];
        }
        else if (ballIsBesidePoles){
            counterBallWasBesidePoles = 0;
            know->variables["goalKeeperClearMode"] = false;
            know->variables["goalKeeperOneTouchMode"] = false;
            AHZSkills = gpa[goalKeeperAgent->id()];
            DBUG("Ball is beside the poles" , D_AHZ);
            if(f > 3){
                //if(fabs(wm->ball->pos.y - wm->field->ourGoalR().y) < 0.1 || fabs(wm->ball->pos.y - wm->field->ourGoalL().y) < 0.1) {
                gpa[goalKeeperAgent->id()]->setDivemode(false);
                gpa[goalKeeperAgent->id()]->setSlowmode(true);
                gpa[goalKeeperAgent->id()]->setAvoidpenaltyarea(false);
                gpa[goalKeeperAgent->id()]->setTargetpos(wm->ball->pos); //HINT : gpa->init
                gpa[goalKeeperAgent->id()]->setTargetdir(wm->ball->pos - goalKeeperTarget);
                goalKeeperAgent->action = gpa[goalKeeperAgent->id()];
                ROS_INFO_STREAM("???????????????????");
                //}

            }
            else{
                gpa[goalKeeperAgent->id()]->setDivemode(false);
                gpa[goalKeeperAgent->id()]->setSlowmode(true);
                gpa[goalKeeperAgent->id()]->setAvoidpenaltyarea(false);
                gpa[goalKeeperAgent->id()]->setTargetpos(goalKeeperTarget); //HINT : gpa->init
                gpa[goalKeeperAgent->id()]->setBallobstacleradius(0.2);
                gpa[goalKeeperAgent->id()]->setTargetdir(wm->ball->pos - goalKeeperTarget);
                goalKeeperAgent->action = gpa[goalKeeperAgent->id()];
            }
            drawer->draw(Circle2D(goalKeeperTarget , 0.3) , QColor(Qt::blue));
        }
        else if (goalKeeperClearMode && !dangerForGoalKeeperClear) {

            if (lastStateForGoalKeeper == QString("noBesidePoleMode") || counterBallWasBesidePoles < 10){
                counterBallWasBesidePoles++;
                kickSkill->setTarget(Vector2D(0 , wm->ball->pos.y));
                drawer->draw(Segment2D(wm->ball->pos , Vector2D(0 , wm->ball->pos.y)) , QColor(Qt::blue));
            }
            else{
                if (wm->ball->pos.y >= 0) {
                    kickSkill->setTarget(Vector2D(-10 , -4) - wm->field->ourGoal());
                } else {
                    kickSkill->setTarget(Vector2D(-10 , 4) - wm->field->ourGoal());
                }
            }
            know->variables["goalKeeperClearMode"] = true;
            know->variables["goalKeeperOneTouchMode"] = false;
            //drawer->draw(wm->ball->pos , QColor(Qt::red));
            drawer->draw(wm->ball->pos + wm->ball->vel , QColor(Qt::red));
            ROS_INFO_STREAM("pos");
            ROS_INFO_STREAM(wm->ball->pos.x + wm->ball->vel.x);
            ROS_INFO_STREAM(wm->ball->pos.y + wm->ball->vel.y);
            if (wm->ball->vel.length() > 0.4 && wm->ball->vel.length() < 1.3) {
                ROS_INFO_STREAM("9");
                AHZSkills = gpa[goalKeeperAgent->id()];
                DBUG("Clear slow ball" , D_AHZ);
                gpa[goalKeeperAgent->id()]->setDivemode(false);
                gpa[goalKeeperAgent->id()]->setSlowmode(false);
                gpa[goalKeeperAgent->id()]->setAvoidpenaltyarea(false);
                kickSkill->setKickspeed(0);
                ROS_INFO_STREAM(goalKeeperTarget.x);
                ROS_INFO_STREAM(goalKeeperTarget.y);
                if(wm->field->isInOurPenaltyArea(wm->ball->pos + wm->ball->vel))
                    goalKeeperTarget = Segment2D(wm->ball->pos , wm->ball->pos + wm->ball->vel).nearestPoint(goalKeeperAgent->pos());
                else
                    goalKeeperTarget = wm->ball->pos;
                gpa[goalKeeperAgent->id()]->setTargetpos(goalKeeperTarget); //HINT : gpa->init
                gpa[goalKeeperAgent->id()]->setTargetdir(wm->ball->pos - wm->field->ourGoal());
            }
            else {
                ROS_INFO_STREAM("10");
                DBUG("Clear Mode" , D_AHZ);
                AHZSkills = kickSkill;
                kickSkill->setTolerance(4);
                kickSkill->setDontkick(false);
                kickSkill->setSlow(false);
                kickSkill->setSpin(0);
                kickSkill->setAvoidpenaltyarea(false);
                kickSkill->setGoaliemode(true);
                kickSkill->setChip(true);
                kickSkill->setKickspeed(5);
            }
        }
        else {
            if (goalKeeperOneTouch) {
                ROS_INFO_STREAM("11");
                know->variables["goalKeeperClearMode"] = false;
                know->variables["goalKeeperOneTouchMode"] = true;
                AHZSkills = gpa[goalKeeperAgent->id()];
                DBUG("One touch Mode" , D_AHZ);
                gpa[goalKeeperAgent->id()]->setSlowmode(false);
                gpa[goalKeeperAgent->id()]->setDivemode(true);
                gpa[goalKeeperAgent->id()]->setTargetpos(goalKeeperTarget); //HINT : gpa->init
                gpa[goalKeeperAgent->id()]->setTargetdir(-goalKeeperTarget + wm->ball->pos);
                gpa[goalKeeperAgent->id()]->setAvoidpenaltyarea(false);
                gpa[goalKeeperAgent->id()]->setNoavoid(true);
                goalKeeperAgent->action = gpa[goalKeeperAgent->id()];
            }
            else if (dangerForGoalKeeperClear) {
                drawer->draw(Circle2D(goalKeeperTarget , 0.5), QColor(Qt::red));
                ROS_INFO_STREAM("12");
                if (dangerForInsideOfThePenaltyArea) {
                    ROS_INFO_STREAM("13");
                    know->variables["goalKeeperClearMode"] = true;
                    know->variables["goalKeeperOneTouchMode"] = false;
                    DBUG("Danger Mode" , D_AHZ);
                    AHZSkills = kickSkill;
                    kickSkill->setTolerance(10);
                    kickSkill->setDontkick(false);
                    kickSkill->setSlow(false);
                    kickSkill->setSpin(0);
                    kickSkill->setChip(false);
                    kickSkill->setAvoidpenaltyarea(false);
                    kickSkill->setGoaliemode(true);
                    if (wm->ball->pos.y >= 0) {
                        ROS_INFO_STREAM("14");
                        kickSkill->setTarget(Vector2D(-10 , -4) - wm->field->ourGoal());
                    }
                    else {
                        ROS_INFO_STREAM("15");
                        kickSkill->setTarget(Vector2D(-10 , 4) - wm->field->ourGoal());
                    }
                    kickSkill->setChip(true);
                    kickSkill->setKickspeed(5);
                }
                else {
                    ROS_INFO_STREAM("16");
                    know->variables["goalKeeperClearMode"] = false;
                    know->variables["goalKeeperOneTouchMode"] = false;
                    AHZSkills = gpa[goalKeeperAgent->id()];
                    gpa[goalKeeperAgent->id()]->setSlowmode(false);
                    gpa[goalKeeperAgent->id()]->setDivemode(false);
                    kickSkill->setKickspeed(0);
                    gpa[goalKeeperAgent->id()]->setTargetpos(goalKeeperTarget); //HINT : gpa->init
                    gpa[goalKeeperAgent->id()]->setTargetdir(goalKeeperAgent->pos() - wm->field->ourGoal());
                    gpa[goalKeeperAgent->id()]->setAvoidpenaltyarea(false);
                    gpa[goalKeeperAgent->id()]->setNoavoid(true);
                    goalKeeperAgent->action = gpa[goalKeeperAgent->id()];
                }
            }
            else {
                ROS_INFO_STREAM("17");
                //// strict follow
                know->variables["goalKeeperClearMode"] = false;
                know->variables["goalKeeperOneTouchMode"] = false;
                AHZSkills = gpa[goalKeeperAgent->id()];
                gpa[goalKeeperAgent->id()]->setSlowmode(false);
                gpa[goalKeeperAgent->id()]->setDivemode(false);
                gpa[goalKeeperAgent->id()]->setTargetpos(goalKeeperTarget); //HINT : gpa->init
                gpa[goalKeeperAgent->id()]->setTargetdir(wm->ball->pos - wm->field->ourGoal());
                gpa[goalKeeperAgent->id()]->setAvoidpenaltyarea(false);
                goalKeeperAgent->action = gpa[goalKeeperAgent->id()];
            }
            drawer->draw(Circle2D(goalKeeperTarget , 0.05) , 0 , 360 , "black" , true);
        }
        DBUG(QString("beside flag : %1").arg(ballIsBesidePoles) , D_AHZ);
    }
}

bool DefensePlan::defenseOneTouchOrNot() {
    //// First of All this function checks if other defense agents (that isn't
    //// one toucher) can be one toucher , those role changes to it. Then we
    //// check that if ball is shot to the our goal, we active the one touch flag.
    Vector2D sol[2];
    bool isIntersect = false;
    if(!defenseAgents.empty()){
        QList<int> otherAgents;
        otherAgents.clear();
        if (oneToucher < 0 || oneToucher > defenseAgents.size() - 1) {
            oneToucher = 0;
        }
        for (int i = 0; i < defenseAgents.size(); i++) {
            if (i != oneToucher) {
                otherAgents.append(i);
            }
        }
        for (int i = 0 ; i < otherAgents.size() ; i++) {
            Segment2D otherAgentIntersect(wm->ball->pos , pointForKick);
            Vector2D sol1 , sol2;
            Circle2D circ(defenseAgents.at(otherAgents.at(i))->pos() , Robot::robot_radius_new);
            int num = circ.intersection(otherAgentIntersect , &sol1 , &sol2);
            Segment2D oneToucherToOther(defensePoints[oneToucher], defensePoints[otherAgents.at(i)]);
            Line2D ballWay(wm->ball->pos , wm->ball->vel + wm->ball->pos);
            Segment2D ballWay1(wm->ball->pos , wm->ball->vel + wm->ball->pos);
            //////// danger for defense one touch ///////
            for (int i = 0 ; i < defenseAgents.size() ; i++) {
                if(Circle2D(defenseAgents.at(i)->pos() , Robot::robot_radius_new).intersection(ballWay1 , &sol[0] , &sol[1])) {
                    isIntersect = true;
                    break;
                }
            }
            if (oneToucherToOther.intersection(ballWay).valid() && oneToucherToOther.length() < 2*Robot::robot_radius_new && isIntersect) { /// must be test
                return false;
            }
            //////////////////////
            if (pointForKick.valid() && (pointForKick.dist(defenseAgents.at(oneToucher)->pos()) - pointForKick.dist(defenseAgents.at(otherAgents.at(i))->pos()) > 0.2
                                         || (num && defenseAgents.at(oneToucher)->pos().dist(defenseAgents.at(otherAgents.at(i))->pos()) > 2*Robot::robot_radius_new))){ //// ?????
                int temp = oneToucher;
                oneToucher = otherAgents.at(i);
                otherAgents[i] = temp;
                setPointToKick();
            } else if (!pointForKick.valid()) {
                int temp = oneToucher;
                oneToucher = otherAgents.at(i);
                otherAgents[i] = temp;
                setPointToKick();
                if (!pointForKick.valid()) {
                    return false;
                }
            }
        }
    }
    bool oneTouchFlag = false;
    Segment2D ballLine = Segment2D(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm() * 10);
    Segment2D goalLine(wm->field->ourGoal() + Vector2D(0, 1) , wm->field->ourGoal() + Vector2D(0, -1));
    Vector2D goalLineIntersect = goalLine.intersection(ballLine);
    bool ballDistVelFlag = defenseCheckBallDangerForOneTouch();
    bool isItClearInFrontOfBall = know->isPointClear(pointForKick , wm->ball->pos , 0.025);
    if (ballDistVelFlag && isItClearInFrontOfBall) {
        if (goalLineIntersect.valid()) {
            oneTouchFlag = true;
        }
    }
    if (oneTouchFlag) {
        if (!isInOneTouch) {
            isInOneTouch = true;
            double ballDist = wm->ball->pos.dist(pointForKick);
            double cycle =  ballDist / wm->ball->vel.length();
            oneTouchCycleTest = cycle / LOOP_TIME_BYKK;
        }
    }
    drawer->draw(Circle2D(defenseAgents.at(oneToucher)->pos() , 0.2) , 0 , 360 , "blue" , true);
    return oneTouchFlag;
}

void DefensePlan::setPointToKick() {
    //// This function produce a point that one toucher agent must be place on it.
    //// This point is the intersection of the ball line && a line that is
    //// perpendicular on the ball line && is drawn from kicker pos of the agent.

    if (oneToucher < 0 || oneToucher >= defenseAgents.size()) {
        oneToucher = 0;
    }
    Vector2D ballPos = wm->ball->pos;
    Line2D ballLine(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm());
    if (!defenseAgents.empty()) {
        Line2D perpBallLine(defenseAgents.at(oneToucher)->self.getKickerPos() , defenseAgents.at(oneToucher)->self.getKickerPos() + wm->ball->vel.norm());
        perpBallLine = perpBallLine.perpendicular(defenseAgents.at(oneToucher)->self.getKickerPos());
        Vector2D intersect = perpBallLine.intersection(ballLine);
        if (intersect.valid()) {
            if (wm->field->isInField(intersect) && !wm->field->isInOurPenaltyArea(intersect)) {
                pointForKick = intersect;
            } else {
                Vector2D* inter1 = getIntersectWithDefenseArea(ballLine, ballPos);
                if (inter1 != nullptr && inter1->valid()) {
                    pointForKick = *inter1;
                } else {
                    pointForKick.invalidate();
                    return;
                }
            }
        }
        else {
            Vector2D *inter = getIntersectWithDefenseArea(Line2D(ballPos, wm->field->ourGoal()), ballPos);
            if (inter == nullptr || !inter->valid()) {
                pointForKick.invalidate();
                return;
            }
            pointForKick = *inter;
            delete inter;
        }
        oneToucherDir = (Vector2D::unitVector(CKnowledge::oneTouchAngle(pointForKick, Vector2D(0, 0), wm->ball->vel, //TODO: add action property
                                                                        (pointForKick - ballPos).norm()
                                                                        , wm->field->oppGoal()
                                                                        , 0.2
                                                                        , 0.9)));
        pointForKick -= oneToucherDir * (Robot::center_from_kicker_new + CBall::radius);
    }
}

Vector2D* DefensePlan::getIntersectWithDefenseArea(const Circle2D& circle, bool upperPoint) {
    //// This function get us intersection between a circle and defense area
    //// (3 part). the boolian ,"upperPOint", is used for choosing only one of the
    //// intersection points according to the Y of the point.For example if input
    //// circle has two intersection with defense area && "upperPoint" is 1 , this
    //// function returns the point that has a more Y in proportion of the Y cecnter
    //// of the input circle.

    Vector2D* intersectionWithBottomCircle[2];
    Vector2D* intersectionWithTopCircle[2];
    Vector2D* intersectionWithLine[2];
    for (int i = 0; i < 2; i++) {
        intersectionWithBottomCircle[i] = new Vector2D();
        intersectionWithTopCircle[i] = new Vector2D();
        intersectionWithLine[i] = new Vector2D();
    }
    int intersectionWithLineCount = circle.intersection(defenseAreaLine, intersectionWithLine[0], intersectionWithLine[1]);
    int intersectionWithBottomCircleCount = defenseAreaBottomCircle.intersection(circle, intersectionWithBottomCircle[0], intersectionWithBottomCircle[1]);
    int intersectionWithTopCircleCount = defenseAreaTopCircle.intersection(circle, intersectionWithTopCircle[0], intersectionWithTopCircle[1]);

    std::vector<Vector2D*> points;
    for (int i = 0; i < intersectionWithLineCount; i++) {
        points.push_back(intersectionWithLine[i]);
    }
    for (int i = 0; i < intersectionWithBottomCircleCount; i++) {
        float angle = getDegree(defenseAreaBottomCircle.center() + Vector2D(1, 0), defenseAreaBottomCircle.center(), *intersectionWithBottomCircle[i]);
        if (angle <= 0 and angle >= -90) {
            points.push_back(intersectionWithBottomCircle[i]);
        } else {
            delete intersectionWithBottomCircle[i];
        }
    }
    for (int i = 0; i < intersectionWithTopCircleCount; i++) {
        float angle = getDegree(defenseAreaTopCircle.center() + Vector2D(1, 0), defenseAreaTopCircle.center(), *intersectionWithTopCircle[i]);
        if (angle >= 0 and angle <= 90) {
            points.push_back(intersectionWithTopCircle[i]);
        } else {
            delete intersectionWithTopCircle[i];
        }
    }

    Vector2D* retPoint = nullptr;
    for (vector<Vector2D*>::const_iterator it = points.begin(); it != points.end(); it++) {
        if ((upperPoint && (*it)->y > circle.center().y) || (!upperPoint && (*it)->y < circle.center().y)) {
            retPoint = (*it);
            break;
        }
    }
    for (auto &point : points)
        if (point != retPoint) {
            delete point;
        }

    return retPoint;
}

void DefensePlan::checkDefenseExeptions() {
    if (!defenseAgents.empty()) {
        setPointToKick();
        doOneTouch = defenseOneTouchOrNot();
        bool forceBeingInOneTouch = checkStillBeingInOneTouch();
        if (forceBeingInOneTouch) {
            oneTouchPointFlag = false;
        }
        if (doOneTouch || forceBeingInOneTouch) {
            drawer->draw("OneTouch", Vector2D(0, 0), "White" , 20);
            defExceptions.active = true;
            defExceptions.exeptionMode = defOneTouch;
        }
        else {
            defExceptions.active = false;
            defExceptions.exeptionMode = NoneExep;
        }
    }
}

QPair<Vector2D , Vector2D> DefensePlan::avoidCircularPenaltyAreaByArash(Agent* agent, const Vector2D& point) {
    Vector2D agentPos = agent->pos();
    QPair<Vector2D , Vector2D> temp;
    double distFromGoal = 1.6;
    if (agentPos.dist(wm->field->ourGoal()) < distFromGoal) {
        agentPos = wm->field->ourGoal() + Vector2D().setPolar(distFromGoal, (agentPos - wm->field->ourGoal()).th().degree());
        Vector2D* inter = getIntersectWithDefenseArea(Segment2D(agentPos, wm->field->ourGoal()), agentPos);
        if (inter != nullptr && inter->isValid()) {
            agentPos = wm->field->ourGoal() + Vector2D().setPolar(inter->dist(wm->field->ourGoal()) + 0.1, (agentPos - wm->field->ourGoal()).th().degree());
            delete inter;
        }
    }
    drawer->draw(Circle2D(agentPos , 0.02) , 0 , 360 , "brown" , true);
    Vector2D* inter = getIntersectWithDefenseArea(Segment2D(agentPos, point), agentPos);
    if (inter == nullptr || agentPos.dist(*inter) < 0.1) {
        temp.first = point;
        temp.second = agent->pos() - wm->field->ourGoal();
        return temp;
    }
    drawer->draw(Circle2D(*inter , 0.02) , 0 , 360 , "pink" , true);
    Vector2D crossPoint = *inter;
    delete inter;
    AngleDeg deg = (crossPoint - wm->field->ourGoal()).th();
    AngleDeg mainDeg = (point - wm->field->ourGoal()).th();
    AngleDeg diff = mainDeg - deg;
    double s = min(10.0, fabs(diff.degree()));
    double finalDeg = deg.degree() + s * sign(diff.degree());
    Vector2D finalPos = wm->field->ourGoal() + Vector2D().setPolar(100, finalDeg);
    inter = getIntersectWithDefenseArea(Segment2D(finalPos, wm->field->ourGoal()), finalPos);
    Vector2D retPoint;
    if (inter != nullptr) {
        retPoint = *inter;
        delete inter;
    } else {
        retPoint.invalidate();
    }
    drawer->draw(Circle2D(retPoint , 0.02) , 0 , 360 , "cyan" , true);
    Vector2D vecDiff = retPoint - crossPoint;
    double dist = crossPoint.dist(point);
    if (dist > 1) {
        retPoint = retPoint + vecDiff.setLength(dist * 2);
    }
    drawer->draw(Circle2D(retPoint , 0.02) , 0 , 360 , "blue" , true);
    temp.first = retPoint;
    temp.second = agent->pos() - wm->field->ourGoal();
    return temp;
}

Vector2D DefensePlan::runDefenseOneTouch() {
    if (!defenseAgents.empty()) {
        if (!oneTouchPointFlag) {
            defensePoints[oneToucher] = pointForKick;
            defenseDirs[oneToucher] = oneToucherDir;
            oneTouchPointFlag = true;
            calcPointForOneTouch();
            defenseTargets[oneToucher] = defensePoints[oneToucher];
            defenseTargets[oneToucher] = avoidCircularPenaltyAreaByArash(defenseAgents[oneToucher], defenseTargets[oneToucher]).first;
            return defenseTargets[oneToucher];
        }
        return Vector2D(-100, 100);
    }
}

bool DefensePlan::checkStillBeingInOneTouch(){
    Segment2D ballLine = Segment2D(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm() * 10);
    Segment2D goalLine(wm->field->ourGoal() + Vector2D(0, 1.1) , wm->field->ourGoal() + Vector2D(0, -1.1));
    Vector2D goalLineIntersect = goalLine.intersection(ballLine);
    ////////////////////////////Commented by Arash.Z/////////////////////////
    ////check oneTouch timer so it wont switch state machine
    if (!isInOneTouch) {
        cycleCounter = 0;
        return false;
    }
    if (!goalLineIntersect.valid()) {
        cycleCounter = 0;
        isInOneTouch = false;
        return false;
    }
    drawer->draw(QString("cyclePredict %1").arg(oneTouchCycleTest - cycleCounter) , Vector2D(1, 1.4) , "white");
    cycleCounter++;
    if (cycleCounter < oneTouchCycleTest + 5) {
        return true;
    } else {
        cycleCounter = 0;
        isInOneTouch = false;
        return false;
    }
}

void DefensePlan::runDefenseExeptions() {
    if (!defenseAgents.empty()) {
        Vector2D agentTarget;
        if (defExceptions.exeptionMode == defOneTouch) {
            know->variables["defenseOneTouchMode"] = true;
            agentTarget = runDefenseOneTouch();
            drawer->draw(QString("Defense OneTouch"), Vector2D(0, 2), "red");
            if (agentTarget.x != -100) {
                drawer->draw(QString("Defense OneTouch"), Vector2D(0, 2), "red");
                assignSkill(defenseAgents.at(oneToucher) , gpa[defenseAgents.at(oneToucher)->id()]);
                gpa[defenseAgents.at(oneToucher)->id()]->setTargetpos(defenseTargets[oneToucher]); //HINT : gpa->init
                gpa[defenseAgents.at(oneToucher)->id()]->setTargetdir(defenseDirs[oneToucher]);
                drawer->draw(Circle2D(defenseTargets[0] , 0.05) , 0 , 360 , "black" , true);
                defExceptions.exepAgentId = defenseAgents.at(oneToucher)->id();
            }
        }
        else {
            know->variables["defenseOneTouchMode"] = false;
        }
    }
}

int DefensePlan::decideNumOfMarks() {
    //// This function returns the "defenseCount" in all states, except when ball
    //// is near the corners , returns the 1.

    playOnMode = gameState->isStart();
    playOffMode = gameState->theirDirectKick() || gameState->theirIndirectKick();
    if (defenseCount > 0){
        if (gameState->isStop()) {
            return defenseCount - defenseNumber();
        }
        if (playOffMode) {
            return defenseCount;
        }
        else if (know->variables["transientFlag"].toBool()) {
            return defenseCount;
        }
        else if (playOnMode) {
            return 0;
        }
    }
    return 0;
}

Vector2D DefensePlan::ballPrediction(bool _isGoalie) {
    //// When ballLine is in field we predict the ball line : If ball moves toward the
    //// our field, we consider the ballLine (ballPos + ballVel) && If moves toward
    //// the opponent field we consider the ballPos + ballVel.y for the location
    //// of the ball. When ballLine is out of field, we consider the intersection
    //// of field with ballLine.(algorithm is just like when ballLine isn't in the
    //// field).Also , If the ballLine has intersection with opponent agent, we
    //// consider the intersection point for the locaiton of the ball.

    Vector2D BallPos = wm->ball->pos;
    Vector2D BallVel = wm->ball->vel  * 0.25;
    Segment2D ballPosVel(BallPos, BallPos + (BallVel));
    Vector2D predictedBall = BallPos;
    Vector2D solu[6];
    Rect2D fieldRect(Vector2D(- wm->field->_FIELD_WIDTH / 2.0 , - wm->field->_FIELD_HEIGHT / 2.0) + Vector2D(-0.005, -0.005), Vector2D(wm->field->_FIELD_WIDTH / 2.0 , wm->field->_FIELD_HEIGHT / 2.0) + Vector2D(+0.005, +0.005));
    double dist2Ball = 1000;
    //    predictedBall = wm->ball->pos;
    //    return predictedBall;
    if (BallVel.x > 0 && BallPos.x > 0) {
        return BallPos;
    }
    if(wm->opp.activeAgentsCount() > 0) {
        for (int i = 0 ; i < wm->opp.activeAgentsCount() ; i++) {
            Circle2D oppCircle(wm->opp.active(i)->pos, 0.1);
            if (oppCircle.intersection(ballPosVel, &solu[0], &solu[1]) > 0 && BallVel.length() > 0.5) {
                if (wm->opp.active(i)->pos.dist(BallPos) < dist2Ball) {
                    dist2Ball = wm->opp.active(i)->pos.dist(BallPos);
                    predictedBall = wm->opp.active(i)->pos;
                }
            }
        }
    }
    if (dist2Ball != 1000) {
        drawer->draw(QString("Def Predicted Level 1"), Vector2D(0, 2), "red");
        drawer->draw(predictedBall);
        return predictedBall;
    } else if (wm->field->isInField(BallPos + BallVel) && BallVel.length() > 0.5) {
        if ((BallVel.x <= 0.2 || _isGoalie)) {
            predictedBall = BallPos + BallVel;
        } else {
            drawer->draw(QString("Def Predicted"), Vector2D(0, 1), "red");
            predictedBall = BallPos + Vector2D(0, BallVel.y);
        }
        drawer->draw(predictedBall);
        return predictedBall;
    } else if (BallVel.length() > 0.5) {
        fieldRect.intersection(ballPosVel, &solu[0], &solu[1]);
        if (BallVel.x <= 0 || _isGoalie) {
            predictedBall = (BallPos.dist(solu[0]) < BallPos.dist(solu[1]) && solu[0].isValid()) ? (solu[0]) : (solu[1]);
        } else {
            drawer->draw(QString("Def Predicted Level 2"), Vector2D(0, 2), "red");
            predictedBall = (BallPos.dist(solu[0]) < BallPos.dist(solu[1])) ? Vector2D(BallPos.x, solu[0].y) : Vector2D(BallPos.x, solu[1].y);
        }
    } else {
        predictedBall = BallPos;
        drawer->draw(QString("Def follow"), Vector2D(0, 2), "red");
    }

    if(!_isGoalie){
        if(wm->field->ourBigPenaltyArea(1, 0.2, 0).contains(wm->ball->pos)){
            drawer->draw(QString("1"), Vector2D(2, 2), "red");
            wm->field->ourBigPenaltyArea(1, 0.4, 0).intersection(Line2D(wm->field->ourGoal() , wm->ball->pos) , &solu[2] , &solu[3]);
            predictedBall = solu[2].dist(wm->field->center()) < solu[3].dist(wm->field->center()) ? solu[2] : solu[3];
        }
        else if(Circle2D(wm->field->ourGoal() , RADIUS_FOR_CRITICAL_DEFENSE_AREA).contains(wm->ball->pos)){
            drawer->draw(QString("2"), Vector2D(2, 2), "red");
            Circle2D(wm->field->ourGoal() , RADIUS_FOR_CRITICAL_DEFENSE_AREA + 0.1).intersection(Line2D(wm->ball->pos , wm->field->ourGoal()) , &solu[4] , &solu[5]);
            predictedBall = solu[4].isValid() && solu[4].dist(wm->field->center()) < solu[5].dist(wm->field->center()) ? solu[4] : solu[5];
        }
    }

    if(!_isGoalie && (wm->ball->pos.x < wm->field->ourGoal().x || wm->ball->pos.x > wm->field->oppGoal().x)){
        predictedBall = wm->field->center() - Vector2D(4 , 0);
    }
    drawer->draw(predictedBall , "black");
    return predictedBall;
}

void DefensePlan::findOppAgentsToMark(){
    oppAgentsToMark.clear();
    oppAgentsToMarkPos.clear();
    for(int i = 0 ; i < wm->opp.activeAgentsCount() ; i++){
        if (wm->opp.activeAgentID(i) != wm->opp.data->goalieID) {
            oppAgentsToMark.append(wm->opp.active(i));
        }
    }
    if(gameState->theirIndirectKick() || gameState->isStop()) {
        int nearestToBall = -1;
        double nearestToBallDist = 100000;
        for(int i = 0 ; i < oppAgentsToMark.count() ; i++){
            if((oppAgentsToMark[i]->pos).dist(wm->ball->pos) < nearestToBallDist){
                nearestToBall = i;
                nearestToBallDist = oppAgentsToMark[i]->pos.dist(wm->ball->pos);
            }
        }
        if(nearestToBall != -1){
            oppAgentsToMark.removeOne(oppAgentsToMark[nearestToBall]);
        }
    }
    if(gameState->theirKickoff()){
        for(int i = 0; i < oppAgentsToMark.count(); i++){
            if(oppAgentsToMark[i]->pos.x > conf.OppOmitLimitKickOff){
                oppAgentsToMark.removeOne(oppAgentsToMark[i]);
                i--;
            }
        }
    }
    else if(gameState->theirIndirectKick() || gameState->theirDirectKick() || gameState->isStop() || know->variables["transientFlag"].toBool()) {
        for (int i = 0; i < oppAgentsToMark.count(); i++) {
            if (oppAgentsToMark[i]->pos.x > conf.OppOmitLimitPlayoff) {
                oppAgentsToMark.removeOne(oppAgentsToMark[i]);
                i--;
            }
        }
    }
    for(int i = 0; i < oppAgentsToMark.count(); i++) {
        oppAgentsToMarkPos.append(posvel(oppAgentsToMark[i], conf.VelReliability));
    }
}

Vector2D DefensePlan::posvel(CRobot* opp, double VelReliabiity) {
    //// This function predicts the opponent agent with considering the position
    //// && velocity of the opponent agent.

    if (VelReliabiity == 0) {
        return opp->pos;
    }
    if (!opp->vel.length() > 0.5  || opp->vel.x > 0 || wm->field->isInOurPenaltyArea(opp->pos)) {
        VelReliabiity = 0;
    }
    Segment2D tempseg;
    Vector2D solutions[2];
    Vector2D desiredSolution;
    Vector2D temppos = opp->pos + VelReliabiity * opp->vel;
    tempseg.assign(opp->pos, opp->pos + VelReliabiity * opp->vel);
    drawer->draw(tempseg, QColor(Qt::yellow));
    wm->field->ourBigPenaltyArea(1, 0.3, 0).intersection(tempseg , &solutions[0] , &solutions[1]);
    desiredSolution = solutions[0].dist(opp->pos) < solutions[1].dist(opp->pos) ? solutions[0] : solutions[1];
    if (wm->field->isInField(desiredSolution) && desiredSolution.isValid() && tempseg.length() != 0) {
        return desiredSolution;
    } else if (temppos.x < -4.4) {
        return Vector2D(-4.4, (opp->pos + VelReliabiity * opp->vel).y) ;
    } else {
        return opp->pos + VelReliabiity * opp->vel;
    }
}

void DefensePlan::findPos(int _markAgentSize){
    //// In this function, we choose the different plans of the mark in different
    //// conditions && states.Some flags are used to stay in previous state && for
    //// not switching between PlayOff && PlayOn.

    bool playOff = ((gameState->theirDirectKick())/*|| (knowledge->getGameState() == CKnowledge::TheirKickOff)*/ || (gameState->theirIndirectKick()));
    bool MantoManAllTransientFlag = conf.ManToManAllTransiant;
    xLimitForblockingPass = 0;
    manToManMarkBlockPassFlag = conf.PlayOffManToMan;
    stopMode = gameState->isStop();
    markPoses.clear();
    markAngs.clear();
    ///////////////// Man To Man AllTransiant Mode for Mark ////////////////////
    DBUG(know->variables["lastStateForMark"].toString() , D_AHZ);
    DBUG(know->variables["stateForMark"].toString() , D_AHZ);
    if (MantoManAllTransientFlag) {
        if (know->variables["transientFlag"].toBool()) {
            segmentpershoot = 0.1;
        } else {
            segmentpershoot = conf.ShootRatioBlock / 100;
        }
        segmentperpass = conf.PassRatioBlock / 100;
    }
    else {
        segmentpershoot = conf.ShootRatioBlock / 100;
        segmentperpass = conf.PassRatioBlock / 100;
    }
    //////////////// Determine the plan of mark from GUI ////////////////////
    if (manToManMarkBlockPassFlag || wm->ball->pos.x > xLimitForblockingPass) {
        if (playOff || stopMode) {
            know->variables["stateForMark"] = QString("BlockPass");
            manToManMarkBlockPassInPlayOff(oppAgentsToMarkPos, _markAgentSize , conf.PassRatioBlock / 100);
        }
        else if (know->variables["transientFlag"].toBool()) {
            if (know->variables["lastStateForMark"] == QString("BlockPass")) {
                know->variables["stateForMark"] = QString("BlockPass");
                manToManMarkBlockPassInPlayOff(oppAgentsToMarkPos, _markAgentSize , conf.PassRatioBlock / 100);
            }
            else {
                know->variables["stateForMark"] = QString("BlockShot");
                manToManMarkBlockShotInPlayOff(_markAgentSize);
            }
        }
    }
    else {
        if (playOff || stopMode) {
            know->variables["stateForMark"] = QString("BlockShot");;
            manToManMarkBlockShotInPlayOff(_markAgentSize);
        }
        else {
            if (know->variables["lastStateForMark"].toString() == QString("BlockShot")) {
                know->variables["stateForMark"] = QString("BlockShot");;
                manToManMarkBlockShotInPlayOff(_markAgentSize);
            }
            else {
                know->variables["stateForMark"] = QString("BlockPass");
                manToManMarkBlockPassInPlayOff(oppAgentsToMarkPos, _markAgentSize , conf.PassRatioBlock / 100);
            }
        }
    }
    know->variables["lastStateForMark"] = know->variables["stateForMark"];
}

QList<Vector2D> DefensePlan::ShootBlockRatio(double ratio, Vector2D opp) {
    //// This function produces a point between opponent agents to be mark &&
    //// ourGoal to block the shot path. Also checks if this point is in the
    //// penalty area, produces a point that is intersection of penalty area with
    //// shot path.
    Vector2D solutions[2];
    QList<Vector2D> tempQlist;
    tempQlist.clear();
    Segment2D tempSeg = getBisectorSegment(wm->field->ourGoalL() , opp , wm->field->ourGoalR());
    Vector2D intersectionWithOurGoalLine = tempSeg.intersection(Segment2D(wm->field->ourGoalL() , wm->field->ourGoalR()));
    drawer->draw(tempSeg , "black");
    Vector2D pos = know->getPointInDirection(wm->field->ourGoal() , opp , ratio);
    if(wm->field->isInOurPenaltyArea(opp)){
        wm->field->ourBigPenaltyArea(1,Robot::robot_radius_new,0).intersection(tempSeg, &solutions[0] , &solutions[1]);
        tempQlist.append(solutions[0].isValid() && solutions[0].dist(opp) < solutions[1].dist(opp) ? solutions[0] : solutions[1]);
        tempQlist.append(tempQlist.first() - intersectionWithOurGoalLine);
    }
    else{
        if(wm->field->isInOurPenaltyArea(pos)){
            wm->field->ourBigPenaltyArea(1, Robot::robot_radius_new, 0).intersection(tempSeg, &solutions[0] , &solutions[1]);
            tempQlist.append(solutions[0].isValid() && solutions[0].dist(opp) < solutions[1].dist(opp) ? solutions[0] : solutions[1]);
            tempQlist.append(tempQlist.first() - intersectionWithOurGoalLine);
        }
        else{
            tempQlist.append(pos);
            tempQlist.append(opp - intersectionWithOurGoalLine);
        }
    }
    return tempQlist;
}

bool DefensePlan::isInTheIndirectAreaShoot(Vector2D opp) {
    //// This function checks the point that is resulted from block shot plan,
    //// is in the ball circle or not.

    Circle2D indirectAvoidCircle(wm->ball->pos, ballCircleR + 0.2);
    if(indirectAvoidCircle.contains(ShootBlockRatio(segmentpershoot, opp).first()) && !know->variables["transientFlag"].toBool()) {
        return true;
    } else {
        return false;
    }
}

QList<Vector2D> DefensePlan::indirectAvoidShoot(Vector2D opp) {
    //// If a point (that is resulted from block shot plan) is in the ball circle,
    //// this funcrion produces a suitable point instead of it.This point is
    //// intersection of ball circle && shot path.

    QList<Vector2D> temp;
    Segment2D tempseg = Segment2D(wm->field->ourGoal() , opp);
    double indirectAvoidRadius = 0.5 + 0.2;
    Circle2D indirectAvoidCircle(wm->ball->pos, indirectAvoidRadius);
    Vector2D sol1, sol2, sol;
    indirectAvoidCircle.intersection(tempseg, &sol1, &sol2);
    sol = sol1.isValid() && sol1.dist(wm->ball->pos) < sol2.dist(wm->ball->pos) ? sol1 : sol2;
    temp.clear();
    temp.append(sol);
    temp.append(wm->ball->pos - wm->field->ourGoal());
    return temp;
}

bool DefensePlan::isInTheIndirectAreaPass(Vector2D opp) {
    //// This function checks the point that is resulted from block pass plan,
    //// is in the ball circle or noShootBlockRatiot.
    DBUG(QString("IndirectAreaPass"), D_HAMED);
    double indirectAvoidRadius = 0.5 + 0.2;
    Circle2D indirectAvoidCircle(wm->ball->pos, indirectAvoidRadius);
    if (indirectAvoidCircle.contains(PassBlockRatio(segmentperpass, opp).first()) && !know->variables["transientFlag"].toBool()) {
        return true;
    }

    return false;

}

QList<Vector2D> DefensePlan::indirectAvoidPass(Vector2D opp) {
    //// If a point(that is resulted from block pass plan) is in the ball circle
    //// , this function produces a suitable point instead of it.The point is
    //// intersection of ball circle && pass path.

    Segment2D tempseg;
    tempseg.assign(wm->ball->pos, opp + 10 * (opp - wm->ball->pos));
    double indirectAvoidRadius = 0.5 + .2;
    Circle2D indirectAvoidCircle(wm->ball->pos, indirectAvoidRadius);
    Vector2D sol1, sol2, sol;
    indirectAvoidCircle.intersection(tempseg, &sol1, &sol2);
    if (sol1.valid()) {
        sol = sol1;
    } else if (sol2.valid()) {
        sol = sol2;
    }
    QList<Vector2D> temp;
    temp.clear();
    temp.append(sol);
    temp.append(wm->ball->pos - opp);
    return temp;
}

QList<Vector2D> DefensePlan::PassBlockRatio(double ratio, Vector2D opp) {
    //// This function produces a point that block the pass path.Also if the
    //// resulted point is in the penalty area, this function geneates a suitable
    //// point.
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
    else{
        opponentAgentsToBeMarkCircle.intersection(Segment2D(wm->ball->pos , opp), &sol1 , &sol2);
        Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opp), &sol3 , &sol4);
        passBlocker.append(know->getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2, Segment2D(sol3 , opp).length() < Segment2D(sol4 , opp).length() ? sol3 : sol4, ratio));
    }
    passBlocker.append(wm->ball->pos - passBlocker.first());
    return passBlocker;

}

void DefensePlan::fillDefencePositionsTo(Vector2D *poses) {
    //// This function sends the points (that is generated in the defense plan)
    //// to the "coach.cpp".

    for (int i = 0 ; i < defenseAgents.count() ; i++) {
        poses[i] = defensePoints[i];
    }
}

Vector2D DefensePlan::NearestDistanceToBallSegment(Vector2D point) {
    //// This function gets us the nearest point of input point in proportion of
    //// the our goal line, if intersection of ball line && our goal line is valid.

    Segment2D OurGoalAreaLine(wm->field->ourGoal() + Vector2D(0, -0.6) , wm->field->ourGoal() + Vector2D(0, 0.6));
    Segment2D Ballsegment(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm() * 10);
    Vector2D  OurGoalAreaLineIntersectionByBallSegment(OurGoalAreaLine.intersection(Ballsegment));
    Vector2D NearestTarget = OurGoalAreaLine.nearestPoint(point);
    if (OurGoalAreaLineIntersectionByBallSegment.valid()) {
        return NearestTarget;
    }
}

int DefensePlan::predictMostDangrousOppToBall() {
    //// This function gets us the ID of the agents that is in front of the ball
    //// && probably ball reaches to their.

    if (wm->opp.activeAgentsCount() > 0) {
        Vector2D OppPos[wm->opp.activeAgentsCount()];
        for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
            OppPos[i] = wm->opp.active(i)->pos;
        }
        Vector2D OppPosProjectionsOnBallSegmentByVel[wm->opp.activeAgentsCount()];
        Segment2D BallSegmentByVel(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm() * 10);
        double DistanceFromProjectionsToBallPos[wm->opp.activeAgentsCount()];
        double DistanceFromOppPosToProjections[wm->opp.activeAgentsCount()];
        double BallVelocity = wm->ball->vel.length();
        double MinDistanceToBallPosY = 10000;
        double  DistanceToBallPosY[wm->opp.activeAgentsCount()];
        int MinDistanceToBallPosYRoboID = 100;
        for (int i = 0 ; i < wm->opp.activeAgentsCount() ; i++) {
            OppPosProjectionsOnBallSegmentByVel[i] = NearestDistanceToBallSegment(OppPos[i]);
            DistanceFromProjectionsToBallPos[i] = (wm->ball->pos - OppPosProjectionsOnBallSegmentByVel[i]).length();
            DistanceFromOppPosToProjections[i] = (OppPosProjectionsOnBallSegmentByVel[i] - OppPos[i]).length();
            Segment2D OppSegmentByvel(wm->opp.active(i)->pos, wm->opp.active(i)->pos + wm->opp.active(i)->vel.norm() * 10);
            Segment2D OppPosTotheirProjections(OppPosProjectionsOnBallSegmentByVel[i], OppPos[i]);
            Segment2D OppSegmentByDir(wm->opp.active(i)->pos, wm->opp.active(i)->pos + wm->opp.active(i)->dir * 10);
            if (OppPosTotheirProjections.intersection(BallSegmentByVel).isValid()) {
                // is front to ball
                if (wm->opp.active(i)->vel.length() > 0.5) {
                    // is moving
                    if (OppSegmentByvel.intersection(BallSegmentByVel).valid() && DistanceFromProjectionsToBallPos[i] * BallVelocity / DistanceFromOppPosToProjections[i] > 5) {
                        // find the lowest amount of distToBallY
                        DistanceToBallPosY[i] = fabs(OppPos[i].y - wm->ball->pos.y);
                        if (MinDistanceToBallPosY > DistanceToBallPosY[i]) {
                            MinDistanceToBallPosY = DistanceToBallPosY[i];
                            MinDistanceToBallPosYRoboID = wm->opp.active(i)->id;
                        }
                    }
                } else {
                    // is n't moving
                    if (OppSegmentByDir.intersection(BallSegmentByVel).valid() && DistanceFromOppPosToProjections[i] < 0.3) {
                        DistanceToBallPosY[i] = fabs(wm->opp.active(i)->pos.y - wm->ball->pos.y);
                        if (DistanceToBallPosY[i] < MinDistanceToBallPosY) {
                            MinDistanceToBallPosY = DistanceToBallPosY[i];
                            MinDistanceToBallPosYRoboID = wm->opp.active(i)->id;
                        }
                    }
                }
            } else {
                // is behind to ball
            }
        }
        return MinDistanceToBallPosYRoboID;
    } else {
        return 100;
    }
}

Vector2D DefensePlan::strictFollowBall(Vector2D _ballPos) {
    //// This function is  the main behavior of goalkeeper that is a geometric
    //// behavior.

    //////////////////////// Variables of this function //////////////////////
    Vector2D ballPos = _ballPos;
    Segment2D goal2Ball;
    QList<Vector2D> tempSol;
    Vector2D target(Vector2D(0, 0));
    Vector2D goalKeeperTargetOffSet = Vector2D(0.11 , -0.06);
    QList<Circle2D> defs;
    double AZBisecOpenAngle = 0, AZBigestOpenAngle = 0, AZDangerPercent = 0, aimLessChord = 0;
    double topFaceLength;
    double bottomFaceLength;
    double ballheight;
    double nearestDist2Ball;
    int nearestDef2BallId = -1 ;
    int g;
    //////////////////////////////////////////////////////////////////////////
    tempSol.clear();
    drawer->draw("ahhhhhh" , Vector2D(-2 , 0) , "red");
    if (goalKeeperAgent != nullptr) {
        ballPos = _ballPos;
        Segment2D goalLine(wm->field->ourGoal() + Vector2D(0, -0.8) , wm->field->ourGoal() + Vector2D(0, 0.8));
        Segment2D downFieldLine(Vector2D(-wm->field->_FIELD_WIDTH / 2, -wm->field->_FIELD_HEIGHT / 2), Vector2D(-wm->field->_FIELD_WIDTH / 2, wm->field->_FIELD_HEIGHT / 2));
        //////////////////////////////// Appending circles on defense agents /////////////////////////////////////////
        for (g = 0; g < defenseAgents.count() ; g++) {
            defs.append(Circle2D(defenseAgents[g]->pos(), Robot::robot_radius_new));
            if (defenseAgents[g]->pos().dist(wm->ball->pos) < nearestDist2Ball) {
                nearestDef2BallId = defenseAgents[g]->id();
                nearestDist2Ball = defenseAgents[g]->pos().dist(wm->ball->pos);
            }
        }
        drawer->draw(ballPrediction(true));
        ///////////////////////////// Empty region between defense agents //////////////////////////
        know->getEmptyAngle(ballPos, wm->field->ourGoalL(), wm->field->ourGoalR(), defs, AZDangerPercent, AZBisecOpenAngle, AZBigestOpenAngle, true);
        ////////// Bisector of triangle that is made up of with this points : [ballPossition , topGoal , bottom Goal]  //////////////////////////
        Segment2D AZBisecOpenSeg(ballPos , ballPos + Vector2D(cos(_PI * (AZBisecOpenAngle) / 180), sin(_PI * (AZBisecOpenAngle) / 180)).norm() * 12);
        ////////// Top and bottom line of triangle that is made up of with this points : [ballPossition , topGoal , bottom Goal]  //////////////////////////
        Segment2D AZTopOfOpenSeg(ballPos , ballPos + Vector2D(cos(_PI * (AZBisecOpenAngle + (AZBigestOpenAngle / 2)) / 180), sin(_PI * (AZBisecOpenAngle + (AZBigestOpenAngle / 2)) / 180)).norm() * 12);
        Segment2D AZBottomOfOpenSeg(ballPos , ballPos + Vector2D(cos(_PI * (AZBisecOpenAngle - (AZBigestOpenAngle / 2)) / 180), sin(_PI * (AZBisecOpenAngle - (AZBigestOpenAngle / 2)) / 180)).norm() * 12);
        ///////////// Intersection of the top and bottom line of the triangle with goalLine ////////////////////////////////////////////
        Vector2D openAngGoalIntersectionTop(AZTopOfOpenSeg.intersection(goalLine));
        Vector2D openAngGoalIntersectionBottom(AZBottomOfOpenSeg.intersection(goalLine));
        /////////////////// Real length of top and bottom line of the triangle for talles //////////////////////////////////////////////////////////////
        topFaceLength = ballPos.dist(openAngGoalIntersectionTop);
        bottomFaceLength = ballPos.dist(openAngGoalIntersectionBottom);
        ///////////////////  top and bottom line of the triangle for talles for talles (with real length)//////////////////////////////////////////////////////////////
        Segment2D topFaceLength_forTalles(ballPos , ballPos + Vector2D(cos(_PI * (AZBisecOpenAngle + (AZBigestOpenAngle / 2)) / 180), sin(_PI * (AZBisecOpenAngle + (AZBigestOpenAngle / 2)) / 180)).norm()*topFaceLength);
        Segment2D bottomFaceLength_forTalles(ballPos , ballPos + Vector2D(cos(_PI * (AZBisecOpenAngle - (AZBigestOpenAngle / 2)) / 180), sin(_PI * (AZBisecOpenAngle - (AZBigestOpenAngle / 2)) / 180)).norm()*bottomFaceLength);
        //////////////////////////Height of the triangle ///////////////////////////////////////////////////////
        ballheight = ballPos.dist(downFieldLine.nearestPoint(ballPos));
        Line2D aimLessLine(Vector2D(0, 0), Vector2D(-1, -1));
        drawer->draw(AZBisecOpenSeg, "red");
        goal2Ball.assign(wm->field->ourGoal(), wm->ball->pos);
        if (Vector2D::angleOf(wm->ball->pos, wm->field->ourGoal(), wm->field->ourGoalL()).degree() < 15 + angleDegreeThrNotStop) {
            target = wm->field->ourGoalL() + goalKeeperTargetOffSet;
            angleDegreeThrNotStop = 2;
        } else if (Vector2D::angleOf(wm->ball->pos, wm->field->ourGoal(), wm->field->ourGoalR()).degree() < 15 + angleDegreeThrNotStop) {
            target = wm->field->ourGoalR() + goalKeeperTargetOffSet;
            angleDegreeThrNotStop = 2;
        } else {
            angleDegreeThrNotStop = 0;
            if (goalKeeperAgent->pos().dist(AZBisecOpenSeg.nearestPoint(goalKeeperAgent->pos())) > 0.3 + thr) {
                target = AZBisecOpenSeg.nearestPoint(goalKeeperAgent->pos());
                thr = 0;
                if (!wm->field->isInField(target)) {
                    target = AZBisecOpenSeg.intersection(goalLine);
                }
            } else {
                thr = 0.3;
                if (topFaceLength < bottomFaceLength) {
                    aimLessChord = bottomFaceLength_forTalles.nearestPoint(openAngGoalIntersectionTop).dist(openAngGoalIntersectionTop);
                    DBUG(QString("top koochik tar"), D_SEPEHR);
                    aimLessLine = Line2D(bottomFaceLength_forTalles.nearestPoint(openAngGoalIntersectionTop), openAngGoalIntersectionTop);
                } else {
                    aimLessChord = topFaceLength_forTalles.nearestPoint(openAngGoalIntersectionBottom).dist(openAngGoalIntersectionBottom);
                    DBUG(QString("bottom koochik tar"), D_SEPEHR);
                    aimLessLine = Line2D(topFaceLength_forTalles.nearestPoint(openAngGoalIntersectionBottom), openAngGoalIntersectionBottom);
                    drawer->draw(Segment2D(topFaceLength_forTalles.nearestPoint(openAngGoalIntersectionBottom), openAngGoalIntersectionBottom), QColor(Qt::black));
                }
                if (aimLessChord > 2 * Robot::robot_radius_new) {
                    DBUG(QString("chord > 2radius"), D_SEPEHR);
                    aimLessLine = Line2D(Vector2D(ballPos.x - (.18 * ballheight / aimLessChord), ballPos.y), Vector2D(ballPos.x - (.18 * ballheight / aimLessChord), ballPos.y - 0.1));
                    drawer->draw(Segment2D(Vector2D(ballPos.x - (.18 * ballheight / aimLessChord), ballPos.y), Vector2D(ballPos.x - (.18 * ballheight / aimLessChord), ballPos.y - 1)), QColor(Qt::black));
                    if (AZBisecOpenSeg.intersection(aimLessLine).isValid()) {
                        if (defenseCount == 2) {
                            if (know->getEmptyAngle(ballPos, wm->field->ourGoalL(), wm->field->ourGoalR(), defs, AZDangerPercent, AZBisecOpenAngle, AZBigestOpenAngle, true) > 10 + threshOld) {
                                target = AZBisecOpenSeg.intersection(aimLessLine);
                                threshOld = 0;
                            } else {
                                threshOld = 5;
                                target = know->getPointInDirection(wm->field->ourGoal() , wm->ball->pos , 0.1);
                            }
                        } else if (defenseCount == 1) {
                            target = getGoaliePositionInOneDef(wm->ball->pos , 1 , 1.5);
                        } else {
                            tempSol.append(wm->AHZOurPAreaIntersect(Segment2D(wm->field->ourGoal() , wm->ball->pos), QString("goalKeeper")));
                            if (tempSol.size() == 1) {
                                target = tempSol.at(0);
                            } else if (tempSol.size() == 2) {
                                target = tempSol.at(0).dist(wm->ball->pos) < tempSol.at(1).dist(wm->ball->pos) ? tempSol.at(0) : tempSol.at(1);
                            }
                        }
                    } else {
                        if (defenseCount == 2) {
                            if (know->getEmptyAngle(ballPos, wm->field->ourGoalL(), wm->field->ourGoalR(), defs, AZDangerPercent, AZBisecOpenAngle, AZBigestOpenAngle, true) > 10 + threshOld) {
                                target = AZBisecOpenSeg.intersection(aimLessLine);
                                threshOld = 0;
                            } else {
                                threshOld = 5;
                                target = know->getPointInDirection(wm->field->ourGoal() , wm->ball->pos , 0.1);
                            }
                        } else if (defenseCount == 1) {
                            target = getGoaliePositionInOneDef(wm->ball->pos , 1 , 1.5);
                        } else {
                            tempSol.append(wm->AHZOurPAreaIntersect(Segment2D(wm->field->ourGoal() , wm->ball->pos), QString("goalKeeper")));
                            if (tempSol.size() == 1) {
                                target = tempSol.at(0);
                            } else if (tempSol.size() == 2) {
                                target = tempSol.at(0).dist(wm->ball->pos) < tempSol.at(1).dist(wm->ball->pos) ? tempSol.at(0) : tempSol.at(1);
                            }
                        }
                    }
                } else {
                    if (defenseCount == 2) {

                        if (know->getEmptyAngle(ballPos, wm->field->ourGoalL(), wm->field->ourGoalR(), defs, AZDangerPercent, AZBisecOpenAngle, AZBigestOpenAngle, true) > 10 + threshOld) {

                            tempSol.append(wm->AHZOurPAreaIntersect(Segment2D(wm->field->ourGoal() , wm->ball->pos), QString("goalKeeper")));
                            if (tempSol.size() == 1) {
                                target = tempSol.at(0);
                            } else if (tempSol.size() == 2) {
                                target = tempSol.at(0).dist(wm->ball->pos) < tempSol.at(1).dist(wm->ball->pos) ? tempSol.at(0) : tempSol.at(1);
                            }
                            threshOld = 0;
                        } else {
                            threshOld = 5;

                            target = know->getPointInDirection(wm->field->ourGoal() , wm->ball->pos , 0.1); // move to knowledge

                        }
                    } else if (defenseCount == 1) {
                        target = getGoaliePositionInOneDef(wm->ball->pos , 1 , 1.5);
                    } else {
                        tempSol.append(wm->AHZOurPAreaIntersect(AZBisecOpenSeg, QString("goalKeeper")));
                        if (tempSol.size() == 1) {
                            target = tempSol.at(0);
                        } else if (tempSol.size() == 2) {
                            target = tempSol.at(0).dist(wm->ball->pos) < tempSol.at(1).dist(wm->ball->pos) ? tempSol.at(0) : tempSol.at(1);
                        }
                    }
                }
            }
            if (!wm->field->isInOurPenaltyArea(target) && defenseCount != 1) {
                tempSol.append(wm->AHZOurPAreaIntersect(Segment2D(wm->field->ourGoal() , wm->ball->pos), QString("goalKeeper")));
                if (tempSol.size() == 1) {
                    target = tempSol.at(0);
                } else if (tempSol.size() == 2) {
                    target = tempSol.at(0).dist(wm->ball->pos) < tempSol.at(1).dist(wm->ball->pos) ? tempSol.at(0) : tempSol.at(1);
                }
            }
            if (!wm->field->isInField(target) || target.x < -4.3) {
                target = know->getPointInDirection(wm->field->ourGoal() , wm->ball->pos , 0.25);
            }
        }
    }
    DBUG(QString("x: %1 , y: %2").arg(target.x).arg(target.y), D_ATOUSA);
    return target;
}

Vector2D DefensePlan::getGoaliePositionInOneDef(Vector2D _ballPos, double _limit1, double _limit2) {
    auto *tempCDef = new CDefPos();
    Vector2D goalieTarget;
    double tempBestRadius = _ballPos.dist(wm->field->ourGoal()) / 2;
    if (tempCDef->findBestRadius(tempDefPos.size) != -1) {
        if (wm->ball->pos.x < -2.5) {
            tempBestRadius = tempDefPos.pos[0].dist(wm->field->ourGoal()) - 0.5;
            DBUG(QString("radius:%1").arg(tempBestRadius), D_ATOUSA);
        } else {
            tempBestRadius = tempCDef->findBestRadius(tempDefPos.size);
        }
    }
    if (tempBestRadius > _limit2) {
        tempBestRadius = _limit2;
    } else if (tempBestRadius < _limit1) {
        tempBestRadius = _limit1;
    }

    kk2Angles tempAngles = tempCDef->getIntersections(wm->ball->pos, tempBestRadius);
    //double tempOpenAngle = fabs(tempAngles.angle2 - tempAngles.angle1);
    //drawer->draw(QString::number(tempBestRadius), Vector2D(-1, -_FIELD_HEIGHT/2 - 0.2));

    double agentAngle = tempCDef->getRobotAngle(tempBestRadius);
    drawer->draw(QString::number(agentAngle), Vector2D(-1, wm->field->_FIELD_HEIGHT / 2 - 0.4));
    //double openAngleAfterPositioning = tempOpenAngle - (agentAngle * _size);

    //draw(Circle2D(wm->field->ourGoal(), tempBestRadius), "yellow");

    if (wm->ball->pos.y + wm->ball->vel.y < 0 + goalieThr) {
        goalieTarget = tempCDef->getXYByAngle(tempAngles.angle2 - agentAngle / 2, tempBestRadius);
        goalieThr = 0.2;
        //oneDefThr = 1;
    } else {
        goalieTarget = tempCDef->getXYByAngle(tempAngles.angle1 + agentAngle / 2, tempBestRadius);
        goalieThr = -0.2;
        //oneDefThr = -1;
    }

    return goalieTarget;
}

bool DefensePlan::defenseCheckBallDangerForOneTouch() {
    //// This function checks that the ball is shot to the our goal, is danger to
    //// intercept its line or not (according the distance of ball from our goal
    //// && ball velocity)

    double defCircleRad = 0;
    Circle2D defCircle;
    if (defenseAgents.count() > 0) {
        for (int i = 0; i < defenseAgents.count(); i++) {
            if (defenseAgents[i]->pos().dist(wm->ball->pos) > defCircleRad) { //// ??????
                defCircleRad = defenseAgents[i]->pos().dist(wm->field->ourGoal());
            }
        }
    }
    if (defCircleRad > 0) {
        defCircle = Circle2D(wm->field->ourGoal(), defCircleRad);
    } else {
        defCircle = Circle2D((wm->field->ourGoal() - Vector2D(0.2, 0)), 1.50);
    }
    double ballVel = wm->ball->vel.length();
    double ballDist = wm->ball->pos.dist(wm->field->ourGoal());
    return ((ballDist < 7.5 && ballVel > 3)
            || (ballDist < 5 && ballVel > 2.5)
            || (ballDist < 4 && ballVel > 2)
            || (ballDist < 3 && ballVel > 1.5)
            || (ballDist < 2 && ballVel > 0.5)) && !defCircle.contains(wm->ball->pos);
}

void DefensePlan::calcPointForOneTouch() {
    if (wm->field->isInOurPenaltyArea(defensePoints[0]) || !wm->field->fieldRect().contains(defensePoints[0])) { ////// ?????
        Vector2D* inter2 = getIntersectWithDefenseArea(Line2D(wm->ball->pos , defensePoints[0]) , wm->ball->pos);
        if (inter2 != nullptr && inter2->valid()) {
            defensePoints[0] = *inter2;
            delete inter2;
        } else {
            drawer->draw("oneDefenseAndGoalie! ERROR1" , Vector2D(-0.1, 2.2) , "red");
        }
    }
}

QList<QPair<Vector2D, double> > DefensePlan::sortdangerpassplayoff(QList<Vector2D> oppposdanger) {
    double danger = 0;
    /////////////// Polygon
    double radius = .1;
    double treshold = 2;
    Vector2D sol1, sol2, sol3;
    Vector2D _pos1 = wm->ball->pos;

    Vector2D _pos2 = wm->ball->pos + (2 * wm->ball->vel.norm() * wm->ball->vel.length());
    Line2D _path(_pos1, _pos2);
    Polygon2D _poly;
    Circle2D(_pos2, radius + treshold).
            intersection(_path.perpendicular(_pos2), &sol1, &sol2);

    sol3 = sol1;
    _poly.addVertex(sol1);
    //sol3 = sol1;
    _poly.addVertex(sol2);
    drawer->draw(sol1);
    drawer->draw(sol2);
    Circle2D(_pos1, Robot::robot_radius_new).
            intersection(_path.perpendicular(_pos1), &sol1, &sol2);
    drawer->draw(sol1);
    drawer->draw(sol2);
    _poly.addVertex(sol2);
    _poly.addVertex(sol1);
    _poly.addVertex(sol3);

    drawer->draw(_poly, "cyan");

    double KAP = 1; //Angle parameter
    double KDBP = 1; //distancetoball
    double KDIP = 2; //distancetointersect

    double AngleP;
    double distanceToBallProjectionP;
    double distanceToIntersectP;


    double RangeofAngleP = 90;
    double RangeofdistanceToIntersectP =  radius + treshold;
    double RangeofdistanceToBallProjectionP = Segment2D(Vector2D(-1.0 * wm->field->_FIELD_WIDTH / 2, -1.0 * wm->field->_FIELD_HEIGHT / 2), Vector2D(wm->field->_FIELD_WIDTH / 2 , wm->field->_FIELD_HEIGHT / 2)).length();
    double danger2 = 0;


    /////////////////////


    double KA = 1; //Angle Coefficient
    double KDB = 0; //Distance To Ball
    double KDG = 1; //Distnce To Goal
    double RangeofAngle = Vector2D::angleOf(wm->field->ourGoalR(), Vector2D(-1.0 * (wm->field->_FIELD_WIDTH / 2 - wm->field->_PENALTY_DEPTH), 0), wm->field->ourGoalL()).degree();
    //drawer->draw(Vector2D(-1.0 * (_FIELD_WIDTH - _GOAL_WIDTH), 0), QColor(Qt::red));
    // double RangeofAngle2 = Vector2D::angleOf(wm->field->ou,Vector2D(0, -1.0 * (_FIELD_WIDTH - _GOAL_WIDTH)), wm->field->ourGoalL()).degree();

    double RangeofDistancetoBall = fabs(Segment2D(Vector2D(wm->field->_FIELD_WIDTH / 2, wm->field->_FIELD_HEIGHT / 2), Vector2D(-1.0 * wm->field->_FIELD_WIDTH / 2, -1.0 * wm->field->_FIELD_HEIGHT / 2)).length());

    double RangeofDistancetoGoal = fabs(Segment2D(Vector2D(wm->field->_FIELD_WIDTH / 2, wm->field->_FIELD_HEIGHT / 2), wm->field->ourGoal()).length());

    //double RangeofTempDis = 2;
    double angle, distancetoball, distancetogoal, danger1  = 0;


    QPair<Vector2D, double> temp;
    QList<QPair<Vector2D, double> > output;
    double Polycontain = 0;
    for (int i = 0; i < oppposdanger.count(); i++) {
        if (_poly.contains(oppposdanger[i])) {
            drawer->draw(Circle2D(oppposdanger[i] , 0.3) , "black");
            Polycontain = 1;
        } else {
            Polycontain = 0;
        }
        temp.first = oppposdanger[i];


        angle = Vector2D::angleOf(wm->field->ourGoalR(), oppposdanger[i], wm->field->ourGoalL()).degree();
        distancetoball = (oppposdanger[i] - wm->ball->pos).length();
        distancetogoal = (oppposdanger[i] - wm->field->ourGoal()).length();

        ////poly

        AngleP = Vector2D::angleOf(oppposdanger[i], wm->ball->pos, _path.projection(oppposdanger[i])).degree();
        distanceToIntersectP = _path.dist(oppposdanger[i]); //distanse of opponent to the path
        distanceToBallProjectionP = (wm->ball->pos - _path.projection(oppposdanger[i])).length(); //distance of the ball to the projection of opponent to the path

        danger1 = (KA * fabs(angle) / RangeofAngle) + (KDB * (1 - (distancetoball / RangeofDistancetoBall))) + (KDG * (1 - (distancetogoal / RangeofDistancetoGoal)));
        danger2 = KAP * (1 - AngleP / RangeofAngleP) + KDBP * (1 - distanceToBallProjectionP / RangeofdistanceToBallProjectionP) + KDIP * (1 - distanceToIntersectP / RangeofdistanceToIntersectP);
        // DBUG(QString("angle: %1, rangeofangle: %2, distansetoball:%3, RangeofDistancetoBall:%4,distancetogoal:%5,rangeofdistansetogoal:%6").arg(angle).arg(RangeofAngle).arg(distancetoball).arg(RangeofDistancetoBall).arg(distancetogoal).arg(RangeofDistancetoGoal),D_HAMED);
        // DBUG(QString("angleP: %1, rangeofangleP: %2, distansetoballProjectionP:%3, RangeofDistancetoBallProjectionP:%4,distancetointersect:%5,rangeofdistansetointesrsect:%6").arg(AngleP).arg(RangeofAngleP).arg(distanceToBallProjectionP).arg(RangeofdistanceToBallProjectionP).arg(distanceToIntersectP).arg(RangeofdistanceToIntersectP),D_HAMED);
        if (wm->ball->vel.length() < .1) {
            danger = danger1;
        } else {
            danger = 100.0 * Polycontain + fabs(danger2) + fabs(danger1);
        }

        temp.second = danger;
        output.append(temp);
        //        drawer->draw(QString("HMD danger=%1").arg(danger), oppposdanger[i] + Vector2D(0, 0.3), QColor(Qt::red));
        //drawer->draw(_poly, QColor(Qt::blue));



        //        drawer->draw(QString("mindistance%1").arg(mintempdis), oppposdanger[i] + Vector2D(0,0.5), QColor(Qt::blue));
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

QList<QPair<Vector2D, double> > DefensePlan::sortdangerpassplayon(QList<Vector2D> oppposdanger) {

    double KA = 1; //Angle Coefficient
    double KDB = 1;  //Distance To Ball
    double KDG = 1;  //Distnce To Goal
    double RangeofAngle = Vector2D::angleOf(wm->field->ourGoalR(),
                                            Vector2D(-1.0 * (wm->field->_FIELD_WIDTH / 2 - wm->field->_PENALTY_DEPTH), 0),
                                            wm->field->ourGoalL()).degree();
    //drawer->draw(Vector2D(-1.0 * (_FIELD_WIDTH - _GOAL_WIDTH), 0), QColor(Qt::red));
    // double RangeofAngle2 = Vector2D::angleOf(wm->field->ou,Vector2D(0, -1.0 * (_FIELD_WIDTH - _GOAL_WIDTH)), wm->field->ourGoalL()).degree();

    double RangeofDistancetoBall = fabs(Segment2D(Vector2D(wm->field->_FIELD_WIDTH / 2, wm->field->_FIELD_HEIGHT / 2),
                                                  Vector2D(-1.0 * wm->field->_FIELD_WIDTH / 2,
                                                           -1.0 * wm->field->_FIELD_HEIGHT / 2)).length());

    double RangeofDistancetoGoal = fabs(Segment2D(Vector2D(wm->field->_FIELD_WIDTH / 2, wm->field->_FIELD_HEIGHT / 2),
                                                  wm->field->ourGoal()).length());

    double RangeofTempDis = 2;
    double angle, distancetoball, distancetogoal, danger;


    QPair<Vector2D, double> temp;
    QList<QPair<Vector2D, double> > output;
    for (int i = 0; i < oppposdanger.count(); i++) {
        temp.first = oppposdanger[i];


        angle = Vector2D::angleOf(wm->field->ourGoalR(), oppposdanger[i], wm->field->ourGoalL()).degree();
        distancetoball = (oppposdanger[i] - wm->ball->pos).length();
        distancetogoal = (oppposdanger[i] - wm->field->ourGoal()).length();
        danger = (KA * fabs(angle) / RangeofAngle) + (KDB * 1 - (distancetoball / RangeofDistancetoBall)) +
                (KDG * 1 - (distancetogoal / RangeofDistancetoGoal));


        temp.second = danger;
        output.append(temp);
        //drawer->draw(QString("HMD danger=%1").arg(danger), oppposdanger[i] + Vector2D(0,0.3), QColor(Qt::red));


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


        //        drawer->draw(QString("mindistance%1").arg(mintempdis), oppposdanger[i] + Vector2D(0,0.5), QColor(Qt::blue));




    }

    ///sorting the Qlist
    for (int i = 0; i < output.count(); i++) {
        for (int j = 0; j < output.count() - 1; j++) {
            if (output[j].second < output[j + 1].second) {
                output.swap(j, j + 1);
            }
        }
    }

    //for(int i=0; i<output.count(); i++)
    //{
    //drawer->draw(QString("HMD Danger New%1" ).arg(output[i].second),output[i].first + Vector2D(0,.2),QColor(Qt::red));
    //}

    return output;
}

Vector2D DefensePlan::getMarkPlayoffPredictWaitPos(){
    if(!know->variables["transientFlag"].toBool())
        beforeTransientPassDir = wm->opp[know->nearestOppToBall()]->dir;
    else{
        Vector2D sol1,sol2;
        if(ballIsBounced){
             int solNum = wm->field->ourBigPenaltyArea(1,Robot::robot_radius_new,0).
                                            intersection(Line2D(ballBouncePos,playOffStartBallPos),&sol1,&sol2);
        if(solNum == 2)
           return playOffStartBallPos.dist(sol1) > playOffStartBallPos.dist(sol2) ? sol1 : sol2;
        }
        else{
            int solNum = wm->field->ourBigPenaltyArea(1,Robot::robot_radius_new,0).
                        intersection(Line2D(playOffStartBallPos,playOffStartBallPos + ((playOffPassDir+beforeTransientPassDir)/2)),&sol1,&sol2);
        if(solNum == 2)
            return playOffStartBallPos.dist(sol1) > playOffStartBallPos.dist(sol2) ? sol1 : sol2;
        }
        return Vector2D(5000,5000);
    }
}
