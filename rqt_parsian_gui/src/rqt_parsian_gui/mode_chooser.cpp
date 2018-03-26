#include <rqt_parsian_gui/mode_chooser.h>



namespace rqt_parsian_gui {

ModeChooser::ModeChooser() : rqt_gui_cpp::Plugin() {
    setObjectName("noOne");
}

void ModeChooser::initPlugin(qt_gui_cpp::PluginContext& context) {

    n = getNodeHandle();
    n_private = getPrivateNodeHandle();
    // create QWidget

    modeWidget = new ModeChooserWidget(n);
    context.addWidget(modeWidget);

}

void ModeChooser::shutdownPlugin() {
    ROS_INFO("Sddf");
    modeWidget->saveTeamConfig();
    n.shutdown();
    n_private.shutdown();

}

}

PLUGINLIB_EXPORT_CLASS(rqt_parsian_gui::ModeChooser, rqt_gui_cpp::Plugin)
