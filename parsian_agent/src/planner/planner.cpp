#include <planner/planner.h>

CPlanner::CPlanner(int _ID) {
    obst.clear();
    threshold = conf->Target_Distance_Threshold;
    wayPointProb = conf->Waypoint_Catch_Probablity;
    goalProb = conf->Goal_Probablity;
    stepSize = conf->Extend_Step;
    goal.invalidate();
    counter = 0;
    flag = false;
    pointStep = 0.04;
    this->ID = _ID;
    readyToPlan = false;
    qRegisterMetaType< vector<Vector2D> >("vector<Vector2D>");
    qRegisterMetaType< Vector2D >("Vector2D");
    qRegisterMetaType< QList<int> >("QList<int>");
}

CPlanner::~CPlanner() = default;

Vector2D CPlanner::chooseTarget(Vector2D &GOAL) {
    double p = drand48();
    unsigned long idx;
    if (p <= goalProb) {
        return GOAL;
    } else if (! result.empty() && goalProb < p && p < wayPointProb + goalProb) {
        idx = rand() % result.size();
        return result[idx];
    } else {
        // random state is in a big field (1 meter larger from each side)
        Vector2D a;
        a.assign((2.0 * drand48() - 1.0) * (field._FIELD_WIDTH + 2) / 2.0 , (2.0 * drand48() - 1.0) * (field._FIELD_HEIGHT + 2) / 2.0);
        return a;
    }
}

state *CPlanner::extendTree(state *&NEAREST , Vector2D &GOAL , Vector2D &TARGET , vector<Vector2D> &res) {
    Vector2D temp;
    if (NEAREST->pos.dist(GOAL) < stepSize) {
        temp = GOAL;
    } else {
        temp = NEAREST->pos + (TARGET - NEAREST->pos).norm() * stepSize;
    }

    if (obst.check(temp , NEAREST->pos)) {
        //    if( NEAREST->parent == nullptr || fabs(Vector2D::angleBetween(NEAREST->pos - NEAREST->parent->pos , temp - NEAREST->pos ).degree()) < 20 )
        return new state(temp , NEAREST , res);
    } else if (NEAREST->depth < 4 &&  obst.check(temp)) {
        //    if( NEAREST->parent == nullptr || fabs(Vector2D::angleBetween(NEAREST->pos - NEAREST->parent->pos , temp - NEAREST->pos ).degree()) < 20 )
        return new state(temp , NEAREST , res);
    }
    return nullptr;
}

bool CPlanner::validState(state *&node) {
    return node && node->pos.x >= (-(field._FIELD_WIDTH / 2 + 1)) && node->pos.x <= ((field._FIELD_WIDTH / 2 + 1)) \
           && node->pos.y >= (-(field._FIELD_HEIGHT / 2 + 1)) && node->pos.y <= ((field._FIELD_HEIGHT / 2 + 1));
}


void CPlanner::checkAgent() {
    //// check whether start-point is inside an obstacle or not!
    QList<int> obsid;
    if (!obst.check(Rgoal , obsid)) {
        for (int i = 0; i < obsid.count(); i++) {
            double lastRad = obst.obs[obsid.at(i)].rad.x;
            double newRad = obst.obs[obsid.at(i)].pos.dist(Rgoal) - 0.03;
            obst.obs[obsid.at(i)].rad.x = lastRad < newRad ? lastRad : newRad;
        }
    }

    //// check whether target-point is inside an obstacle or not!
    obsid.clear();
    if (!obst.check(goal , obsid)) {
        for (int i = 0; i < obsid.count(); i++) {
            //// except penalty area (((( DOUBLE DEFENDE ))))
            if (obst.obs[obsid.at(i)].pos.dist(Vector2D(-3.20 , 0)) < 0.1 && fabs(obst.obs[obsid.at(i)].rad.x - 1.1) < 0.2) {
                continue;
            }

            //// except center circle
            if (obst.obs[obsid.at(i)].pos.dist(Vector2D(0 , 0)) < 0.1 && fabs(obst.obs[obsid.at(i)].rad.x - 0.5) < 0.1) {
                continue;
            }

            double lastRad = obst.obs[obsid.at(i)].rad.x;
            double newRad = obst.obs[obsid.at(i)].pos.dist(goal) - 0.07;
            obst.obs[obsid.at(i)].rad.x = lastRad < newRad ? lastRad : newRad;
        }
    }
}


