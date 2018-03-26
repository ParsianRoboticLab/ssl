#include <parsian_ai/plans/positioningplan.h>

#define INF 1000000000

//TODOs:
//  1)wm [SOLVED]
//  2)editdata ==> must transform the class
//  3)waithere ==> must be implant with message
//  4)positionIntent
//  5)knowledge & Cknowledge [SOLVED]
//  6)gpa  [SOLVED]
//  7)_FIELD_WIDTH [SOLVED]
//  8)knowledge->getGameMode [SOLVED]


//DOUBTs:
//  1)qswap instead of swap

PositioningPlan::PositioningPlan() {
    for (int i = 0; i < _MAX_NUM_PLAYERS ; i++) {
        gpa[i] = new GotopointavoidAction();
        gpa[i]->setNoavoid(false);
        //gpa[i]->setOneTouchMode(true);
        gpa[i]->setAvoidpenaltyarea(true);

        gps[i] = new GotopointAction();
        //gps[i]->setBallmode(false);
        gps[i]->setSlowmode(false);
        //gps[i]->setSlowShot(false);
        //gps[i]->setTurningDist( 0.1);
    }
}

void PositioningPlan::init(const QList<Agent*> & _agents , QString playMode) {

    dynamicPositioners.clear();
    staticPositioners.clear();
    agents.clear();
    agents.append(_agents);
    posCount = 0;

    double minDist = 1000.0;
    int minID = -1;
    Vector2D temp;

    if (playMode == "OurFreeKick6" && minID != -1) {
        for (int i = minID ; i < posCount - 1 ; i++) {
            rolesPositions[i] = rolesPositions[i + 1];
        }
        posCount--;
    }

    for (int i = 0 ; i < agents.size() ; i++) {
        dynamicPositioners.append(i);
    }
}

void PositioningPlan::staticExec() {

}

void PositioningPlan::staticInit(QList< holdingPoints > &_staticPoints) {

}

bool PositioningPlan::isDifferentSequence(QList<Vector2D> first, QList<Vector2D> second) {
    if (first.size() != second.size()) {
        return true;
    }
    for (int i = 0 ; i < first.size() ; i++) {
        if (first.at(i).dist(second.at(i)) > 1e-2) {
            return true;
        }
    }
    return false;
}

Vector2D PositioningPlan::findFacePoint(positioningType type) {
    switch (type) {
    case positioningType::ONETOUCH://TODO it's pedar darar
        break;
    case positioningType::TOBALL:
        return wm->ball->pos;
        break;
    case positioningType::TOOPPGOAL:
        return wm->field->oppGoal();
        break;
    case positioningType::TOOURGOAL:
        return wm->field->ourGoal();
        break;
    }
    return Vector2D::INVALIDATED;
}

