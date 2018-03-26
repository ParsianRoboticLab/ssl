#include "parsian_util/core/knowledge.h"


double CKnowledge::getEmptyAngle(const CField& field, Vector2D p , Vector2D p1 , Vector2D p2 , QList<Circle2D> obs , double &percent ,
                                 double &mostOpenAngle , double &biggestAngle , bool oppGoal , bool _draw) {

    bool drawn = false;
//    QColor rect_color;
    int g , b;
    Vector2D goal_pos;

    if (oppGoal) {
        goal_pos = field.oppGoal();
    } else {
        goal_pos = field.ourGoal();
    }

    double gx1 = p1.x;
    double gy1 = p1.y;
    double gx2 = p2.x;
    double gy2 = p2.y;
    double x = p.x;
    double y = p.y;
    //similar codes can be found in cl/evalpos.c
    double d, a1, a2, a, l, a0, q1, q2, al;
    double la, lb, lc;
    double ox, oy;
    la = gy2 - gy1;
    lb = gx1 - gx2;
    lc = -gx1 * la - gy1 * lb;
    bool inobs = false;
    range tmpr{};
    int count = 0;
    int i, j;
    d = 0;
    range r[20];
    bool flag[20];
    for (i = 0; i < 20; i++) {
        flag[i] = false;
    }
    al = getangle(x, y, (gx1 + gx2) * 0.5, (gy1 + gy2) * 0.5);
    q1 = getangle(x, y, gx1, gy1) - al;
    q2 = getangle(x, y, gx2, gy2) - al;
    q1 = normalang(q1);
    q2 = normalang(q2);
    if (normalang(q1 - q2) > 0) {
        a = q1;
        q1 = q2;
        q2 = a;
    }
    double openangle = 0;
    for (i = 0; i < obs.count(); ++i) {
        ox = obs[i].center().x;
        oy = obs[i].center().y;
        double rad = obs[i].radius();
        l = len(x, y, ox, oy);
        if (l < rad) {
            inobs = true;
            break;
        }
        a1 = ox * la + oy * lb + lc;
        a2 = x * la + y * lb + lc;
        if (a1 > 0) {
            a1 = 1;
        } else {
            a1 = -1;
        }
        if (a2 > 0) {
            a2 = 1;
        } else {
            a2 = -1;
        }
        a1 = a1 * a2;
        if (a1 > 0) {
            a = normalang(getangle(x, y, ox, oy) - al);
            a0 = asin(rad / l);
            a1 = a - a0;
            a2 = a + a0;
            if (a1 < -M_PI * 0.95 * 0.5) {
                a1 = -M_PI * 0.95 * 0.5;
            }
            if (a2 < -M_PI * 0.95 * 0.5) {
                a2 = -M_PI * 0.95 * 0.5;
            }
            if (a1 > +M_PI * 0.95 * 0.5) {
                a1 =  M_PI * 0.95 * 0.5;
            }
            if (a2 > +M_PI * 0.95 * 0.5) {
                a2 =  M_PI * 0.95 * 0.5;
            }
            if (normalang(a1 - a2) > 0) {
                a = a1;
                a1 = a2;
                a2 = a;
            }
            a1 = normalang(a1);
            a2 = normalang(a2);
            if (normalang(a1 - q1) <= 0) {
                a1 = q1;
            }
            if (normalang(a1 - q2) >= 0) {
                a1 = q2;
            }
            if (normalang(a2 - q1) <= 0) {
                a2 = q1;
            }
            if (normalang(a2 - q2) >= 0) {
                a2 = q2;
            }
            if (normalangabs(a1 - a2) >= 0.001) {
                r[count].a = a1;
                r[count].b = a2;
                count ++;
            }
        }
    }
    if (!inobs) {
        for (i = 0; i < count; i++)
            for (j = 0; j < count - 1; j++) {
                if (normalang(r[j].a - r[j + 1].a) > 0) {
                    tmpr = r[j];
                    r[j] = r[j + 1];
                    r[j + 1] = tmpr;
                }
            }
        for (i = 0; i < count - 1; i++) {
            if (normalang(r[i + 1].a - r[i].b) < 0) {
                r[i + 1].a = r[i].a;
                if (normalang(r[i + 1].b - r[i].b) < 0) {
                    r[i + 1].b = r[i].b;
                }
                flag[i] = true;
            }
        }
        bool changed = false;
        if (count > 0) {
            double lastBlockedDir = q1;
            int k = 0;
            mostOpenAngle = 0;
            biggestAngle = 0;
            for (i = 0; i < count; i++) {
                if (! flag[i]) {
                    if ((k == 0) && (normalang(q1 - r[i].a) >= 0)) {

                    }
                    /*                    else if ((i == count-1) && (normalang(r[count-1].b - q2) >= 0))
                    {

                    }*/
                    else {
                        float dist = normalangabs(r[i].a - lastBlockedDir);
                        float bisect = normalang(normalang(r[i].a - lastBlockedDir) / 2.0 + lastBlockedDir);
                        if (dist >= biggestAngle) {
                            biggestAngle = dist;
                            mostOpenAngle = bisect;
                            changed = true;
                        }
                    }
                    if (_draw) {
                        Line2D line1(p, p + Vector2D::unitVector((lastBlockedDir + al) * 180.0 / M_PI));
                        Line2D line2(p, p + Vector2D::unitVector(((r[i].a + al) * 180.0 / M_PI)));
                        Line2D goal_line(goal_pos, Vector2D(goal_pos.x, goal_pos.y + 2.0));
                        Vector2D p3(line1.intersection(goal_line));
                        Vector2D p4(line2.intersection(goal_line));

                        Polygon2D polygon_draw;
                        polygon_draw.addVertex(p);
                        polygon_draw.addVertex(p3);
                        polygon_draw.addVertex(p4);
                        polygon_draw.addVertex(p);
//                        draw( polygon_draw, rect_color, true);
                        drawn = true;

                        //draw(Segment2D(p, p+Vector2D::unitVector((bisect+al) * 180.0 / M_PI)*5 ), "black");
                        //draw(Segment2D(p, p+Vector2D::unitVector((r[i].a+al) * 180.0 / M_PI)*5 ), "black");
                        //draw(Segment2D(p, p+Vector2D::unitVector((lastBlockedDir+al) * 180.0 / M_PI)*5 ), "purple");
                    }
                    lastBlockedDir = r[i].b;
                    k ++;
                }
            }
            if (normalang(r[count - 1].b - q2) <= 0) {
                float dist = normalangabs(q2 - lastBlockedDir);
                float bisect = normalang(normalang(q2 - lastBlockedDir) / 2.0 + lastBlockedDir);
                if (dist >= biggestAngle) {
                    biggestAngle = dist;
                    mostOpenAngle = bisect;
                    changed = true;
                }
                if (_draw) {
                    Line2D line1(p, p + Vector2D::unitVector((lastBlockedDir + al) * 180.0 / M_PI));
                    Line2D line2(p, p + Vector2D::unitVector(((q2 + al) * 180.0 / M_PI)));
                    Line2D goal_line(goal_pos, Vector2D(goal_pos.x, goal_pos.y + 2.0));
                    Vector2D p1(line1.intersection(goal_line));
                    Vector2D p2(line2.intersection(goal_line));

                    Polygon2D polygon_draw;
                    polygon_draw.addVertex(p);
                    polygon_draw.addVertex(p1);
                    polygon_draw.addVertex(p2);
                    polygon_draw.addVertex(p);
//                    draw( polygon_draw, rect_color, true);
                    drawn = true;

                    //                        draw(Segment2D(p, p+Vector2D::unitVector((bisect+al) * 180.0 / M_PI)*5 ), "blue");
                    //                        draw(Segment2D(p, p+Vector2D::unitVector((q2+al) * 180.0 / M_PI)*5 ), "red");
                    //                        draw(Segment2D(p, p+Vector2D::unitVector((lastBlockedDir+al) * 180.0 / M_PI)*5 ), "orange");
                }
            }
        }
        for (i = 0; i < count; i++) {
            if (! flag[i]) {
                d += normalang(r[i].b - r[i].a);
                flag[i] = true;
            }
        }
        openangle = d;
        openangle = normalangabs(q2 - q1) - openangle;
        d /= normalangabs(q2 - q1);
        d = 1 - d;
        if (!changed || (d < 0.001)) {
            if (count == 0) {
                biggestAngle = normalangabs(q2 - q1);
            } else {
                biggestAngle = 0;
            }
            mostOpenAngle = normalang(normalang(q2 - q1) / 2.0 + q1);
        }
        mostOpenAngle = normalang(mostOpenAngle + al);
        biggestAngle *= 180.0 / M_PI;
        mostOpenAngle *= 180.0 / M_PI;
    } else {
        mostOpenAngle = normalang(0.5 * (q1 + q2) + al);
        biggestAngle = 0;
        mostOpenAngle *= 180.0 / M_PI;
        //                         draw(Segment2D(p, p+Vector2D::unitVector((mostOpenAngle) )*5 ), "blue");

        d = 0.0;
    }
    percent = d;

    if (_draw && !drawn) {
        Vector2D p3, p4;
        if (oppGoal) {
            p3 = field.oppGoalL();
            p4 = field.oppGoalR();
        } else {
            p3 = field.ourGoalL();
            p4 = field.ourGoalR();
        }
        Polygon2D polygon_draw;
        polygon_draw.addVertex(p);
        polygon_draw.addVertex(p3);
        polygon_draw.addVertex(p4);
        polygon_draw.addVertex(p);

//        draw( polygon_draw, rect_color, true);
    }
    return openangle * 180.0 / M_PI;
}

