#!/usr/bin/env python
# license removed for brevity
from pprint import pprint

import rospy
from parsian_msgs.msg import parsian_behavior
from parsian_msgs.msg import parsian_ai_status
from dynamic_reconfigure.server import Server
from parsian_ai.cfg import behavior_serverConfig
import best_selector


class BehaviorServer:
    def __init__(self):
        rospy.init_node('behavior_server')
        self.selector = best_selector.BestSelector()
        self.srv = Server(behavior_serverConfig, self.cfg_callback)
        self.eval_sub = rospy.Subscriber('/eval', parsian_behavior, self.getEvals, queue_size=1, buff_size=2 ** 24)
        self.ai_status_sub = rospy.Subscriber('/ai_status', parsian_ai_status, self.correct_probability, queue_size=1,
                                              buff_size=2 ** 24)
        self.selected_pub = rospy.Publisher('/behavior', parsian_behavior, queue_size=1, latch=True)
        self.timer = rospy.Timer(rospy.Duration(1), self.publisher_cb, oneshot=False)

        self.timeHasPassed = True
        self.last_best_behavior = None

        rospy.loginfo('behavior server inited')
        rospy.spin()

    def getEvals(self, msg):
        self.selector.update_data(msg)

    def publisher_cb(self, event):
        if self.last_best_behavior is not None:
            if not self.timeHasPassed:
                self.selected_pub.publish(self.last_best_behavior)
        best_behavior = self.selector.get_best()
        if best_behavior is not -1:
            if self.timeHasPassed:
                self.last_best_behavior = best_behavior
                rospy.Timer(rospy.Duration(3), self.timer_cb, oneshot=True)
                self.selected_pub.publish(best_behavior)
                self.timeHasPassed = False

    def correct_probability(self, msg):
        self.selector.update_success_rate(msg)

    def cfg_callback(self, config, level):
        rewards_penalties = {}
        for group in config["groups"]["groups"]:
            if group is not "Behavior_Server":
                name = config["groups"]["groups"][group]["name"]
                penalty = config["groups"]["groups"][group]["parameters"]["penalty"]
                reward = config["groups"]["groups"][group]["parameters"]["reward"]
                rewards_penalties[name] = {"penalty": penalty, "reward": reward}
        self.selector.update_rewards_penalties(rewards_penalties)
        self.selector.update_config(config["queue_size"], config["threshold_amount"], config["upper_boundary"], config["lower_boundary"])

        return config

    def timer_cb(self, event):
        self.timeHasPassed = True


if __name__ == '__main__':
    bs = BehaviorServer()
    rospy.spin()
