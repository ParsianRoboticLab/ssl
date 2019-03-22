#include <rqt_parsian_gui/playoff/playoff.h>



namespace rqt_parsian_gui
{

    PlayOff::PlayOff() : rqt_gui_cpp::Plugin()
    {
        setObjectName("PlayOff");
    }

    void PlayOff::initPlugin(qt_gui_cpp::PluginContext& context)
    {

        n = getNodeHandle();
        n_private = getPrivateNodeHandle();

        server_update =  n.serviceClient<parsian_msgs::parsian_update_plans>("/update_plans");
        subscriber = n.subscribe<parsian_msgs::parsian_playoff_client>("/playoff_client", 1000, boost::bind(& PlayOff::sub, this, _1));


        // create QWidget

        playoffWidget = new PlayOffWidget();
        playoffWidget->setServerUpdateService(server_update);
        context.addWidget(playoffWidget);

    }

    void PlayOff::shutdownPlugin() {
        n.shutdown();
        n_private.shutdown();
    }

    void PlayOff::sub(const parsian_msgs::parsian_playoff_clientConstPtr& _msg) {
        ROS_INFO_STREAM("salam");

    }

}

PLUGINLIB_EXPORT_CLASS(rqt_parsian_gui::PlayOff, rqt_gui_cpp::Plugin)
