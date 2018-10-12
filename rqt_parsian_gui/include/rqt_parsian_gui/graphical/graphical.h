//
// Created by parsian-ai on 8/29/18.
//

#ifndef RQT_PARSIAN_GUI_GRAPHICAL_H
#define RQT_PARSIAN_GUI_GRAPHICAL_H


#include <rqt_gui_cpp/plugin.h>
#include <QWidget>
#include <QPushButton>
#include <QAction>
#include <QLabel>
#include <QVBoxLayout>
#include <ros/ros.h>
#include <rosbag/bag.h>
#include <rosbag/view.h>
#include <pluginlib/class_list_macros.h>
#include <parsian_msgs/parsian_world_model.h>
#include <parsian_msgs/parsian_draw_buffer.h>
#include <parsian_msgs/parsian_team_config.h>
#include <parsian_util/core/worldmodel.h>
#include <ros/package.h>
#include <rqt_parsian_gui/graphical/logger.h>
#include <rqt_parsian_gui/graphical/logger.h>
#include <rqt_parsian_gui/graphical/soccerview.h>


namespace rqt_parsian_gui {

    class GraphicalClient : public rqt_gui_cpp::Plugin {
    Q_OBJECT
    public:

        GraphicalClient();
        ~GraphicalClient();
        virtual void initPlugin(qt_gui_cpp::PluginContext& context);
        virtual void shutdownPlugin();

        ros::NodeHandle n;
        ros::NodeHandle n_private;
        ros::NodeHandle n_color;

        ros::Subscriber wm_sub;
        ros::Subscriber db_sub;
        ros::Subscriber log_wm_sub;
        ros::Subscriber draw_sub;
        ros::Subscriber log_draw_sub;
        ros::Subscriber color_sub;

        ros::Timer timer;

        rosbag::Bag *bag;


        double radius = 0.0215;
        bool isLogMode;
        bool isReplayMode;
        QAction* LogMode;
        QAction* ReplayMode;

        QColor ourCol;
        QColor oppCol;

        QVBoxLayout* mainLayout;

        void wmCb(const parsian_msgs::parsian_world_modelConstPtr& _wm);
        void dbCb(const parsian_msgs::parsian_draw_bufferConstPtr& _db);
        void logwmCb(const parsian_msgs::parsian_world_modelConstPtr& _wm);
        void drawCb(const parsian_msgs::parsian_drawsConstPtr& _draw);
        void logdrawCb(const parsian_msgs::parsian_drawsConstPtr& _draw);
        void colorCb(const parsian_msgs::parsian_team_configConstPtr& _color);
        void timerCb(const ros::TimerEvent& _timer);
        virtual bool eventFilter(QObject *, QEvent *);

        // Comment in to signal that the plugin has a way to configure it
        // bool hasConfiguration() const;
        // void triggerConfiguration();
    private:
        QWidget* widget_;
        bool updated;
        parsian_msgs::parsian_world_modelConstPtr mywm;
        parsian_msgs::parsian_team_configConstPtr mycolor;
        GLSoccerView* view;

    public slots:
        void playLog();
        void startLog();

    };
}  // namespace rqt_example_cpp

#endif //RQT_PARSIAN_GUI_GRAPHICAL_H
