//
// Created by parsian-ai on 3/23/18.
//

#ifndef PARSIAN_AI_EXPRIMENTAL_H
#define PARSIAN_AI_EXPRIMENTAL_H


#include <QStringList>

#include <algorithm>

#include <QtCore/QMap>
#include <parsian_util/core/agent.h>
#include <QtCore/QTime>
#include <QtCore/QFile>
#include <QPair>
#include <QTextStream>
#include "parsian_util/base.h"
#include <parsian_ai/util/worldmodel.h>
#include <parsian_ai/plans/plans.h>
#include <parsian_ai/plays/plays.h>
#include <parsian_ai/roles/stop.h>

#include <parsian_util/action/autogenerate/gotopointaction.h>
#include <parsian_util/action/autogenerate/gotopointavoidaction.h>
#include <parsian_util/action/autogenerate/kickaction.h>
#include <parsian_util/geom/geom.h>


#include <parsian_msgs/plan_service.h>

class Exprimental {
public:
    GotopointavoidAction *mygpa;
    KickAction *myKick;
    KickAction *myKick2;
    OnetouchAction *myOT;
    Exprimental(Agent** _agents) {
        agents = _agents;
        gpa = new GotopointavoidAction();
        pos  << Vector2D(1,-3) << Vector2D(1,3) << Vector2D(4.5,3) << Vector2D(1,3);
        dist << 0.3 << 1 << 0.3 << 0.3;
        agents[8]->action = gpa;
        state = 0;
        mygpa = new GotopointavoidAction();
        myKick = new KickAction;
        myKick2 = new KickAction;
        myOT = new OnetouchAction;
    }
    ~Exprimental() {

    }
    void reset() {

    }
    void execute() {
        int skillAgent = 5;
        Rect2D penaltyArea(Vector2D(-6,-1.3),Vector2D(-4.7,1.3));
        Vector2D finalPos = mousePos;
        Segment2D directPath(agents[skillAgent]->pos(),finalPos);
        Vector2D A(-3.5,-1.3) , B(-3.5 , 1.3);
        Vector2D target;
        mygpa->setTargetdir(agents[skillAgent]->pos() - wm->field->ourGoal());

        Vector2D sol1,sol2;

        if(penaltyArea.intersection(directPath,&sol1,&sol2) == 2)
        {
            if((sol1.y == -1.3 && sol2.y == 1.3) || (sol2.y == -1.3 && sol1.y == 1.3)) {
                ROS_INFO_STREAM("sol1 , sol2 "<< sol1.y << sol2.y);

                if(agents[skillAgent]->pos().dist(A) < agents[skillAgent]->pos().dist(B) ) {
                    target = A;
                } else {
                    target = B;
                }
            } else if(sol1.y == -1.3 || sol2.y == -1.3) {
                ROS_INFO_STREAM("ahz "<< sol1.y << sol2.y);
                target = A + Vector2D(0,-0.5);
            } else if(sol1.y == 1.3 || sol2.y == 1.3){
                target = B + Vector2D(0,0.5);
            }
            drawer->draw(sol1);
            drawer->draw(sol2);
        } else {
            target = finalPos;
        }

        drawer->draw(target,QColor(Qt::red));



        mygpa->setTargetpos(target);
       mygpa->setNoavoid(true);
        mygpa->setAvoidpenaltyarea(false);
        myKick->setTarget(Vector2D(6,0));
//        myKick->setChipdist(0.1);
        myKick->setIskickchargetime(true);
        myKick->setKickchargetime(600);
        myKick->setSpin(10);
        //myKick->setKickspeed(0.5);
        myKick->setChip(true);
        myKick2->setTarget(Vector2D(6,-4.5));
//        myKick->setChipdist(0.1);
        myKick2->setIskickchargetime(true);
        myKick2->setKickchargetime(400);
        myKick2->setSpin(10);
        //myKick->setKickspeed(0.5);
        myKick2->setChip(true);

        ROS_INFO_STREAM("salam2");
        agents[5]->action = mygpa;
        agents[1]->action = myKick2;
        myOT->setWaitpos(mousePos);
        myOT->setKickspeed(800);
        myOT->setFastestpoint(false);

        myOT->setTarget(wm->field->oppGoal());
        agents[0]->action = myOT;

    }
private:
    int state;
    QList<Vector2D> pos;
    QList<double> dist;
    GotopointavoidAction *gpa;
    Agent** agents;
};

#endif //PARSIAN_AI_EXPRIMENTAL_H
