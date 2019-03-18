#include <rqt_parsian_gui/taskRunner.h>



namespace rqt_parsian_gui
{

    TaskRunner::TaskRunner() : rqt_gui_cpp::Plugin()
    {
        setObjectName("noOne");
    }

    void TaskRunner::initPlugin(qt_gui_cpp::PluginContext& context)
    {
        ROS_INFO_STREAM("Kian");
        n = getNodeHandle();
        n_private = getPrivateNodeHandle();
       // create QWidget

        taskRunnerWidget = new TaskRunnerWidget(n);
        context.addWidget(taskRunnerWidget);
//         ROS_INFO("kasra");

    }

    void TaskRunner::shutdownPlugin() {
        n.shutdown();
        n_private.shutdown();
        taskRunnerWidget->timer.stop();
    }

}

PLUGINLIB_EXPORT_CLASS(rqt_parsian_gui::TaskRunner, rqt_gui_cpp::Plugin)
