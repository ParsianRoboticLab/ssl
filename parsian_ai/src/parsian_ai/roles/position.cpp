#include "position.h"
#include <passevaluation.h>
#include <geom/delaunay_triangulation.h>


//INIT_ROLE(CRolePosition, "position");

QMap<QString, EditData*> CRolePosition::editData;

CRolePosition::CRolePosition(Agent *_agent) : CRole(_agent) {
    bool closing = false;
    double t, t0 = 0;
    Vector2D pos[11];
    searchRects.append(Rect2D(0, _FIELD_HEIGHT / 2.0, _FIELD_WIDTH / 2.0, _FIELD_HEIGHT));
    gotopoint = new CSkillGotoPointAvoid(_agent);
    shoot = new CBehaviourKick;
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

CRolePosition::~CRolePosition() {
    delete gotopoint;
}

void CRolePosition::execute() {

    bool kickOff = false;
    bool stop = false;
    stop = knowledge->getGameMode() == CKnowledge::Stop && knowledge->getGameState() == CKnowledge::Stop;

    for (int i = 0; i < _MAX_NUM_PLAYERS; i++) {
        halfworld->positioningPoints[i] = targets[i];
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



    if (kickOff) {
        count = 2;
    }

    if (!editData.contains(configFileName)) {
        configFileName = "";
    }

    Vector2D p[5];
    if (kickOff) {
        p[0] = wm->field->getRegion(wm->field->OurMidFieldTopWing).center();
        p[1] = wm->field->getRegion(wm->field->OurMidFieldBottomWing).center();

//    if (!lastBestPoint1.valid())
//      lastBestPoint1 = p[0];
//    pos[0].initialPoint = lastBestPoint1;


//    if (!lastBestPoint2.valid())
//      lastBestPoint2 = p[1];
//    pos[1].initialPoint = lastBestPoint2;

//    pos[0].ready = true;
//    pos[0].mode = Position::Offense;
//    pos[0].minDistToOppGoal = 1.5;

//    pos[0].penaltyRadius = 1.5;
//    pos[0].mode = Position::Offense;


//    pos[1].ready = true;
//    pos[1].mode = Position::Offense;
//    pos[1].minDistToOppGoal = 1.5;

//    pos[1].penaltyRadius = 1.5;
//    pos[1].mode = Position::Offense;


//    pos[0].ready = true;
//    pos[0].rects.clear();
//    pos[0].rects.append(mywm->field->getRegion(wm->field->OurMidFieldTopWing));
//    nearestBestPos(mywm, pos[0]);
//    targets[0] = pos[0].point;

//    pos[1].ready = true;
//    pos[1].rects.clear();
//    pos[1].rects.append(mywm->field->getRegion(wm->field->OurMidFieldBottomWing));
//    nearestBestPos(mywm, pos[1]);
//    targets[1] = pos[1].point;
    } else {
//    lastBestPoint1.invalidate();
//    lastBestPoint2.invalidate();
//    for (int i=0;i<count;i++)
//    {
//      p[i] = editData[configFileName]->formation()->getPosition(i+1, mywm->ball->pos);
//      pos[i].initialPoint = p[i];
//      pos[i].ready = true;
//      pos[i].point = p[i];
//      {
//        if ((pos[i].point - mywm->ball->pos).length() < 0.7)
//        {
//          pos[i].point = (pos[i].point-mywm->ball->pos).norm()*0.7 + mywm->ball->pos;
//        }
//        if (!mywm->field->isInField(pos[i].point))
//        {
//          Vector2D sol[2];
//          int c = mywm->field->fieldRect().intersection(Line2D(pos[i].point, mywm->ball->pos).perpendicular(pos[i].point), &sol[0], &sol[1]);
//          double bestD = 10000;
//          int best_i = -1;
//          for (int i=0;i<c;i++)
//          {
//            double d = (mywm->ball->pos - sol[i]).length();
//            if (d < bestD)
//            {bestD = d; best_i = i;}
//          }
//          if (best_i == -1 || c==0)
//          {
//            pos[i].point = (mywm->ball->pos - mywm->field->ourGoal()).norm() + mywm->ball->pos;
//          }
//          else {
//            pos[i].point = sol[best_i];
//          }
//        }
//      }
//      if (!stop) nearestBestPos(mywm, pos[i]);
//      targets[i] = pos[i].point;
//    }
    }


    if (knowledge->getGameMode() == CKnowledge::Stop) {
        gotopoint->setBallObstacleRadius(0.5);
    } else {
        gotopoint->setBallObstacleRadius(0.0);
    }

    info()->matchPositions();


    if (info()->oneToucher.isEmpty()) {
        info()->oneToucherDist2Ball = 100;
    }

    bool playmakerIsFar = true;
    if (knowledge->getPlayMaker() != NULL)
        if (knowledge->getPlayMaker()->pos().dist(agent->pos()) < 0.45) {
            playmakerIsFar = false;
        }

//    if(agent->pos().dist(wm->ballCatchTarget(agent->self())) < info()->oneToucherDist2Ball
//      &&  (agent->canOneTouch() && coming > 0.3 && ang<policy()->PlayMaker_OneTouchAngleThreshold() )
//            && playmakerIsFar)
//    {
//        draw(QString("Position OneTouch"),Vector2D(-2,1),"magenta",18);
//    shoot->agents.clear();
//    shoot->agents.append(agent);
//    shoot->setSlowKick(false);
//    shoot->setChipToGoal(false);
//    shoot->execute();
//    info()->oneToucher.append(agent->self()->id);
//    info()->oneToucherDist2Ball = agent->pos().dist(wm->ballCatchTarget(agent->self()));
//    return;
//  }


    if (knowledge->getGameState() == CKnowledge::OurKickOff) {
        gotopoint->setAvoidCenterCircle(true);
    } else {
        gotopoint->setAvoidCenterCircle(false);
    }

    gotopoint->setNoAvoid(false);
    gotopoint->setAvoidPenaltyArea(true);

    debug(QString("phase : %1").arg(knowledge->disturbationPhase), D_SEPEHR);


    gotopoint->setAgent(agent);
//    gotopoint->setMaxVelocity(-1.0);
    gotopoint->setInterceptMode(false);
    gotopoint->setLookForward(false);
    gotopoint->setBallMode(false);
//    gotopoint->setFastW(false);

    bool oneTouchKick = false;

    gotopoint->setAvoidPenaltyArea(true);
    agent->canRecvPass = ((gotopoint->getFinalPos() - agent->pos()).length() < 0.7);

    gotopoint->setPlan2(false);

    gotopoint->setTargetLook(Vector2D(0, 0) , Vector2D(0, 0));

    gotopoint->execute();
    gotopoint->setMaxVelocity(-1.0);
    if (oneTouchKick) {
        agent->setKick(agent->kickSpeedValue(8, false));
    }
}

double CRolePosition::progress() {
    draw(QString("distance for positioner : %1").arg((positioningPos - agent->pos()).length()), Vector2D(1, -2), "blue");
    return ((positioningPos - agent->pos()).length() > 0.7) ? 1.5 : 0;
}

void CRolePositionInfo::reset() {
    calculated = false;
}

void CRolePositionInfo::matchPositions() {
    if (lastFrameCalculated != knowledge->frameCount) {
        lastFrameCalculated = knowledge->frameCount;
    } else {
        return;
    }
    int tmpIndices[_MAX_NUM_PLAYERS];
    bool flags[_MAX_NUM_PLAYERS];
    for (int i = 0; i < _MAX_NUM_PLAYERS; i++) {
        flags[i] = false;
        tmpIndices[i] = -1;
    }
    double sumDist = 0;

    int c = count();
    posAgents.clear();
    for (int i = 0; i < c; i++) {
        if (static_cast<CRolePosition*>(robot(i)->skill)->getDefaultPositioning()) {
            posAgents.append(i);
        }
    }

    QList<int> qq;
    //positioningPointsCount > count() should be true
    for (int i = 0; i < knowledge->positioningPointsCount; i++) {
        qq << i;
    }
    QList<QList<int> > q = generateSubsets(qq, posAgents.count());
    double minDist = 100;
    QList<int> best;
    //for (int i=0;i<)

    for (int i = 0; i < knowledge->positioningPointsCount; i++) {
        draw(knowledge->positioningPoints[i], 1, "blue");
    }

    for (int i = 0; i < q.count(); i++) {
        QList<QList<int> > p = generateCombinations(q[i]);
        for (int k = 0; k < p.count(); k++) {
            double dist = 0;
            for (int j = 0; j < posAgents.count(); j++) {
                dist += (knowledge->positioningPoints[p[k][j]] - robot(posAgents[j])->pos()).length();
            }
            if (dist < minDist) {
                minDist = dist;
                best.clear();
                best.append(p[k]);
            }
        }
    }

    for (int i = 0; i < best.count(); i++) {
        tmpIndices[posAgents[i]] = best[i];
    }
    sumDist = minDist;
    bool agentsChanged = false;

    int c = count();

    if (lastAgents.count() == c) {
        for (int i = 0; i < c; i++) {
            if (lastAgents[i] != robot(i)->id()) {
                agentsChanged = true;
                break;
            }
        }
    } else {
        agentsChanged = true;
    }

    lastAgents.clear();
    for (int i = 0; i < c; i++) {
        lastAgents.append(robot(i)->id());
    }

    if (fabs(lastError - sumDist) > 0.25 || agentsChanged) {
        for (int i = 0; i < _MAX_NUM_PLAYERS; i++) {
            indices[i] = tmpIndices[i];
        }
        //indices[i] = tmpIndices[i];
        lastError = sumDist;
    }
    /*  QString s;
      for (int i=0;i<2;i++)
        s = s + QString(" %1").arg(indices[i]);
      s = s + QString("  = %1, %2").arg(knowledge->positioningPointsCount).arg(count());
      draw(s, Vector2D(0,0));*/
}

CRolePositionInfo::CRolePositionInfo(QString _roleName) : CRoleInfo(_roleName) {
    oneToucher.clear();
    oneToucherDist2Ball = +100;
    lastError = 100;
    lastFrameCalculated = 0;
}

int CRolePositionInfo::findPlayMaker() {
    playMakerID = -1;
    for (int i = 0; i < knowledge->agentCount(); i++)
        if (knowledge->getAgent(i) != NULL && knowledge->getAgent(i)->skillName == CRolePlayMake::Name) {
            playMakerID = knowledge->getAgent(i)->id();
        }
    return playMakerID;
}
