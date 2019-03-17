#!/usr/bin/env python
# created by kian and hamid in parsian ai

import rospy
import point
import math
import rospkg
from enum import Enum
from parsian_msgs.msg import parsian_robot_task
from parsian_msgs.msg import vector2D
from parsian_msgs.msg import parsian_robot_command
from parsian_msgs.msg import parsian_skill_gotoPointAvoid
from parsian_msgs.msg import parsian_skill_no
from parsian_msgs.msg import parsian_world_model
from parsian_msgs.msg import ssl_refree_command
from parsian_msgs.msg import ssl_refree_wrapper
from parsian_msgs.msg import parsian_robot
from parsian_msgs.msg import parsian_skill_receivePass
from parsian_msgs.msg import parsian_skill_kick
from Plans.pass_shoot_plan import Pass_Shoot_Plan
from Plans.kick import Kick_Plan
from dynamic_reconfigure.server import Server
from parsian_ai.cfg import def_testConfig



##------------------------------States------------------------------------##
class Attack_Plan(Enum):
    No_Plan = 0
    Pass_Shoot_Plan = 1
    Kick_Plan = 2






##------------------------------Main Node------------------------------------##
class AutoDefenseTest():
    def __init__(self):

        self.wm = parsian_world_model
        self.wm_sub = rospy.Subscriber('/world_model', parsian_world_model, self.wm_cb, queue_size=1, buff_size=2 ** 24)
        self.wm_sub = rospy.Subscriber('/referee', ssl_refree_wrapper, self.ref_cb, queue_size=1, buff_size=2 ** 24)
        self.pass_shoot_plan = Pass_Shoot_Plan()
        self.kick_plan = Kick_Plan()
        self.srv = Server(def_testConfig, self.cfg_callback)
        self.ready_to_run = False ##should br false and assign true in cfg
        self.plan = Attack_Plan.Kick_Plan
        self.isforecestart = False




    def wm_cb(self, wm):
        self.wm = wm

        if self.ready_to_run:

            if (self.plan == Attack_Plan.Pass_Shoot_Plan):
                if (self.pass_shoot_plan.execute(self.wm, self.isforecestart)):
                    rospy.loginfo("pass shoot finish")
                    self.ready_to_run = False

            if (self.plan == Attack_Plan.Kick_Plan):
                if (self.kick_plan.execute(self.wm, self.isforecestart)):
                    rospy.loginfo("kick finish")
                    self.ready_to_run = False


    def ref_cb(self, ref):
        if ref.command.command == 3:
            self.isforecestart = True
        else:
            self.isforecestart = False

    def cfg_callback(self, config, level):
        if config.Plan == 0:
            self.resetall()
            self.plan = Attack_Plan.No_Plan
            self.ready_to_run = False
        if config.Plan == 1:
            self.resetall()
            self.plan = Attack_Plan.Kick_Plan
            self.ready_to_run = True
        if config.Plan == 2:
            self.resetall()
            self.plan = Attack_Plan.Pass_Shoot_Plan
            self.ready_to_run = True
        return config

    def resetall(self):
        self.pass_shoot_plan.resetplan()
        self.kick_plan.resetplan()


if __name__ == '__main__':
    try:
        rospy.init_node('def_test', anonymous=True)
        rospy.loginfo("def_test_node is running")
        def_test = AutoDefenseTest()
        rospy.spin()
    except rospy.ROSInterruptException:
        pass