Vector2D CKnowledge::getReflectPos(const CField& field, Vector2D goal, double dist, Vector2D _ballpos) {
    Vector2D res;
    Segment2D dummySeg(goal, goal + Vector2D(-5, 0));
    Vector2D nearest(dummySeg.nearestPoint(_ballpos));
    Vector2D sol1, sol2;
    Rect2D oppField(0, field._FIELD_HEIGHT / 2 - 0.01, field._FIELD_WIDTH / 2 + 0.01, field._FIELD_HEIGHT - 0.01);
    Circle2D oppCircle(Vector2D(field._FIELD_WIDTH / 2, 0) - Vector2D(1, 0), dist);

    res.x = nearest.x;
    res.y = nearest.y * 2 - _ballpos.y;

    dummySeg.assign(field.oppGoal(), (res - field.oppGoal()).norm() * 12);
    //    oppField.intersection(dummySeg,&sol1,&sol2);
    oppCircle.intersection(dummySeg, &sol1, &sol2);
    if (field.isInField(sol1)) {
        res = sol1;
    } else {
        res = sol2;
    }

    return res;
}

int CKnowledge::getNearestRobotToPoint(CTeam _team, Vector2D _point) {
    double minDist = 1.0e13;
    int nearest = -1;
    ROS_INFO_STREAM("HA");
    for (int i = 0; i < _team.activeAgentsCount(); i++) {
        if (_team.active(i)->inSight <= 0) {
            continue;
        }
        ROS_INFO_STREAM("HA" << _team.activeAgentsCount());
        double dist = (_team.active(i)->pos - _point).length();
        if (dist < minDist) {
            minDist = dist;
            nearest = _team.active(i)->id;
            ROS_INFO_STREAM("HA" << nearest);
        }
    }
    return nearest;
}

