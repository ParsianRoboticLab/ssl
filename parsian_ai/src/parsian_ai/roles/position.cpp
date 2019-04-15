
#include <parsian_ai/roles/position.h>

#include "parsian_ai/roles/position.h"
//#include <passevaluation.h>
//#include <geom/delaunay_triangulation.h>


//INIT_ROLE(CRolePosition, "position");


CRolePosition::CRolePosition(Agent *_agent) : CRole(_agent) {
    bool closing = false;
    double t, t0 = 0;
    Vector2D pos[11];
    lookat = NULL;
    position.invalidate();
    direction = 0.0;
    indirectGoogooli = false;
    indirectKhafan = false;
    positioningPos.assign(0.0, 0.0);
    indirectKhafanhigh = false;
    disturb = false;
    stop = false;

}

void CRolePosition::execute() {
    update();
    switch (positionSkill) {
        case PositionSkill::Ready:
            agent->action = receiveSkill;
            break;
        case PositionSkill::Move:
            agent->action = moveSkill;
            break;
        case PositionSkill::OneTouch:
            agent->action = oneTouchSkill;
            break;
        case PositionSkill::NoSkill:;
            break;
    }
    return;


    bool kickOff = false;
    bool stop = false;
//    stop = knowledge->getGameMode() == CKnowledge::Stop && knowledge->getGameState() == CKnowledge::Stop;

    for (int i = 0; i < _MAX_NUM_PLAYERS; i++) {
//        halfworld->positioningPoints[i] = targets[i];
    }

    QString configFileName = "";
    int count = 2;
    if (configFileName == "1attacker") {
        count = 1;
    } else if (configFileName == "2attacker") {
        count = 2;
    } else if (configFileName == "3attacker") {
        count = 3;
    } else if (configFileName == "large5") {
        count = 5;
    } else if (configFileName == "TI6") {
        count = 6;
    }

    if (stop) {
        configFileName = "stop2";
        count = 2;
    }

//    gotopoint->setAgent(agent);
//    gotopoint->setMaxVelocity(-1.0);
//    gotopoint->setInterceptMode(false);
//    gotopoint->setLookForward(false);
//    gotopoint->setBallMode(false);
//    gotopoint->setFastW(false);

    bool oneTouchKick = false;

//    gotopoint->setAvoidPenaltyArea(true);
//    agent->canRecvPass = ((gotopoint->getFinalPos() - agent->pos()).length() < 0.7);

//    gotopoint->setPlan2(false);

//    gotopoint->setTargetLook(Vector2D(0, 0) , Vector2D(0, 0));

//    gotopoint->execute();
//    gotopoint->setMaxVelocity(-1.0);
    if (oneTouchKick) {
//        agent->setKick(agent->kickSpeedValue(8, false));
    }
}

void CRolePosition::update() {
    switch (positionSkill) {
        case PositionSkill::Ready:
            receiveSkill->setTarget(target);
            receiveSkill->setReceiveradius(receiveRadius);
            break;
        case PositionSkill::Move:
            moveSkill->setTargetpos(target);
            moveSkill->setTargetdir(targetDir);
            moveSkill->setAvoidpenaltyarea(true);
            moveSkill->setSlowmode(false);
            moveSkill->setBallobstacleradius(.2);
            break;
        case PositionSkill::OneTouch:
            oneTouchSkill->setWaitpos(waitPos);
            oneTouchSkill->setTarget(target);
            break;
        case PositionSkill::NoSkill:
        default:
            break;
    }
}



//void CRolePositionInfo::matchPositions() {
////    if (lastFrameCalculated != knowledge->frameCount) {
////        lastFrameCalculated = knowledge->frameCount;
////    } else {
////        return;
////    }
//    int tmpIndices[_MAX_NUM_PLAYERS];
//    bool flags[_MAX_NUM_PLAYERS];
//    for (int i = 0; i < _MAX_NUM_PLAYERS; i++) {
//        flags[i] = false;
//        tmpIndices[i] = -1;
//    }
//    double sumDist = 0;
//
//    int c = count();
//    posAgents.clear();
////    for (int i = 0; i < c; i++) {
////        if (static_cast<CRolePosition*>(robot(i)->skill)->getDefaultPositioning()) {
////            posAgents.append(i);
////        }
////    }
//
//    QList<int> qq;
//    //positioningPointsCount > count() should be true
////    for (int i = 0; i < knowledge->positioningPointsCount; i++) {
////        qq << i;
////    }
//    QList<QList<int> > q = generateSubsets(qq, posAgents.count());
//    double minDist = 100;
//    QList<int> best;
//    //for (int i=0;i<)
//
////    for (int i = 0; i < knowledge->positioningPointsCount; i++) {
////        draw(knowledge->positioningPoints[i], 1, "blue");
////    }
//
//    for (int i = 0; i < q.count(); i++) {
//        QList<QList<int> > p = generateCombinations(q[i]);
//        for (int k = 0; k < p.count(); k++) {
//            double dist = 0;
//            for (int j = 0; j < posAgents.count(); j++) {
////                dist += (knowledge->positioningPoints[p[k][j]] - robot(posAgents[j])->pos()).length();
//            }
//            if (dist < minDist) {
//                minDist = dist;
//                best.clear();
//                best.append(p[k]);
//            }
//        }
//    }
//
//    for (int i = 0; i < best.count(); i++) {
//        tmpIndices[posAgents[i]] = best[i];
//    }
//    sumDist = minDist;
//    bool agentsChanged = false;
//
//    if (lastAgents.count() == c) {
//        for (int i = 0; i < c; i++) {
//            if (lastAgents[i] != robot(i)->id()) {
//                agentsChanged = true;
//                break;
//            }
//        }
//    } else {
//        agentsChanged = true;
//    }
//
//    lastAgents.clear();
//    for (int i = 0; i < c; i++) {
//        lastAgents.append(robot(i)->id());
//    }
//
//    if (fabs(lastError - sumDist) > 0.25 || agentsChanged) {
//        for (int i = 0; i < _MAX_NUM_PLAYERS; i++) {
//            indices[i] = tmpIndices[i];
//        }
//        //indices[i] = tmpIndices[i];
//        lastError = sumDist;
//    }
//    /*  QString s;
//      for (int i=0;i<2;i++)
//        s = s + QString(" %1").arg(indices[i]);
//      s = s + QString("  = %1, %2").arg(knowledge->positioningPointsCount).arg(count());
//      draw(s, Vector2D(0,0));*/
//}
//
//CRolePositionInfo::CRolePositionInfo(QString _roleName) : CRoleInfo(_roleName) {
//    oneToucher.clear();
//    oneToucherDist2Ball = +100;
//    lastError = 100;
//    lastFrameCalculated = 0;
//}