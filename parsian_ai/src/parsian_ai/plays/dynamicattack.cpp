#include <parsian_ai/plays/dynamicattack.h>
#include <parsian_ai/plays/plays.h>

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

    for (int i = 0; i < 8; i++) {
        roleAgents[i] = new CRoleDynamic();
    }
    roleAgentPM = new CRoleDynamic();
    roleAgentPM->setIsPlayMake(true);

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
    for (auto &roleAgent : roleAgents) {
        delete roleAgent;
    }

    delete roleAgentPM;

}

void CDynamicAttack::init(QList<Agent*>& _agents) {
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
    ROS_INFO_STREAM("ali:state  "<< static_cast<int>(attackState));
    globalExecute(agents.size());
    for (auto &p : semiDynamicPosition) {
        drawer->draw(Circle2D(p, .1), QColor(Qt::red), true);
    }
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
    currentPlan.mode = DynamicMode::NoMode;
    currentPlan.agentSize = agentSize;

    //// We Don't have the ball -- counter-attack, blocking, move forward
    //// And Ball is in our field
    if (isBallInOurField) {
        ROS_INFO_STREAM("kian: dont have the ball");
        currentPlan.mode = DynamicMode::NotWeHaveBall;
        if (conf.ChipForward
//            && evalmovefwd()
                ) {
            currentPlan.playmake.init(PlayMakeSkill::Chip, DynamicRegion::Forward);
        } else {
            currentPlan.playmake.init(PlayMakeSkill::Chip, DynamicRegion::Goal);
        }
        for (size_t i = 0; i < agentSize; i++) {
            currentPlan.positionAgents[i].region = DynamicRegion::Near;
            currentPlan.positionAgents[i].skill = positionSkill;
        }
    }
        //// we have ball and
        //// shot prob is more than 50%
    else if (directShot && attackState == DynamicAttackState::PlaymakeControl) {
        ROS_INFO_STREAM("ali: diret shot");
        currentPlan.mode = DynamicMode::DirectKick;
        currentPlan.playmake.init(PlayMakeSkill::Shot, DynamicRegion::Goal);
        for (size_t i = 0; i < agentSize; i++) {
            currentPlan.positionAgents[i].region = DynamicRegion::Best;
            currentPlan.positionAgents[i].skill = PositionSkill::Ready;
        }
    } else if (attackState == DynamicAttackState::PlaymakePass) {
        ROS_INFO_STREAM("ali: pass mode");
        currentPlan.mode = DynamicMode::Pass;
        currentPlan.playmake.init(PlayMakeSkill::Pass, DynamicRegion::Best);
        for (size_t i = 0; i < agentSize; i++) {
            currentPlan.positionAgents[i].region = DynamicRegion::Best;
            currentPlan.positionAgents[i].skill = PositionSkill::OneTouch;
        }
    } else {
        ROS_INFO_STREAM("ali: pass mode");
        currentPlan.mode = DynamicMode::Pass;
        currentPlan.playmake.init(PlayMakeSkill::NoSkill, DynamicRegion::Best);
        for (size_t i = 0; i < agentSize; i++) {
            currentPlan.positionAgents[i].region = DynamicRegion::Best;
            currentPlan.positionAgents[i].skill = PositionSkill::OneTouch;
        }
    }
}

void CDynamicAttack::assignTasks() {
    if (playmake != nullptr) {
        playMake();
    }
    if (currentPlan.agentSize > 0) {
        ROS_INFO_STREAM("kian: too if positioning : currentPlan.agentSize:" << currentPlan.agentSize);
        positioning(semiDynamicPosition);
    }
}

/**
 * @brief CDynamicAttack::dynamicPlanner
 * @param agentSize number of positioning Agents
 */