double CKnowledge::kickTimeEstimation(CAgent *_agent, Vector2D _target, const CBall& _ball, const double& _VMax, double AccMaxForward, double DecMax, double AccMaxNormal) {
    QList<int> ourRelax, oppRelax;
    Vector2D finalPos;
    Vector2D ballPosInFuture;
    Vector2D s1, s2;
    Segment2D ballPath(_ball.pos, _ball.pos + _ball.vel.norm() * 10);
    Circle2D robotAreaNear(_agent->pos(), 0.4);

    if (_ball.vel.length() > 0.2) {
        if ((robotAreaNear.intersection(ballPath, &s1, &s2) != 0) && _ball.whenBallReachToPoint(_ball.pos.dist(_agent->pos())) >= 0) {
            return _ball.whenBallReachToPoint(_ball.pos.dist(_agent->pos()));
        }

        for (double i = 0 ; i < 3 ; i += 0.03) {
            ballPosInFuture = _ball.getPosInFuture(i);
            finalPos = ballPosInFuture - (_target - ballPosInFuture).norm() * 0.11;
            if (timeNeeded(_agent, finalPos, _VMax, AccMaxForward, DecMax, AccMaxNormal) <= i + 0.1) {
                //draw(finalPos,1,QColor(Qt::blue));
                return i;
            }
        }

    }

    finalPos = _ball.pos - (_target - _ball.pos).norm() * 0.11;
//    draw(finalPos);
    return 100 - timeNeeded(_agent, finalPos, _VMax, AccMaxForward, DecMax, AccMaxNormal);

}

