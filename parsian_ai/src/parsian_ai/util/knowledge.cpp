#include <parsian_ai/util/knowledge.h>
#include <parsian_ai/util/worldmodel.h>
#include <ros/ros.h>

#include <cmath>

Knowledge::Knowledge() {

    initVariables();

}

Knowledge::~Knowledge() = default;

void Knowledge::initVariables() {
    variables[Def_Var].toMap()["defenseClearer"] = 0;
    variables[Def_Var].toMap()["transientFlag"] = false;
    variables[Def_Var].toMap()["stateForMark"] = QString("");
    variables[Def_Var].toMap()["lastStateForMark"] = QString("");
    variables[Def_Var].toMap()["defenseOneTouchMode"] = false;
    variables[Def_Var].toMap()["defenseClearMode"] = false;
    variables[Def_Var].toMap()["goalKeeperClearMode"] = false;
    variables[Def_Var].toMap()["goalKeeperOneTouchMode"] = false;
    variables[Def_Var].toMap()["clearing"] = "false";

}
///////////////////////////////////////////////////////
// utility Functions for handling emptyAngle Structures
///////////////////////////////////////////////////////

bool operator < (const emptyAngleStruct& a, const emptyAngleStruct& b) {
    return AngleDeg::normalize_angle(a.angle - b.angle) < 0.0;
}

bool operator > (const emptyAngleStruct& a, const emptyAngleStruct& b) {
    return AngleDeg::normalize_angle(a.angle - b.angle) > 0.0;
}

bool operator <= (const emptyAngleStruct& a, const emptyAngleStruct& b) {
    return AngleDeg::normalize_angle(a.angle - b.angle) <= 0.0;
}

bool operator >= (const emptyAngleStruct& a, const emptyAngleStruct& b) {
    return AngleDeg::normalize_angle(a.angle - b.angle) >= 0.0;
}

bool operator == (const emptyAngleStruct& a, const emptyAngleStruct& b) {
    return AngleDeg::normalize_angle(a.angle - b.angle) == 0.0;
}

/////////////////////////////
// Functions For Calculations
/////////////////////////////

Vector2D Knowledge::getPointInDirection(Vector2D firstPoint, Vector2D secondPoint, double proportion) {
    firstPoint = firstPoint + (secondPoint - firstPoint).norm() * proportion * (Segment2D(secondPoint , firstPoint).length());
    return firstPoint;
}

inline double getAngle(float x1, float y1, float x2, float y2) {
    return std::atan2(y2 - y1, x2 - x1);
}

inline float normalang(float dir) {
    const float _2PI = 2.0 * M_PI;
    if (dir < -2.0 * M_PI || 2.0 * M_PI < dir) {
        dir = std::fmod(dir, _2PI);
    }
    if (dir < -M_PI) {
        dir += 2.0 * M_PI;
    }
    if (dir > M_PI) {
        dir -= 2.0 * M_PI;
    }
    return dir;
}

inline float len(float x1, float y1, float x2, float y2) {
    return std::hypot(x1 - x2, y1 - y2);
}