void CDynamicAttack::dynamicPlanner(int agentSize) {
    for (size_t i = 0; i < 8; i++) {
        matchingIDs[i] = -1;
    }
    for (int i = 0; i < REGION_NUM; i++)
        drawer->draw(regions[i].rectangle);
    updateAttackState();
    makePlan(agentSize);

//    if (agentSize > 0 && (lastAgentCount != agentSize || isPlayMakeChanged())) {
    chooseBestPositons();

    assignId();

    chooseReceiverAndBestPosForPass();
//
//    }

    assignTasks();
    for (size_t i = 0; i < currentPlan.agentSize; i++) {
        if (matchingIDs[i] >= 0) {
            roleAgents[i]->execute();
        } else {
            DBUG(QString("[dynamicAttack - %1] mahiAgentID buged").arg(__LINE__), D_MAHI);
        }
    }
    ROS_INFO("MAHI : 8");

    if (playmake != nullptr && playmake->id() != -1) {
        roleAgentPM->execute();
        ROS_INFO("MAHI : 9");

    }
    lastAgentCount = agentSize;

}

void CDynamicAttack::playMake() {

    drawer->draw(Circle2D(playmake->pos(), 0.1), QColor(Qt::red), true);
    if (teamConfig.color == teamConfig.BLUE) {
        drawer->draw(Circle2D(playmake->pos() + playmake->dir() * 0.08, 0.06), QColor(Qt::blue), true);
    } else {
        drawer->draw(Circle2D(playmake->pos() + playmake->dir() * 0.08, 0.06), QColor(Qt::yellow), true);
    }

    roleAgentPM->setAgent(playmake);
    roleAgentPM->setAvoidPenaltyArea(true);

    Vector2D og = wm->ball->pos - wm->field->ourGoal();
    switch (currentPlan.playmake.skill) {
        case PlayMakeSkill::Dribble:
            ROS_INFO_STREAM("kian: drrible");//<< currentPlan.playmake.skill);
            ROS_INFO_STREAM("kian: passpos: " << currentPlan.passPos.x << ", " << currentPlan.passPos.y
                                              << "//////////////////");
            roleAgentPM->setTargetDir(currentPlan.passPos);
            roleAgentPM->setTarget(oppRob);
            roleAgentPM->setChip(false);
            roleAgentPM->setNoKick(false);
            roleAgentPM->setSelectedPlayMakeSkill(PlayMakeSkill::Dribble); // skill Dribble
            break;
        case PlayMakeSkill::Pass:
            ROS_INFO_STREAM("ali: pass");
            ROS_INFO_STREAM("ali: passpos: " << currentPlan.passPos.x << ", " << currentPlan.passPos.y
                                             << "//////////////////");
            roleAgentPM->setChip(chipOrNot(currentPlan.passPos, 0.5, 0.1));
            roleAgentPM->setTarget(currentPlan.passPos);
            roleAgentPM->setEmptySpot(false);
            roleAgentPM->setNoKick(false);
            if (roleAgentPM->getChip()) {
//            roleAgentPM->setChipDist(appropriateChipSpeed());       //TODO: set chip distanse not speed
                roleAgentPM->setChipDist(conf.MediumDistChip);
            } else {
//            roleAgentPM->setKickSpeed(appropriatePassSpeed());
                roleAgentPM->setKickSpeed(conf.MediumSpeedPass);
            }

            roleAgentPM->setSelectedPlayMakeSkill(PlayMakeSkill::Pass);// Skill Kick
            break;

        case PlayMakeSkill::Chip:
            ROS_INFO_STREAM("chip");
            roleAgentPM->setNoKick(false);
            if (currentPlan.playmake.region == DynamicRegion::Goal) {
                roleAgentPM->setTarget(wm->field->oppGoal());
                roleAgentPM->setChip(true);
                if (wm->ball->pos.x < -2) {
                    roleAgentPM->setChipDist(conf.HighDistChip);
                } else if (wm->ball->pos.x > 4) {
                    roleAgentPM->setChipDist(conf.LowDistChip);
                } else {
                    roleAgentPM->setChipDist(conf.MediumDistChip);

                }
            } else if (currentPlan.playmake.region == DynamicRegion::Forward) {
//                roleAgentPM->setTarget(move_fwd_target);
                roleAgentPM->setTarget(wm->field->oppGoal());
                roleAgentPM->setChip(false);
                roleAgentPM->setKickSpeed(conf.LowDistChip);
            } else {
                roleAgentPM->setChip(true);
                roleAgentPM->setTarget(wm->field->oppGoal());
                roleAgentPM->setChipDist(conf.LowDistChip);
            }
            roleAgentPM->setSelectedPlayMakeSkill(PlayMakeSkill::Chip);// Skill Chip
            break;
        case PlayMakeSkill::Shot : {
            roleAgentPM->setEmptySpot(true);
            roleAgentPM->setChip(false);
            roleAgentPM->setNoKick(false);
            roleAgentPM->setTarget(wm->field->oppGoal());
            roleAgentPM->setKickSpeed(conf.HighSpeedPass); // TODO : 8m/s by profiller
            roleAgentPM->setSelectedPlayMakeSkill(PlayMakeSkill::Shot); // Skill Kick
            break;
        }
        default:
            ROS_INFO_STREAM("default");
            roleAgentPM->setEmptySpot(true);
            roleAgentPM->setChip(false);
            roleAgentPM->setNoKick(false);
            roleAgentPM->setTarget(wm->field->oppGoal());
            // Parsa : ino hamintory avaz kardam kar kard...
            roleAgentPM->setKickSpeed(conf.MediumSpeedPass); // TODO : 8m/s by profiller
            roleAgentPM->setSelectedPlayMakeSkill(PlayMakeSkill::Shot); // Skill Kick
            break;
    }
}

