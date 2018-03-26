//
// Created by parsian-ai on 12/7/17.
//

#include <parsian_ai/util/agent.h>
#include <parsian_ai/util/worldmodel.h>
#include <parsian_ai/config.h>
#include <parsian_util/core/knowledge.h>

Agent::Agent(int id) : CAgent(id) {
}

Agent::Agent(const parsian_msgs::parsian_agent &_agent) : CAgent(_agent) {
}

Vector2D Agent::oneTouchCheck(Vector2D positioningPos, Vector2D* oneTouchDirection) {
    Vector2D oneTouchDir = Vector2D::unitVector(CKnowledge::oneTouchAngle(pos(), Vector2D(0, 0), (pos() - wm->ball->pos).norm(),
                           pos() - wm->ball->pos, wm->field->oppGoal(), 0.2, 0.9));
    Vector2D q;
    q.invalidate();
    bool oneTouchKick = false;
    if (wm->ball->vel.length() > 0.1 && (wm->ball->pos - pos()).length() < 1.0) {
        oneTouchKick = true;
    }

    if ((wm->ball->vel * (pos() - wm->ball->pos).norm()) > 0.1) {
        Line2D l(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm());
        q = l.projection(positioningPos);
        DBUG("case", D_ERROR);
        if (q.valid() && (q - positioningPos).length() < 1.0) {
            DBUG("case2", D_ERROR);
            if ((wm->ball->pos - pos()).length() < 1.0) {
                oneTouchKick = true;
            }
            q -= (self.centerFromKicker() + CBall::radius) * oneTouchDir;
        }
    }
    if (oneTouchKick) {
        if (fabs(Vector2D::angleBetween(dir(), wm->field->oppGoal() - pos()).degree()) < 45) {
            DBUG("case3", D_ERROR);
//            setKick(chipDistanceValue(8 , false));
        }
        DBUG("case4", D_ERROR);
    }
    *oneTouchDirection = oneTouchDir;
    return q;
}