void PositioningPlan::execute() {

//  debugAgents("Position:");

    staticExec();

    int scores[_MAX_NUM_PLAYERS][_MAX_NUM_PLAYERS];
    for (int i = 0 ; i < _MAX_NUM_PLAYERS ; i++) {
        for (int j = 0 ; j < _MAX_NUM_PLAYERS ; j++) {
            scores[i][j] = -32000;
        }
    }

    for (int i = 0 ; i < posCount ; i++) {
        for (int j = 0 ; j < dynamicPositioners.count() ; j++) {
            scores[i][j] = -(int)(agent(dynamicPositioners.at(j))->pos().dist(rolesPositions[i]) * 100.0);
            for (int k = 0 ; k < staticPositioners.count() ; k++) {
                scores[i][j] += (int)(rolesPositions[i].dist(lastStaticPositioningTargets[k]) * 100.0);
            }
            if (last_match[j] == i) {
                scores[i][j] += 1;
            }
        }
    }

    getBipartiteGraph(posCount, dynamicPositioners.count(), scores);

    double score;
    Vector2D tempTarget;
    double max_score[_MAX_NUM_PLAYERS];
    Vector2D finalTarget[_MAX_NUM_PLAYERS];
    bool doNotSearch = gameState->ourPenaltyKick() ||
                       gameState->theirPenaltyKick();


    for (int i = 0 ; i < dynamicPositioners.count() ; i++) {
        max_score[i] = PositioningObject(agent(dynamicPositioners.at(i))->id(), rolesPositions[match[i]], rolesPositions[match[i]],
                                         ((positioningTargets[i].isValid()) ? positioningTargets[i] : rolesPositions[match[i]])).Score();
        finalTarget[i] = rolesPositions[match[i]];
//        debug(QString("i: %1, target:(%2,%3)").arg(agent(i)->id()).arg(finalTarget[i].x).arg(finalTarget[i].y),D_ERROR,"red");
        if (match[i] > -1 && !doNotSearch) {
            for (double dist = 0.2 ; dist <= 0.8 ; dist += 0.2) {
                for (double ang = -180.0 ; ang <= 180.0 ; ang += 60.0) {
                    tempTarget = rolesPositions[match[i]] + Vector2D::polar2vector(dist, ang);
                    if (isValidPoint(tempTarget)) {
                        score = PositioningObject(agent(dynamicPositioners.at(i))->id(), rolesPositions[match[i]], tempTarget,
                                                  ((positioningTargets[i].isValid()) ? positioningTargets[i] : tempTarget)).Score();
                        if (max_score[i] < score) {
                            max_score[i] = score;
                            finalTarget[i] = tempTarget;
                        }
                    }
                }
            }
        }
    }

    for (int i = 0 ; i < dynamicPositioners.count(); i++) {
        if (finalTarget[i].isValid() &&
                (!positioningTargets[i].isValid() ||
                 agent(dynamicPositioners.at(i))->pos().dist(positioningTargets[i]) < 0.05 ||
                 match[i] != last_match[i])) {
            positioningTargets[i] = finalTarget[i];
            last_match[i] = match[i];
        }
        if (positioningTargets[i].isValid()) {
            assignSkill(dynamicPositioners.at(i), gpa[dynamicPositioners.at(i)]);
            //gpa[dynamicPositioners.at(i)]->init(positioningTargets[i], wm->ball->pos-positioningTargets[i]);
            gpa[dynamicPositioners.at(i)]->setTargetpos(positioningTargets[i]);
            gpa[dynamicPositioners.at(i)]->setTargetdir(wm->ball->pos - positioningTargets[i]);


            if (gameState->ourKickoff()) {
                gpa[dynamicPositioners.at(i)]->setAvoidcentercircle(true);
            } else {
                gpa[dynamicPositioners.at(i)]->setAvoidcentercircle(false);
            }
            if (gameState->isStop()) {
                gpa[dynamicPositioners.at(i)]->setBallobstacleradius(.1);
            } else {
                gpa[dynamicPositioners.at(i)]->setBallobstacleradius(0);
            }
        }
    }

    for (int i = 0 ; i < staticPositioners.count() ; i++) {
        int idx = staticPositioners.at(i);
        int curID = agent(idx)->id();
        max_score[curID] = PositioningObject(curID, staticPositioningTargetsInput[curID], staticPositioningTargetsInput[curID],
                                             ((staticPositioningTargets[curID].isValid()) ? staticPositioningTargets[curID] : staticPositioningTargetsInput[curID])).Score();
        finalTarget[curID] = staticPositioningTargetsInput[curID];
        if (staticEscapeRadius[curID] > 0.0 && !doNotSearch) {
            for (double dist = 0.2 ; dist <= staticEscapeRadius[curID] ; dist += 0.2) {
                for (double ang = -180.0 ; ang <= 180.0 ; ang += 60.0) {
                    tempTarget = staticPositioningTargetsInput[curID] + Vector2D::polar2vector(dist, ang);
                    if (isValidPoint(tempTarget, true)) {
                        score = PositioningObject(curID, staticPositioningTargetsInput[curID], tempTarget,
                                                  ((staticPositioningTargets[curID].isValid()) ? staticPositioningTargets[curID] : tempTarget)).Score();
                        if (max_score[curID] < score) {
                            max_score[curID] = score;
                            finalTarget[curID] = tempTarget;
                        }
                    }
                }
            }
        }

    }


    for (int i = 0 ; i < staticPositioners.count() ; i++) {
        int idx = staticPositioners.at(i);
        int curID = agent(idx)->id();
        if (finalTarget[curID].isValid() &&
                (staticPositioningTargets[curID].isValid() == false ||
                 wm->our[curID]->pos.dist(staticPositioningTargets[curID]) < 0.05)) {
            staticPositioningTargets[curID] = finalTarget[curID];
        }
        if (staticPositioningTargets[curID].isValid()) {
            assignSkill(idx, gpa[curID]);
            //gpa[curID]->init(staticPositioningTargets[curID], staticPositioningFacePoints[curID]-staticPositioningTargets[curID]);
            gpa[curID]->setTargetpos(staticPositioningTargets[curID]);
            gpa[curID]->setTargetdir(staticPositioningFacePoints[curID] - staticPositioningTargets[curID]);

            if (gameState->ourKickoff()) {
                gpa[curID]->setAvoidcentercircle(true);
            } else {
                gpa[curID]->setAvoidcentercircle(false);
            }
            if (gameState->isStop()) {
                gpa[curID]->setBallobstacleradius(.1);
            } else {
                gpa[curID]->setBallobstacleradius(0);
            }
        }
    }
}