void CPlanner::runPlanner() {
    checkAgent();

    if (nodes.head == nullptr) {
        result.clear();
        Rresult.clear();
        averageDir.assign(0, 0);
        return;
    }

    //if (obst.check(Rgoal,goal)) {
      //  result.clear();
       // Rresult.clear();
       // averageDir.assign(0, 0);
    //    return;
   // }


    state *nearestToGoal = nullptr , *nearest = nullptr;
    Vector2D target;
    state *RnearestToGoal = nullptr , *Rnearest = nullptr;
    Vector2D Rtarget;
    state *middleNode = nullptr;
    double RnearestDist , nearestDist;
    int nnn = 0 , Rnnn = 0;
    bool marginFlag = true , RmarginFlag = true;


    obst.obsMargin = Robot::robot_radius_new + 0.03;

    nearestToGoal = nodes.findNearest(goal);
    nearestDist = nearestToGoal->pos.dist(goal);
    while (nearestDist + EPSILON > threshold
            && nodes.size() < _PLANNER_EXTEND_POINT_LIMIT
            && nnn < _PLANNER_EXTEND_MAX_ATTEMPT) {

        target = chooseTarget(goal);

        nearest = nodes.findNearest(target);

        state *newNode = extendTree(nearest , goal , target , result);

        if (validState(newNode) && nodes.findNearest(newNode->pos)->pos.dist(newNode->pos) > threshold) {
            marginFlag = true;
            obst.obsMargin = Robot::robot_radius_new + 0.03;
            nnn = 0;
            nearest->next = newNode;
            nodes.add(newNode);

            if (nearestDist + EPSILON > newNode->pos.dist(goal)) {
                nearestToGoal = newNode;
                nearestDist = nearestToGoal->pos.dist(goal);
            }
        } else {
            delete newNode;
            nnn++;
        }

        if (nnn == _PLANNER_EXTEND_MAX_ATTEMPT && marginFlag) {
            marginFlag = false;
            obst.obsMargin = 0.05;
            nnn = 0;
        }
    }

    obst.obsMargin = Robot::robot_radius_new + 0.03;

    if (Rnodes.head && nearestDist + EPSILON > threshold) {
        RnearestToGoal = Rnodes.findNearest(Rgoal);
        RnearestDist = RnearestToGoal->pos.dist(Rgoal);
        while (RnearestDist + EPSILON > threshold && Rnodes.size() < _PLANNER_EXTEND_POINT_LIMIT && Rnnn < _PLANNER_EXTEND_MAX_ATTEMPT) {

            Rtarget = chooseTarget(Rgoal);

            Rnearest = Rnodes.findNearest(Rtarget);

            state *newNode = extendTree(Rnearest , Rgoal , Rtarget , Rresult);

            if (validState(newNode) && Rnodes.findNearest(newNode->pos)->pos.dist(newNode->pos) > threshold) {
                RmarginFlag = true;
                obst.obsMargin = Robot::robot_radius_new + 0.03;
                Rnnn = 0;
                Rnearest->next = newNode;
                Rnodes.add(newNode);

                if (RnearestDist + EPSILON > newNode->pos.dist(Rgoal)) {
                    RnearestToGoal = newNode;
                    RnearestDist = RnearestToGoal->pos.dist(Rgoal);
                }
            } else {
                delete newNode;
                Rnnn++;
            }
            if (Rnnn == _PLANNER_EXTEND_MAX_ATTEMPT && RmarginFlag) {
                RmarginFlag = false;
                obst.obsMargin = 0.05;
                Rnnn = 0;
            }
        }
    }

    obst.obsMargin = Robot::robot_radius_new + 0.03;

    vector <Vector2D> temp;
    state *lll = nullptr;

    if (nearestToGoal->pos.dist(goal) + EPSILON < threshold) {
        temp.clear();
        while (nearestToGoal) {
            temp.push_back(nearestToGoal->pos);
            nearestToGoal = nearestToGoal->parent;
        }
    } else if (RnearestToGoal && RnearestToGoal->pos.dist(Rgoal) + EPSILON < threshold) {
        lll = nullptr;
        while (RnearestToGoal) {
            nearestToGoal = new state(RnearestToGoal->pos , lll , result);
            Rnodes.add(nearestToGoal);
            lll = nearestToGoal;
            RnearestToGoal = RnearestToGoal->parent;
        }

        temp.clear();
        while (nearestToGoal) {
            temp.push_back(nearestToGoal->pos);
            nearestToGoal = nearestToGoal->parent;
        }
        /*if( CProfiler::getTime()*1000.0 - drawTimer*1000.0 > 100.0 )
        draw("Target" , Vector2D(0.3,-2.2) , "red");*/
    } else {
        bool found = false;
        state *copyNearest = nearestToGoal;
        double mn = 1000;
        state *one = nullptr, *two = nullptr;
        for (int i = 0 ; i < nodes.size() ; i++) {
            for (int j = 0 ; j < Rnodes.size() ; j++) {
                middleNode = Rnodes.allNodes[j];
                nearestToGoal = nodes.allNodes[i];
                if (obst.check(middleNode->pos , nearestToGoal->pos) && \
                        ((result.size() && middleNode->dist + nearestToGoal->dist < mn) || (result.size() == 0 && middleNode->dist + nearestToGoal->dist + middleNode->pos.dist(nearestToGoal->pos) < mn))) {
                    if (result.size()) {
                        mn = (middleNode->dist + nearestToGoal->dist);
                    } else {
                        mn =  middleNode->dist + nearestToGoal->dist + middleNode->pos.dist(nearestToGoal->pos);
                    }
                    one = middleNode;
                    two = nearestToGoal;

                    found = true;
                }
            }
        }
        if (std::fabs(mn - 1000.0) > EPSILON && one != nullptr && two != nullptr) {
            auto num = static_cast<int>((one->pos.dist(two->pos)) / stepSize);
            for (int i = 1 ; i < num ; i++) {
                two = new state(two->pos + (one->pos - two->pos).norm()*stepSize , two , result);
                nodes.add(two);
            }
            while (one) {
                two = new state(one->pos , two , result);
                nodes.add(two);
                one = one->parent;
            }
            nearestToGoal = two;
        }

        if (! found) {
            nearestToGoal = copyNearest;
            flag = true;
        }

        temp.clear();
        while (nearestToGoal) {
            temp.push_back(nearestToGoal->pos);
            nearestToGoal = nearestToGoal->parent;
        }
    }

    reverse(temp.begin() , temp.end());
    result.assign(temp.begin() , temp.end());
    Rresult.assign(result.begin() , result.end());
    reverse(Rresult.begin() , Rresult.end());


    resultModified.clear();
    RresultModified.clear();
    //////////////////////////////////////////////////path smoothing
    if (temp.size() > 1) {
        resultModified.push_back(std::move(temp.back()));
    } else {
        resultModified.push_back(target);
    }

    auto compNodeNum = static_cast<int>(temp.size() - 1);
    for (int i = ((int)(temp.size())) - 1 ; i > 1 ; i--) {
        if (i < compNodeNum) {
            for (int k = 0 ; k < i ; k ++) {
                if (obst.check(temp[i], temp[k])) {
                    resultModified.push_back(temp[k]);
                    //draw(temp[k]);
                    compNodeNum = k;
                    break;
                }
                if ((k == i - 1)) {
                    resultModified.push_back(temp[k]);
                    //draw(temp[k]);
                    compNodeNum = k;
                }
            }
        }
    }

    if (resultModified.size() == 1) {
        resultModified.push_back(temp[0]);
    }

    //////////////////////////////////////////////////////
    RresultModified.assign(resultModified.begin() , resultModified.end());
    reverse(RresultModified.begin() , RresultModified.end());

    // miangin vazn dar, az direction harkat
//    Vector2D newDir;
//    if (resultModified.size() > 1) {
//        newDir = (resultModified[1] - resultModified[0]).norm();
//    } else {
//        newDir = (goal - Rgoal).norm();
//    }
//    int cntcnt = 8;
//    int vd = 1;
//    if (dirs.count() > cntcnt - 1) {
//        averageDir = (averageDir * (cntcnt + (vd - 1)) - dirs.at(0) - (vd - 1) * dirs.last() + vd * newDir) / (cntcnt + (vd - 1));
//        dirs.pop_front();
//    } else {
//        if (dirs.count()) {
//            averageDir = (averageDir * (dirs.count() + (vd - 1)) - (vd - 1) * dirs.last() + vd * newDir) / (dirs.count() + vd);
//        } else {
//            averageDir = newDir;
//        }
//    }
//    dirs.push_back(newDir);

    if (conf->Draw_Path) {
        for (int j = 1; j < resultModified.size(); j++) {
            drawer->draw(Segment2D(resultModified[j - 1], resultModified[j]), QColor(
                             static_cast<int>(255 / (resultModified.size() / (double) j)),
                             static_cast<int>(255 / (resultModified.size() / (double) j)),
                             static_cast<int>(255 / (resultModified.size() / (double) j))));
            drawer->draw(result[j]);
        }
        //Draw();
        obst.draw();
    }

}