double CKnowledge::timeNeeded(CAgent *_agentT, Vector2D posT,
                              double vMax, double AccMaxForward, double DecMax, double AccMaxNormal) {

    double _x3;
    double acc;
    double dec = DecMax;
    double xSat;
    Vector2D tAgentVel = _agentT->vel();
    Vector2D tAgentDir = _agentT->dir();
    double veltan = (tAgentVel.x) * cos(tAgentDir.th().radian()) + (tAgentVel.y) * sin(tAgentDir.th().radian());
    double offset = 0;
    double velnorm = -1 * (tAgentVel.x) * sin(tAgentDir.th().radian()) + (tAgentVel.y) * cos(tAgentDir.th().radian());
    double distEffect = 1, angCoef = 0.003;
    double dist = 0;
    double rrtAngSum = 0;
    QList <Vector2D> _result;
    Vector2D _target;

    _result.clear();

    if (_result.size() >= 3) {
        for (int i = 0 ; i < _result.size() - 1; i++) {
            dist += _result[i].dist(_result[i + 1]);
        }
        for (int i = 1 ; i < _result.size() - 1; i++) {
            rrtAngSum += fabs(Vector2D::angleBetween(_result[i] - _result[i - 1] , _result[i + 1] - _result[i]).degree());
        }
        distEffect = dist / _agentT->pos().dist(posT);
        distEffect += rrtAngSum * angCoef;
        distEffect = std::max(1.0, distEffect);
    }

    if (tAgentVel.length() < 0.2) {
        acc = (AccMaxForward + AccMaxNormal) / 2;

    } else {
        acc = AccMaxForward * (fabs(veltan) / tAgentVel.length()) + AccMaxNormal * (fabs(velnorm) / tAgentVel.length());
    }

    double vMaxReal = sqrt(((_agentT->pos().dist(posT) + (tAgentVel.length() * tAgentVel.length() / 2 * acc)) * 2 * acc * dec) / (acc + dec));
    vMaxReal = min(vMaxReal, 4);
    vMax = min(vMax, vMaxReal);
    xSat = sqrt(((vMax * vMax) - (tAgentVel.length() * tAgentVel.length())) / acc) + sqrt((vMax * vMax) / dec);
    _x3 = (-1 * tAgentVel.length() * tAgentVel.length()) / (-2 * fabs(DecMax)) ;

    if (_agentT->pos().dist(posT) < _x3) {
        return std::max(0.0, (tAgentVel.length() / DecMax - offset) * distEffect);
    }

    if (tAgentVel.length() < (vMax)) {
        if (_agentT->pos().dist(posT) > xSat) {
            return std::max(0.0, (-1 * offset + vMax / dec + (vMax - tAgentVel.length()) / acc + (_agentT->pos().dist(posT) - ((vMax * vMax / (2 * dec)) + ((vMax + tAgentVel.length()) * (vMax - tAgentVel.length()) / acc)) / 2) / vMax) * distEffect);
        }
        return std::max(0.0, (vMax / dec + (vMax - tAgentVel.length()) / acc - offset) * distEffect);

    } else {
        return std::max(0.0, (vMax / dec + (_agentT->pos().dist(posT) - ((vMax * vMax / (2 * dec)))) / vMax - offset) * distEffect);
    }

}

double CKnowledge::oneTouchAngle(Vector2D pos,
                                 Vector2D vel,
                                 Vector2D ballVel,
                                 Vector2D ballDir,
                                 Vector2D goal,
                                 double lambda,
                                 double gamma) {
    const double &ang1 = (-ballDir).th().degree();
    const double &ang2 = (goal - pos).th().degree();
    double theta = AngleDeg::normalize_angle(ang2 - ang1);
    double th = fabs(theta) * _DEG2RAD;
    float vkick = 8; // agent->self()->kickValueSpeed(kickSpeed, false);// + Vector2D::unitVector(self().pos.d).innerProduct(self().vel);
    double v = (ballVel - vel).length();
    double th1;
    double fmin = 1e10;
    double f;
    double th1best = 0;
    for (int k = 0; k < 6000; k++) {
        th1 = ((float)k / 6000.0) * th;
        f  = gamma * v * (1.0 / std::tan(th - th1)) * sin(th1) - lambda * v * cos(th1) - vkick;
        if (fabs(f) < fmin) {
            fmin = fabs(f);
            th1best = th1;
        }
    }
    th1 = th1best;
    th1 *= _RAD2DEG;
    AngleDeg::normalize_angle(th1);
    double ang = 0;
    if (theta > 0) {
        ang = ang1 + th1;
    } else {
        ang = ang1 - th1;
    }

    return ang;
}


inline double CKnowledge::getangle(double x1, double y1, double x2, double y2) {
    return atan2(y2 - y1, x2 - x1);
}

inline double CKnowledge::len(double x1, double y1, double x2, double y2) {
    return hypot(x1 - x2, y1 - y2);
}

inline double CKnowledge::len2(double x1, double y1, double x2, double y2) {
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}


inline double CKnowledge::normalang(double dir) {
    const double _2PI = 2.0 * M_PI;
    if (dir < -2.0 * M_PI || 2.0 * M_PI < dir) {
        dir = fmod(dir, _2PI);
    }
    if (dir < -M_PI) {
        dir += 2.0 * M_PI;
    }
    if (dir > M_PI) {
        dir -= 2.0 * M_PI;
    }
    return dir;
}

inline double CKnowledge::normalangabs(double dir) {
    const double _2PI = 2.0 * M_PI;
    if (dir < -2.0 * M_PI || 2.0 * M_PI < dir) {
        dir = fmod(dir, _2PI);
    }
    if (dir < -M_PI) {
        dir += 2.0 * M_PI;
    }
    if (dir > M_PI) {
        dir -= 2.0 * M_PI;
    }
    if (dir < 0) {
        return -dir;
    }
    return dir;
}