void PositioningPlan::reset() {
    posCount = 0;
    dynamicPositioners.clear();
    staticPositioners.clear();
    agents.clear();
    if (gameState->isStop()) {
        positionStaticPoints.clear();
        for (int i = 0 ; i < _MAX_NUM_PLAYERS ; i++) {
            staticStateNo[i] = 0;
            executedCycles[i] = 0;
            staticPositioningTargets[i].invalidate();
        }
    }
}

bool PositioningPlan::isValidPoint(Vector2D target , bool callFromStaticPositioner) {
    if ((
                gameState->ourKickoff()) &&
            (target.x > -Robot::robot_radius_new || target.length() < 0.6)) {
        return false;
    }

    if (target.absX() > wm->field->_FIELD_WIDTH * 0.5 - Robot::robot_radius_new) {
        return false;
    }

    if (target.absY() > wm->field->_FIELD_WIDTH * 0.5 - Robot::robot_radius_new) {
        return false;
    }

    Vector2D closerTarget = target + (wm->field->ourGoal() - target).norm() * 0.2;
    if (wm->field->isInOurPenaltyArea(target) || wm->field->isInOurPenaltyArea(closerTarget)) {
        return false;
    }

    if ((gameState->theirDirectKick() ||
            gameState->theirIndirectKick()) &&
            (target.dist(wm->ball->pos) < 0.6)) {
        return false;
    }

    closerTarget = target + (wm->field->oppGoal() - target).norm() * 0.3;
    if ((gameState->ourDirectKick() ||
            gameState->ourIndirectKick()) &&
            (wm->field->isInOppPenaltyArea(target) || wm->field->isInOppPenaltyArea(closerTarget))) {
        return false;
    }

    if (!callFromStaticPositioner) {
        for (int i = 0 ; i < staticPositioners.count() ; i++) {
            if (staticPositioningTargets[i].isValid() &&
                    staticPositioningTargets[i].dist(target) < 1.0) {
                return false;
            }
        }
    }

    double obsDeg = (target - wm->ball->pos).th().degree();
    double obsDist = target.dist(wm->ball->pos);
    double obsAng = asin(Robot::robot_radius_new / obsDist) * _RAD2DEG;
    double lowerbound = obsDeg - obsAng;
    double upperbound = obsDeg + obsAng;
    double topPost = (wm->field->oppGoalL() - wm->ball->pos).th().degree();
    double downPost = (wm->field->oppGoalR() - wm->ball->pos).th().degree();

    if (lowerbound > downPost && lowerbound < topPost) {
        return false;
    }

    return !(upperbound > downPost && upperbound < topPost);

}


