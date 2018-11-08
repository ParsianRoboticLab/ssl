#include "planner/obstacle.h"

//global functions
double boundTo(double x, double low, double high) {
    if (x < low) {
        x = low;
    }
    if (x > high) {
        x = high;
    }
    return (x);
}


//====================================================================//
//    Obstacle class implementation
//====================================================================//

double obstacle::margin(state s , double obsMargin) {
    Vector2D p;
    double d;

    switch (type) {
        case OBS_CIRCLE:
            d = pos.dist(s.pos);
            return (d - rad.x - obsMargin);
        case OBS_RECTANGLE:
            p.assign(boundTo(s.pos.x, pos.x - rad.x, pos.x + rad.x),
                     boundTo(s.pos.y, pos.y - rad.y, pos.y + rad.y));
            d = p.dist(s.pos);
            return (d - obsMargin);
        default:break;
    }

    return (0.0);
}

bool obstacle::check(state s , double obsMargin) {
    double dx, dy;

    if (type == OBS_CIRCLE || type == OBS_RECTANGLE) {
        dx = fabs(s.pos.x - pos.x);
        dy = fabs(s.pos.y - pos.y);
        if (dx > rad.x + obsMargin || dy > rad.y + obsMargin) {
            return (true);
        }
    }

    return (margin(s , obsMargin) > 0.0);
}

bool obstacle::check(state s0, state s1 , double obsMargin) {
    Vector2D p;
    Vector2D c[4];
    int i;
    double d;

    d = s0.pos.dist(s1.pos);
    if (d < EPSILON) {
        return (check(s0 , obsMargin));
    }

    switch (type) {
        case OBS_CIRCLE:
            p = Segment2D(s0.pos, s1.pos).nearestPoint(pos);
            d = p.dist(pos);
            return (d > rad.x + obsMargin);

        case OBS_RECTANGLE:
            c[0].assign(pos.x - rad.x, pos.y - rad.y);
            c[1].assign(pos.x + rad.x, pos.y - rad.y);
            c[2].assign(pos.x + rad.x, pos.y + rad.y);
            c[3].assign(pos.x - rad.x, pos.y + rad.y);
            for (i = 0; i < 4; i++) {
                d = Segment2D(s0.pos, s1.pos).dist(Segment2D(c[i], c[(i + 1) % 4]));
                if (d < obsMargin) {
                    return (false);
                }
            }

            return (check(s0 , obsMargin));

        default:break;
    }

    return (true);
}


//====================================================================//
//    CObstacles class implementation
//====================================================================//

CObstacles::CObstacles() {
    obs.clear();
    targetPosition.invalidate();
    agentPos.invalidate();
    obsMargin = Robot::robot_radius_new;
}

void CObstacles::add_rectangle(double left_x, double top_y, double l, double w) {
    obstacle temp;
    temp.type = OBS_RECTANGLE;
    temp.pos.assign(left_x + l/2, top_y - w/2);
    temp.vel.assign(0, 0);
    temp.rad.assign(l, w);
    Rect2D testRect(left_x, top_y, l, w); // = Rect2D::from_center(cx,cy,w,h);
    if (!testRect.contains(targetPosition)) {
//        drawer->draw(testRect,QColor(Qt::red));
        obs.append(temp);
    }

}

void CObstacles::add_rectangle_from_center(double cx, double cy, double w, double h) {
    obstacle temp;
    temp.type = OBS_RECTANGLE;
    temp.pos.assign(cx, cy);
    temp.vel.assign(0, 0);
    temp.rad.assign(w / 2 , h / 2);
    Rect2D testRect(cx - (w / 2), cy + (h / 2), w, h); // = Rect2D::from_center(cx,cy,w,h);
    if (!testRect.contains(targetPosition)) {
//        drawer->draw(testRect,QColor(Qt::red));
        obs.append(temp);
    }

}


void CObstacles::add_circle(double x, double y, double radius,
                            double vx, double vy) {
    obstacle temp;
    temp.type = OBS_CIRCLE;
    temp.pos.assign(x, y);
    temp.vel.assign(vx, vy);
    temp.rad.assign(radius , radius);
    Circle2D testCircle(Vector2D(x, y), radius);

    if (!testCircle.contains(targetPosition) && !testCircle.contains(agentPos)) {
//        drawer->draw(testCircle,QColor(Qt::red));
        obs.append(temp);
    }
}


bool CObstacles::check(Vector2D p) {
    state s;
    s.pos.assign(p.x, p.y);
    s.parent = NULL;
    return (check(s));
}

bool CObstacles::check(Vector2D p, QList<int> &id) {
    state s;
    s.pos.assign(p.x, p.y);
    s.parent = NULL;
    return (check(s, id));
}

bool CObstacles::check(state s) {
    for (int i = 0 ; i < obs.count() ; i++)
        if (!obs[i].check(s , obsMargin)) {
            return false;
        }
    return true;
}

bool CObstacles::check(state s, QList<int> &id) {
    bool isSafe = true;
    id.clear();
    for (int i = 0 ; i < obs.count() ; i++)
        if (!obs[i].check(s , obsMargin)) {
            id.append(i);
            isSafe = false;
        }
    return isSafe;
}

bool CObstacles::check(state s0, state s1) {
    for (int i = 0 ; i < obs.count() ; i++)
        if (!obs[i].check(s0 , s1 , obsMargin)) {
            return false;
        }
    return true;
}

bool CObstacles::check(state s0, state s1, QList<int> &id) {
    id.clear();
    bool isSafe = true;
    for (int i = 0 ; i < obs.count() ; i++)
        if (!obs[i].check(s0 , s1 , obsMargin)) {
            id.append(i);
            isSafe = false;
        }
    return isSafe;
}

bool CObstacles::check(Vector2D p1 , Vector2D p2) {
    state s1 , s2;

    s1.pos.assign(p1.x, p1.y);
    s1.parent = NULL;

    s2.pos.assign(p2.x, p2.y);
    s2.parent = NULL;
    return (check(s1 , s2));
}


void CObstacles::draw() {
    for (int i = 0; i < obs.count(); i++) {
        if (obs[i].type == OBS_CIRCLE) {
            drawer->draw(Circle2D(obs[i].pos, obs[i].rad.x), 0, 360, QColor("red"));
        } else if (obs[i].type == OBS_RECTANGLE)
            drawer->draw(Rect2D(
                    Vector2D(obs[i].pos.x - obs[i].rad.x, obs[i].pos.y + obs[i].rad.y),
                    obs[i].rad.x * 2.0, obs[i].rad.y * 2.0), QColor("red"));
    }
}