void CPlanner::Draw() {


//    for (int i = 0 ; i < nodes.size() ; i++)
//        if (nodes.allNodes[i]->parent) {
//            drawer->draw(Segment2D(nodes.allNodes[i]->pos , nodes.allNodes[i]->parent->pos) , "white");
//        }

    for (int i = 0 ; i < Rnodes.size() ; i++)
        if (Rnodes.allNodes[i]->parent) {
            drawer->draw(Segment2D(Rnodes.allNodes[i]->pos , Rnodes.allNodes[i]->parent->pos) , "gray");
        }

}

void CPlanner::resetPlanner(Vector2D _goal) {
    if (result.empty() || lastGoal.dist(_goal) > 0.05 || (result[result.size() - 1].dist(lastGoal) > 10 * threshold)) {
        //    if( result.size() == 0 )
        //      qDebug() << "RESULT.SIZE()";
        //    else if( lastGoal.dist(_goal) > 0.05 ){
        //      qDebug() << lastGoal.x << lastGoal.y << "-------" << _goal.x << _goal.y;
        //      qDebug() << "GOAL";
        //    }
        //    else if( result[result.size()-1].dist(lastGoal) > threshold ){
        //      qDebug() << "THRESHOLD";
        //    }

        result.clear();
        Rresult.clear();
        averageDir.assign(0, 0);
        dirs.clear();
        /*draw("NO WAYPOINTS" , Vector2D(-0.2,-2.2) , "red");*/
    } else {
        //    for( int i=0 ; i<2 && result.size() ; i++ )
        //      result.erase(result.begin()+0);
        //    for( int i=0 ; i<result.size() ; i++ )
        //      draw(result[i] , 0 , "red");
    }

    nodes.removeAll();
    Rnodes.removeAll();
}