//unique
/*!*****************************************************************************************************************/
//Depth_First_search
//definition :
///iterating on the graph to have completely iteration

bool PositioningPlan::dfs(int vertex_off , int max_count_off_mrk , int adj[_MAX_NUM_PLAYERS][_MAX_NUM_PLAYERS] , int mark[] , int s[] , int t[]) {
    int i_marker;
    s[vertex_off] = 1;
    mark[vertex_off] = 1;

    for (i_marker = 0 ; i_marker < max_count_off_mrk ; i_marker++)
        if (adj[vertex_off][i_marker])
            if (t[i_marker] = 1 , match[i_marker] == -1 || (!mark[match[i_marker]] && dfs(match[i_marker] , max_count_off_mrk , adj , mark , s , t))) {
                return match[i_marker] = vertex_off , true;
            }

    return false;

}
//unique
/*!*****************************************************************************************************************/

bool PositioningPlan::matching(int max_count_off_mrk , int adj[_MAX_NUM_PLAYERS][_MAX_NUM_PLAYERS] , int s[] , int t[] , int matching_checker[]) {
    int i_vertex_off;
    int mark[_MAX_NUM_PLAYERS];
    for (int i = 0 ; i < _MAX_NUM_PLAYERS ; i ++) {
        mark[i] = 0 ;
    }
    for (i_vertex_off = 0 ; i_vertex_off < max_count_off_mrk ; i_vertex_off++)
        if (!matching_checker[i_vertex_off] && !mark[i_vertex_off] && dfs(i_vertex_off , max_count_off_mrk , adj , mark , s , t)) {
            for (int i = 0 ; i < _MAX_NUM_PLAYERS ; i ++) {
                mark[i] = 0 ;
            }
            matching_checker[i_vertex_off] = 1;
        }

    for (i_vertex_off = 0 ; i_vertex_off < max_count_off_mrk ; i_vertex_off++)
        if (!matching_checker[i_vertex_off]) {
            return false;
        }

    return true;
}
//unique
/*!*****************************************************************************************************************/

void PositioningPlan::getBipartiteGraph(int count_target , int count_positioner , int weight[_MAX_NUM_PLAYERS][_MAX_NUM_PLAYERS]) {
    /*!*****************************************************************************************************************/

    int i, j;
    int matching_checker[_MAX_NUM_PLAYERS], s[_MAX_NUM_PLAYERS], t[_MAX_NUM_PLAYERS];
    int cover[2][_MAX_NUM_PLAYERS];
    int adj[_MAX_NUM_PLAYERS][_MAX_NUM_PLAYERS];
    for (int i1 = 0 ; i1 < _MAX_NUM_PLAYERS ; i1 ++) {
        matching_checker[i1] = 0 ;
        match[i1] = -1 ;
        for (int j1 = 0 ; j1 < _MAX_NUM_PLAYERS ; j1 ++) {
            adj[i1][j1] = 0 ;
        }
    }
    /*!*****************************************************************************************************************/

    /*!*****************************************************************************************************************/
    int max_count_off_mrk = 0 ;
    max_count_off_mrk = std::max(count_target , count_positioner);

    for (i = 0; i < max_count_off_mrk; i++) {
        int index = 0;
        //!saving number of best weighted positioner for i'th target
        for (j = 1; j < max_count_off_mrk; j++)
            if (weight[i][index] <= weight[i][j]) {
                index = j;
            }
        //!END
        cover[1][i] = 0;
        cover[0][i] = weight[i][index];//!storing maximum weight of edges that between i & index
        adj[i][index] = 1;
    }

    /*!*****************************************************************************************************************/

    while (!matching(max_count_off_mrk, adj, s, t, matching_checker)) {
        for (int i = 0 ; i < _MAX_NUM_PLAYERS ; i ++) {
            s[i] = 0 ;
            t[i] = 0 ;
        }
        matching(max_count_off_mrk, adj, s, t, matching_checker);
        int min = INF;

        for (i = 0; i < max_count_off_mrk; i++)
            for (j = 0; j < max_count_off_mrk; j++)
                if (s[i] && !t[j] && !adj[i][j])
                    if (cover[0][i] + cover[1][j] - weight[i][j] < min) {
                        min = cover[0][i] + cover[1][j] - weight[i][j];
                    }

        for (i = 0; i < max_count_off_mrk; i++)
            if (s[i]) {
                cover[0][i] -= min;
            }

        for (i = 0; i < max_count_off_mrk; i++)
            if (t[i]) {
                cover[1][i] += min;
            }

        for (i = 0; i < max_count_off_mrk; i++)
            for (j = 0; j < max_count_off_mrk; j++)
                if ((s[i] && !t[j]) || (!s[i] && t[j]))
                    if ((cover[0][i] + cover[1][j] - weight[i][j]) == 0) {
                        adj[i][j] = 1;
                    } else {
                        adj[i][j] = 0;
                    }
    }
}

