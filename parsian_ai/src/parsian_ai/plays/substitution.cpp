#include "parsian_ai/plays/substitution.h"

CSubstitution::CSubstitution() {
    for(int i{}; i < _MAX_NUM_PLAYERS; i++)
        gpa.push_back(new GotopointavoidAction);
}

CSubstitution::~CSubstitution() {
    for(int i{}; i < _MAX_NUM_PLAYERS; i++)
        delete gpa[i];
}

void CSubstitution::reset(){
    positioningPlan.reset();
    executedCycles = 0;
    for(int i{}; i < _MAX_NUM_PLAYERS; i++)
        delete gpa[i];
    for(int i{}; i < _MAX_NUM_PLAYERS; i++)
        gpa.push_back(new GotopointavoidAction);
}

void CSubstitution::init(QList<Agent*>& _agents) {
    agents = _agents;
    initMaster();
}


void CSubstitution::execute_x(){
    for(auto agent: agents)
        ROS_INFO_STREAM("kianf : " << agent->id());
    ROS_INFO_STREAM("kianf : ----------------------------");

    QList<Vector2D> positions = generatepositions(agents.size());
    for(int i{}; i < agents.size(); i++)
    {
        gpa[agents[i]->id()]->setSlowmode(true);
        if(positions.size() >= i+1)
            gpa[agents[i]->id()]->setTargetpos(positions[i]);
        else
            gpa[agents[i]->id()]->setTargetpos(Vector2D{0, wm->field->_FIELD_HEIGHT/2});
        gpa[agents[i]->id()]->setAvoidpenaltyarea(true);

        gpa[agents[i]->id()]->setBallobstacleradius(0.50);
        agents[i]->action = gpa[agents[i]->id()];

    }

}

QList<Vector2D> CSubstitution::generatepositions(int count)
{
    float radius = 0.5;
    float dist = 0.3;
    float thresholdFromtop = 0.2;
    float angle_step = (dist/radius) * 180/3.14;//degree
    QList<Vector2D> positions;

    //up side region
    Vector2D centerup{0, wm->field->_FIELD_HEIGHT/2};
    Circle2D locationup{centerup, radius};
    drawer->draw(locationup, QColor(Qt::black));

    for(float ang{180}; ang <= 360; ang += angle_step)
    {
        Vector2D *sol1, *sol2;
        sol1 = new Vector2D{};
        sol2 = new Vector2D{};
        Line2D direction(centerup, ang);
        locationup.intersection(direction, sol1, sol2);
        if(sol1->y < wm->field->_FIELD_HEIGHT/2 - thresholdFromtop)
            positions.push_back(*sol1);
        else if(sol2->y < wm->field->_FIELD_HEIGHT/2 - thresholdFromtop)
            positions.push_back(*sol2);
        if(positions.size() == count)
            return positions;
    }

    //down side region
    Vector2D centerdown{0, -wm->field->_FIELD_HEIGHT/2};
    Circle2D locationdown{centerdown, radius};
    drawer->draw(locationdown, QColor(Qt::black));

    for(float ang{0}; ang <= 180; ang += angle_step)
    {
        Vector2D *sol1, *sol2;
        sol1 = new Vector2D{};
        sol2 = new Vector2D{};
        Line2D direction(centerdown, ang);
        locationdown.intersection(direction, sol1, sol2);
        if(sol1->y > -wm->field->_FIELD_HEIGHT/2 + thresholdFromtop)
            positions.push_back(*sol1);
        else if(sol2->y > -wm->field->_FIELD_HEIGHT/2 + thresholdFromtop)
            positions.push_back(*sol2);
        if(positions.size() == count)
            return positions;
    }

    if(positions.size() < count)
        for(int i{}; i < count - positions.size(); i++)
            positions.push_back(positions[i%positions.size()]);
    return positions;

//    for(auto pos: positions)
//        drawer->draw(pos, QColor(Qt::white), 0.07);
}