void CDynamicAttack::positioning(QList<Vector2D> _points) {
    // hamid pos
    ROS_INFO_STREAM("hamid inside positioning2");
    bool check = false;
    for (int i = 0; i < currentPlan.agentSize; i++) {
        if (matchingIDs[i] >= 0) {
            roleAgents[i]->setAgent(agents.at(i));
            roleAgents[i]->setAvoidPenaltyArea(true);
            if (i < _points.size()) {
                switch (currentPlan.positionAgents[i].skill) {
                    case PositionSkill::Ready: // Ready For Pass
                        ROS_INFO_STREAM("kian: too switch set : skill: ready");
                        roleAgents[i]->setTarget(_points.at(i));
                        roleAgents[i]->setReceiveRadius(.5);
                        roleAgents[i]->setSelectedPositionSkill(PositionSkill::Ready);// Receive Skill

                        break;
                    case PositionSkill::OneTouch: // OneTouch Reflects
                        ROS_INFO_STREAM("kian: too switch set : skill: onetouch");
                        roleAgents[i]->setWaitPos(_points.at(i));
                        roleAgents[i]->setReceiveRadius(
                                std::max(0.5, 2 - roleAgents[i]->getAgent()->pos()
                                        .dist(roleAgents[i]->getTarget())));

                        // TODO : fix the target
                        roleAgents[i]->setTarget(wm->field->oppGoal());
                        roleAgents[i]->setSelectedPositionSkill(PositionSkill::OneTouch);// OneTouch Skill

                        break;
                    case PositionSkill::Move:
                        ROS_INFO_STREAM("kian: too switch set : skill: move");
                        roleAgents[i]->setReceiveRadius(
                                std::max(0.5, 2 - roleAgents[i]->getAgent()->pos()
                                        .dist(roleAgents[i]->getTarget())));
                        roleAgents[i]->setTarget(_points.at(i));
                        roleAgents[i]->setTargetDir(wm->ball->pos - roleAgents[i]->getAgent()->pos());
                        roleAgents[i]->setSelectedPositionSkill(PositionSkill::Move);

                        break;
                    case PositionSkill::NoSkill:
                        ROS_INFO_STREAM("kian: too switch set : skill: noskill");
                        roleAgents[i]->setSelectedPositionSkill(PositionSkill::Ready);// Receive Skill

                        break;
                        //                    case PositionSkill::Pass:break;
                        //                    case PositionSkill::CatchBall:break;
                        //                    case PositionSkilll::Shot:break;
                        //                    case PositionSkill::Keep:break;
                        //                    case PositionSkill::Chip:break;
                        //                    case PositionSkill::Dribble:break;
                }

                if (roleAgents[i]->getTarget() == currentPlan.passPos) {
                    check = true;
                }
                //debug(QString("pos positions : %1 %2").arg(roleAgents[i]->getTarget().x).arg(roleAgents[i]->getTarget().y), D_PARSA);
            }
        } else {
            DBUG("[dynamicAttack] mahiagent ha eshtebahe chera ?", D_MAHI);
        }
    }
    ROS_INFO_STREAM("hamid end of positioning");
    //assert(check);
}


