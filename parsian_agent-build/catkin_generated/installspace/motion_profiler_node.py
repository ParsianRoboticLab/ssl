#!/usr/bin/env python
# license removed for brevity
import roslib
import rospy
from parsian_msgs.msg import parsian_robot_command
from parsian_msgs.msg import parsian_robot_task
from parsian_msgs.msg import parsian_world_model

# Import required Python code.
import motion_profiler
from point import Point
from dynamic_reconfigure.server import Server
from parsian_agent.cfg import motion_proConfig


class MotionProfilerNode:
    def __init__(self):
        self.config = motion_proConfig

        rospy.init_node('motion_profiler', anonymous=True)
        self.profiler = motion_profiler.MotionProfiler()
        self.srv = Server(motion_proConfig, self.cfg_callback)
        self.wm_sub = rospy.Subscriber('world_model', parsian_world_model, self.wmCallback,
                                       queue_size=1, buff_size=2 ** 24)
        self.rbt_cmd_sub = []
        self.task_pub = []
        for i in range(0, 12):
            self.rbt_cmd_sub.append(rospy.Subscriber('/agent_' + str(i) + '/command', parsian_robot_command,
                                                     self.rcCallback, queue_size=1, buff_size=2 ** 24))

            self.task_pub.append(rospy.Publisher('/agent_' + str(i) + '/task', parsian_robot_task, queue_size=1, latch=True))

        rospy.spin()

    def rcCallback(self, data):
        if self.config["start"]:
            self.profiler.set_robot_command(data)

    def wmCallback(self, data):
        if self.config["start"]:
            self.profiler.wmCallback(data)
            self.task_pub[self.config["robot_id"]].publish(self.profiler.get_task())

    def cfg_callback(self, config, level):
        self.config = config
        self.profiler.reset(config["robot_id"], Point(config["start_x"], config["start_y"]),
                            Point(config["end_x"], config["end_y"]), init_phase=config["init_phase"],
                            dist_step=config["dist_step"], ang_step=config["ang_step"])
        return config


if __name__ == '__main__':
    try:
        MotionProfilerNode()
    except rospy.ROSInterruptException:
        pass
