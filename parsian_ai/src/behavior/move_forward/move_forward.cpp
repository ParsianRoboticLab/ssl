#include <behavior/move_forward/move_forward.h>

BehaviorMove_Forward::BehaviorMove_Forward()
: Behavior("Move_Forward", "describe me") {

}

BehaviorMove_Forward::~BehaviorMove_Forward() {

}

int BehaviorMove_Forward::execute() {
    return 0;
}

double BehaviorMove_Forward::eval() {
    //_behav = nullptr;
   Vector2D oppgoal{OPP_GOAL_X, OPP_GOAL_Y};
   double default_dist{oppgoal.dist(Vector2D(wm->ball->pos.x +  wm->ball->vel.x, wm->ball->pos.y +  wm->ball->vel.y)) / 2.0};
   //ROS_INFO_STREAM("kian: " << default_dist);
   QList<Segment2D> obstacles;
   left.assign(Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y - default_dist}, Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y});
   Segment2D right{Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y + default_dist}, Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y}};
   obstacles.push_back(left);
   obstacles.push_back(right);
   drawer->draw(Circle2D(Vector2D(wm->ball->pos.x +  wm->ball->vel.x, wm->ball->pos.y +  wm->ball->vel.y), default_dist), QColor(100, 255, 50), false);
   //ROS_INFO_STREAM("debug: 1");
   for (int i{}; i < wm->opp.activeAgentsCount(); i++) {
       if (wm->opp.active(i)->pos.x + wm->opp.active(i)->vel.x > wm->ball->pos.x + wm->ball->vel.x + 0.001) {
           if (Vector2D(wm->opp.active(i)->pos.x + wm->opp.active(i)->vel.x, wm->opp.active(i)->pos.y + wm->opp.active(i)->vel.y).dist(Vector2D(wm->ball->pos.x +  wm->ball->vel.x, wm->ball->pos.y +  wm->ball->vel.y)) < default_dist) {
               Segment2D temp{Vector2D{wm->opp.active(i)->pos.x + wm->opp.active(i)->vel.x, wm->opp.active(i)->pos.y + wm->opp.active(i)->vel.y}, Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y}};
               drawer->draw(temp, QColor(50, 55, 155));
               obstacles.push_back(temp);
           }
       }
   }
  // ROS_INFO_STREAM("debug: 2");
   drawer->draw(left, QColor(50, 55, 155));
   drawer->draw(right, QColor(50, 55, 155));
   sortobstacles(obstacles);
//   for(int i{}; i < obstacles.size(); i++)
//   {
//       ROS_INFO_STREAM("kian::: "<<obstacles[i].a().y);
//       //drawer->draw(obstacles[i], QColor(50, 55, 155));
//   }
   //ROS_INFO_STREAM("debug: 3");
   QList<double> angles;
   //ROS_INFO_STREAM("kian: " << (1/3.14)*180*angleOfTwoSegment(Segment2D{Vector2D{5,7}, Vector2D{1,0}}, Segment2D{Vector2D{5,7}, Vector2D{3,0}}));
    for(int i{}; i < obstacles.size() - 1; i++)
    {
        angles.push_back(angleOfTwoSegment(obstacles[i], obstacles[i + 1]));
    }
//    for(int i{}; i<angles.size(); i++)
//    {
//        ROS_INFO_STREAM("kian11: " << angles[i]*180/3.14 );
//    }
    ROS_INFO("kian1: -------" );
    //ROS_INFO_STREAM("debug: 4");
    QList<QPair<Vector2D, double>> result;
    double angsum{};
    double nearestoppdist{};
    for(int i{}; i<angles.size(); i++)
    {
        QPair<Vector2D, double> tmp;
        angsum = angleOfTwoSegment(obstacles[0], obstacles[i]);
        double ang{angsum + angles[i]/2.0};
        //ROS_INFO_STREAM("kian: " << angsum*180/3.14);
        Vector2D tmp1{};
        tmp1.setPolar(1, AngleDeg{-90 + ang*180/3.14});
        tmp.first = tmp1;
        tmp.first.x += wm->ball->pos.x + wm->ball->vel.x;
        tmp.first.y += wm->ball->pos.y + wm->ball->vel.y;
        if(obstacles[i].a().dist(Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y}) > obstacles[i+1].a().dist(Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y}))
            nearestoppdist = obstacles[i+1].a().dist(Vector2D{wm->ball->pos.x, wm->ball->pos.y});
        else
            nearestoppdist = obstacles[i].a().dist(Vector2D{wm->ball->pos.x, wm->ball->pos.y});
        //ROS_INFO_STREAM("kian: " << nearestoppdist);
        tmp.second = angles[i] * nearestoppdist;//angleWide(prob) * nearestDist(prob) * diffrenceWithPI/2(effectivity)
        result.push_back(tmp);
    }
    //ROS_INFO_STREAM("debug: 5");
    for(int i{}; i <result.size(); i++)
        drawer->draw(Segment2D{Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y}, result[i].first}, QColor(50, 10, 50));
    //ROS_INFO_STREAM("debug: 6");
    double maxeval{-1};
    for(int i{}; i <result.size() - 1; i++)
    {
        if(result[i].second > result[i+1].second)
            maxeval = i;
        else
            maxeval = i+1;
    }
    //ROS_INFO_STREAM("debug: 7");
    if(maxeval != -1)
        drawer->draw(Segment2D{Vector2D{wm->ball->pos.x + wm->ball->vel.x, wm->ball->pos.y + wm->ball->vel.y}, result[maxeval].first}, QColor(250, 10, 50));
    //ROS_INFO_STREAM("debug: 8");
    return 0.0;

}


void BehaviorMove_Forward::swap(Segment2D *xp, Segment2D *yp)
{
    Segment2D temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void BehaviorMove_Forward::sortobstacles(QList<Segment2D> &obstacles)
{
    int j{}, n{obstacles.size()};
    for (int i{}; i < n-1; i++)
        for (int j{}; j < n-i-1; j++)
            if(angleOfTwoSegment(obstacles[j], left) > angleOfTwoSegment(obstacles[j+1], left))
                swap(&obstacles[j], &obstacles[j+1]);
}


double BehaviorMove_Forward::angleOfTwoSegment(const Segment2D &xp, const Segment2D &yp)
{
    double theta1 = std::atan2(xp.a().y-xp.b().y,xp.a().x-xp.b().x);
    double theta2 = std::atan2(yp.a().y-yp.b().y,yp.a().x-yp.b().x);
    double diff = fabs(theta1-theta2);
    return diff;
}

double BehaviorMove_Forward::findmax(const QList<double> &list)
{
    double max{-100000};
    for(int i{}; i < list.size(); i++)
    {
        if(list[i] > max)
            max = list[i];
    }
    return max;
}
