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

        // create QWidget

        playoffWidget = new PlayOffWidget();
        playoffWidget->setServerUpdateService(server_update);
        context.addWidget(playoffWidget);

    }

    void PlayOff::shutdownPlugin() {
        n.shutdown();
        n_private.shutdown();
    }

}

PLUGINLIB_EXPORT_CLASS(rqt_parsian_gui::PlayOff, rqt_gui_cpp::Plugin)
