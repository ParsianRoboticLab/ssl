#include <parsian_ai/plays/dynamicattack.h>
#include <parsian_ai/plays/plays.h>
#include <parsian_util/tools/drawer.h>

const int CDynamicAttack::REGION_NUM = 7;

CDynamicAttack::CDynamicAttack() {
    // NEW PASS
    attackState = DynamicAttackState::PlaymakeControl;
    createRegions();
    clearRobotsRegionsWeights();
    PMfromCoach = true;
    dribbleIntention.start();
    playmakeIntention.start();
    lastPMInitWasDribble = false;
    //isShotInPass = false;
    lastPassPosLoc = Vector2D(5000, 5000);
    positioningIntentionInterval = 500;
    shotInPass = false;
    passFlag = false;
    repeatFlag = false;
    passerID = -1;
    counter = 0;

    currentPlan.reset();

    lastPasserRoleIndex = -1;

    isBallInOurField = wm->ball->pos.x < 0;

    for (bool &i : goToDynamic) {
        i = false;
    }

    lastAgentCount = -1;
}

CDynamicAttack::~CDynamicAttack() {

}

void CDynamicAttack::init(QList<Agent *> &_agents) {
    agents.clear();
    agents.append(_agents);
    initMaster();
}

void CDynamicAttack::reset() {
    executedCycles = 0;
    DBUG(QString("Dynamic Attack Reset"), D_MAHI);
}

void CDynamicAttack::execute_x() {
    ROS_INFO_STREAM("Dynamic Attack : " << agents.size());
    ROS_INFO_STREAM("ali:state  " << static_cast<int>(attackState));
    globalExecute(agents.size());
}

void CDynamicAttack::globalExecute(int agentSize) {

    isBallInOurField = wm->ball->pos.x < 0;
    dynamicPlanner(agentSize);
}

bool CDynamicAttack::evalmovefwd() {
    Vector2D oppgoal = wm->field->oppGoal();
    double default_dist{
            oppgoal.dist(Vector2D(wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y)) / 2.0};
    //ROS_INFO_STREAM("kian: " << default_dist);
    QList<Segment2D> obstacles;
    left.assign(Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y - default_dist},
                Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y});
    Segment2D right{Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y + default_dist},
                    Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y}};
    obstacles.push_back(left);
    obstacles.push_back(right);
    // drawer->draw(Circle2D(Vector2D(wm->ball->pos.x +  wm->ball->vel.x, wm->ball->pos.y +  wm->ball->vel.y), default_dist), QColor(100, 255, 50), false);
    //ROS_INFO_STREAM("debug: 1");
    for (int i{}; i < wm->opp.activeAgentsCount(); i++) {
        if (wm->opp.active(i)->pos.x + wm->opp.active(i)->vel.x > wm->ball->pos.x + wm->ball->vel.x + 0.001) {
            if (Vector2D(wm->opp.active(i)->pos.x + wm->opp.active(i)->vel.x,
                         wm->opp.active(i)->pos.y + wm->opp.active(i)->vel.y).dist(
                    Vector2D(wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y)) < default_dist) {
                Segment2D temp{Vector2D{wm->opp.active(i)->pos.x + wm->opp.active(i)->vel.x,
                                        wm->opp.active(i)->pos.y + wm->opp.active(i)->vel.y},
                               Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y}};
                //                   drawer->draw(temp, QColor(50, 55, 155));
                obstacles.push_back(temp);
            }
        }
    }
    // ROS_INFO_STREAM("debug: 2");
    // drawer->draw(left, QColor(50, 55, 155));
    // drawer->draw(right, QColor(50, 55, 155));
    sortobstacles(obstacles);
    //   for(int i{}; i < obstacles.size(); i++)
    //   {
    //       ROS_INFO_STREAM("kian::: "<<obstacles[i].a().y);
    //       //drawer->draw(obstacles[i], QColor(50, 55, 155));
    //   }
    //ROS_INFO_STREAM("debug: 3");
    QList<double> angles;
    //ROS_INFO_STREAM("kian: " << (1/3.14)*180*angleOfTwoSegment(Segment2D{Vector2D{5,7}, Vector2D{1,0}}, Segment2D{Vector2D{5,7}, Vector2D{3,0}}));
    for (int i{}; i < obstacles.size() - 1; i++) {
        angles.push_back(angleOfTwoSegment(obstacles[i], obstacles[i + 1]));
    }
    //    for(int i{}; i<angles.size(); i++)
    //    {
    //        ROS_INFO_STREAM("kian11: " << angles[i]*180/3.14 );
    //    }
    ROS_INFO("kian1: -------");
    //ROS_INFO_STREAM("debug: 4");
    QList<QPair<Vector2D, double>> result;
    QList<double> angsum;
    QList<double> nearestoppdist;
    for (int i{}; i < angles.size(); i++) {
        QPair<Vector2D, double> tmp;
        angsum.push_back(angleOfTwoSegment(obstacles[0], obstacles[i]));
        double ang{angsum[angsum.size()] + angles[i] / 2.0};
        //ROS_INFO_STREAM("kian: " << angsum*180/3.14);
        Vector2D tmp1{};
        tmp1.setPolar(1, AngleDeg{-90 + ang * 180 / 3.14});
        tmp.first = tmp1;
        tmp.first.x += wm->ball->pos.x + wm->ball->vel.x;
        tmp.first.y += wm->ball->pos.y + wm->ball->vel.y;
        if (obstacles[i].a().dist(Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y}) >
            obstacles[i + 1].a().dist(Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y})) {
            nearestoppdist.push_back(obstacles[i + 1].a().dist(Vector2D{wm->ball->pos.x, wm->ball->pos.y}));
        } else {
            nearestoppdist.push_back(obstacles[i].a().dist(Vector2D{wm->ball->pos.x, wm->ball->pos.y}));
        }
        //ROS_INFO_STREAM("kian1: " << angles[i] * nearestoppdist);
        tmp.second = angles[i] *
                     nearestoppdist[nearestoppdist.size()];//angleWide(prob) * nearestDist(prob) * diffrenceWithPI/2(effectivity)
        result.push_back(tmp);
    }
    //ROS_INFO_STREAM("debug: 5");
    //        for(int i{}; i <result.size(); i++)
    // drawer->draw(Segment2D{Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y}, result[i].first}, QColor(50, 10, 50));
    //ROS_INFO_STREAM("debug: 6");
    double maxeval{-1};
    int whichres = -1;
    for (int i{}; i < result.size(); i++) {
        if (result[i].second > maxeval) {
            maxeval = result[i].second;
            whichres = i;
        }
    }
    //ROS_INFO_STREAM("debug: 7");
    move_fwd_target = Vector2D{100, 0};
    last_move_fwd_target = Vector2D{100, 0};
    if (whichres != -1) {
        //    drawer->draw(Segment2D{Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y}, result[whichres].first}, QColor(250, 10, 50));
        if (angles[whichres] * 180 / 3.14 > 30 || nearestoppdist[whichres] > 0.6) {
            if (wm->ball->pos.y + wm->ball->vel.y > 1.4 && angsum[whichres] * 180 / 3.14 > 90) {
                move_fwd_target = result[whichres].first;
                last_move_fwd_target = move_fwd_target;
                return true;
            } else if (wm->ball->pos.y + wm->ball->vel.y < -1.4 && angsum[whichres] * 180 / 3.14 < 90) {
                move_fwd_target = result[whichres].first;
                last_move_fwd_target = move_fwd_target;
                return true;
            } else if (wm->ball->pos.y + wm->ball->vel.y > -1.2 && wm->ball->pos.y + wm->ball->vel.y < 1.2) {
                move_fwd_target = result[whichres].first;
                last_move_fwd_target = move_fwd_target;
                return true;
            } else {
                move_fwd_target = last_move_fwd_target;
            }
        }
    }
    return false;

}

