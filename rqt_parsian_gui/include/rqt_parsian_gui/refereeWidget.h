
#ifndef RQT_REFEREEWIDGET_H
#define RQT_REFEREEWIDGET_H

//
// Created by noOne on 10/19/17.
//

#define _MAX_NUM_PLAYERS 12

#include <ros/ros.h>
#include <QWidget>
#include <QString>
#include <QStringList>
#include <QButtonGroup>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVariant>
#include <QCheckBox>
#include <QLineEdit>
#include <parsian_msgs/ssl_force_referee.h>
#include <parsian_msgs/parsian_team_config.h>

namespace rqt_parsian_gui
{

    class RefereeWidget : public QWidget {
    Q_OBJECT
    public:
        explicit RefereeWidget(ros::NodeHandle & n);
        virtual ~RefereeWidget();
        void teamConfCb(const parsian_msgs::parsian_team_configConstPtr& _conf);
        ros::Timer timer;
        ros::Subscriber teamConfSub;
    public slots:
        void SetManualGS(int);
        void SetEnable(int);
    protected:

    private:
        int enable,id;
        QHBoxLayout *mainLayout;
        QPushButton *btnRefs[14];
        QLineEdit *posX, *posY;
        QStringList strRefNames , strRefCommands;
        ros::Publisher refereePub;
        void timerCb(const ros::TimerEvent& _timer);
        parsian_msgs::parsian_team_config teamConfig;
        parsian_msgs::ssl_force_refereePtr refree_command;

    };
}

#endif //RQT_REFEREEWIDGET_H
