#include <rqt_parsian_gui/referee.h>



namespace rqt_parsian_gui
{

    Referee::Referee() : rqt_gui_cpp::Plugin()
    {
        setObjectName("noOne");
    }

    void Referee::initPlugin(qt_gui_cpp::PluginContext& context)
    {

        n = getNodeHandle();
        n_private = getPrivateNodeHandle();
        // create QWidget

        refereeWidget = new RefereeWidget(n);
        context.addWidget(refereeWidget);

    }

    void Referee::shutdownPlugin() {
        n.shutdown();
        n_private.shutdown();
        refereeWidget->timer.stop();
    }

}

PLUGINLIB_EXPORT_CLASS(rqt_parsian_gui::Referee, rqt_gui_cpp::Plugin)
