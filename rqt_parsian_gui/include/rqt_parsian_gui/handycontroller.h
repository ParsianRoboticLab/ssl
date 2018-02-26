/*
  Copyright 2016 Lucas Walter
*/
#ifndef RQT_PARSIAN_GUI_HANDUCONTROLLER_H
#define RQT_PARSIAN_GUI_HANDUCONTROLLER_H

#include <rqt_gui_cpp/plugin.h>
#include <ros/ros.h>
#include <QWidget>
#include <QPushButton>
#include <QKeyEvent>
#include <QString>
#include <QIcon>
#include <QTimer>
#include <ros/package.h>
#include <pluginlib/class_list_macros.h>
#include <rqt_parsian_gui/ui_handycontroller.h>
#include <parsian_msgs/parsian_robot_command.h>
#include <parsian_msgs/parsian_world_model.h>
#include <parsian_util/base.h>
//#include <parsian_util/core/worldmodel.h>


// main TODOs:
//     1)the cmd publishes in /rqt_parsian_gui/robot_command0 not in the /robot_command0 [solved]
//     2)the cmd is not Ptr and I dont know what to do [solved]
//     3)the robots does not act very fluent and smooth [solved]
//     4)the right and left actions doesnt work correctly [so does the old code]
//     5)the plugin crashes in the use of close icon [solved]
//     6)the event filter run more than once [solved]
//     7)the pushbutton icons
//     8)working in real not tested

namespace rqt_parsian_gui
{
    class HandyController
            : public rqt_gui_cpp::Plugin
    {
    Q_OBJECT
    public:

        HandyController();
        virtual void initPlugin(qt_gui_cpp::PluginContext& context);
        virtual void shutdownPlugin();
        virtual void saveSettings(qt_gui_cpp::Settings& plugin_settings,
                                  qt_gui_cpp::Settings& instance_settings) const;
        virtual void restoreSettings(const qt_gui_cpp::Settings& plugin_settings,
                                     const qt_gui_cpp::Settings& instance_settings);


        virtual bool eventFilter(QObject *, QEvent *);
        void jacobian(double vx, double vy, double w, double &v1, double &v2, double &v3, double &v4);
        void takeAction(EActionType action);
        void setRobotVel(double _vtan , double _vnorm , double _w );
        void waitHere();


        ros::Publisher publisher;
        ros::Publisher wmpublisher;
        parsian_msgs::parsian_robot_commandPtr cmd;

        QString pkgPath;


    public slots:
        void leftpressed();
        void uppressed();
        void rightpressed();
        void downpressed();
        void angle1pressed();
        void angle2pressed();
        void kickpressed();
        void chippressed();
        void haltpressed();
        void released(QKeyEvent *);
        void releasedother();
        void releasednav();
        void rollerpressed();
        void agentidchanged(QString);
        void speedchanged(int);
        void kickspeedchanged(int);
        void chipkickspeedchanged(int);
        void rollerspeedchanged(int);
        void wmtimedOut();

        // Comment in to signal that the plugin has a way to configure it
        // bool hasConfiguration() const;
        // void triggerConfiguration();
    public:
        QWidget* widget_;
        Ui::HandyController mUI;
        int agentID, speed, kickspeed, chipkickspeed, rollerspeed;
        ros::NodeHandle nh;
        ros::NodeHandle nh_private;
        ros::Timer timer_;
        QTimer* wmTimer;
        ros::Subscriber m_wm_sub;
        void m_wmCb(const parsian_msgs::parsian_world_modelConstPtr& _wm);
        bool wmtimedout;
        void timerCb(const ros::TimerEvent& event);

    signals:
        void startwmtimer(int);
        void stopwmtimer();




    };
}       // namespace rqt_example_cpp
#endif  // RQT_PARSIAN_GUI_HANDUCONTROLLER_H
