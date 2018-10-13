//
// Created by parsian-ai on 8/29/18.
//

#include <rqt_parsian_gui/graphical/graphical.h>

namespace rqt_parsian_gui {
    GraphicalClient::GraphicalClient() {
        qApp->installEventFilter(this);
        setObjectName("Graphical_Client");
        updated = false;
    }

    GraphicalClient::~GraphicalClient() {

    }

    void GraphicalClient::initPlugin(qt_gui_cpp::PluginContext &context) {
        rqt_gui_cpp::Plugin::initPlugin(context);
        n = getNodeHandle();
        n_private = getPrivateNodeHandle();

        wm_sub = n.subscribe("/world_model", 1000, &GraphicalClient::wmCb, this);
        db_sub = n.subscribe("/buffer_draws", 100000, &GraphicalClient::dbCb, this);
        log_wm_sub = n.subscribe("/log/world_model", 1000, &GraphicalClient::logwmCb, this);
        draw_sub = n.subscribe("/draws", 1000, &GraphicalClient::drawCb, this);
        log_draw_sub = n.subscribe("/log/draws", 1000, &GraphicalClient::logdrawCb, this);
        color_sub = n.subscribe("/team_config", 1000, &GraphicalClient::colorCb, this);
        timer = n.createTimer(ros::Duration(0.02), &GraphicalClient::timerCb, this);
        parsian_msgs::parsian_team_configPtr team_config{new parsian_msgs::parsian_team_config};

        // access standalone command line arguments
        QStringList argv = context.argv();

        //Soccer View
        QWidget* w = new QWidget();
        mainLayout = new QVBoxLayout();
        w->setLayout(mainLayout);
        view = new GLSoccerView();
        ourCol = QColor("blue");
        oppCol = QColor("yellow");

        bag = new rosbag::Bag();

        LogMode = new QAction(this);
        LogMode->setShortcut(*new QKeySequence(tr("Ctrl+L")));
        grayMode = new QAction(this);
        grayMode->setShortcut(*new QKeySequence(tr("Ctrl+G")));
        ReplayMode = new QAction(this);
        ReplayMode->setShortcut(*new QKeySequence(tr("Ctrl+R")));
        isLogMode = false;
        isReplayMode = false;

        view->addAction(LogMode);
        view->addAction(ReplayMode);
        view->addAction(grayMode);

        connect(LogMode, SIGNAL(triggered(bool)), this, SLOT(startLog()));
        connect(ReplayMode, SIGNAL(triggered(bool)), this, SLOT(playLog()));
        connect(grayMode, SIGNAL(triggered(bool)), this, SLOT(changeGray()));

        auto* logger = new Logger();
        auto* statusBox = new QLabel("Status Box will be here");

        logger->setMaximumHeight(50);
        mainLayout->addWidget(logger);
        mainLayout->addWidget(view);
        mainLayout->addWidget(statusBox);
        context.addWidget(w);

        server.reset(new dynamic_reconfigure::Server<monitor_config::monitorConfig>(getPrivateNodeHandle()));
        dynamic_reconfigure::Server<monitor_config::monitorConfig>::CallbackType f;
        f = boost::bind(&GraphicalClient::ConfigServerCallBack, this, _1, _2);
        server->setCallback(f);

    }

    void GraphicalClient::shutdownPlugin() {
        delete view;
        rqt_gui_cpp::Plugin::shutdownPlugin();
    }

    void GraphicalClient::wmCb(const parsian_msgs::parsian_world_modelConstPtr &_wm) {
        return;
//        view->updatePacket(_wm);
    }

    void GraphicalClient::logwmCb(const parsian_msgs::parsian_world_modelConstPtr &_wm) {
        return;
//        view->updatePacket(_wm);
    }

    void GraphicalClient::drawCb(const parsian_msgs::parsian_drawsConstPtr &_draw) {
        return;
//        view->updateDraws(_draw);
    }

    void GraphicalClient::logdrawCb(const parsian_msgs::parsian_drawsConstPtr &_draw) {
        return;
//        view->updateDraws(_draw);
    }

    void GraphicalClient::colorCb(const parsian_msgs::parsian_team_configConstPtr &_color) {
        return;
        view->updateConfig(_color);
    }

    void GraphicalClient::timerCb(const ros::TimerEvent &_timer) {
        if (updated)
            view->redraw();
        updated = false;
    }

    void GraphicalClient::startLog() {
    }

    void GraphicalClient::playLog() {

    }

    void GraphicalClient::changeGray() {
        view->toggleColor();
    }

    bool GraphicalClient::eventFilter(QObject *, QEvent * event) {
        if (event->type() == QEvent::KeyPress) {
            auto * e = static_cast<QKeyEvent*>(event);
            if (e->key() == Qt::Key_Space) {
                view->resetView();
                return true;
            }
        }
        return false;
    }

    void GraphicalClient::dbCb(const parsian_msgs::parsian_draw_bufferConstPtr &_wm) {
        view->updateDB(_wm);
        updated = true;
    }

    void GraphicalClient::ConfigServerCallBack(const monitor_config::monitorConfig &config, uint32_t level) {
        view->updateConfig(config);
    }
}

PLUGINLIB_EXPORT_CLASS(rqt_parsian_gui::GraphicalClient, rqt_gui_cpp::Plugin)
