#!/usr/bin/env python

import rospy
import smach
import smach_ros




if __name__ == '__main__':

    rospy.init_node('state_machine_node', anonymous=True)
    sleep_rate = rospy.Rate(10)

    while not rospy.is_shutdown():
        print('hamid is king')
        sleep_rate.sleep()