void CPlanner::initPathPlanner(Vector2D _goal, const QList<int>& _ourRelaxList, const QList<int>& _oppRelaxList , const bool& _avoidPenaltyArea , const bool& _avoidCenterArea , const double& _ballObstacleRadius) {

    int idx = ID;

    if (wm->our[idx]->pos.dist(_goal) < 0.1) {
        resultModified.clear();
        resultModified.push_back(wm->our[idx]->pos);
        resultModified.push_back(_goal);
        averageDir = _goal - wm->our[idx]->pos;
        emitPlan(resultModified , averageDir);
        return;

    }

    if (! wm->field->marginedField().contains(_goal)) {
        auto *sol1 = new Vector2D() , *sol2 = new Vector2D();
        wm->field->marginedField().intersection(Segment2D(Vector2D(0, 0) , _goal) , sol1 , sol2);
        if (sol1->valid()) {
            _goal = *sol1;
        } else {
            _goal = Vector2D(0, 0);
        }
    }

    resetPlanner(_goal);
    lastGoal = goal;
    goal = _goal;
    Rgoal = wm->our[idx]->pos;
    avoidPenaltyArea = _avoidPenaltyArea;
    avoidCenterArea = _avoidCenterArea;

    ourRelaxList.clear();
    for (int i : _ourRelaxList) {
        ourRelaxList.append(i);
    }
    ourRelaxList.append(ID);


    oppRelaxList.clear();
    for (int i : _oppRelaxList) {
        oppRelaxList.append(i);
    }

    ballObstacleRadius = _ballObstacleRadius;
    stepSize     = conf->Extend_Step;
    threshold    = conf->Target_Distance_Threshold;
    wayPointProb = conf->Waypoint_Catch_Probablity;
    if (! result.empty()) {
        goalProb = conf->Goal_Probablity;
    } else {
        goalProb = 0.5;
    }

    robotVel = wm->our[idx]->vel;

    nodes.first = new state((wm->our[idx]->pos + wm->our[idx]->vel * 0.09) , nullptr , result);
    nodes.add(nodes.first);

    Rnodes.first = new state(_goal , nullptr , Rresult);
    Rnodes.add(Rnodes.first);
    counter++;
    readyToPlan = true;
}