void CDynamicAttack::swap(Segment2D *xp, Segment2D *yp) {
    Segment2D temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void CDynamicAttack::sortobstacles(QList<Segment2D> &obstacles) {
    int n{obstacles.size()};
    for (int i{}; i < n - 1; i++)
        for (int j{}; j < n - i - 1; j++)
            if (angleOfTwoSegment(obstacles[j], left) > angleOfTwoSegment(obstacles[j + 1], left))
                swap(&obstacles[j], &obstacles[j + 1]);
}

double CDynamicAttack::angleOfTwoSegment(const Segment2D &xp, const Segment2D &yp) {
    double theta1 = std::atan2(xp.a().y - xp.b().y, xp.a().x - xp.b().x);
    double theta2 = std::atan2(yp.a().y - yp.b().y, yp.a().x - yp.b().x);
    double diff = fabs(theta1 - theta2);
    return diff;
}

double CDynamicAttack::findmax(const QList<double> &list) {
    double max{-100000};
    for (int i{}; i < list.size(); i++) {
        if (list[i] > max)
            max = list[i];
    }
    return max;
}

void CDynamicAttack::makePlan(int agentSize) {

    //// Initialize Plan with null values
    currentPlan.agentSize = agentSize;

    //// We Don't have the ball -- counter-attack, blocking, move forward
    //// And Ball is in our field
    if (isBallInOurField) {
        ROS_INFO_STREAM("kian: dont have the ball");
        if (conf.ChipForward
//            && evalmovefwd()
                ) {
            playMakeRole.setSelectedPlayMakeSkill(PlayMakeSkill::Chip);
        } else {
            playMakeRole.setSelectedPlayMakeSkill(PlayMakeSkill::Chip);
        }
        for (size_t i = 0; i < agentSize; i++) {
            positionRoles[i]->setSelectedPositionSkill(positionSkill);
        }
    }
        //// we have ball and
        //// shot prob is more than 50%
    else if (directShot && attackState == DynamicAttackState::PlaymakeControl) {
        ROS_INFO_STREAM("ali: diret shot");
        playMakeRole.setSelectedPlayMakeSkill(PlayMakeSkill::Shot);
        for (size_t i = 0; i < agentSize; i++) {
            positionRoles[i]->setSelectedPositionSkill(PositionSkill::Ready);
        }
    } else if (attackState == DynamicAttackState::PlaymakePass) {
        ROS_INFO_STREAM("ali: pass mode");
        playMakeRole.setSelectedPlayMakeSkill(PlayMakeSkill::Pass);
        for (size_t i = 0; i < agentSize; i++) {
            positionRoles[i]->setSelectedPositionSkill(PositionSkill::OneTouch);
        }
    } else {
        ROS_INFO_STREAM("ali: pass mode");
        playMakeRole.setSelectedPlayMakeSkill(PlayMakeSkill::NoSkill);
        for (size_t i = 0; i < agentSize; i++) {
            positionRoles[i]->setSelectedPositionSkill(PositionSkill::OneTouch);
        }
    }
}

void CDynamicAttack::assignTasks() {
    if (playMakeAgent != nullptr) {
        playMake();
    }

    if (supportAgent != nullptr) {
        support();
    }
    if (currentPlan.agentSize > 0) {
        ROS_INFO_STREAM("kian: too if positioning : currentPlan.agentSize:" << currentPlan.agentSize);
        positioning(passPoints);//semiDynamicPosition);
        //positioning(amirSemiDynamicPosition);
    }
}

/**
 * @brief CDynamicAttack::dynamicPlanner
 * @param agentSize number of positioning Agents
 */
void CDynamicAttack::dynamicPlanner(int agentSize) {
    for (int i = 0; i < REGION_NUM; i++)
        drawer->draw(regions[i].rectangle);

    updateAttackState();
    makePlan(agentSize);
//    if (agentSize > 0 && (lastAgentCount != agentSize || isPlayMakeChanged())) {
    chooseBestPositons();
    assignId();
    chooseReceiverAndBestPosForPass();
//    }
    assignTasks();
    // execute roles
    for (size_t i = 0; i < matchingIDs.size(); i++) {
        if (matchingIDs[i] >= 0) {
            positionRoles[i]->execute();
        } else {
            DBUG(QString("[dynamicAttack - %1] mahiAgentID buged").arg(__LINE__), D_MAHI);
        }
    }

    if (playMakeAgent != nullptr && playMakeAgent->id() != -1) {
        playMakeRole.execute();
    }
    if (supportAgent != nullptr && supportAgent->id() != -1) {
        supportRole.execute();
    }
    lastAgentCount = agentSize;

}

void CDynamicAttack::playMake() {

    drawer->draw(Circle2D(playMakeAgent->pos(), 0.1), QColor(Qt::red), true);
    if (teamConfig.color == teamConfig.BLUE) {
        drawer->draw(Circle2D(playMakeAgent->pos() + playMakeAgent->dir() * 0.08, 0.06), QColor(Qt::blue), true);
    } else {
        drawer->draw(Circle2D(playMakeAgent->pos() + playMakeAgent->dir() * 0.08, 0.06), QColor(Qt::yellow), true);
    }

    playMakeRole.setAgent(playMakeAgent);
    playMakeRole.setAvoidPenaltyArea(true);

    Vector2D og = wm->ball->pos - wm->field->ourGoal();
    switch (playMakeRole.getSelectedPlayMakeSkill()) {
        case PlayMakeSkill::Dribble:
            ROS_INFO_STREAM("kian: drrible");//<< currentPlan.playmake.skill);
            playMakeRole.setTargetDir(currentPlan.recievePoint.point);
            playMakeRole.setTarget(oppRob);
            playMakeRole.setChip(false);
            playMakeRole.setNoKick(false);
            playMakeRole.setSelectedPlayMakeSkill(PlayMakeSkill::Dribble); // skill Dribble
            break;
        case PlayMakeSkill::Pass:
            ROS_INFO_STREAM("ali: pass");
            playMakeRole.setChip(chipOrNot(currentPlan.recievePoint.point, 0.5, 0.1));
            playMakeRole.setTarget(currentPlan.recievePoint.point);
            playMakeRole.setEmptySpot(false);
            playMakeRole.setNoKick(false);
            if (playMakeRole.getChip()) {
//            roleAgentPM->setChipDist(appropriateChipSpeed());       //TODO: set chip distanse not speed
                playMakeRole.setChipDist(conf.MediumDistChip);
            } else {
//            roleAgentPM->setKickSpeed(appropriatePassSpeed());
                playMakeRole.setKickSpeed(conf.MediumSpeedPass);
            }

            break;

        case PlayMakeSkill::Chip:
            ROS_INFO_STREAM("chip");
            playMakeRole.setNoKick(false);
            playMakeRole.setTarget(wm->field->oppGoal());
            playMakeRole.setChip(true);
            if (wm->ball->pos.x < -2) {
                playMakeRole.setChipDist(conf.HighDistChip);
            } else if (wm->ball->pos.x > 4) {
                playMakeRole.setChipDist(conf.LowDistChip);
            } else {
                playMakeRole.setChipDist(conf.MediumDistChip);

            }

            break;
        case PlayMakeSkill::Shot : {
            playMakeRole.setEmptySpot(true);
            playMakeRole.setChip(false);
            playMakeRole.setNoKick(false);
            playMakeRole.setTarget(wm->field->oppGoal());
            playMakeRole.setKickSpeed(conf.HighSpeedPass); // TODO : 8m/s by profiller
            break;
        }
        default:
            ROS_INFO_STREAM("default");
            playMakeRole.setEmptySpot(true);
            playMakeRole.setChip(false);
            playMakeRole.setNoKick(false);
            playMakeRole.setTarget(wm->field->oppGoal());
            // Parsa : ino hamintory avaz kardam kar kard...
            playMakeRole.setKickSpeed(conf.MediumSpeedPass); // TODO : 8m/s by profiller
            playMakeRole.setSelectedPlayMakeSkill(PlayMakeSkill::Shot); // Skill Kick
            break;
    }
}

void CDynamicAttack::positioning(QList<passPoint> &passPoints) {//(QList<Vector2D> _points) {
    for (int i = 0; i < matchingIDs.size(); i++) {
        if (matchingIDs[i] >= 0) {
            positionRoles[i]->setAgent(agents.at(i));
            positionRoles[i]->setAvoidPenaltyArea(true);
            if (i < passPoints.size()) {
                switch (positionRoles[i]->getSelectedPositionSkill()) {
                    case PositionSkill::Ready: // Ready For Pass
                        ROS_INFO_STREAM("kian: too switch set : skill: ready");
                        positionRoles[i]->setTarget(passPoints.at(i).point);
                        positionRoles[i]->setReceiveRadius(.5);
                        positionRoles[i]->setSelectedPositionSkill(PositionSkill::Ready);// Receive Skill

                        break;
                    case PositionSkill::OneTouch: // OneTouch Reflects
                        ROS_INFO_STREAM("kian: too switch set : skill: onetouch");
                        positionRoles[i]->setWaitPos(passPoints.at(i).point);
                        positionRoles[i]->setReceiveRadius(
                                std::max(0.5, 2 - positionRoles[i]->getAgent()->pos()
                                        .dist(positionRoles[i]->getTarget())));

                        // TODO : fix the target
                        positionRoles[i]->setTarget(wm->field->oppGoal());
                        positionRoles[i]->setSelectedPositionSkill(PositionSkill::OneTouch);// OneTouch Skill

                        break;
                    case PositionSkill::Move:
                        ROS_INFO_STREAM("kian: too switch set : skill: move");
                        positionRoles[i]->setReceiveRadius(
                                std::max(0.5, 2 - positionRoles[i]->getAgent()->pos()
                                        .dist(positionRoles[i]->getTarget())));
                        positionRoles[i]->setTarget(passPoints.at(i).point);
                        positionRoles[i]->setTargetDir(wm->ball->pos - positionRoles[i]->getAgent()->pos());
                        positionRoles[i]->setSelectedPositionSkill(PositionSkill::Move);

                        break;
                    case PositionSkill::NoSkill:
                        ROS_INFO_STREAM("kian: too switch set : skill: noskill");
                        positionRoles[i]->setSelectedPositionSkill(PositionSkill::Ready);// Receive Skill

                        break;
                        //                    case PositionSkill::Pass:break;
                        //                    case PositionSkill::CatchBall:break;
                        //                    case PositionSkilll::Shot:break;
                        //                    case PositionSkill::Keep:break;
                        //                    case PositionSkill::Chip:break;
                        //                    case PositionSkill::Dribble:break;
                }

                //debug(QString("pos positions : %1 %2").arg(roleAgents[i]->getTarget().x).arg(roleAgents[i]->getTarget().y), D_PARSA);
            }
        } else {
            DBUG("[dynamicAttack] positioning agent ha eshtebahe chera ?", D_MAHI);
        }
    }
    //assert(check);
}


inline bool CDynamicAttack::chipOrNot(Vector2D target,
                                      double _radius, double _treshold) {
    return !isPathClear(wm->ball->pos, target, _radius, _treshold);
}

bool CDynamicAttack::keepOrNot() {
    return true;
}


bool CDynamicAttack::isClear(Vector2D _pos1, Vector2D _pos2,
                             double _radius, double treshold, QString str, Vector2D point) {
    Vector2D sol1, sol2, sol3;
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

    if (str == "positionInOurWay") {
        if (_poly.contains(point)) {
            return false;
        }
    } else if (str == "isPassPathOpen") {
        if (treshold < 0.15) {
            ROS_INFO_STREAM("amirf draw1");
            drawer->draw(_poly, QColor(180, 180, 180));
        }


        for (int i{}; i < wm->opp.activeAgentsCount(); i++) {
            if (_poly.contains(wm->opp.active(i)->pos)) {
                return false;
            }
        }
    } else if (str == "positionClear") {
        if (treshold < 0.15) {
            ROS_INFO_STREAM("amirf draw2");
            drawer->draw(_poly, QColor(200, 200, 200));
        }


        for (int i{}; i < wm->opp.activeAgentsCount(); i++) {

            if (wm->opp.active(i)->id != wm->opp.data->goalieID) {
                if (_poly.contains(wm->opp.active(i)->pos)) {
                    //ROS_INFO_STREAM("amirq im here!");
                    return false;
                }
            }
        }
        for (int i{}; i < passPoints.size(); i++) {
            if (_poly.contains(passPoints[i].point)) //it shouldnt be ouractive. it should be position.
            {
                //ROS_INFO_STREAM("amirq im here!");
                return false;     //TODO: make it right
            }
        }
    }
    return true;
}


bool CDynamicAttack::isPathClear(Vector2D _pos1, Vector2D _pos2,
                                 double _radius, double treshold) {
    Vector2D sol1, sol2, sol3;
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

    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (_poly.contains(wm->opp.active(i)->pos)) {
            return false;
        }
    }

    for (int i = 0; i < wm->our.activeAgentsCount(); i++) {
        if (_poly.contains(wm->our.active(i)->pos)) {
            return false;
        }
    }

    return true;
}

