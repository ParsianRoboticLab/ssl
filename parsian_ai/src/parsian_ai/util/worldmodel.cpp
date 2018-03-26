#include <parsian_ai/util/worldmodel.h>

WorldModel::WorldModel() : CWorldModel() {

}

WorldModel::WorldModel(const parsian_msgs::parsian_world_model &_pwm) : CWorldModel(_pwm) {

}

WorldModel::~WorldModel() = default;

bool WorldModel::AHZIsInOurPenaltyArea(Vector2D point) {
    Circle2D c1(field->ourGoal() + Vector2D(0, -field->_GOAL_WIDTH / 4), 0.85);
    Circle2D c2(field->ourGoal() + Vector2D(0, +field->_GOAL_WIDTH / 4), 0.85);
    Rect2D r(field->ourGoal() + Vector2D(+0, -field->_GOAL_WIDTH / 4), field->ourGoal() + Vector2D(0.85, +field->_GOAL_WIDTH / 4));
    drawer->draw(c1, 0, 90, "blue", false);
    drawer->draw(c2, 90, 180, "blue", false);
    drawer->draw(r, "blue");
    if (r.contains(point)) {
        return true;
    }
    if (c1.contains(point)) {
        double th = (point - c1.center()).th().degree();
        if ((th < 0) && (th > -90)) {
            return true;
        }
    }
    if (c2.contains(point)) {
        double th = (point - c2.center()).th().degree();
        if ((th > 0) && (th < 90)) {
            return true;
        }
    }
    return false;
}

QList<Vector2D> WorldModel::AHZOurPAreaIntersect(Segment2D segment, QString role) {
    QList<Vector2D> results;
    results.clear();
    double radius = 0;
    if (role == "mark") {
        radius = 1.150;
    } else if (role == "goalKeeper") {
        radius = 0.85;
    }
    Circle2D c1(field->ourGoal() + Vector2D(0, -field->_GOAL_WIDTH / 4), radius);
    Circle2D c2(field->ourGoal() + Vector2D(0, +field->_GOAL_WIDTH / 4), radius);
    Segment2D s(field->ourGoal() + Vector2D(+radius, -field->_GOAL_WIDTH / 4), field->ourGoal() + Vector2D(+radius, +field->_GOAL_WIDTH / 4));
    drawer->draw(c1, 0, 90, "orange", false);
    drawer->draw(c2, 90, 180, "orange", false);
    drawer->draw(s, "orange");
    int n;
    Vector2D vSol1, vSol2;
    n = c1.intersection(segment, &vSol1, &vSol2);
    if (n == 1) {

        double th = (vSol1 - c1.center()).th().degree();
        if ((th < 0) && (th > -90)) {
            results.append(vSol1);
        }
    }
    if (n == 2) {
        double th;
        th = (vSol1 - c1.center()).th().degree();
        if ((th < 0) && (th > -90)) {
            results.append(vSol1);
        }
        th = (vSol2 - c1.center()).th().degree();
        if ((th < 0) && (th > -90)) {
            results.append(vSol2);
        }
    }

    n = c2.intersection(segment, &vSol1, &vSol2);
    if (n == 1) {
        double th = (vSol1 - c2.center()).th().degree();
        if ((th > 0) && (th < 90)) {
            results.append(vSol1);
        }
    }
    if (n == 2) {
        double th;
        th = (vSol1 - c2.center()).th().degree();
        if ((th > 0) && (th < 90)) {
            results.append(vSol1);
        }
        th = (vSol2 - c2.center()).th().degree();
        if ((th > 0) && (th < 90)) {
            results.append(vSol2);
        }
    }

    vSol1 = s.intersection(segment);
    if (vSol1.valid()) {
        results.append(vSol1);
    }

    for (int i = 0; i < results.count(); i++) {
        drawer->draw(Circle2D(results[i], 0.05), 0, 360, "purple", true);
    }

    return results;
}

void WorldModel::updateRef(const parsian_msgs::ssl_refree_wrapperConstPtr _ref) {
    our.data->goalieID = _ref->us.goalie;
    opp.data->goalieID = _ref->them.goalie;
    m_ballplacementPoint = Vector2D(_ref->designated_position);
}

Vector2D WorldModel::ballplacementPoint() {
    return m_ballplacementPoint;
}

WorldModel * wm;