double CPlanner::timeEstimator(Vector2D _pos, Vector2D _vel, Vector2D _dir, Vector2D posT) {

    double _x3;
    double acc = conf->AccMaxForward;
    double dec = conf->DecMax;
    double xSat;
    Vector2D tAgentVel = _vel;
    Vector2D tAgentDir = _dir;
    double veltan = (tAgentVel.x) * cos(tAgentDir.th().radian()) + (tAgentVel.y) * sin(tAgentDir.th().radian());
    double offset = 0;
    double velnorm = -1 * (tAgentVel.x) * sin(tAgentDir.th().radian()) + (tAgentVel.y) * cos(tAgentDir.th().radian());
    double distCoef = 1, distEffect = 1, angCoef = 0.003;
    double dist = 0;
    double rrtAngSum = 0;
    QList <Vector2D> _result;
    Vector2D _target;

    double tAgentVelTanjent =  tAgentVel.length() * cos(Vector2D::angleBetween(posT - _pos , _vel.norm()).radian());
    double vXvirtual = (posT - _pos).x;
    double vYvirtual = (posT - _pos).y;
    double veltanV = (vXvirtual) * cos(tAgentDir.th().radian()) + (vYvirtual) * sin(tAgentDir.th().radian());
    double velnormV = -1 * (vXvirtual) * sin(tAgentDir.th().radian()) + (vYvirtual) * cos(tAgentDir.th().radian());
    double accCoef = 1, realAcc = 4;

    accCoef = atan(fabs(veltanV) / fabs(velnormV)) / _PI * 2;
    acc = accCoef * conf->AccMaxForward + (1 - accCoef) * conf->AccMaxNormal;

    double tDec = conf->VelMax / dec;
    double tAcc = (conf->VelMax - tAgentVelTanjent) / acc;
    dist = posT.dist(_pos);
    double dB = tDec * conf->VelMax / 2 + tAcc * (conf->VelMax + tAgentVelTanjent) / 2;

    if (dist > dB) {
        return tAcc + tDec + (dist - dB) / conf->VelMax;
    } else {
        return ((1 / dec) + (1 / acc)) * sqrt(dist * (2 * dec * acc / (acc + dec)) + (tAgentVelTanjent * tAgentVelTanjent / (2 * acc))) - (tAgentVelTanjent) / acc;
    }



}

void CPlanner::createObstacleProb(CObstacles &obs, Vector2D _pos, Vector2D _vel, Vector2D _ang, Vector2D &_center,
                                  double &_rad, Vector2D agentPos, Vector2D agentVel, Vector2D agentGoal, Vector2D agentDir) {
    Segment2D obstaclePath;
    Vector2D intersectPoint;
    double timeForObs = 0;
    ///TODO: should read from vartypes
    double maxA = 4;
    double maxObstRad = 0.4;
    double maxTime = 0.2;
    if (_vel.length() < 0.2) {
        _center = _pos;
        _rad = Robot::robot_radius_new;
    } else {
        obstaclePath.assign(_pos, _pos + _vel.norm() * 10);
        intersectPoint = agentPath.intersection(obstaclePath);
        if (intersectPoint.isValid()) {
            timeForObs  = timeEstimator(agentPos, agentVel, agentDir, agentGoal);
            timeForObs *= agentPos.dist(intersectPoint) / agentPos.dist(agentGoal);
            timeForObs *= 1;
            timeForObs  = min(maxTime, timeForObs);
            for (double i = 0; i < maxTime ; i += 0.1) {

                timeForObs += i;
                _center = _pos + _vel * timeForObs;
                _rad = maxA * timeForObs * timeForObs ;

                _rad = min(maxObstRad, _rad);
                _rad = min(agentPos.dist(_center) - 0.3, _rad);
                _rad = min(_pos.dist(_center) - 0.3 , _rad);
                _rad += Robot::robot_radius_new;
                if (timeForObs >= 0) {
                    obs.add_circle(_center.x , _center.y , _rad , 0 , 0);
                }
            }
        } else {
            _center = _pos;
            _rad = Robot::robot_radius_new;
        }
    }

    obs.add_circle(_center.x , _center.y , _rad , 0 , 0);
}