bool CDynamicAttack::isPlayMakeChanged() {

    if (playMakeAgent != nullptr) {
        if (playMakeAgent->id() != lastPlayMakerId) {
            lastPlayMakerId = playMakeAgent->id();
            return true;
        }
    }
    return false;
}

int CDynamicAttack::appropriatePassSpeed() {

    double tempDistance = 0;
    double speed = 0;
    if (playMakeAgent != nullptr) {
        tempDistance = playMakeAgent->pos().dist(currentPlan.recievePoint.point);

        if (tempDistance < 2) {
            speed = conf.LowSpeedPass;

        } else if (tempDistance > 4) {
            speed = conf.HighSpeedPass;

        } else {
            speed = conf.MediumSpeedPass;

        }


    } else {
        speed = conf.MediumSpeedPass;
    }
    return static_cast<int>(speed);
}


/*bool CDynamicAttack::isPositionInOurWay(Vector2D _pos1, Vector2D _pos2,
                                        double _radius, double treshold, Vector2D point) {
    Vector2D sol1, sol2, sol3;
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
    drawer->draw(_poly);


    //for (int i = 0; i < wm->our.activeAgentsCount(); i++) {
    if (_poly.contains(point)) {
        return false;
    }
    //}


    return true;
}*/

/*bool CDynamicAttack::isPassPathOpen(Vector2D _pos1, Vector2D _pos2,
                                    double _radius, double treshold) {

    //ROS_INFO_STREAM("amirk im here");
    Vector2D sol1, sol2, sol3;
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


    if (treshold < 0.15) {
        ROS_INFO_STREAM("amirf draw1");
        drawer->draw(_poly, QColor(180, 180, 180));
    }


    for (int i{}; i < wm->opp.activeAgentsCount(); i++) {
        if (_poly.contains(wm->opp.active(i)->pos)) {
            return false;
        }
    }
    return true;
}*/


/*bool CDynamicAttack::isPositionClear(Vector2D _pos1, Vector2D _pos2, double _radius, double treshold) {
    Vector2D sol1, sol2, sol3;
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
    if (treshold < 0.15) {
        ROS_INFO_STREAM("amirf draw2");
        drawer->draw(_poly, QColor(200, 200, 200));
    }


    for (int i{}; i < wm->opp.activeAgentsCount(); i++) {

        if (wm->opp.active(i)->id != wm->opp.data->goalieID) {
            if (_poly.contains(wm->opp.active(i)->pos)) {
                //ROS_INFO_STREAM("amirq im here!");
                return false;
            }
        }
    }
    for (int i{}; i < passPoints.size(); i++) {
        if (_poly.contains(passPoints[i].point)) //it shouldnt be ouractive. it should be position.
        {
            //ROS_INFO_STREAM("amirq im here!");
            return false;     //TODO: make it right
        }
    }
    return true;
}*/


int CDynamicAttack::appropriateChipSpeed() {

    double tempDistance = 0;
    double speed = 0;
    if (playMakeAgent != nullptr) {
        tempDistance = playMakeAgent->pos().dist(currentPlan.recievePoint.point);


        if (tempDistance < 2) {
            speed = conf.LowDistChip;

        } else if (tempDistance > 4) {
            speed = conf.HighDistChip;

        } else {
            speed = conf.MediumDistChip;

        }
    } else {
        speed = conf.MediumSpeedPass;
    }
    return static_cast<int>(speed);
}

void CDynamicAttack::swapPlaymakeInPass() {
    if (playmakeIntention.elapsed() <= 1000) {
        //        agents.append(playmake);
        //        playmakeID = receiver->id();
        //        playmake = receiver;
        //        agents.removeOne(receiver);
    }
}

bool CDynamicAttack::isInpass() {
    Line2D ballpath{wm->ball->pos, wm->ball->pos + wm->ball->vel.norm() * 10};
    Circle2D receiverRegion(currentPlan.recievePoint.point, 1.3);
    Vector2D sol1;
    Vector2D sol2;
    if (playMakeAgent->pos().dist(wm->ball->pos) > 0.7) {
        if (receiverRegion.intersection(ballpath, &sol1, &sol2) != 0 && wm->ball->vel.length() > 1.2)
            return true;
    }
    return false;
}

