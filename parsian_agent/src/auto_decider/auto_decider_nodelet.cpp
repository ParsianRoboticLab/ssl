#include <pluginlib/class_list_macros.h>
#include <nodelet/nodelet.h>
#include <ros/ros.h>
#include <parsian_msgs/parsian_robots_status.h>
#include <parsian_msgs/parsian_robot.h>
#include <parsian_msgs/parsian_world_model.h>
#include <parsian_util/geom/vector_2d.h>
#include <parsian_msgs/parsian_robot_fault.h>
#include <parsian_util/tools/blackboard.h>
#include <QList>

using namespace rcsc;
# define buffer_size  200
# define threshold 0.25

//TODOS
//1)select what state the damaged robot is
//2)what kick(or what ever)fault = true means (dameged or not)
//3)whats the best number f0or threshold and buffersize

namespace auto_decider {
    class Decider : public nodelet::Nodelet {
    public:
        parsian_msgs::parsian_robot_faultPtr res;
        ros::Publisher pub;
        ros::Subscriber robo_sub,wm_sub;
        bool shootSensors[12];
        bool isNear[12];
        QList<bool> faults[12];
    private:
        virtual void onInit() {

            ros::NodeHandle &private_nh = getPrivateNodeHandle();
            ros::NodeHandle &nh = getNodeHandle();
            pub = private_nh.advertise<parsian_msgs::parsian_robot_fault>("/autofault", 5);
            robo_sub = nh.subscribe("/robots_status", 100, &Decider::statusCb, this);
            wm_sub = nh.subscribe("/world_model", 100, &Decider::wmCb, this);
        }

        void statusCb(const parsian_msgs::parsian_robots_status msg) {
            for (int i = 0; i < 12; i++)
                shootSensors[i] = msg.status[i].shootSensor;
        }

        void wmCb(const parsian_msgs::parsian_world_model msg) {
            for (int i = 0; i < 12; i++)
                isNear[i] = Vector2D(msg.our[i].pos).dist(msg.ball.pos) < threshold;
            faultdetect();
        }

        void faultdetect() {


            for (int i = 0; i < 12; i++) {
                res.reset(new parsian_msgs::parsian_robot_fault);
                res->robot_id=i;
                faults[i].append(!isNear[i] && shootSensors[i]);

                if (faults[i].size() > buffer_size)
                    faults[i].removeFirst();

                int sum = 0;
                for (auto fault : faults[i])
                    sum+= fault;


                if(sum > faults[i].size() * .7)
                    res->select = 2;
                else
                    res->select = 0;

                PDEBUG(QString("faults %1 = ").arg(i).toStdString(),sum,D_ALI);
                PDEBUG(QString("select %2 = ").arg(i).toStdString(),res->select,D_ALI);
                pub.publish(res);
            }
        }

    };
}
PLUGINLIB_DECLARE_CLASS(auto_decider, Decider, auto_decider::Decider, nodelet::Nodelet)