inline bool CDynamicAttack::chipOrNot(Vector2D target,
                                      double _radius, double _treshold) {
    return !isPathClear(wm->ball->pos, target, _radius, _treshold);
}

bool CDynamicAttack::keepOrNot() {
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

    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (_poly.contains(wm->opp.active(i)->pos)) {
            return false;
        }
    }

    return true;
}

bool CDynamicAttack::isPlayMakeChanged() {

    if (playmake != nullptr) {
        if (playmake->id() != lastPlayMakerId) {
            lastPlayMakerId = playmake->id();
            return true;
        }
    }
    return false;
}

int CDynamicAttack::appropriatePassSpeed() {

    double tempDistance = 0;
    double speed = 0;
    if (playmake != nullptr) {
        tempDistance = playmake->pos().dist(currentPlan.passPos);

        if (false) { // dynamic pass Speed // FALSED IN ROS
            //            if (tempDistance < 2) {
            //                speed = knowledge->getProfile(mahiPlayMaker->id(), tempDistance) + policy()->DynamicPlay_LowSpeedPass();
            //
            //            } else if(tempDistance > 4) {
            //                speed = knowledge->getProfile(mahiPlayMaker->id(), tempDistance) + policy()->DynamicPlay_HighSpeedPass();
            //
            //            } else {
            //                speed = knowledge->getProfile(mahiPlayMaker->id(), tempDistance) + policy()->DynamicPlay_MediumSpeedPass();
            //
            //            }

        } else {
            if (tempDistance < 2) {
                speed = conf.LowSpeedPass;

            } else if (tempDistance > 4) {
                speed = conf.HighSpeedPass;

            } else {
                speed = conf.MediumSpeedPass;

            }

        }
    } else {
        speed = conf.MediumSpeedPass;
    }
    return static_cast<int>(speed);
}