double fRand(double fMin, double fMax) {
    double f = (double) rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

void CDynamicAttack::chooseReceiverAndBestPosForPass() {
    if (attackState == DynamicAttackState::PlaymakePass) {
        currentPlan.recievePoint.point = regions[regionPriority[0]].rectangle.center();
        drawer->draw(currentPlan.recievePoint.point, QColor(0, 0, 100), .5);
    }

//    optimalPositionsForRecivers.clear();
//    QList<double> probs;
//    QList<Vector2D> points;
//    Vector2D *bestPosition = new Vector2D[matchingIDs.size()];
//    for (auto &robotID : matchingIDs) {
//
//        if (!wm->field->isInField(wm->our[robotID]->pos)) continue;
//        Circle2D c{wm->our[robotID]->pos, 4};
//        Line2D path{playmake->pos(), wm->our[robotID]->pos};
//        Vector2D sol1, sol2;
//        sol1.invalidate();
//        sol2.invalidate();
//        c.intersection(path.perpendicular(wm->our[robotID]->pos), &sol1, &sol2);
//        double oppRed{1};
//        Segment2D recieveSegment{sol1, sol2};
//        QList<Circle2D> obstacles;
//        for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
//            obstacles.append(Circle2D(wm->opp.active(i)->pos, oppRed));
//        }
//
//        for (int i = 0; i < wm->our.activeAgentsCount(); i++) {
//            if (wm->our.active(i)->id == playmake->id() || wm->our.active(i)->id == robotID) continue;
//            obstacles.append(Circle2D(wm->our.active(i)->pos + wm->our.active(i)->vel, 0.1));
//
//        }
//
//        validateSegment(recieveSegment);
//        double angle = 0, biggestAngle = 0, prob = 0;
//        CKnowledge::getEmptyAngle(*wm->field, playmake->pos(), recieveSegment.a(), recieveSegment.b(), obstacles, prob,
//                                  angle, biggestAngle);
//        points.append(recieveSegment.intersection(Line2D(playmake->pos(), angle)));
//        probs.append(prob);
//
//    }
//
//    for (int i = 0; i < points.size(); i++) {
//        drawer->draw(Circle2D(points[i], probs[i]), QColor(Qt::darkMagenta), true);
//    }
//
//    if (points.isEmpty()) {
//        currentPlan.passPos = wm->field->oppGoal();
//        currentPlan.passID = -1;
//        return;
//    }
//    for (int i = 0; i < points.count(); i++) {
//        if (probs[i] < 0.5) continue;
//        double bestOneTouchFactor = -100;
//        int bestReceiver = -1;
//        for (int j{0}; j < matchingIDs.count(); j++) {
//            double tempOverall = calcOneTouchAngleFactor(points[i]);
//            if (tempOverall > bestOneTouchFactor) {
//                bestReceiver = matchingIDs[i];
//                bestOneTouchFactor = tempOverall;
//            }
//        }
//    }

}

double CDynamicAttack::getDynamicValue(const Vector2D &_dynamicPos) const {
    double defMoveAngle, openaAngle;
    defMoveAngle = Vector2D::angleOf(wm->ball->pos, wm->field->oppGoal(), _dynamicPos).degree();
    return defMoveAngle;
}


bool CDynamicAttack::isRightTimeToPass() {
    double minDist = 99999, tempDist;
    int tempDefIndex = 0;

    for (int i = 0; i < agents.size(); i++) {
        tempDist = agents.at(i)->pos().dist(currentPlan.recievePoint.point);
        if (tempDist < minDist) {
            minDist = tempDist;
            tempDefIndex = i;
        }
    }
    if (semiDynamicPosition.size() > tempDefIndex) {
        if (agents.at(tempDefIndex)->pos()
                    .dist(semiDynamicPosition.at(tempDefIndex)) < conf.Area) {
            return true;
        }
    }
    return false;
}

void CDynamicAttack::checkPoints(QList<Vector2D> &_points) {

    Rect2D validField(-wm->field->_FIELD_WIDTH / 2,
                      wm->field->_FIELD_HEIGHT / 2,
                      wm->field->_FIELD_WIDTH,
                      wm->field->_FIELD_HEIGHT);

    for (int i = 0; i < _points.size() - 1; i++) {
        if (!validField.contains(_points.at(i))) {
            _points.removeAt(i);
        }
    }
}

int CDynamicAttack::minHorizontalDistID(const QList<Vector2D> &_points) {
    double tempDist, minDist = 100000;
    int tempIndex = -1;

    for (int i = 0; i < _points.size(); i++) {
        tempDist = fabs(wm->ball->pos.y - _points.at(i).y);
        if (lastPassPos == i) {
            tempDist -= 2;
        }
        if (tempDist < minDist && fabs(wm->ball->pos.y - _points.at(i).y) > 0.2) {
            minDist = tempDist;
            tempIndex = i;
        }
    }
    lastPassPos = tempIndex;
    return tempIndex;
}

int CDynamicAttack::maxHorizontalDistID(const QList<Vector2D> &_points) {
    double tempDist, maxDist = -1;
    int tempIndex = -1;

    for (int i = 0; i < _points.size(); i++) {
        tempDist = fabs(wm->ball->pos.y - _points.at(i).y);
        if (lastPassPos == i) {
            tempDist += 2;
        }
        if (tempDist > maxDist && fabs(wm->ball->pos.y - _points.at(i).y) > 0.2) {
            maxDist = tempDist;
            tempIndex = i;
        }
    }
    lastPassPos = tempIndex;
    return tempIndex;
}

void CDynamicAttack::setDefenseClear(bool _isDefenseClearing) {
    isDefenseClearing = _isDefenseClearing;
}

void CDynamicAttack::setDirectShot(bool _directShot) {
    directShot = _directShot;
}

void CDynamicAttack::setPositions(QList<int> _positioningRegion) {
    regionsList.clear();
    dynamicPosition.clear();
    for (int i : _positioningRegion) {
        regionsList.append(i);
        //        dynamicPosition.append(knowledge->getStaticPoses
        //                               (_positioningRegion.at(i))); // TODO : Static Pos
    }
}

void CDynamicAttack::setPlayMake(Agent *_playMake) {
    playMakeAgent = _playMake;
}

void CDynamicAttack::setWeHaveBall(bool _ballPoss) {
    isWeHaveBall = _ballPoss;
}

void CDynamicAttack::setNoPlanException(bool _noPlanException) {
    noPlanException = _noPlanException;
}

void CDynamicAttack::setCritical(bool _critical) {
    critical = _critical;
}

void CDynamicAttack::setBallInOppJaw(bool _ballInOppJaw) {
    ballInOppJaw = _ballInOppJaw;
}

void CDynamicAttack::setFast(bool _fast) {
    fast = _fast;
}

bool CDynamicAttack::isPlanFailed() {
    return false;
}

bool CDynamicAttack::isPlanDone() {

    return true;
}

void CDynamicAttack::createRegions() {
    regions = new FieldRegion[REGION_NUM];
    // <make rectangles>
    QList<Rect2D> rectangles;
    Size2D rectSize1(wm->field->_FIELD_WIDTH / 6, (wm->field->_FIELD_HEIGHT - wm->field->_PENALTY_WIDTH) / 2);
    Size2D rectSize2((wm->field->_FIELD_WIDTH / 3 - wm->field->_PENALTY_DEPTH) / 2, wm->field->_PENALTY_WIDTH);
    Size2D rectSize3(wm->field->_FIELD_WIDTH / 6, wm->field->_FIELD_HEIGHT);
    // region 0,1,2,3
    rectangles.append(Rect2D(wm->field->oppCornerL() - Vector2D(rectSize1.length(), 0), rectSize1));
    rectangles.append(Rect2D(wm->field->oppCornerR() - Vector2D(rectSize1.length(), -rectSize1.width()), rectSize1));
    rectangles.append(Rect2D(wm->field->oppCornerL() - Vector2D(2 * rectSize1.length(), 0), rectSize1));
    rectangles.append(
            Rect2D(wm->field->oppCornerR() - Vector2D(2 * rectSize1.length(), -rectSize1.width()), rectSize1));
    // region 4,5
    rectangles.append(Rect2D(wm->field->oppCornerL() -
                             Vector2D(rectSize2.length() + wm->field->_PENALTY_DEPTH, rectSize1.width()), rectSize2));
    rectangles.append(Rect2D(wm->field->oppCornerL() -
                             Vector2D(2 * rectSize2.length() + wm->field->_PENALTY_DEPTH, rectSize1.width()),
                             rectSize2));
    // region 6
    rectangles.append(Rect2D(wm->field->center() + Vector2D(0, rectSize3.width() / 2), rectSize3));


    // </make rectangles>

    // <make eval points>
    QList<Vector2D> points[REGION_NUM];
    for (int i{0}; i < rectangles.length(); i++) {
        if (rectangles[i].size().width() > 1 && rectangles[i].size().length() > 1) {

            points[i].push_back(Vector2D(rectangles[i].left() + .5, rectangles[i].top() - .5));
            points[i].push_back(Vector2D(rectangles[i].center().x, rectangles[i].top() - .5));
            points[i].push_back(Vector2D(rectangles[i].right() - .5, rectangles[i].top() - .5));

            points[i].push_back(Vector2D(rectangles[i].left() + .5, rectangles[i].center().y));
            points[i].push_back(rectangles[i].center());
            points[i].push_back(Vector2D(rectangles[i].right() - .5, rectangles[i].center().y));

            points[i].push_back(Vector2D(rectangles[i].left() + .5, rectangles[i].bottom() + .5));
            points[i].push_back(Vector2D(rectangles[i].center().x, rectangles[i].bottom() + .5));
            points[i].push_back(Vector2D(rectangles[i].right() - .5, rectangles[i].bottom() + .5));

        } else
            ROS_INFO_STREAM("ali : region " << i << " is too small");
    }
    // </make eval points>

    // <fill the regions>
    int _id = 0;
    for (int i{0}; i < REGION_NUM; i++) {
        regions[i] = FieldRegion(rectangles[i], points[i]);
        regions[i].id = _id++;
    }
}


void CDynamicAttack::regionByBall(int ballR) {
    regionPriority.clear();
    switch (ballR) {
        case 0:
            //regionPriority << 4 << 2 << 1 << 5 << 3 << 6;
        {
            regions[4].pointPriority = 6;
            regions[2].pointPriority = 5;
            regions[1].pointPriority = 4;
            regions[5].pointPriority = 6; // for onetouch chance. otherwise this is 3
            regions[3].pointPriority = 2;
            regions[6].pointPriority = 1;
            regions[0].pointPriority = 1;
            break;
        }
        case 1:
            //regionPriority << 4 << 3 << 0 << 5 << 2 << 6;
        {
            regions[4].pointPriority = 6;
            regions[3].pointPriority = 5;
            regions[0].pointPriority = 4;
            regions[5].pointPriority = 6; // for onetouch chance. otherwise this is 3
            regions[2].pointPriority = 2;
            regions[6].pointPriority = 1;
            regions[1].pointPriority = 1;
            break;
        }
        case 2:
            //regionPriority << 4 << 0 << 5 << 3 << 1 << 6;
        {
            regions[4].pointPriority = 6;
            regions[0].pointPriority = 4;
            regions[5].pointPriority = 5;
            regions[3].pointPriority = 3;
            regions[1].pointPriority = 2;
            regions[6].pointPriority = 1;
            regions[2].pointPriority = 1;
            break;
        }
        case 3:
            //regionPriority << 4 << 1 << 5 << 2 << 0 << 6;
        {
            regions[4].pointPriority = 6;
            regions[1].pointPriority = 4;
            regions[5].pointPriority = 5;
            regions[2].pointPriority = 3;
            regions[0].pointPriority = 2;
            regions[6].pointPriority = 1;
            regions[3].pointPriority = 1;
            break;
        }
        case 4:
            //regionPriority << 0 << 1 << 2 << 3 << 5 << 6;
        {
            regions[0].pointPriority = 6;
            regions[1].pointPriority = 5;
            regions[2].pointPriority = 4;
            regions[3].pointPriority = 3;
            regions[5].pointPriority = 2;
            regions[6].pointPriority = 1;
            regions[4].pointPriority = 1;
            break;
        }
        case 5:
            //regionPriority << 0 << 1 << 2 << 3 << 6 << 4;
        {
            regions[0].pointPriority = 6;
            regions[1].pointPriority = 5;
            regions[2].pointPriority = 4;
            regions[3].pointPriority = 3;
            regions[6].pointPriority = 2;
            regions[4].pointPriority = 1;
            regions[5].pointPriority = 1;
            break;
        }
        case 6:
            //regionPriority << 0 << 1 << 2 << 3 << 4 << 5;
        {
            regions[0].pointPriority = 6;
            regions[1].pointPriority = 5;
            regions[2].pointPriority = 4;
            regions[3].pointPriority = 3;
            regions[4].pointPriority = 2;
            regions[5].pointPriority = 6; //if it is in corner it is good.
            regions[6].pointPriority = 1;
            break;
        }
        default:
            //regionPriority << 0 << 1 << 2 << 3 << 4 << 5;
        {
            regions[0].pointPriority = 6;
            regions[1].pointPriority = 5;
            regions[2].pointPriority = 4;
            regions[3].pointPriority = 3;
            regions[4].pointPriority = 2;
            regions[5].pointPriority = 1;
            regions[6].pointPriority = 0;
            break;
        }
    }
}

void CDynamicAttack::oppInregion() {
    for (size_t i{}; i < REGION_NUM; i++) {
        regions[i].oppInside = 0;
        regions[i].oppInNeighbor = 0;
    }
    for (size_t i{}; i < wm->opp.activeAgentsCount(); i++) {
        Vector2D position{wm->opp.active(i)->pos};
        if (regions[0].rectangle.contains(position)) {
            regions[0].oppInside++;
            regions[2].oppInNeighbor++;
            regions[4].oppInNeighbor++;
            regions[5].oppInNeighbor++;
        } else if (regions[1].rectangle.contains(position)) {
            regions[1].oppInside++;
            regions[3].oppInNeighbor++;
            regions[4].oppInNeighbor++;
            regions[5].oppInNeighbor++;
        } else if (regions[2].rectangle.contains(position)) {
            regions[2].oppInside++;
            regions[0].oppInNeighbor++;
            regions[4].oppInNeighbor++;
            //regions[5].oppInNeighbor++;
        } else if (regions[3].rectangle.contains(position)) {
            regions[3].oppInside++;
            regions[1].oppInNeighbor++;
            regions[4].oppInNeighbor++;
            //regions[5].oppInNeighbor++;
        } else if (regions[4].rectangle.contains(position)) {
            regions[4].oppInside++;
            regions[0].oppInNeighbor++;
            regions[1].oppInNeighbor++;
            regions[5].oppInNeighbor++;
        } else if (regions[5].rectangle.contains(position)) {
            regions[5].oppInside++;
            regions[0].oppInNeighbor++;
            regions[1].oppInNeighbor++;
            regions[2].oppInNeighbor++;
            regions[3].oppInNeighbor++;
            regions[4].oppInNeighbor++;
        } else if (regions[6].rectangle.contains(position)) {
            regions[6].oppInside++;
            regions[2].oppInNeighbor++;
            regions[3].oppInNeighbor++;
            //regions[5].oppInNeighbor++;
        }

    }
}

void CDynamicAttack::chooseBestPositons() {
    clearRobotsRegionsWeights();

    // get the search regions
    QList<Rect2D> searchRegions;
    for (int i{0}; i < REGION_NUM; i++) {
        searchRegions.append(regions[i].rectangle);
    }
    QList<Rect2D> avoidRects;
    avoidRects.append(wm->field->oppPenaltyRect());

    QList<FieldRegion> sortRegions;

    ROS_INFO_STREAM("amirn regions done once");
    if (attackState == DynamicAttackState::PlaymakeControl) {
        int ballR = -1;
        for (int i{0}; i < REGION_NUM; i++)
            if (regions[i].rectangle.contains(wm->ball->pos + wm->ball->vel))ballR = regions[i].id;

        regionByBall(ballR);
        oppInregion();
        ROS_INFO_STREAM("amirn regions done once2");

        for (size_t i{}; i < REGION_NUM; i++) {
            if (regions[i].rectangle.contains(currentPlan.recievePoint.point))
                regions[i].pointPriority = -1000;
            else
                regions[i].pointPriority -= (regions[i].oppInside + regions[i].oppInNeighbor);
            sortRegions.append(regions[i]);
        }


        std::sort(sortRegions.begin(), sortRegions.end()); // baraks sort shode!!!

        for (int i{REGION_NUM - 1}; i >= 0; i--) {
            regionPriority.append(sortRegions[i].id);

            //ROS_INFO_STREAM("amiry ids : " << sortRegions[i].id << " and point is : " << sortRegions[i].pointPriority << " and i is : " << i);
        }
    }
}

int CDynamicAttack::getNearestRegionToRobot(Vector2D agentPos) {
    for (int i{0}; i < REGION_NUM; i++) {
        if (regions[i].rectangle.contains(agentPos))
            return regions[i].id;
    }
    return -1;
}

void CDynamicAttack::assignId() {
    if (regionPriority.isEmpty() || playMakeAgent == nullptr) return;
    QList<int> robotIDs;
    MWBM matcher;
    auto supporterID = -1;
    if (supportAgent)
        supporterID = supportAgent->id();
    int last_matched_receiver = -1;
    if (currentPlan.recievePoint.ID != -1)
        last_matched_receiver = matchingIDs[currentPlan.recievePoint.ID];

    for (const auto &a : agents) {
        if (a->id() != playMakeAgent->id() && a->id() != supporterID) robotIDs.append(a->id());
    }

    matcher.create(robotIDs.count(), robotIDs.count());

    for (int i{0}; i < robotIDs.count(); i++) {
        for (int j{0}; j < robotIDs.count(); j++) {
            auto agentPos = agents.at(i)->pos();
            if (i == currentPlan.recievePoint.ID && j == last_matched_receiver)
                matcher.setWeight(i, j, 0);
            else
                matcher.setWeight(i, j,
                                  agentPos.dist(regions[regionPriority[j]].rectangle.center())); //TODO : not center
        }
    }


//    matcher.findMaxMinMatching();

    matcher.findMatching();

    semiDynamicPosition.clear();

    //for (int v = 0; v < robotIDs.count(); v++) {
    // todo : find best pos in region from searchRegions.points
    //semiDynamicPosition.append(regions[regionPriority[matcher.getMatch(v)]].rectangle.center());
    //  matchingIDs[robot_id] = matcher.getMatch(v);
    //}


    matchingIDs.clear();
    for (int v = 0; v < robotIDs.count(); v++) {
        matchingIDs.append(matcher.getMatch(v));
        ROS_INFO_STREAM("alii v " << v << "match " << matchingIDs[v]);
    }

    passPositions(robotIDs);



    //finalPassReciever();


}


void CDynamicAttack::passPositions(const QList<int> &robotIDs) {

    bestPos(robotIDs);
    isChipOrPass(passPoints);
    findOneTouch(passPoints);
    isInPosition(passPoints);
    toPassOrNotToPass(passPoints);
    passPriority(passPoints);
    stayPassReciever(passPoints);
    showPasser(passPoints);
    passDecision();
}


void CDynamicAttack::passDecision() {
    //choose pass reciever
    if (currentPlan.recievePoint.ID == -1) {
        ROS_INFO_STREAM("amirty : point is about to change");
        for (int i{}; i < passPoints.size(); i++) {
            if (passPoints[i].stay == 1) {
                currentPlan.recievePoint.ID = passPoints[i].ID;
                currentPlan.recievePoint.point.x = passPoints[i].point.x;
                currentPlan.recievePoint.point.y = passPoints[i].point.y;
                break;
            }
        }
    }

    Segment2D ballseg{wm->ball->seg()};
    Line2D pointGoal{currentPlan.recievePoint.point, wm->field->oppGoal()};
    /*if (ballseg.intersection(pointGoal).dist(recievePoint.point) > 0.5){
        recievePoint.ID = -1;
        ROS_INFO_STREAM("amirty : pass calnceled!");
    }*/
}


RecievePoint::RecievePoint() {
    ID = -1;
    point.x = 0;
    point.y = 0;
}


void CDynamicAttack::stayPassReciever(QList<passPoint> &passPoints) {

    /*for (int i{}; i < passPoints.size(); i++) {
        passPoints[i].chance = passPoints[i].stay;
    }
    std::sort(passPoints.begin(), passPoints.end());

    for (int i{passPoints.size() - 1}; i >= 0; i--) {
        amirSemiDynamicPosition.append(passPoints[i].point);
    }*/


}


void CDynamicAttack::showPasser(QList<passPoint> &passPoints) {
    for (int i{}; i < passPoints.size(); i++) {
        /*if(passPoints[i].amIReciever)
        {
            ROS_INFO_STREAM("amirf 5");
            if(passPoints[i].chipOrPass && passPoints[i].oneTouch)
                drawer->draw(passPoints[i].point,QColor(170,100,160),0.3);
            else if(passPoints[i].chipOrPass && !passPoints[i].oneTouch)
                drawer->draw(passPoints[i].point,QColor(40,140,220),0.3);
            else if(!passPoints[i].chipOrPass && passPoints[i].oneTouch)
                drawer->draw(passPoints[i].point,QColor(230,30,30),0.3);
            else if(!passPoints[i].chipOrPass && !passPoints[i].oneTouch)
                drawer->draw(passPoints[i].point,QColor(240,100,10),0.3);

        }*/
        if (passPoints[i].stay) {
            drawer->draw(passPoints[i].point, QColor(255, 255, 0), 0.4);
            //ROS_INFO_STREAM("amirtr id : " << agents[matcher.getMatch(i)]->id());
            passPoints[i].ID = agents[matchingIDs[i]]->id();
        }
    }
}


void CDynamicAttack::passPriority(QList<passPoint> &passPoints) {
    for (int i{}; i < passPoints.size(); i++) {
        if (passPoints[i].amIReciever && passPoints[i].oneTouch) {
            passPoints[i].stay = 1;
            //playmake.setchip
            //playmake.onetouch
        } else if (passPoints[i].amIReciever) {
            //passPoints[i].stay = 1;
            //playmake.setchip
        }
    }
}


passPoint::passPoint() {
    point.x = 0;
    point.y = 0;
    amIReciever = false;
    stay = false;
    inPostion = false;
    oneTouch = false;
    chipOrPass = false;
    finalPassReciever = false;
    int ID = -1;

}

passPoint::passPoint(vector2D p) {
    point.x = p.x;
    point.y = p.y;
    amIReciever = false;
    stay = false;
    inPostion = false;
    oneTouch = false;
    chipOrPass = false;
    finalPassReciever = false;
    ID = -1;
}


void CDynamicAttack::isChipOrPass(QList<passPoint> &passPoints) {
    double dist_treshold{4.5};
    for (int i{}; i < passPoints.size(); i++) {
        passPoint p{passPoints[i]};
        if (playMakeAgent->pos().dist(p.point) < dist_treshold) {
            if (isPathClear(playMakeAgent->pos(), p.point, Robot::robot_radius_new, 0.2))
                p.chipOrPass = false;
            else {
                p.chipOrPass = true;
            }
        } else {
            if (isPathClear(playMakeAgent->pos(), p.point, Robot::robot_radius_new, 0.2))
                p.chipOrPass = false;
        }
    }

}


void CDynamicAttack::isInPosition(QList<passPoint> &passPoints) {
    for (int i{}; i < passPoints.size(); i++) {
        for (int j{}; j < wm->our.activeAgentsCount(); j++) {
            //if(passPoints[i].inPostion)//edit
            //continue;
            if (wm->our.active(j)->pos.dist(passPoints[i].point) < 0.1) {
                passPoints[i].inPostion = true;
                break;
            } else
                passPoints[i].inPostion = false;
        }
    }
}

void CDynamicAttack::toPassOrNotToPass(QList<passPoint> &passPoints) {
    //ROS_INFO_STREAM("amirf 2");
    double chip_dist_treshold{4.5};
    for (int i{}; i < passPoints.size(); i++) {
        if (passPoints[i].inPostion) {
            //ROS_INFO_STREAM("amirf they are in position");
            if (playMakeAgent->pos().dist(wm->ball->pos) < 0.5) {
                //ROS_INFO_STREAM("amirf in if");
                if (isClear(wm->ball->pos, passPoints[i].point,
                            Robot::robot_radius_new + (wm->ball->pos.dist(passPoints[i].point) / 15), 0.1,
                            "passPathOpen") &&
                    isClear(passPoints[i].point, wm->field->oppGoal(),
                            wm->field->oppGoalL().y - wm->field->oppGoal().y, 0.1, "positionClear")) {
                    passPoints[i].amIReciever = true;

                } else if (playMakeAgent->pos().dist(passPoints[i].point) < chip_dist_treshold &&
                           isClear(passPoints[i].point, wm->field->oppGoal(),
                                   wm->field->oppGoalL().y - wm->field->oppGoal().y, 0.1, "positionClear")
                           && passPoints[i].chipOrPass) //this is for chip then onetouch
                {
                    passPoints[i].amIReciever = true;
                } else {
                    passPoints[i].amIReciever = false;
                }
            }
        }
    }
}

void CDynamicAttack::findOneTouch(QList<passPoint> &passPoints) {
    for (int i{}; i < passPoints.size(); i++) {

        Segment2D posGoal(passPoints[i].point, wm->field->oppGoal());
        Segment2D playMakePos(wm->ball->pos, passPoints[i].point);
        //double angle{angleOfTwoSegment(posGoal, playMakePos)};
        double angle{std::fabs(Vector2D::angleBetween(wm->field->oppGoal() - passPoints[i].point,
                                                      wm->ball->pos - passPoints[i].point).degree())};
        //ROS_INFO_STREAM("amirp angle : " << angle);
        //ROS_INFO_STREAM("amirp point.x : " << passPoints[i].point.x);
        //ROS_INFO_STREAM("amirp point.y : " << passPoints[i].point.y);
        if (angle < conf.MaxOnetouchAngle &&
            (passPoints[i].region == 4 || passPoints[i].region == 5 || passPoints[i].region == 2 ||
             passPoints[i].region == 3 || passPoints[i].region == 1
             || passPoints[i].region == 0)) {
            //ROS_INFO_STREAM("amirw here 1");
            if (isClear(passPoints[i].point, wm->field->oppGoal(),
                        wm->field->oppGoalL().y - wm->field->oppGoal().y, 0.05, "positionClear")) {
                passPoints[i].oneTouch = true;
                //ROS_INFO_STREAM("amirw point for onetouch.x = " << passPoints[i].point.x);
                //ROS_INFO_STREAM("amirw point for onetouch.y = " << passPoints[i].point.y);
                //drawer->draw(passPoints[i].point, QColor(255,255,255), 0.3);
            }
        }
    }

}


void CDynamicAttack::bestPos(const QList<int> &robotIDs) {

    //stayPassReciever();

    for (int v{}; v < robotIDs.count(); v++) {
        double chance{0};
        Vector2D tmp_point;

        for (size_t i{}; i < regions[0].points.size(); i++) {
            auto tmp_chance = calcRegionProperties(v, i);
            if (tmp_chance >= chance) {
                chance = tmp_chance;
                tmp_point = regions[regionPriority[matchingIDs[v]]].points[i];
            }
        }
        regions[regionPriority[matchingIDs[v]]].chance = chance;
//        regions[regionPriority[matchingIDs[v]]].theirNearestRobot = max_dist;
        semiDynamicPosition.append(tmp_point);
    }

    if (robotIDs.size() > semiDynamicPosition.size()) {
        for (int i{semiDynamicPosition.size()}; i < robotIDs.size(); i++) {
            semiDynamicPosition.append(regions[regionPriority[matchingIDs[i]]].rectangle.center());
        }
    }

    passPoints.clear();

    for (int i{}; i < semiDynamicPosition.size(); i++) {
        passPoint tmp;
        tmp.point.x = semiDynamicPosition[i].x;
        tmp.point.y = semiDynamicPosition[i].y;
        for (int j{}; j < REGION_NUM; j++) {
            if (regions[j].rectangle.contains(tmp.point)) {
                tmp.region = j;
            }
        }
        passPoints.append(tmp);
    }
}

void CDynamicAttack::finalPassReciever() {
    for (int i{}; i < passPoints.size(); i++) {
        if (passPoints[i].amIReciever && passPoints[i].oneTouch) {
            passPoints[i].stay = 1;
            break;
        }
    }
}


CRobot *CDynamicAttack::findOppGoalKeaper() {
    return (wm->opp.active(wm->opp.data->goalieID));
}


Vector2D CDynamicAttack::getBestPosToShootToGoal(Vector2D from,
                                                 double &regionWidth, bool oppGaol) {
    Rect2D playingField(wm->field->ourCornerL(), wm->field->oppCornerR());
    if (!playingField.contains(from)) {
        regionWidth = 0.0;
        double goalProbablity = 0.0;
        auto shootPos = Vector2D(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
        return shootPos;
    }
    Vector2D goal;
    Vector2D goalL;
    Vector2D goalR;
    if (oppGaol) {
        goal = wm->field->oppGoal();
        goalL = wm->field->oppGoalL();
        goalR = wm->field->oppGoalR();
    } else {
        goal = wm->field->ourGoal();
        goalL = wm->field->ourGoalL();
        goalR = wm->field->ourGoalR();
    }
    double StepOnGoal = _GOAL_WIDTH / _GOAL_STEP;
    double MaxRegionWidth = 0, MaxRegionTemp = 0;
    double BeginPos = 0, EndPos = 0;
    Vector2D MaxRegionCenter(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE), RegionCenterTemp;
    bool WasLastPosClear = false;
    auto totalSteps = static_cast<size_t>((goalL.y - goalR.y) / StepOnGoal);
    for (size_t step{0}; step < totalSteps; step++) {
        const double y = goalR.y + step * StepOnGoal;
        Vector2D pos(goal.x, y);
        if (!WasLastPosClear)
            BeginPos = y;

        WasLastPosClear = this->isPathClear(pos, from, (ROBOT_RADIUS + 2 * CBall::radius), true);
        EndPos = y;
        if (WasLastPosClear) {
            RegionCenterTemp = Segment2D(goalL, goalR).intersection(Line2D(from, (from + Vector2D(
                    Vector2D(goal.x, BeginPos) - from).rotate(
                    Vector2D::angleBetween(Vector2D(goal.x, BeginPos) - from,
                                           Vector2D(goal.x, EndPos) -
                                           from).degree() / 2))));
            if (RegionCenterTemp.x == Vector2D::ERROR_VALUE || RegionCenterTemp.y == Vector2D::ERROR_VALUE)
                RegionCenterTemp = Vector2D(goal.x, (EndPos - BeginPos) / 2);
            MaxRegionTemp = (EndPos - BeginPos + 0.001) * from.dist(Line2D(goalL, goalR).projection(from)) /
                            from.dist(RegionCenterTemp);
            if (MaxRegionWidth < MaxRegionTemp) {
                MaxRegionWidth = MaxRegionTemp;
                MaxRegionCenter = RegionCenterTemp;
            }
        }
    }
    //    if( MaxRegionCenter.x != Vector2D::ERROR_VALUE  )
    //    {
    //        if( oppGaol )
    //        {
    //            regionWidth = (MaxRegionWidth / _GOAL_WIDTH) * 0.7 +
    //                          ((Vector2D::dirTo_deg(from,goalL) - Vector2D::dirTo_deg(from,goalR)) / 180.0) * 0.3;
    //            goalProbablity = regionWidth;
    //        }
    //        else
    //        {
    //            double dirL = Vector2D::dirTo_deg(from,goalL);
    //            dirL = dirL < 0.0? dirL + 360.0 : dirL;
    //            double dirR = Vector2D::dirTo_deg(from,goalR);
    //            dirR = dirR < 0.0? dirR + 360.0 : dirR;

    //            regionWidth = (MaxRegionWidth / _GOAL_WIDTH) * 0.7 +
    //                          ((dirR - dirL) / 180.0) * 0.3;
    //            goalProbablity = regionWidth;
    //        }
    //        shootPos = MaxRegionCenter;
    //        draw(Circle2D(shootPos, 0.05), 0, 360, "blue", true);
    //        return shootPos;
    //    }
    double goalProbablity;
    Vector2D shootPos;
    if (MaxRegionCenter.x != Vector2D::ERROR_VALUE && MaxRegionCenter.y != Vector2D::ERROR_VALUE) {
        regionWidth = MaxRegionWidth / _GOAL_WIDTH;
        goalProbablity = regionWidth;
        Vector2D shootPos = MaxRegionCenter;
        //        draw(Segment2D(from,shootPos) , "red");
        return shootPos;
    }
    regionWidth = 0.0;
    goalProbablity = 0.0;
    shootPos = Vector2D(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
    return shootPos;
}

bool CDynamicAttack::isPathClear(Vector2D point, Vector2D from, double rad,
                                 bool considerRelaxedIDs) {
    Vector2D posIntersect1(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
    Vector2D posIntersect2(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
    Segment2D l(from, point);
    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if ((wm->opp.active(i)->inSight > 0.0)) {
            Circle2D c(wm->opp.active(i)->pos, rad);
            if (c.intersection(l, &posIntersect1, &posIntersect2) != 0) {
                return false;
            }
        }
    }
    for (int i = 0; i < wm->our.activeAgentsCount(); i++) {
        if (wm->our.active(i)->inSight > 0.0) {
            Circle2D c(wm->our.active(i)->pos, rad);
            if (c.intersection(l, &posIntersect1, &posIntersect2) != 0) {
                return false;
            }
        }
    }
    return true;
}

int CDynamicAttack::getNearestOppToPoint(Vector2D point) {
    double minDist = 10000.0;
    int nearest = -1;
    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (wm->opp.active(i)->inSight <= 0) {
            continue;
        }
        double dist = (wm->opp.active(i)->pos - point).length();
        if (dist < minDist) {
            minDist = dist;
            nearest = wm->opp.active(i)->id;
        }
    }
    return nearest;
}

void CDynamicAttack::clearRobotsRegionsWeights() {
    for (int i{0}; i < 11; i++) {
        for (int j{0}; j < 9; j++) {
            robotRegionsWeights[i][j] = -1.0;
            bestPointForRobotsInRegions[i][j].invalidate();
        }
    }
}

double CDynamicAttack::calcReceiverDistanceFactor(Vector2D point, int passReceiverID, int region_id) {

    return 1.0 - (wm->our[passReceiverID]->pos - point).length() /
                 (regions[region_id].rectangle.topLeft() -
                  regions[region_id].rectangle.bottomRight()).length();
}

double CDynamicAttack::calcSenderDistanceFactor(Vector2D passSenderPos, Vector2D point) {
    auto passSenderDist = (passSenderPos - point).length();
    if (passSenderDist > 10.0)
        return 0;
    else if (passSenderDist < 0.5)
        return 0;
    else
        return 1.0 - passSenderDist / 5;
}

double CDynamicAttack::caclClearPathFactor(Vector2D point, Vector2D passSenderPos, double robot_raduis) {
    if (isPathClear(point, passSenderPos, robot_raduis, true))
        return 1.0;
    else
        return 0.0;
}

double CDynamicAttack::calcOneTouchAngleFactor(Vector2D robotPos) {
    double fieldWidth = wm->field->_FIELD_WIDTH;
    double penaltyWidth = wm->field->_PENALTY_WIDTH;
    Vector2D robotBallDir = (playMakeAgent->pos() - robotPos).norm();
    double oneTouchAngle = 60;

    if (robotBallDir.x <= 0)
        return 0;

    auto forwardRotatedDir = robotBallDir.rotate(oneTouchAngle);
    auto backwardRotatedDir = robotBallDir.rotate(-oneTouchAngle);

    double alpha = 0.1;
    Ray2D leftRay(robotPos, forwardRotatedDir * alpha);
    Ray2D rightRay(robotPos, backwardRotatedDir * alpha);

    Line2D oppCornerLine(wm->field->fieldRect().topRight(), wm->field->fieldRect().bottomRight());
    Vector2D highIntersect(Vector2D::INVALIDATED), lowIntersect(Vector2D::INVALIDATED);

    if (leftRay.intersection(oppCornerLine) != Vector2D::INVALIDATED)
        highIntersect = leftRay.intersection(oppCornerLine);
    if (rightRay.intersection(oppCornerLine) != Vector2D::INVALIDATED)
        lowIntersect = rightRay.intersection(oppCornerLine);

    if (highIntersect == Vector2D::INVALIDATED && lowIntersect == Vector2D::INVALIDATED)
        return 0.0;

//    double alpha = 0.1;
//    Line2D leftLine(robotPos, forwardRotatedDir*alpha);
//    Line2D rightLine(robotPos, backwardRotatedDir*alpha);
//
//    double highIntersect = leftLine.getY(wm->field->oppGoal().x);
//    double lowIntersect = rightLine.getY(wm->field->oppGoal().x);
//
//    if(highIntersect<= -penaltyWidth/2)
//        return 0.0;
//

    auto effectiveHigh = ((highIntersect - (Vector2D(wm->field->oppGoal()))).length() > fieldWidth / 2) ?
                         fieldWidth / 2
                                                                                                        : highIntersect.dist(
                    wm->field->oppGoal());
    auto effectiveLow = ((highIntersect - (Vector2D(wm->field->oppGoal()))).length() > (fieldWidth / 2)) ? -(
            fieldWidth / 2) : lowIntersect.dist(wm->field->oppGoal());

    double penaltyOffset = 0.3;
    auto extendedWidth = penaltyWidth + 2 * penaltyOffset;

    auto resultRatio = ((effectiveHigh > extendedWidth / 2) ? extendedWidth / 2 : effectiveHigh
                                                                                  - (effectiveLow <
                                                                                     -extendedWidth / 2)
                                                                                  ? -extendedWidth : effectiveLow) /
                       extendedWidth;
    return resultRatio;
}

double CDynamicAttack::calcWidenessFactor(Vector2D passSenderPos, Vector2D point) {
    double widenessAngle = fabs(
            (passSenderPos - wm->field->oppGoal()).th().degree() - (wm->field->oppGoal() - point).th().degree());
    if (widenessAngle < 0.005)
        return 0.0;
    if (widenessAngle > 170)
        return 1.0;
    return widenessAngle / 180.0;
}

Vector2D CDynamicAttack::getEmptyTarget(const Vector2D &_position, const double &_radius) {
    Vector2D tempTarget;
    QList<Vector2D> finalTargets;
    Vector2D optimalTarget{6, 0};
    finalTargets.clear();
    bool opp{false};
    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (Circle2D(wm->opp.active(i)->pos, 0.6).contains(_position)
            || !wm->field->isInField(_position)
            || wm->field->isInOppPenaltyArea(_position)
            || wm->field->isInOurPenaltyArea(_position)) {
            opp = true;
            break;
        }
    }
    if (!opp)
        finalTargets.append(tempTarget);
    for (int dist_step = 1; dist_step < (_radius / 0.2); dist_step++) {
        auto dist = dist_step * 0.2;

        for (size_t ang_step = 0; ang_step <= 20 * dist; ang_step++) {
            auto ang = -180 + 18.0 * ang_step / dist;
            opp = false;
            tempTarget = _position + Vector2D::polar2vector(dist, ang);
            for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
                if (Circle2D(wm->opp.active(i)->pos, 0.6).contains(tempTarget)
                    || !wm->field->isInField(tempTarget)
                    || wm->field->isInOppPenaltyArea(tempTarget)
                    || wm->field->isInOurPenaltyArea(tempTarget)) {
                    opp = true;
                    break;
                }

            }
            if (!opp) {
                finalTargets.append(tempTarget);
            }
        }
    }
    double optimalmindist = 10000;
    for (const auto &target : finalTargets) {
        double mindist{10000};
        for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
            double dist = wm->opp.active(i)->pos.dist(target);
            if (dist < mindist)
                mindist = dist;
        }
        if (mindist < optimalmindist)
            optimalTarget = target;

    }


    return optimalTarget;
}