PositioningPlan::PositioningObject::PositioningObject(const int _positioner ,
        const Vector2D _homePos ,
        const Vector2D _target ,
        const Vector2D _last_target)
    : positioner(_positioner),
      homePos(_homePos) ,
      target(_target) ,
      last_target(_last_target) {
    //! largest score is best score
    score = target_to_goal_openness() * target_to_goal_openness_coeff +
            target_to_ball_openness() * target_to_ball_openness_coeff +
            target_opp_mean_dist()    * target_opp_mean_dist_coeff    +
//            target_to_home_pos_dist() * target_to_home_pos_dist_coeff +
            target_to_last_target()   * target_to_last_target_coeff;
//      debug(QString("**************************************"),D_HOSSEIN,"red");
//      debug(QString("Positioner agent id:     %1").arg(positioner),D_HOSSEIN,"red");
//      debug(QString("Target position:         (%1,%2)").arg(target.x).arg(target.y),D_ERROR,"red");
//      debug(QString("Target to goal openness: %1").arg(target_to_goal_openness() * target_to_goal_openness_coeff),D_HOSSEIN,"red");
//      debug(QString("Target to ball openness: %1").arg(target_to_ball_openness() * target_to_ball_openness_coeff),D_HOSSEIN,"red");
//      debug(QString("Target opp mean dist:    %1").arg(target_opp_mean_dist()    * target_opp_mean_dist_coeff),D_HOSSEIN,"red");
//      debug(QString("Target to last target:   %1").arg(target_to_last_target() * target_to_last_target_coeff),D_HOSSEIN,"red");
}

double PositioningPlan::PositioningObject::target_to_goal_openness() {
    QList<int> ourRelaxIDS, oppRelaxIDS;
    ourRelaxIDS.push_back(positioner);
    Vector2D pos = target + wm->our[positioner]->dir.setLengthVector(Robot::center_from_kicker_new);
    return getOpenness(pos, wm->field->oppGoalL(), wm->field->oppGoalR(), ourRelaxIDS, oppRelaxIDS);
}

double PositioningPlan::PositioningObject::target_to_ball_openness() {
    QList<int> ourRelaxIDS, oppRelaxIDS;
    ourRelaxIDS.push_back(positioner);
    Vector2D pos = target + wm->our[positioner]->dir.setLengthVector(Robot::center_from_kicker_new);
    AngleDeg perp = (pos - wm->ball->pos).dir().degree() + 90.0;
    Vector2D goalL = pos + Vector2D::polar2vector(0.3, perp);
    Vector2D goalR = pos - Vector2D::polar2vector(0.3, perp);
    return getOpenness(wm->ball->pos, goalL, goalR, ourRelaxIDS, oppRelaxIDS);
}