int factorial(int n) {
    if (n == 1 | n == 0) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

double Knowledge::getEmptyAngle(Vector2D p, Vector2D p1, Vector2D p2,
                                QList<Circle2D> obs, double& percent,
                                double &mostOpenAngle,
                                double& biggestAngle, bool oppGoal,
                                bool _draw) {
    bool drawn = false;
    QColor rect_color;
    if (oppGoal) {
        int r , g , b;
        QColor("darkcyan").getRgb(&r , &g, &b);
        rect_color =  QColor(r, g, b, 21);
    } else {
        int r , g , b;
        QColor("magenta").getRgb(&r , &g, &b);
        rect_color = QColor(r , g, b, 21);
    }
    Vector2D goal_pos;

    if (oppGoal) {
        goal_pos = wm->field->oppGoal();
    } else {
        goal_pos = wm->field->ourGoal();
    }

    float gx1 = p1.x;
    float gy1 = p1.y;
    float gx2 = p2.x;
    float gy2 = p2.y;
    float x = p.x;
    float y = p.y;
    //similar codes can be found in cl/evalpos.c
    double d, a1, a2, a, l, a0, q1, q2, al;
    double la, lb, lc;
    float ox, oy;
    la = gy2 - gy1;
    lb = gx1 - gx2;
    lc = -gx1 * la - gy1 * lb;
    bool inobs = false;
    int par = 0;
    bool tmp;
    CKnowledge::range tmpr{};
    int count = 0;
    int i, j;
    d = 0;
    CKnowledge::range r[20];
    bool flag[20];
    for (i = 0; i < 20; i++) {
        flag[i] = false;
    }
    al = getAngle(x, y, (gx1 + gx2) * 0.5, (gy1 + gy2) * 0.5);
    q1 = getAngle(x, y, gx1, gy1) - al;
    q2 = getAngle(x, y, gx2, gy2) - al;
    q1 = normalang(q1);
    q2 = normalang(q2);
    if (normalang(q1 - q2) > 0) {
        a = q1;
        q1 = q2;
        q2 = a;
    }
    float openangle = 0;
    for (i = 0; i < obs.count(); ++i) {
        ox = obs[i].center().x;
        oy = obs[i].center().y;
        float rad = obs[i].radius();
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
            a = normalang(getAngle(x, y, ox, oy) - al);
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
            if (std::fabs(normalang(a1 - a2)) >= 0.001) {
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
            float lastBlockedDir = q1;
            int k = 0;
            mostOpenAngle = 0;
            biggestAngle = 0;
            for (i = 0; i < count; i++) {
                if (flag[i] == false) {
                    if ((k == 0) && (normalang(q1 - r[i].a) >= 0)) {

                    }
                    /*                    else if ((i == count-1) && (normalang(r[count-1].b - q2) >= 0))
                    {

                    }*/
                    else {
                        float dist = std::fabs(normalang(r[i].a - lastBlockedDir));
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
                        Vector2D p1(line1.intersection(goal_line));
                        Vector2D p2(line2.intersection(goal_line));

                        Polygon2D polygon_draw;
                        polygon_draw.addVertex(p);
                        polygon_draw.addVertex(p1);
                        polygon_draw.addVertex(p2);
                        polygon_draw.addVertex(p);
                        drawer->draw(polygon_draw, rect_color, true);
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
                float dist = std::fabs(normalang(q2 - lastBlockedDir));
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
                    drawer->draw(polygon_draw, rect_color, true);
                    drawn = true;

                    //                        draw(Segment2D(p, p+Vector2D::unitVector((bisect+al) * 180.0 / M_PI)*5 ), "blue");
                    //                        draw(Segment2D(p, p+Vector2D::unitVector((q2+al) * 180.0 / M_PI)*5 ), "red");
                    //                        draw(Segment2D(p, p+Vector2D::unitVector((lastBlockedDir+al) * 180.0 / M_PI)*5 ), "orange");
                }
            }
        }
        for (i = 0; i < count; i++) {
            if (flag[i] == false) {
                d += normalang(r[i].b - r[i].a);
                flag[i] = true;
            }
        }
        openangle = d;
        openangle = std::fabs(normalang(q2 - q1)) - openangle;
        d /= std::fabs(normalang(q2 - q1));
        d = 1 - d;
        if (!changed || (d < 0.001)) {
            if (count == 0) {
                biggestAngle = std::fabs(normalang(q2 - q1));
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
        Vector2D p1, p2;
        if (oppGoal) {
            p1 = wm->field->oppGoalL();
            p2 = wm->field->oppGoalR();
        } else {
            p1 = wm->field->ourGoalL();
            p2 = wm->field->ourGoalR();
        }
        Polygon2D polygon_draw;
        polygon_draw.addVertex(p);
        polygon_draw.addVertex(p1);
        polygon_draw.addVertex(p2);
        polygon_draw.addVertex(p);
        drawn = true;

        drawer->draw(polygon_draw, rect_color, true);
    }
    return openangle * 180.0 / M_PI;
}


double Knowledge::getEmptyAngle(Vector2D p, Vector2D p1, Vector2D p2, QList<Circle2D> obs, double& percent, double &mostOpenAngle, double& biggestAngle) {
    QList<emptyAngleStruct> r;
    emptyAngleStruct q1{}, q2{};
    q1.begin = false;
    q1.angle = (p1 - p).th().degree();
    q2.begin = true;
    q2.angle = (p2 - p).th().degree();
    if (q2 < q1) {
        emptyAngleStruct tmp{};
        tmp = q1;
        q1 = q2;
        q2 = tmp;
    }
    q1.begin = false;
    q2.begin = true;
    r.append(q1);
    r.append(q2);
    for (int i = 0; i < obs.count(); i++) {
        Vector2D sol1, sol2;
        if (obs[i].tangent(p, &sol1, &sol2) == 2) {
            double ang1 = (sol1 - p).th().degree();
            double ang2 = (sol2 - p).th().degree();
            emptyAngleStruct s1{}, s2{};
            s1.begin = true;
            s2.begin = false;
            if (AngleDeg::normalize_angle(ang1 - ang2) < 0) {
                s1.angle = ang1;
                s2.angle = ang2;
            } else {
                s2.angle = ang1;
                s1.angle = ang2;
            }
            if ((s1 >= q1) && (s1 <= q2)) {
                r.append(s1);
            }
            if ((s2 >= q1) && (s2 <= q2)) {
                r.append(s2);
            }
            if ((s1 <= q1) && (s2 >= q1)) {
                r[0].begin = true;
            }
            if ((s2 >= q2) && (s1 <= q2)) {
                r[1].begin = false;
            }
            /*            if (!(s1 < q1 || (!(s1 < q2)))) r.append(s1);
            else {
                if ((s1 < q1) && (!(s2 < q1))) r[0].begin = true;
                //if (s1 < q1) r[0].begin = true;
            }
            if (!(s2 < q1 || (!(s2 < q2)))) r.append(s2);
            else {
                if ((s1 < q2) && (!(s2 < q2))) r[1].begin = false;
                //if ((s1 < q1) && (!(s2 < q1))) r[0].begin = true;
            }*/
        }
    }

    for (int i = 0; i < r.count(); i++) {
        drawer->draw(Segment2D(p, p + Vector2D::unitVector(r[i].angle)), "red");
    }

    emptyAngles.clear();
    AngleRange rng;
    rng.begin = r[0].angle;
    rng.end = r[0].angle;
    if (r[0].begin) {
        emptyAngles.append(rng);
    }
    rng.begin = r[1].angle;
    rng.end = r[1].angle;
    if (!r[1].begin) {
        emptyAngles.append(rng);
    }
    qSort(r.begin(), r.end());
    double ang = 0.0;
    biggestAngle = 0.0;
    int par = 0;
    for (int i = 0; i < r.count() - 1; i++) {
        if (r[i].begin) {
            par ++;
        }
        if (!r[i].begin) {
            par --;
        }
        if (par < 0) {
            par = 0;
        }
        if (i >= 0) {
            if (!r[i].begin && r[i + 1].begin && (par == 0)) {
                AngleRange tempRange;
                tempRange.begin = r[i].angle;
                tempRange.end = r[i + 1].angle;
                emptyAngles.append(tempRange);
                double d = fabs(AngleDeg::normalize_angle(r[i + 1].angle - r[i].angle));
                ang += d;
                if (d > biggestAngle) {
                    biggestAngle = d;
                    mostOpenAngle = AngleDeg::bisect(AngleDeg(r[i].angle), AngleDeg(r[i + 1].angle)).degree();
                }
                //            draw(Segment2D(p, p + Vector2D::unitVector(r[i-1].angle) * 2.0), "red");
                //            draw(Segment2D(p, p + Vector2D::unitVector(r[i].angle) * 2.0), "blue");
                //draw(Segment2D(p, p + Vector2D::unitVector(r[i-1].angle) * 2.0), "red");

            }
        }
    }
    double totalang = fabs(AngleDeg::normalize_angle(q1.angle - q2.angle));
    percent = ang / totalang;
    return ang;
}

Vector2D Knowledge::getEmptyPosOnGoal(Vector2D from, double &regionWidth, bool oppGoal, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs, double wOpenness, bool _draw) {
    QList<Circle2D> c;
    for (int i = 0; i < wm->our.activeAgentsCount(); i++) {
        if (!ourRelaxedIDs.contains(wm->our.active(i)->id)) {
            c.append(Circle2D(wm->our.active(i)->pos, wm->our.active(i)->robotRadius()));
        }
    }
    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (!oppRelaxedIDs.contains(wm->opp.active(i)->id)) {
            c.append(Circle2D(wm->opp.active(i)->pos, wm->opp.active(i)->robotRadius()));
        }
    }
    Vector2D goalL, goalR;
    if (oppGoal) {
        goalL = wm->field->oppGoalL();
        goalR = wm->field->oppGoalR();
    } else {
        goalL = wm->field->ourGoalL();
        goalR = wm->field->ourGoalR();
    }
    double angle = 0.0, biggestangle = 0.0;
    getEmptyAngle(from, goalL, goalR, c, regionWidth, angle, biggestangle, oppGoal, _draw);
    double goalWidth = fabs(Vector2D::angleBetween(goalL - from, goalR - from).degree());
    regionWidth = wOpenness * regionWidth + (1.0 - wOpenness) * goalWidth / 180.0;
    //    if (wOpenness > 0.5)
    //        regionWidth = regionWidth * goalWidth / 30.0;
    //  debug(QString("regionWidth : %1 wOpenness : %2").arg(regionWidth).arg(wOpenness),D_SEPEHR);
    if (regionWidth > 1.0) {
        regionWidth = 1.0;
    }
    Vector2D p = Segment2D(goalL, goalR).intersection(Line2D(from, AngleDeg(angle)));
    if (p.valid()) {
        return p;
    }
    return (goalL + goalR) / 2.0;
}

int Knowledge::Matching(const QList <Agent*> robots, const QList <Vector2D> pointsToMatch, QList <int> &matchPoints) {
    QList <int> tempForMatch;
    tempForMatch.clear();
    for (int i = 0 ; i < robots.count() ; i++) {
        tempForMatch.append(i);
    }
    double D = 100000000000;
    double tempD = 0;
    QList<QList <int> > combo = generateCombinations(tempForMatch);
    matchPoints.clear();
    if (robots.count() == pointsToMatch.count()) {
        for (int i = 0 ; i < factorial(robots.count()) ; i++) {
            tempD = 0;
            for (int j = 0 ; j < robots.count() ; j++) {
                tempD += pointsToMatch[combo[i][j]].dist(robots[j]->pos());
            }
            if (tempD < D) {
                D = tempD;
                matchPoints.clear();
                matchPoints.append(combo[i]);
            }
        }
        return 1;
    } else {
        return -1;
    }

}

Vector2D Knowledge::getEmptyPosOnGoalForPenalty(double n, bool oppGoal, double th, Agent* ourAgent) {

    Vector2D target, goalieR, goalieL;
    int goalieID;
    CRobot* goalie;
    double distanceR, distanceL;

//    Line2D goalLine;

    AngleDeg rightDeg, leftDeg;

    if (oppGoal) {
//        goalLine.assign(wm->field->oppGoalL(), wm->field->oppGoalR());
        goalieID = wm->opp.data->goalieID;
        goalie = wm->opp[goalieID];
        goalieR = wm->field->oppGoalR();
        goalieL = wm->field->oppGoalL();

    } else {
//        goalLine.assign(wm->field->ourGoalL(), wm->field->ourGoalR());
        goalieID = wm->our.data->goalieID;
        goalie = wm->our[goalieID];
        goalieR = wm->field->ourGoalR();
        goalieL = wm->field->ourGoalL();
    }

    distanceR = wm->opp[goalieID]->pos.dist(goalieR);
    distanceL = wm->opp[goalieID]->pos.dist(goalieL);

    if (ourAgent != nullptr) {

        rightDeg = Vector2D::angleOf(wm->field->ourGoalR(), ourAgent->pos(), goalie->pos);
        leftDeg = Vector2D::angleOf(wm->field->ourGoalL(), ourAgent->pos(), goalie->pos);

        DBUG(QString("%1, %2").arg(rightDeg.abs()).arg(leftDeg.abs()), D_FATEME);

        if (rightDeg.abs() > leftDeg.abs() && fabs(rightDeg.abs() - leftDeg.abs()) > th * 15) {
            target = Vector2D(goalieR.x, goalieR.y + n);
            drawer->draw(target, QColor(Qt::cyan));
        } else {
            target = Vector2D(goalieL.x, goalieL.y - n);
            drawer->draw(target, QColor(Qt::cyan));
        }
    } else {
        if (distanceR > distanceL && fabs(distanceL - distanceR) > th) {
            target = Vector2D(goalieR.x, goalieR.y + n * distanceR);
            drawer->draw(target, QColor(Qt::black));
            //        draw(Segment2D(target, Vector2D(((goalieL+goalieR)/2).x-1, ((goalieL+goalieR)/2).y)), QColor(Qt::darkCyan));
        } else {
            target = Vector2D(goalieL.x, goalieL.y - n * distanceL);
            drawer->draw(target, QColor(Qt::black));
            //        draw(Segment2D(target, Vector2D(((goalieL+goalieR)/2).x-1, ((goalieL+goalieR)/2).y)), QColor(Qt::darkRed));
        }
    }


    return target;
}

bool Knowledge::isPointClear(Vector2D point, Vector2D from, double rad, bool considerRelaxedIDs, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs) {
    Vector2D posIntersect1(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
    Vector2D posIntersect2(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
    Segment2D l(from, point);
    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if ((wm->opp.active(i)->inSight > 0.0)) {
            if (considerRelaxedIDs && oppRelaxedIDs.contains(wm->opp.activeAgentID(i))) {
                continue;
            }
            Circle2D c(wm->opp.active(i)->pos, rad);
            if (c.intersection(l, &posIntersect1, &posIntersect2) != 0) {
                return false;
            }
        }
    }
    for (int i = 0; i < wm->our.activeAgentsCount(); i++) {
        if (wm->our.active(i)->inSight > 0.0) {
            if (considerRelaxedIDs && ourRelaxedIDs.contains(wm->our.activeAgentID(i))) {
                continue;
            }
            Circle2D c(wm->our.active(i)->pos, rad);
            if (c.intersection(l, &posIntersect1, &posIntersect2) != 0) {
                return false;
            }
        }
    }
    return true;
}

bool Knowledge::isPointClear(Vector2D point, Vector2D from, double radBig, double radSmall, bool considerRelaxedIDs, QList<int>ourRelaxedIDs, QList<int>oppRelaxedIDs, QList<int>ourSmallIDs, QList<int>oppSmallIDs) {
    Vector2D posIntersect1(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
    Vector2D posIntersect2(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);

    Segment2D l(from, point);
    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {

        if ((wm->opp.active(i)->inSight > 0.0)) {
            if (considerRelaxedIDs && oppRelaxedIDs.contains(wm->opp.activeAgentID(i))) {
                continue;
            }
            double r = radBig;
            if (oppSmallIDs.contains(wm->opp.activeAgentID(i))) {
                r = radSmall;
            }
            Circle2D c(wm->opp.active(i)->pos, r);
            if (c.intersection(l, &posIntersect1, &posIntersect2) != 0) {
                return false;
            }
        }
    }

    for (int i = 0; i < wm->our.activeAgentsCount(); i++) {
        if (wm->our.active(i)->inSight > 0.0) {
            if (considerRelaxedIDs && ourRelaxedIDs.contains(wm->our.activeAgentID(i))) {
                continue;
            }
            double r = radBig;
            if (ourSmallIDs.contains(wm->our.activeAgentID(i))) {
                r = radSmall;
            }
            Circle2D c(wm->our.active(i)->pos, r);
            if (c.intersection(l, &posIntersect1, &posIntersect2) != 0) {
                return false;
            }
        }
    }
    return true;
}

NewFastestToBall Knowledge::newFastestToBall(double timeStep, QList<int> ourList, QList<int> oppList, const CWorldModel*& wm) {
    ////
    ////Code By Sepehr
    ////

    // reset everything
    NewFastestToBall result;
    if (!wm->field->fieldRect().contains(wm->ball->pos)) {
        return result;
    }

    double t = 0;
    Vector2D ballPredict;

    bool ourCalced[_MAX_NUM_PLAYERS];
    bool oppCalced[_MAX_NUM_PLAYERS];
    for (int i = 0 ; i < _MAX_NUM_PLAYERS ; i++) {
        ourCalced[i] = false;
        oppCalced[i] = false;
    }

    // use the correct Robot acceleration and maximum Velocity below :
    double robotMaxVel;
    ros::param::get("agent_node/VelMax", robotMaxVel);
    double robotMAxAcc;
    ros::param::get("agent_node/AccMaxForward", robotMAxAcc);

    while (t < 20 && (result.ourF.size() < ourList.size() || result.oppF.size() < oppList.size())) {
        // ballPredict = wm->ball->predict(t);

        double tToVMax;
        Vector2D center;
        double radius;
        for (int i = 0; i < ourList.count(); i++) {
            if (ourCalced[i]) {
                continue;
            }
            center = wm->our[ourList[i]]->pos + wm->our[ourList[i]]->vel * t;
            tToVMax = (robotMaxVel - wm->our[ourList[i]]->vel.length()) / robotMAxAcc;
            radius = 0;
            if (tToVMax > t) {
                radius = 0.5 * robotMAxAcc * t * t + wm->our[ourList[i]]->vel.length() * t;
            } else if (tToVMax > 0 && wm->our[ourList[i]]->vel.length() < robotMaxVel) {
                radius = 0.5 * robotMAxAcc * tToVMax * tToVMax + wm->our[ourList[i]]->vel.length() * tToVMax;
                radius += (t - tToVMax) * robotMaxVel;
            } else {
                radius = t * wm->our[ourList[i]]->vel.length();
            }
            radius += Robot::robot_radius_old;
            Circle2D cir = Circle2D(center, radius);
            Vector2D s0, s2;
            if (cir.contains(ballPredict) || cir.intersection(Segment2D(wm->ball->pos, ballPredict), &s0, &s2)) {
                result.ourF.append(std::pair<double, int>(t , ourList[i]));
                ourCalced[i] = true;
                if (result.catch_time > t) {
                    result.catch_time = t;
                    result.isFastestOurs = true;
                }
            }
            //            draw( cir, 0 , 360, "red");
        }
        for (int i = 0; i < oppList.count(); i++) {
            if (oppCalced[i]) {
                continue;
            }
            center = wm->opp[oppList[i]]->pos + wm->opp[oppList[i]]->vel * t;
            tToVMax = (robotMaxVel - wm->opp[oppList[i]]->vel.length()) / robotMAxAcc;
            radius = 0;
            if (tToVMax > t) {
                radius = 0.5 * robotMAxAcc * t * t + wm->opp[oppList[i]]->vel.length() * t;
            } else if (tToVMax > 0 && wm->opp[oppList[i]]->vel.length() < robotMaxVel) {
                radius = 0.5 * robotMAxAcc * tToVMax * tToVMax + wm->opp[oppList[i]]->vel.length() * tToVMax;
                radius += (t - tToVMax) * robotMaxVel;
            } else {
                radius = t * wm->opp[oppList[i]]->vel.length();
            }
            radius += Robot::robot_radius_old;
            Circle2D cir = Circle2D(center, radius) ;
            Vector2D s0, s2;
            if (cir.contains(ballPredict) || cir.intersection(Segment2D(wm->ball->pos, ballPredict), &s0, &s2)) {
                result.oppF.append(std::pair<double, int>(t , oppList[i]));
                oppCalced[i] = true;
                if (result.catch_time > t) {
                    result.catch_time = t;
                    result.isFastestOurs = false;
                }
            }
            //            draw( cir, 0 , 360, "blue");
        }
        t += timeStep;
    }
    qSort(result.ourF.begin() , result.ourF.end());
    qSort(result.oppF.begin() , result.oppF.end());
    if (result.catch_time > 10) {
        result.catch_time = 0;
    }
    return result;
}

FastestToBall Knowledge::findFastestToBall(QList<int> ourList, QList<int> oppList, const CWorldModel*& wm) {
    /////Extracted from DefensePlan
    /////By Pooria
    /////
    FastestToBall f;
    double time = 0.f;
    f.catch_time = 1000;
    while (true) {
        Vector2D ballPos = wm->ball->pos;//(min(time, f.catch_time));
        //            draw(ballPos, 1, "red");
        if (wm->ball->vel.length() < 0.05) {
            ballPos = wm->ball->pos;
        }
        double rad = time * 0.6 + 0.2;
        if (f.ourFastest == -1)
            for (int i = 0; i < ourList.count(); i++) {
                Vector2D playerPos = wm->our[ourList[i]]->pos;
                if (playerPos.dist(ballPos) < rad) {
                    f.ourFastest = ourList[i];
                    f.ourFastestTime = time;
                    if (time < f.catch_time) {
                        f.catch_time = time;
                    }
                    break;
                }
            }

        if (f.oppFastest == -1)
            for (int i = 0; i < oppList.count(); i++) {
                Vector2D playerPos = wm->opp[oppList[i]]->pos;
                if (playerPos.dist(ballPos) < rad) {
                    f.oppFastest = oppList[i];
                    f.oppFastestTime = time;
                    if (time < f.catch_time) {
                        f.catch_time = time;
                    }
                    break;
                }
            }

        if ((f.ourFastest > -1 || ourList.count() == 0) && (f.oppFastest > -1 || oppList.count() == 0)) {
            //        LOG("FFFF: ", f.ourFastest);
            break;
        }
        time += 0.1;
        if (time > 20) {
            if (wm->ball->vel.length() > 0.2) {
                Line2D line(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm());
                if (f.ourFastest == -1 and ourList.count() > 0) {
                    double min = 99999;
                    for (int i = 0; i < ourList.count(); i++) {
                        Vector2D playerPos = wm->our[ourList[i]]->pos;
                        double dist = line.dist(playerPos);
                        if (dist < min) {
                            f.ourFastest = ourList[i];
                            min = dist;
                        }
                    }
                    f.ourFastestTime = time;
                    if (time < f.catch_time) {
                        f.catch_time = time;
                    }
                }
                if (f.oppFastest == -1 and oppList.count() > 0) {
                    double min = 99999;
                    for (int i = 0; i < oppList.count(); i++) {
                        Vector2D playerPos = wm->opp[oppList[i]]->pos;
                        double dist = line.dist(playerPos);
                        if (dist < min) {
                            f.oppFastest = oppList[i];
                            min = dist;
                        }
                    }
                    f.oppFastestTime = time;
                    if (time < f.catch_time) {
                        f.catch_time = time;
                    }
                }
            }
            break;
        }
    }
    f.catch_time = min(time, f.catch_time);
    return f;
}

NewFastestToBall Knowledge::newFastestToBall(double timeStep, QList<int> ourList, QList<int> oppList) {
    ////
    ////Code By Sepehr
    ////

    // reset everything
    NewFastestToBall result;
    if (!wm->field->fieldRect().contains(wm->ball->pos)) {
        return result;
    }

    double t = 0;
    Vector2D ballPredict;

    bool ourCalced[_MAX_NUM_PLAYERS];
    bool oppCalced[_MAX_NUM_PLAYERS];
    for (int i = 0 ; i < _MAX_NUM_PLAYERS ; i++) {
        ourCalced[i] = false;
        oppCalced[i] = false;
    }

    // use the correct Robot acceleration and maximum Velocity below :
    //double robotMaxVel = conf()->BangBang_VelMax();
    //double robotMAxAcc = conf()->BangBang_AccMaxForward();
    double robotMaxVel, robotMAxAcc;

    ros::param::get("agent_node/VelMax", robotMaxVel); // todo
    ros::param::get("agent_node/AccMaxForward", robotMAxAcc);

    while (t < 20 && (result.ourF.size() < ourList.size() || result.oppF.size() < oppList.size())) {
        // ballPredict = wm->ball->predict(t);

        double tToVMax;
        Vector2D center;
        double radius;
        for (int i = 0; i < ourList.count(); i++) {
            if (ourCalced[i]) {
                continue;
            }
            center = wm->our[ourList[i]]->pos + wm->our[ourList[i]]->vel * t;
            tToVMax = (robotMaxVel - wm->our[ourList[i]]->vel.length()) / robotMAxAcc;
            radius = 0;
            if (tToVMax > t) {
                radius = 0.5 * robotMAxAcc * t * t + wm->our[ourList[i]]->vel.length() * t;
            } else if (tToVMax > 0 && wm->our[ourList[i]]->vel.length() < robotMaxVel) {
                radius = 0.5 * robotMAxAcc * tToVMax * tToVMax + wm->our[ourList[i]]->vel.length() * tToVMax;
                radius += (t - tToVMax) * robotMaxVel;
            } else {
                radius = t * wm->our[ourList[i]]->vel.length();
            }
            radius += Robot::robot_radius_old;
            Circle2D cir = Circle2D(center, radius);
            Vector2D s0, s2;
            if (cir.contains(ballPredict) || cir.intersection(Segment2D(wm->ball->pos, ballPredict), &s0, &s2)) {
                result.ourF.append(std::pair<double, int>(t , ourList[i]));
                ourCalced[i] = true;
                if (result.catch_time > t) {
                    result.catch_time = t;
                    result.isFastestOurs = true;
                }
            }
            //            draw( cir, 0 , 360, "red");
        }
        for (int i = 0; i < oppList.count(); i++) {
            if (oppCalced[i]) {
                continue;
            }
            center = wm->opp[oppList[i]]->pos + wm->opp[oppList[i]]->vel * t;
            tToVMax = (robotMaxVel - wm->opp[oppList[i]]->vel.length()) / robotMAxAcc;
            radius = 0;
            if (tToVMax > t) {
                radius = 0.5 * robotMAxAcc * t * t + wm->opp[oppList[i]]->vel.length() * t;
            } else if (tToVMax > 0 && wm->opp[oppList[i]]->vel.length() < robotMaxVel) {
                radius = 0.5 * robotMAxAcc * tToVMax * tToVMax + wm->opp[oppList[i]]->vel.length() * tToVMax;
                radius += (t - tToVMax) * robotMaxVel;
            } else {
                radius = t * wm->opp[oppList[i]]->vel.length();
            }
            radius += Robot::robot_radius_old;
            Circle2D cir = Circle2D(center, radius) ;
            Vector2D s0, s2;
            if (cir.contains(ballPredict) || cir.intersection(Segment2D(wm->ball->pos, ballPredict), &s0, &s2)) {
                result.oppF.append(std::pair<double, int>(t , oppList[i]));
                oppCalced[i] = true;
                if (result.catch_time > t) {
                    result.catch_time = t;
                    result.isFastestOurs = false;
                }
            }
            //            draw( cir, 0 , 360, "blue");
        }
        t += timeStep;
    }
    qSort(result.ourF.begin() , result.ourF.end());
    qSort(result.oppF.begin() , result.oppF.end());
    if (result.catch_time > 10) {
        result.catch_time = 0;
    }
    return result;
}

double Knowledge::chipGoalPropability(bool isOurChip, Vector2D _goaliePos) {
    double GoalDistanceToBall;
    double GoalieDistanseToBall;
    double GoalDistanceToGoalie;
    Vector2D goal, goaliePos;
    if (isOurChip) {
        goal = wm->field->oppGoal();
        goaliePos = wm->opp[wm->opp.data->goalieID]->pos;

    } else {
        goal = wm->field->ourGoal();
        goaliePos = _goaliePos;
    }

    GoalDistanceToBall = wm->ball->pos.dist(goal) / 1.9;
    GoalieDistanseToBall = wm->ball->pos.dist(goaliePos);
    GoalDistanceToGoalie = goaliePos.dist(goal);
    if (goaliePos.dist(wm->ball->pos) < 0.35
            || wm->ball->pos.dist(goal) < 1) {
        return 0;
    } else if (((GoalDistanceToBall - GoalieDistanseToBall) / GoalDistanceToGoalie) * 2 > 0) {
        return ((GoalDistanceToBall - GoalieDistanseToBall) / GoalDistanceToGoalie) * 2;
    } else {
        return 0;
    }
}

int Knowledge::getNearestOppToPoint(Vector2D point) {
    double minDist = 1.0e13;
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

int Knowledge::nearestOppToBall() {
    return getNearestOppToPoint(wm->ball->pos);
}

double Knowledge::chipGoalPropability(bool isOurChip) {
    double GoalDistanceToBall;
    double GoalieDistanseToBall;
    double GoalDistanceToGoalie;
    Vector2D goal, goaliePos;
    if (isOurChip) {
        goal = wm->field->oppGoal();
        goaliePos = wm->opp[wm->opp.data->goalieID]->pos;

    } else {
        goal = wm->field->ourGoal();
        goaliePos = wm->our[wm->our.data->goalieID]->pos;
    }

    GoalDistanceToBall = wm->ball->pos.dist(goal) / 1.9;
    GoalieDistanseToBall = wm->ball->pos.dist(goaliePos);
    GoalDistanceToGoalie = goaliePos.dist(goal);
    if (goaliePos.dist(wm->ball->pos) < 0.35
            || wm->ball->pos.dist(goal) < 1) {
        return 0;
    } else if (((GoalDistanceToBall - GoalieDistanseToBall) / GoalDistanceToGoalie) * 2 > 0) {
        return ((GoalDistanceToBall - GoalieDistanseToBall) / GoalDistanceToGoalie) * 2;
    } else {
        return 0;
    }


}

Knowledge * know = new Knowledge();