void CDynamicAttack::validateSegment(Segment2D &seg) {
    Vector2D sol1, sol2;
    sol1.invalidate();
    sol2.invalidate();
    Vector2D mid;
    mid = (seg.a() + seg.b()) / 2;
    if (wm->field->fieldRect().intersection(Segment2D(seg.a(), mid), &sol1, &sol2)) {
        seg.assign((sol1.isValid()) ? sol1 : sol2, seg.b());
        mid = (seg.a() + seg.b()) / 2;
    }
    sol1.invalidate();
    sol2.invalidate();
    if (wm->field->fieldRect().intersection(Segment2D(mid, seg.b()), &sol1, &sol2)) {
        seg.assign(seg.a(), (sol1.isValid()) ? sol1 : sol2);
        mid = (seg.a() + seg.b()) / 2;
    }
    sol1.invalidate();
    sol2.invalidate();
    if (wm->field->oppPenaltyRect().intersection(Segment2D(seg.a(), mid), &sol1, &sol2)) {
        Vector2D t = (!sol1.isValid()) ? sol2 : (!sol2.isValid()) ? Vector2D(5000, 5000) : (sol1.x < sol2.x) ? sol1
                                                                                                             : sol2;
        seg.assign(t, seg.b());
        mid = (seg.a() + seg.b()) / 2;
    }
    sol1.invalidate();
    sol2.invalidate();
    if (wm->field->oppPenaltyRect().intersection(Segment2D(mid, seg.b()), &sol1, &sol2)) {
        Vector2D t = (!sol1.isValid()) ? sol2 : (!sol2.isValid()) ? Vector2D(5000, 5000) : (sol1.x < sol2.x) ? sol1
                                                                                                             : sol2;
        seg.assign(seg.a(), t);
        mid = (seg.a() + seg.b()) / 2;
    }
    sol1.invalidate();
    sol2.invalidate();
    if (wm->field->oppPenaltyRect().intersection(Segment2D(seg.a(), mid), &sol1, &sol2)) {
        Vector2D t = (!sol1.isValid()) ? sol2 : (!sol2.isValid()) ? Vector2D(5000, 5000) : (sol1.x < sol2.x) ? sol1
                                                                                                             : sol2;
        seg.assign(t, seg.b());
        mid = (seg.a() + seg.b()) / 2;
    }
    sol1.invalidate();
    sol2.invalidate();
    if (wm->field->oppPenaltyRect().intersection(Segment2D(mid, seg.b()), &sol1, &sol2)) {
        Vector2D t = (!sol1.isValid()) ? sol2 : (!sol2.isValid()) ? Vector2D(5000, 5000) : (sol1.x < sol2.x) ? sol1
                                                                                                             : sol2;
        seg.assign(seg.a(), t);
        mid = (seg.a() + seg.b()) / 2;
    }
    sol1.invalidate();
    sol2.invalidate();
    Vector2D t;
    if (t = Segment2D(Vector2D(0, wm->field->_FIELD_WIDTH / 2),
                      Vector2D(0, -wm->field->_FIELD_WIDTH / 2)).intersection(
            Segment2D(seg.a(), mid)), t.isValid()) {
        seg.assign(t, seg.b());
        mid = (seg.a() + seg.b()) / 2;
    }
    sol1.invalidate();
    sol2.invalidate();
    if (t = Segment2D(Vector2D(0, wm->field->_FIELD_WIDTH / 2),
                      Vector2D(0, -wm->field->_FIELD_WIDTH / 2)).intersection(
            Segment2D(mid, seg.b())), t.isValid()) {
        seg.assign(seg.a(), t);
        mid = (seg.a() + seg.b()) / 2;
    }
}