void CPlanner::generateObstacleSpace(CObstacles &obs, QList<int> &ourRelaxList, QList<int> &oppRelaxList, bool avoidPenaltyArea, bool avoidCenterCircle , double ballObstacleRadius, Vector2D agentGoal) {

    obs.clear();
    obs.targetPosition = goal;

    Vector2D agentPos;

    Vector2D agentVel;
    agentPos = wm->our[ID]->pos;
    agentVel = wm->our[ID]->vel;
    obs.agentPos = agentPos;

    agentPath.assign(agentPos, agentGoal);
    Vector2D _center , dummy1, dummy2;
    double rad = 0;
    //TODO : command vel bug :!!
    ROS_INFO_STREAM("OUR" << wm->our.activeAgentsCount());
    for (int j = 0; j < wm->our.activeAgentsCount(); j++) {
        if ((ourRelaxList.contains(wm->our.active(j)->id) == false) && (ID != wm->our.active(j)->id)) {
            drawer->draw(wm->our.active(j)->vel);
            createObstacleProb(obs, wm->our.active(j)->pos, wm->our.active(j)->vel, Vector2D(0, 0), _center, rad, agentPos, agentVel, agentGoal, Vector2D(1, 1));
            obs.add_circle(wm->our.active(j)->pos.x , wm->our.active(j)->pos.y , 0.17 , 0 , 0);

        }
    }

    ROS_INFO_STREAM("OPP" << wm->opp.activeAgentsCount());
    for (int j = 0; j < wm->opp.activeAgentsCount(); j++) {
        if (oppRelaxList.contains(wm->opp.active(j)->id) == false) {

            createObstacleProb(obs, wm->opp.active(j)->pos, wm->opp.active(j)->vel, Vector2D(0, 0), _center, rad, agentPos, agentVel, agentGoal, Vector2D(1, 1));
            double obsRad = std::min(wm->opp.active(j)->pos.dist(wm->our[ID]->pos) - 0.02, 0.2);
            DBUG(QString("obsRad : %1 ID : %2").arg(obsRad).arg(j), D_MHMMD);
            obs.add_circle(wm->opp.active(j)->pos.x , wm->opp.active(j)->pos.y , 0.17 , 0 , 0);
        }
    }

    if (ballObstacleRadius > EPSILON) {
        obs.add_circle(wm->ball->pos.x, wm->ball->pos.y, ballObstacleRadius, wm->ball->vel.x , wm->ball->vel.y);
    }

    if (avoidPenaltyArea) {
        /////////////// Penalty area obstacle //////////////////////////////////////
        obs.add_rectangle_from_center(-1 * (field._FIELD_WIDTH / 2) + field._PENALTY_DEPTH/2, 0 , wm->field->_PENALTY_DEPTH, wm->field->_PENALTY_WIDTH);
        obs.add_rectangle_from_center((field._FIELD_WIDTH / 2) - field._PENALTY_DEPTH/2, 0 , wm->field->_PENALTY_DEPTH, wm->field->_PENALTY_WIDTH);
    }

    //obs.add_rectangle(0,0,0.5,0.5);
    if (avoidCenterCircle) {
        obs.add_circle(0 , 0 , field._CENTER_CIRCLE_RAD , 0 , 0);
    }

    if (!obs.check(goal)) {
        drawer->draw(Vector2D(0, 0));
    }


}

vector<Vector2D> CPlanner::getResultModified() {
    return resultModified;
}
Vector2D CPlanner::getAverageDir() {
    return  averageDir;
}

int CPlanner::getID() {
    return ID;
}

void CPlanner::emitPlan(const vector<Vector2D>& _resultModified, const Vector2D& averageDir) {
    parsian_msgs::parsian_pathPtr path{new parsian_msgs::parsian_path};
    path->averageDir = averageDir.toParsianMessage();
    for (const auto& v : _resultModified) {
        path->results.push_back(v.toParsianMessage());
    }
    path->header.stamp = ros::Time::now();
    path_pub.publish(path);
}

void CPlanner::run() {
    if (wm->our.data->activeAgents.contains(ID)) {
        if (readyToPlan) {
            generateObstacleSpace(obst, ourRelaxList, oppRelaxList, avoidPenaltyArea, avoidCenterArea,
                                  ballObstacleRadius, goal);
            runPlanner();
            emitPlan(getResultModified(), getAverageDir());
            readyToPlan = false;
        }
    }
}

