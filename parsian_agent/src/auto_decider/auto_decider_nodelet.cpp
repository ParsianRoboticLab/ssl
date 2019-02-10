#include <pluginlib/class_list_macros.h>
#include <nodelet/nodelet.h>
#include <ros/ros.h>
#include <parsian_msgs/parsian_robots_status.h>
#include <parsian_msgs/parsian_robot.h>
#include <parsian_msgs/parsian_world_model.h>
#include <parsian_util/geom/vector_2d.h>
#include <parsian_msgs/parsian_robots_fault.h>
#include <parsian_util/tools/blackboard.h>
#include <QList>
#include <QTime>

using namespace rcsc;
# define buffer_size  200
# define threshold 0.25
#define _MAX_NUM_PLAYERS 12
#define status_timeout 10000

//                          about this node
//this node detects robots faults based on wm and robot_status data,
//and publishes the result on /autofualt topic
//the result contains a list of robot_fualt message for all robots
//if the robots dont send their status for more than 'status_timeout' ms
//the node decides to not to detect faults for any robots


namespace auto_decider {

    struct RobotInfo{
        parsian_msgs::parsian_robot_status status;
        bool ballIsNear;
        QList<bool> fault;
    };

    class Decider : public nodelet::Nodelet {
    public:
        parsian_msgs::parsian_robots_faultPtr robotsFault;
        ros::Publisher pub;
        ros::Subscriber robo_sub,wm_sub;
        QList<RobotInfo> robotInfos;
        QTime timer;
    private:
        virtual void onInit() {

            ros::NodeHandle &private_nh = getPrivateNodeHandle();
            ros::NodeHandle &nh = getNodeHandle();
            pub = private_nh.advertise<parsian_msgs::parsian_robots_fault>("/autofault", 5);
            robo_sub = nh.subscribe("/robots_status", 100, &Decider::statusCb, this);
            wm_sub = nh.subscribe("/world_model", 100, &Decider::wmCb, this);
            for(int i{}; i < _MAX_NUM_PLAYERS; i++)
            {
                RobotInfo tmp;
                robotInfos.push_back(tmp);
            }
        }

        void statusCb(const parsian_msgs::parsian_robots_status msg) {
            timer.start();
            for (auto stat: msg.status)
                robotInfos[stat.id].status = stat;
        }

        void wmCb(const parsian_msgs::parsian_world_model msg) {
            for (auto robotinfo: robotInfos)
                robotinfo.ballIsNear = Vector2D(msg.our[robotinfo.status.id].pos).dist(msg.ball.pos) < threshold;
            faultdetect();
        }

        void faultdetect() {
            robotsFault.reset(new parsian_msgs::parsian_robots_fault);
            for (int i = 0; i < robotInfos.size(); i++){
                parsian_msgs::parsian_robot_faultPtr tmp;
                tmp.reset(new parsian_msgs::parsian_robot_fault);
                robotsFault->robots.push_back(*tmp);
            }

            for (auto robotinfo: robotInfos) {
                parsian_msgs::parsian_robot_faultPtr tmp;
                tmp.reset(new parsian_msgs::parsian_robot_fault);
                tmp->robot_id = robotinfo.status.id;
                robotinfo.fault.append(!robotinfo.ballIsNear && robotinfo.status.shootSensor);

                if (robotinfo.fault.size() > buffer_size)
                    robotinfo.fault.removeFirst();

                int sum = 0;
                for (auto fault : robotinfo.fault)
                    sum+= fault;


                if(sum > robotinfo.fault.size() * .7 && timer.elapsed() < status_timeout)
                    tmp->select = 2;
                else
                    tmp->select = 0;

                //PDEBUG(QString("faults %1 = ").arg(i).toStdString(),sum,D_ALI);
                //PDEBUG(QString("select %2 = ").arg(i).toStdString(),tmp->select,D_ALI);
                robotsFault->robots[tmp->robot_id] = *tmp;
            }
            pub.publish(robotsFault);
        }

    };
}
PLUGINLIB_EXPORT_CLASS(auto_decider::Decider, nodelet::Nodelet)