bool CDynamicAttack::inTimePlan() {
    if (playMakeAgent != nullptr) {
        if (wm->ball->pos.dist(playMakeAgent->pos()) < 1.0) {
            return true;
        }

    }
    return false;
}

double CDynamicAttack::calcNotInWayFactor(Vector2D passSenderPos, Vector2D point) {
    Vector2D sol1, sol2, sol3;
    Line2D _path(passSenderPos, wm->field->oppGoal());
    Polygon2D _poly;
    Circle2D(passSenderPos, 2).
            intersection(_path.perpendicular(wm->field->oppGoal()), &sol1, &sol2);

    _poly.addVertex(sol1);
    sol3 = sol1;
    _poly.addVertex(sol2);
    Circle2D(passSenderPos, Robot::robot_radius_new + 0.5).
            intersection(_path.perpendicular(passSenderPos), &sol1, &sol2);

    _poly.addVertex(sol2);
    _poly.addVertex(sol1);
    _poly.addVertex(sol3);

    if (_poly.contains(point)) {
        return 0.0;
    } else {
        return 1.0;
    }

}

bool CDynamicAttack::isPathClearFromOpp(Vector2D _pos1, Vector2D _pos2,
                                        double _radius, double treshold) {
    Vector2D sol1, sol2, sol3;
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

    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (_poly.contains(wm->opp.active(i)->pos)) {
            return false;
        }
    }

    return true;
}

