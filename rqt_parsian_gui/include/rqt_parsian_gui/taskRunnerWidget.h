
#ifndef RQT_TASKRUNNERWIDGET_H
#define RQT_TASKRUNNERWIDGET_H

//
// Created by noOne on 10/19/17.
//

#define _PLAYER_NUMBER 12
#define _TASK_NUM 3

#include <ros/ros.h>
#include <QWidget>
#include <QString>
#include <QGridLayout>
#include <QAction>
#include <QToolButton>
#include <QComboBox>
#include <parsian_msgs/vector2D.h>
#include <parsian_msgs/parsian_robot_task.h>
#include <parsian_msgs/parsian_skill_gotoPointAvoid.h>
#include <parsian_msgs/grsim_ball_replacement.h>
#include <parsian_msgs/parsian_world_model.h>

namespace rqt_parsian_gui
{
  //  #define TASK_NUM 3
  static const char* taskNames[_TASK_NUM] = {"GotoPointAvoid","Kick","Receive"};
    class TaskRunnerWidget:public QWidget {
    Q_OBJECT
    public:
        ros::Timer timer;
        QTimer* wmTimer;
        TaskRunnerWidget(ros::NodeHandle & n);
        virtual ~TaskRunnerWidget();

    public slots:
        void setTask(QAction*);
        void setID(QAction * );

    protected:

    private:
        int agent_id;
        ros::Subscriber worldModelSub;
        ros::Subscriber mousePosSub;
        ros::ServiceClient ballReplacementClient;
        ros::Publisher robTaskPub[_PLAYER_NUMBER];
        parsian_msgs::parsian_robot_taskPtr task;
        QAction ** tasks, **ids;
        QToolButton *toolButton,*agentId;
        QComboBox *comboBoxPN , *comboBoxTask;
        QGridLayout *gridLayout;
        void mousePosCallBack(parsian_msgs::vector2DConstPtr pos);
        //void timerCb(const ros::TimerEvent& _timer);
        void m_wmCb(const parsian_msgs::parsian_world_modelConstPtr& _wm);

    signals:
        void startwmtimer(int);
        void stopwmtimer();
    };
}

#endif //RQT_TASKRUNNERWIDGET_H