int CDynamicAttack::appropriateChipSpeed() {

    double tempDistance = 0;
    double speed = 0;
    if (playmake != nullptr) {
        tempDistance = playmake->pos().dist(currentPlan.passPos);

        if (false) { // dynamic chip Speed
            //            if (tempDistance < 2) {
            //                speed = knowledge->getProfile(mahiPlayMaker->id(), tempDistance, false) + policy()->DynamicPlay_LowSpeedChip();
            //
            //            } else if(tempDistance > 4) {
            //                speed = knowledge->getProfile(mahiPlayMaker->id(), tempDistance, false) + policy()->DynamicPlay_HighSpeedChip();
            //
            //            } else {
            //                speed = knowledge->getProfile(mahiPlayMaker->id(), tempDistance, false) + policy()->DynamicPlay_MediumSpeedChip();
            //
            //            }

        } else {
            if (tempDistance < 2) {
                speed = conf.LowDistChip;

            } else if (tempDistance > 4) {
                speed = conf.HighDistChip;

            } else {
                speed = conf.MediumDistChip;

            }

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
    Circle2D receiverRegion(currentPlan.passPos, 1.3);
    Vector2D sol1;
    Vector2D sol2;
    if (playmake->pos().dist(wm->ball->pos) > 0.7) {
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
        currentPlan.passPos = regions[regionPriority[0]].rectangle.center();
        drawer->draw(currentPlan.passPos, QColor(0, 0, 100), .5);
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
        tempDist = agents.at(i)->pos().dist(currentPlan.passPos);
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


QString CDynamicAttack::getString(const DynamicMode &_mode) const {
    switch (_mode) {
        default:
        case DynamicMode::NoMode:
            return QString("NoMode");
        case DynamicMode::DefenseClear:
            return QString("DefenseClear");
        case DynamicMode::NotWeHaveBall:
            return QString("NotWeHaveBall");
        case DynamicMode::DirectKick:
            return QString("DirectKick");
        case DynamicMode::Fast:
            return QString("Fast");
        case DynamicMode::Critical:
            return QString("Critical");
        case DynamicMode::Plan:
            return QString("NewPlan");
        case DynamicMode::Forward:
            return QString("Ball In Our Field");
        case DynamicMode::NoPositionAgent:
            return QString("No Agent");
    }
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
    playmake = _playMake;
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
    switch (currentPlan.mode) {

        case DynamicMode::NoMode:
            return false;
            break;
        case DynamicMode::CounterAttack:
            break;
        case DynamicMode::DefenseClear:
            break;
        case DynamicMode::DirectKick:
            break;
        case DynamicMode::Fast:
            break;
        case DynamicMode::Critical:
            break;
        case DynamicMode::NotWeHaveBall:
            break;
        case DynamicMode::Plan:
            break;
        case DynamicMode::Forward:
            break;
        case DynamicMode::NoPositionAgent:
            break;
        case DynamicMode::Pass:
            break;
    }
    return false;
}

bool CDynamicAttack::isPlanDone() {
    switch (currentPlan.mode) {

        case DynamicMode::NoMode:
            return true;
            break;
        case DynamicMode::CounterAttack:
            break;
        case DynamicMode::DefenseClear:
            break;
        case DynamicMode::DirectKick:
            break;
        case DynamicMode::Fast:
            break;
        case DynamicMode::Critical:
            break;
        case DynamicMode::NotWeHaveBall:
            break;
        case DynamicMode::Plan:
            break;
        case DynamicMode::Forward:
            break;
        case DynamicMode::NoPositionAgent:
            break;
        case DynamicMode::Pass:
            break;
    }
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

void CDynamicAttack::chooseBestPositons() {
    clearRobotsRegionsWeights();

    // get the search regions
    QList<Rect2D> searchRegions;
    for (int i{0}; i < REGION_NUM; i++) {
        searchRegions.append(regions[i].rectangle);
    }
    QList<Rect2D> avoidRects;
    avoidRects.append(wm->field->oppPenaltyRect());


    if (attackState == DynamicAttackState::PlaymakeControl) {
        int ballR = -1;
        for (int i{0}; i < REGION_NUM; i++)
            if (regions[i].rectangle.contains(wm->ball->pos + wm->ball->vel))ballR = regions[i].id;
        regionPriority.clear();
        switch (ballR) {
            case 0:
                regionPriority << 4 << 2 << 1 << 5 << 3 << 6;
                break;
            case 1:
                regionPriority << 4 << 3 << 0 << 5 << 2 << 6;
                break;
            case 2:
                regionPriority << 4 << 0 << 5 << 3 << 1 << 6;
                break;
            case 3:
                regionPriority << 4 << 1 << 5 << 2 << 0 << 6;
                break;
            case 4:
                regionPriority << 0 << 1 << 2 << 3 << 5 << 6;
                break;
            case 5:
                regionPriority << 0 << 1 << 2 << 3 << 6 << 4;
                break;
            case 6:
                regionPriority << 0 << 1 << 2 << 3 << 4 << 5;
                break;
            default:
                regionPriority << 0 << 1 << 2 << 3 << 4 << 5;
                break;
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
    if (regionPriority.isEmpty() || playmake == nullptr) return;
    QList<int> robotIDs;
    MWBM matcher;
    for (int i = 0; i < 8; i++) { matchingIDs[i] = -1; }
    for (const auto &a : agents) {
        if (a->id() != playmake->id()) robotIDs.append(a->id());
    }

    matcher.create(robotIDs.count(), robotIDs.count());
    for (int i{0}; i < robotIDs.count(); i++) {
        for (int j{0}; j < robotIDs.count(); j++) {
            auto agentPos = agents.at(i)->pos();
            matcher.setWeight(i, j, agentPos.dist(regions[regionPriority[i]].rectangle.center()));
        }
    }
//    matcher.findMaxMinMatching();
    matcher.findMatching();
    semiDynamicPosition.clear();
    for (int v = 0; v < robotIDs.count(); v++) {
        // todo : find best pos in region from searchRegions.points
        semiDynamicPosition.append(regions[regionPriority[matcher.getMatch(v)]].rectangle.center());
        matchingIDs[v] = matcher.getMatch(v);
    }
}


Vector2D CDynamicAttack::getBestPosToShootToGoal(Vector2D from, double &regionWidth, bool oppGaol) {
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
                    Vector2D(goal.x, BeginPos) - from).rotate(Vector2D::angleBetween(Vector2D(goal.x, BeginPos) - from,
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

bool CDynamicAttack::isPathClear(Vector2D point, Vector2D from, double rad, bool considerRelaxedIDs) {
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
    Vector2D robotBallDir = (playmake->pos() - robotPos).norm();
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

    auto effectiveHigh = ((highIntersect - (Vector2D(wm->field->oppGoal()))).length() > fieldWidth / 2) ? fieldWidth / 2
                                                                                                        : highIntersect.dist(
                    wm->field->oppGoal());
    auto effectiveLow = ((highIntersect - (Vector2D(wm->field->oppGoal()))).length() > (fieldWidth / 2)) ? -(
            fieldWidth / 2) : lowIntersect.dist(wm->field->oppGoal());

    double penaltyOffset = 0.3;
    auto extendedWidth = penaltyWidth + 2 * penaltyOffset;

    auto resultRatio = ((effectiveHigh > extendedWidth / 2) ? extendedWidth / 2 : effectiveHigh
                                                                                  - (effectiveLow < -extendedWidth / 2)
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
    if (t = Segment2D(Vector2D(0, wm->field->_FIELD_WIDTH / 2), Vector2D(0, -wm->field->_FIELD_WIDTH / 2)).intersection(
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
    if (playmake != nullptr) {
        if (wm->ball->pos.dist(playmake->pos()) < 1.0) {
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

bool CDynamicAttack::isPathClearFromOpp(Vector2D _pos1, Vector2D _pos2, double _radius, double treshold) {
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
                attackState = DynamicAttackState ::PlaymakePass;
            break;
        case DynamicAttackState::PlaymakePass:
            if (passDone()) {
                attackState = DynamicAttackState::PositioningControl;
                if (isGoodForOneTouch()) {
                positionSkill = PositionSkill::OneTouch;
                oneTouchFailState = 0;
                oneTouchDoneState = 0;
            }
                else
                    positionSkill = PositionSkill::Ready;
            }
            else if (directShot || passFailed())
                attackState = DynamicAttackState ::PlaymakeControl;
            break;
        case DynamicAttackState::PositioningControl:
            if (positionTaskDone())
                attackState = DynamicAttackState ::PlaymakeControl;
            break;
        default:
            break;
    }
}

bool CDynamicAttack::passDone() {
    double ballDistanceToTarget = currentPlan.passPos.dist(wm->ball->pos);
    double ballDistanceToPlaymake = playmake->pos().dist(wm->ball->pos);
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
        if ((wm->ball->vel.length() < .02) || (wm->ball->vel.length() < .1 && wm->ball->pos.dist(currentPlan.passPos) > 2))
            return true;
    if (positionSkill == PositionSkill::OneTouch) {
        double dist = wm->ball->pos.dist(currentPlan.passPos);
        if (dist > 2)
            oneTouchFailState ++;
        if (dist < 1.5)
            oneTouchDoneState ++;

        if (oneTouchDoneState > 30 && dist > 2)
            return true;

        if (oneTouchFailState > 100)
            return true;
    }
    return false;

}

bool CDynamicAttack::passFailed() {
    double ballDistanceToTarget = currentPlan.passPos.dist(wm->ball->pos);
    double ballDistanceToPlaymake = playmake->pos().dist(wm->ball->pos);
    if (ballDistanceToTarget > 3 && ballDistanceToPlaymake > 2)
        return true;
    return false;
}
