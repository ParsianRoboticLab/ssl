#!/usr/bin/python
import rospy
from parsian_msgs.msg import parsian_robot_command
from parsian_msgs.msg import parsian_robot_task
from parsian_msgs.msg import parsian_world_model
from passtest import *

pt = PassTest()
task_pub = []

def wmCallback(wm):
    global pt
    pt.update_wm(wm)
    tasks = pt.get_tasks()
    for task in tasks:
        task_pub[task["id"]].publish(task["msg"])


if __name__ == '__main__':
    n = rospy.init_node('pass_test_node')
    wm_sub = rospy.Subscriber('world_model', parsian_world_model, wmCallback,
                              queue_size=1, buff_size=2 ** 24)
    for i in range(0, 12):
        task_pub.append(rospy.Publisher('/agent_' + str(i) + '/task', parsian_robot_task, queue_size=1, latch=True))
    rospy.spin()