void CDynamicAttack::updateAttackState() {
    switch (attackState) {
        case DynamicAttackState::PlaymakeControl:
            if (!directShot)
                attackState = DynamicAttackState::PlaymakePass;
            break;
        case DynamicAttackState::PlaymakePass:
            if (passDone()) {
                attackState = DynamicAttackState::PositioningControl;
                if (isGoodForOneTouch()) {
                    positionSkill = PositionSkill::OneTouch;
                    oneTouchFailState = 0;
                    oneTouchDoneState = 0;
                } else
                    positionSkill = PositionSkill::Ready;
            } else if (directShot || passFailed())
                attackState = DynamicAttackState::PlaymakeControl;
            break;
        case DynamicAttackState::PositioningControl:
            if (positionTaskDone())
                attackState = DynamicAttackState::PlaymakeControl;
            break;
        default:
            break;
    }
}


bool CDynamicAttack::passDone() {
    double ballDistanceToTarget = currentPlan.recievePoint.point.dist(wm->ball->pos);
    double ballDistanceToPlaymake = playMakeAgent->pos().dist(wm->ball->pos);
    if (ballDistanceToTarget < .3)
        return true;
    if (ballDistanceToPlaymake > 1.5 * ballDistanceToTarget) {
        if (ballDistanceToTarget < 1)
            return true;
    }
    return false;
}

