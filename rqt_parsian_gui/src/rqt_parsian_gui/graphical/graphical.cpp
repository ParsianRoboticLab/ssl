//
// Created by parsian-ai on 8/29/18.
//

#include <rqt_parsian_gui/graphical/graphical.h>

namespace rqt_parsian_gui {
    GraphicalClient::GraphicalClient() {
        qApp->installEventFilter(this);
        setObjectName("Graphical_Client");
    }

    GraphicalClient::~GraphicalClient() {

    }

    void GraphicalClient::initPlugin(qt_gui_cpp::PluginContext &context) {
        rqt_gui_cpp::Plugin::initPlugin(context);
        n = getNodeHandle();
        n_private = getPrivateNodeHandle();

        wm_sub = n.subscribe("/world_model", 1000, &GraphicalClient::wmCb, this);
        log_wm_sub = n.subscribe("/log/world_model", 1000, &GraphicalClient::logwmCb, this);
        draw_sub = n.subscribe("/draws", 1000, &GraphicalClient::drawCb, this);
        log_draw_sub = n.subscribe("/log/draws", 1000, &GraphicalClient::logdrawCb, this);
        color_sub = n.subscribe("/team_config", 1000, &GraphicalClient::colorCb, this);
        timer = n.createTimer(ros::Duration(0.080), &GraphicalClient::timerCb, this);
        parsian_msgs::parsian_team_configPtr team_config{new parsian_msgs::parsian_team_config};

        // access standalone command line arguments
        QStringList argv = context.argv();

        //Soocer View

        view = new GLSoccerView();
        view->show();

        // create QWidget
        widget_ = new QWidget();

        widget_->setWindowTitle("nadia");
        ourCol = QColor("blue");
        oppCol = QColor("yellow");

        bag = new rosbag::Bag();

        LogMode = new QAction(this);
        LogMode->setShortcut(*new QKeySequence(tr("Ctrl+L")));
        ReplayMode = new QAction(this);
        ReplayMode->setShortcut(*new QKeySequence(tr("Ctrl+R")));
        isLogMode = false;
        isReplayMode = false;

        view->addAction(LogMode);
        view->addAction(ReplayMode);

        connect(LogMode, SIGNAL(triggered(bool)), this, SLOT(startLog()));
        connect(ReplayMode, SIGNAL(triggered(bool)), this, SLOT(playLog()));



        context.addWidget(view);



    }

    void GraphicalClient::shutdownPlugin() {
        delete view;
        rqt_gui_cpp::Plugin::shutdownPlugin();
    }

    void GraphicalClient::wmCb(const parsian_msgs::parsian_world_modelConstPtr &_wm) {
        view->updatePacket(_wm);
        ROS_INFO("HA");
    }

    void GraphicalClient::logwmCb(const parsian_msgs::parsian_world_modelConstPtr &_wm) {
        return;
        view->updatePacket(_wm);
    }

    void GraphicalClient::drawCb(const parsian_msgs::parsian_drawConstPtr &_draw) {
        return;
        view->updateDraws(_draw);
    }

    void GraphicalClient::logdrawCb(const parsian_msgs::parsian_drawConstPtr &_draw) {
        return;
        view->updateDraws(_draw);
    }

    void GraphicalClient::colorCb(const parsian_msgs::parsian_team_configConstPtr &_color) {
        return;
        view->updateConfig(_color);
    }

    void GraphicalClient::timerCb(const ros::TimerEvent &_timer) {

    }

    void GraphicalClient::startLog() {

    }

    void GraphicalClient::playLog() {

    }

}

PLUGINLIB_EXPORT_CLASS(rqt_parsian_gui::GraphicalClient, rqt_gui_cpp::Plugin)
