
#ifndef RQT_TASKRUNNERWIDGET_H
#define RQT_TASKRUNNERWIDGET_H

//
// Created by noOne on 10/19/17.
//

#define _MAX_NUM_PLAYERS 12

#include <ros/ros.h>
#include <QWidget>
#include <QString>
#include <QGridLayout>
#include <QAction>
#include <QToolButton>
#include <parsian_msgs/vector2D.h>
#include <parsian_msgs/parsian_robot_task.h>
#include <parsian_msgs/parsian_skill_gotoPointAvoid.h>
#include <parsian_msgs/grsim_ball_replacement.h>
namespace rqt_parsian_gui
{
    #define TASK_NUM 3
    static const char* taskNames[TASK_NUM] = {"Ball Placement","GotoPointAvoid","GotoPoint"};
    class TaskRunnerWidget:public QWidget {
    Q_OBJECT
    public:
        TaskRunnerWidget(ros::NodeHandle & n);
        virtual ~TaskRunnerWidget();

    public slots:
        void setTask(QAction*);
        void setID(QAction * );
    protected:

    private:
        int agent_id;
        ros::Timer timer;
        parsian_msgs::grsim_ball_replacement *client ;
        ros::Subscriber mousePosSub;
        ros::ServiceClient ballReplacementClient;
        ros::Publisher *robTaskPub;
        parsian_msgs::parsian_robot_taskPtr task;
        QAction ** tasks, **ids;
        QToolButton *toolButton,*agentId;
        QGridLayout *gridLayout;
        void mousePosCallBack(parsian_msgs::vector2DConstPtr);
        void timerCb(const ros::TimerEvent& _timer);

    };
}

#endif //RQT_TASKRUNNERWIDGET_H
