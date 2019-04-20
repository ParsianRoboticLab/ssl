#!/usr/bin/env python
# license removed for brevity

import rospy
import point
import math
import rospkg
from enum import Enum
from os import path
import time
import signal
import sys
from parsian_msgs.msg import parsian_robot_task
from parsian_msgs.msg import vector2D
from parsian_msgs.msg import parsian_robot_command
from parsian_msgs.msg import parsian_skill_gotoPointAvoid
from parsian_msgs.msg import parsian_skill_no
from parsian_msgs.msg import parsian_world_model
from parsian_msgs.msg import parsian_robot
from parsian_msgs.msg import parsian_skill_receivePass
from parsian_msgs.msg import parsian_skill_kick
from parsian_msgs.msg import parsian_draw
from parsian_msgs.msg import parsian_draw_circle
from parsian_msgs.msg import parsian_draw_text
from dynamic_reconfigure.server import Server
from parsian_agent.cfg import kick_profilerConfig


class State(Enum):
    INIT = 0
    FINISH = 1

class ChipTest():
    def __init__(self):

        self.state = State.INIT
        self.m_wm = parsian_world_model
        #self.inittest()
        self.chipspeed = 0
        self.velrecorder = []
        self.velrecflag = False
        self.initonce = False
        self.finishonce = False
        self.init = False
        self.key = 'q'
        self.srv = Server(kick_profilerConfig, self.cfg_callback)

        self.wm_sub = rospy.Subscriber('world_model', parsian_world_model, self.wmCallback, queue_size=1,
                                  buff_size=2 ** 24)










    def cfg_callback(self, config, level):
        #self.confcbonce = True
        self.velrecflag = config.Run
        if self.velrecflag:
            self.init = True
        if self.init and not self.velrecflag:
            self.calculate()
        return config





    def wmCallback(self, data):
        # type:(parsian_world_model) ->object
        #rospy.loginfo(self.state)
        if self.velrecflag:
            self.velrecorder.append(math.hypot(data.ball.vel.x, data.ball.vel.y))








    #calculate the max ball speed, if the desired repeated finished-->increase kickspeed(current_speed),
    # if allkickspeeds done-->go to FINISH state
    def calculate(self):
        self.calculateonce = True
        self.velrecflag = False
        self.velrecorder.sort()
        self.velrecorder.reverse()
        rospy.loginfo('max ball speed: %f ' % (self.velrecorder[0]))



if __name__ == '__main__':
    try:
        rospy.init_node('chip_handy_test', anonymous=True)
        rospy.loginfo("chip_handy_test is running")
        chip = ChipTest()
        rospy.spin()
    except rospy.ROSInterruptException:
        pass