double PositioningPlan::PositioningObject::target_opp_mean_dist() {
    double sum = 0.0;
    int count = 0;

    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (wm->opp.active(i)->pos.dist(target) < 1.0) {
            count++;
            sum += wm->opp.active(i)->pos.dist(target);
        }
    }

    if (count) {
        return sum / count;
    }

    return 1.0;
}

double PositioningPlan::PositioningObject::target_to_home_pos_dist() {
    return target.dist(homePos);
}

double PositioningPlan::PositioningObject::target_to_last_target() {
    return last_target.dist2(target);
}

double PositioningPlan::PositioningObject::getOpenness(Vector2D from, Vector2D p1, Vector2D p2, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs) {
    std::priority_queue < QPair< edgeMode , double > , std::vector< QPair< edgeMode , double > > , Comparar > blockedLines;
    double least = (p1 - from).th().degree();
    double most  = (p2 - from).th().degree();
    if (least > most) {
        qSwap(least, most);
    }

    for (int i = 0; i < wm->our.activeAgentsCount(); i++) {
        if (!ourRelaxedIDs.contains(wm->our.active(i)->id)) {
            double obsDeg = (wm->our.active(i)->pos - from).th().degree();
            double obsDist = wm->our.active(i)->pos.dist(from);
            double obsAng = asin(Robot::robot_radius_new / obsDist) * _RAD2DEG;
            double lowerbound = obsDeg - obsAng;
            double upperbound = obsDeg + obsAng;

            if (lowerbound > most || upperbound < least) {
                continue;
            }

            if (upperbound > most) {
                upperbound = most;
            }

            if (lowerbound < least) {
                lowerbound = least;
            }

            blockedLines.push(qMakePair(edgeMode::TOP, upperbound));
            blockedLines.push(qMakePair(edgeMode::BOT, lowerbound));
        }
    }
    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (!oppRelaxedIDs.contains(wm->opp.active(i)->id)) {
            double obsDeg = (wm->opp.active(i)->pos - from).th().degree();
            double obsDist = wm->opp.active(i)->pos.dist(from);
            double obsAng = asin(Robot::robot_radius_new / obsDist) * _RAD2DEG;
            double lowerbound = obsDeg - obsAng;
            double upperbound = obsDeg + obsAng;

            if (lowerbound > most || upperbound < least) {
                continue;
            }

            if (upperbound > most) {
                upperbound = most;
            }

            if (lowerbound < least) {
                lowerbound = least;
            }

            blockedLines.push(qMakePair(edgeMode::TOP, upperbound));
            blockedLines.push(qMakePair(edgeMode::BOT, lowerbound));
        }
    }

    return 1.0 - coveredArea(blockedLines) / (most - least);
}

double PositioningPlan::PositioningObject::coveredArea(std::priority_queue < QPair< edgeMode , double > , std::vector< QPair< edgeMode , double > > , Comparar >& obstacles) {
    if (obstacles.size() <= 1) {
        return 0.0;
    }

    QPair< edgeMode , double > lastest = obstacles.top();
    obstacles.pop();
    QPair< edgeMode , double > second_lastest = obstacles.top();

    if (lastest.first == edgeMode::TOP && second_lastest.first == edgeMode::TOP) {
        return (lastest.second - second_lastest.second) + coveredArea(obstacles);
    }

    else if (lastest.first == edgeMode::BOT && second_lastest.first == edgeMode::BOT) {
        return (lastest.second - second_lastest.second) + coveredArea(obstacles);
    }

    else if (lastest.first == edgeMode::TOP && second_lastest.first == edgeMode::BOT) {
        return (lastest.second - second_lastest.second) + coveredArea(obstacles);
    }

    else {
        return coveredArea(obstacles);
    }
}
