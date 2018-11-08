#include <parsian_ai/roles/block.h>

CRoleBlockInfo* CRoleBlock::roleInfo{};

CRoleBlock::CRoleBlock(Agent *_agent) : CRole(_agent) {
    gotopoint = new GotopointavoidAction();
}

CRoleBlock::~CRoleBlock() {
    delete gotopoint;
}

void CRoleBlock::execute() {
    info()->findPos(blockGoal);

    Vector2D dir, p;
    p = agent->oneTouchCheck(info()->blockPosition, &dir);
    if (p.valid()) {
        gotopoint->setTargetdir(dir);
        gotopoint->setTargetpos(p);

    } else {

        gotopoint->setTargetpos(info()->blockPosition);
        gotopoint->setTargetdir(dir);

    }
    agent->action = gotopoint;
}

double CRoleBlock::progress() {
    return 0.0;
}

void CRoleBlock::parse(QStringList params) {
    setStop(false);
    setBlockGoal(false);
    for (int i = 0; i < params.count(); i++) {
        if (params[i].trimmed().toLower() == "stop") {
            setStop(true);
        } else if (params[i].trimmed().toLower() == "goal") {
            setBlockGoal(true);
        }
    }
}

CRoleBlockInfo* CRoleBlock::info() {
    return roleInfo;
}

CRoleBlockInfo::CRoleBlockInfo(QString _roleName) : CRoleInfo(_roleName) {

}

void CRoleBlockInfo::findPos(bool blockGoal) {
    double blockDist = 0.5;

    if (wm->ball->pos.x > 1) {
        blockDist = 2.95;
    }

    if (blockGoal) {
        blockPosition = (wm->field->ourGoal() - wm->ball->pos).norm() * (blockDist + Robot::robot_radius_old) + wm->ball->pos;
    } else {
        Vector2D goalPos = (wm->field->ourGoal() - wm->ball->pos).norm() * (blockDist + Robot::robot_radius_old) + wm->ball->pos;
        Vector2D pos;
        int kicker = know->getNearestOppToPoint(wm->ball->pos);
        if (kicker == -1) {
            blockPosition = goalPos;
        } else {
            pos = (wm->ball->pos - wm->opp[kicker]->pos).norm() * (blockDist + Robot::robot_radius_old) + wm->ball->pos;

            Rect2D checkRect = Rect2D(wm->field->fieldRect().left() + Robot::robot_radius_old, wm->field->fieldRect().top() - Robot::robot_radius_old , wm->field->_FIELD_WIDTH - 2 * Robot::robot_radius_old, wm->field->_FIELD_HEIGHT - 2 * Robot::robot_radius_old);
            if (Vector2D::angleBetween((wm->ball->pos - wm->opp[kicker]->pos) , (wm->field->ourGoal() - wm->ball->pos)).abs() < 30 && checkRect.contains(pos)) {
                blockPosition = pos;
            } else {
                blockPosition = goalPos;
            }
            if (gameState->theirKickoff()) {
                if (blockPosition.x > -Robot::robot_radius_old) {
                    if (blockPosition.y > 0) {
                        blockPosition.x = -Robot::robot_radius_old;
                        blockPosition.y = blockDist + Robot::robot_radius_old;
                    } else {
                        blockPosition.x = -Robot::robot_radius_old;
                        blockPosition.y = -1 * blockDist - Robot::robot_radius_old;
                    }
                }
            }
            if (wm->field->isInOurPenaltyArea(blockPosition) || wm->field->isInOurPenaltyArea(wm->ball->pos)) {
                blockPosition.x += 1.2;
            }
        }
    }
}
