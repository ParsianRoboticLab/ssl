#include "parsian_ai/plays/halftimelineup.h"

CHalftimeLineup::CHalftimeLineup() {
    halt = false;
    ready = false;
    for (auto& g : gpa) g = new GotopointavoidAction;
}
void CHalftimeLineup::reset(){
    halt = false;
    ready = false;
    points.clear();
}


void CHalftimeLineup::init(const QList<Agent *> &_agents) {
    setAgentsID(_agents);
    initMaster();

}

CHalftimeLineup::~CHalftimeLineup() {

}

void CHalftimeLineup::execute_x() {
    fillPoints();
    fillGPA();
    if(halt) {
        gameState->setState(States::Halt);
        reset();
    } else if (ready) {
        int temp{0};
        for (auto& a : agents) if (a->dir().dir().isWithin(conf.lineDirection + 85, conf.lineDirection + 95)) temp++;
        if (temp == agents.size()) halt = true;

    } else {
        int temp{0};
        for (int i = 0; i < agents.size(); i++) if (agents[i]->pos().dist(points[i]) < 0.3) temp++;
        if (temp == agents.size()) ready = true;

    }

    for (int i = 0; i < agents.size(); i++) {
        if (points.size() >= agents.size()) {
            gpa[i]->setTargetpos(points[i]);
            agents[i]->action = gpa[i];

        }
    }
}

void CHalftimeLineup::fillPoints() {
    Vector2D startPoint{conf.startPoint_X, conf.startPoint_Y};
    Vector2D endPoint; endPoint.setPolar(agents.count()*0.3, conf.lineDirection); endPoint += startPoint;
    if (wm->field->fieldRect().contains(endPoint)) {
        points.clear();
        for(double i = 0; i < agents.size(); i++) {
            Vector2D point;
            points.append(startPoint*(1.0 - i/agents.size()) + endPoint*(i/agents.size()));
        }
    } else {
        DBUG("End Point is Invalid", D_MAHI);
    }
}

void CHalftimeLineup::fillGPA() {
    if (ready) {
        for (auto& g : gpa) {
            Vector2D dir;
            dir.setDir(conf.lineDirection + 90);
            g->setTargetdir(dir.norm());
        }
    } else {
        for (auto& g : gpa) {
            Vector2D dir;
            dir.setDir(conf.lineDirection);
            g->setTargetdir(dir.norm());
            g->setAvoidpenaltyarea(false);
            g->setAvoidcentercircle(false);
            g->setNoavoid(false);
            g->setRoller(0);

        }
    }

}