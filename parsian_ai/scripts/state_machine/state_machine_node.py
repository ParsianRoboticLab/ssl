#!/usr/bin/env python

import rospy
import smach
import smach_ros
from parsian_msgs.msg import parsian_ai_state
from parsian_msgs.msg import parsian_world_model

global wm
global main_message

def statesCb(data):
    wm = data
    pub.publish(main_message)

# ---------------- Common Messages ----------------#
# this class contains objects of the messages
# that will be sent to the ai node
# each state machine of ai has it's own message


class Messages:
    def __init__(self):
        self.msg_playOff = None
        self.msg_playOn = None
        self.msg_defense = None

        self.all_state_msgs = None


# ---------------- Common Messages ----------------#


if __name__ == '__main__':

    rospy.init_node('state_machine_node', anonymous=True)
    sleep_rate = rospy.Rate(10)

    main_message = parsian_ai_state()
    wm = parsian_world_model()

    pub = rospy.Publisher('/states', parsian_ai_state, queue_size=1)
    sub = rospy.Subscriber('/world_model', parsian_ai_state, statesCb)

    rospy.spin()

