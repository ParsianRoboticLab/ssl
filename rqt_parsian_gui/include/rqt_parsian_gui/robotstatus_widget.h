#include <QOpenGLWidget>
#include <QFile>
#include <QWidget>
#include <QtOpenGL/qgl.h>
#include <QSlider>
#include <QLabel>
#include <QRect>
#include <QGroupBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <parsian_msgs/parsian_robots_status.h>
#include <parsian_msgs/parsian_robot_command.h>
#include <parsian_msgs/parsian_team_config.h>
#include <ros/ros.h>
namespace rqt_parsian_gui {
class RobotStatusWidget : public QWidget {
    Q_OBJECT
public:
    explicit RobotStatusWidget(int color);
    void setMessage(const parsian_msgs::parsian_robot_status msg);
    void setVel(const parsian_msgs::parsian_robot_command msg);
private:

    QString getFileName();
    void draw_dir(double);
    //#######################################################
    QGroupBox   *robot_vel, *battery, *data_loss, *cap_charge, *status, *faults;
    QVBoxLayout *robot_vel_l, *battery_l, *data_loss_l, *cap_charge_l, *status_l, *faults_l;
    QHBoxLayout *motors_l, *encoders_l, *main_layout;
    QProgressBar *battery_percentage, *data_loss_percentage, *cap_charge_percentage;
    QCheckBox ** motors_f, **encoders_f, *kick_f, *chip_f, *shoot_sens_f, *shoot_board_f, *shoot_sens, *spin;
    QLabel *vel_dir, *vel, *vel_ang, * robot_id_l, *board_id_l;
    QString color, robot_id, board_id;
    QPixmap *agent_i;
    QPainter *agent_p;

};
}