bool CDynamicAttack::isGoodForOneTouch() {
    //todo
    return true;
}

bool CDynamicAttack::positionTaskDone() {

    if (positionSkill == PositionSkill::Ready)
        if ((wm->ball->vel.length() < .02) ||
            (wm->ball->vel.length() < .1 && wm->ball->pos.dist(currentPlan.recievePoint.point) > 2))
            return true;
    if (positionSkill == PositionSkill::OneTouch) {
        double dist = wm->ball->pos.dist(currentPlan.recievePoint.point);
        if (dist > 2)
            oneTouchFailState++;
        if (dist < 1.5)
            oneTouchDoneState++;

        if (oneTouchDoneState > 30 && dist > 2)
            return true;

        if (oneTouchFailState > 100)
            return true;
    }
    return false;

}


bool CDynamicAttack::passFailed() {
    double ballDistanceToTarget = currentPlan.recievePoint.point.dist(wm->ball->pos);
    double ballDistanceToPlaymake = playMakeAgent->pos().dist(wm->ball->pos);
    if (ballDistanceToTarget > 3 && ballDistanceToPlaymake > 2)
        return true;
    return false;
}

double CDynamicAttack::calcRegionProperties(int robot_id, int region_index) {
    // finding nearest opp
    double max_dist{-1};

    double tmp_angle{};
    double tmp_chance{};

    double nearest_opp_robot_dist{100000};
    double angle_weight{15}/*conf.PositionOpenAngle}*/, dist_weight{
            0.1/*conf.PositionOppNearest*/}; // TODO: show in controling in game

    double dist_weight1{0.05};
    double angle_weight1{45};
    double treshold1{0.3};
    double treshold2{0.5};

    double PassMarkChance{-5};//conf.OppPassMarkChance};
    //double chance1{-10};
    for (size_t j{}; j < wm->opp.activeAgentsCount(); j++) { // cal nearest_opp_robot
        //auto tmp_d = regions[regionPriority[matchingIDs[v]]].points[i].dist(wm->opp.active(j)->pos);
        auto tmp = regions[regionPriority[matchingIDs[robot_id]]].points[region_index].dist(wm->opp.active(j)->pos);
        if (tmp < nearest_opp_robot_dist) {
            nearest_opp_robot_dist = tmp;
        }
    }

    //if (nearest_opp_robot_dist > max_dist) {
    //max_dist = nearest_opp_robot_dist;
    //tmp_point = regions[regionPriority[matchingIDs[v]]].points[region_index];
    //}


    CRobot *oppGoalKeaper = findOppGoalKeaper();
    double angle_max{};
    if (oppGoalKeaper != nullptr) {
        Segment2D posRobot_oppGoalK{regions[regionPriority[matchingIDs[robot_id]]].points[region_index],
                                    oppGoalKeaper->pos};
        Segment2D posRobot_oppGoalR{regions[regionPriority[matchingIDs[robot_id]]].points[region_index],
                                    wm->field->oppGoalR()};
        Segment2D posRobot_oppGoalL{regions[regionPriority[matchingIDs[robot_id]]].points[region_index],
                                    wm->field->oppGoalL()};
        const double &&angle_R{angleOfTwoSegment(posRobot_oppGoalK, posRobot_oppGoalR)};
        const double &&angle_L{angleOfTwoSegment(posRobot_oppGoalK, posRobot_oppGoalL)};
        if (angle_R > angle_L)
            angle_max = angle_R;
        else
            angle_max = angle_L;
    }


    auto tmp_a = angle_max;
    double tresholdDist{3};
    if (nearest_opp_robot_dist < tresholdDist)
        tmp_chance = nearest_opp_robot_dist * dist_weight + tmp_a * angle_weight;
    else {
        tmp_chance = nearest_opp_robot_dist * dist_weight1 + tmp_a * angle_weight1;

    }

    if (!isClear(/*playmake->pos()*/wm->ball->pos, wm->field->oppGoal(),
                                    wm->field->oppGoalL().y - wm->field->oppGoal().y, treshold2, "positionInOurWay",
                                    regions[regionPriority[matchingIDs[robot_id]]].points[region_index])) {
        tmp_chance = 0;
        //continue;
    }
    if (!isClear(regions[regionPriority[matchingIDs[robot_id]]].points[region_index], wm->field->oppGoal(),
                 wm->field->oppGoalL().y - wm->field->oppGoal().y,
                 0.2, "positionClear")) {
        tmp_chance = 0;
        //continue;
    }
    double passPathWeight{15};
    if (!isClear(/*playmake->pos()*/ wm->ball->pos, regions[regionPriority[matchingIDs[robot_id]]].points[region_index],
                                     (Robot::robot_radius_new + playMakeAgent->pos().dist(
                                             regions[regionPriority[matchingIDs[robot_id]]].points[region_index]) /
                                                                passPathWeight),
                                     treshold1, "passPathOpen")) {
        tmp_chance = 0;
    }
    return tmp_chance;
}

void CDynamicAttack::support() {

    supportRole.setAgent(supportAgent);
    supportRole.setAvoidPenaltyArea(true);
    supportRole.setSelectedSupporterSkill(SupporterSkill::Move);
    supportRole.setTarget(wm->ball->pos + wm->ball->vel / 2 + rcsc::Vector2D(-1, 0));
//    switch (supporter.) {
//    }
}


void CDynamicAttack::setSupporter(Agent *_supporter) {
    supportAgent = _supporter;
}
