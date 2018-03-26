#!/usr/bin/env python
from parsian_msgs.msg import parsian_robot_command
from parsian_msgs.msg import parsian_world_model
from parsian_msgs.msg import parsian_robot_task
from parsian_msgs.msg import ssl_vision_detection
from parsian_msgs.msg import parsian_packets
from parsian_msgs.msg import parsian_path
import rospy


class Latency:
    def __init__(self):
        self.queue = []
        rospy.init_node('latency', anonymous=True)
        rospy.Subscriber('/agent_0/command', parsian_robot_command,
                         self.rcCallback, queue_size=1, buff_size=2 ** 24)
        rospy.Subscriber('/agent_0/task', parsian_robot_task,
                         self.rtCallback, queue_size=1, buff_size=2 ** 24)
        rospy.Subscriber('/vision_detection', ssl_vision_detection,
                         self.visionCallback, queue_size=1, buff_size=2 ** 24)
        rospy.Subscriber('/world_model', parsian_world_model,
                         self.wmCallback, queue_size=1, buff_size=2 ** 24)
        rospy.Subscriber('/packets', parsian_packets,
                         self.packetCallback, queue_size=1, buff_size=2 ** 24)
        rospy.Subscriber('/planner_0/path', parsian_path,
                         self.pathplannerCallback, queue_size=1, buff_size=2 ** 24)
        rospy.spin()

    def rcCallback(self, data):
        pass

    def wmCallback(self, data):
        # type: (parsian_world_model) -> null

        self.queue.append((rospy.get_rostime() - data.Header.stamp).nsecs)
        if len(self.queue) > 500:
            self.queue.pop(0)
        rospy.loginfo(sum(self.queue) / len(self.queue))

    def rtCallback(self, data):
        pass

    def packetCallback(self, data):
        pass

    def visionCallback(self, data):
        pass

    def pathplannerCallback(self, data):
        pass


if __name__ == '__main__':
    try:
        Latency()
    except rospy.ROSInterruptException:
        pass
