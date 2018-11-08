#!/usr/bin/env python
from __future__ import print_function
import rospy
from parsian_msgs.msg import parsian_robot_command
import math

# agent number:
robot_i = 2
gt, a = 0.0, 0.02
dt = 200


def wheelspeed():
    pub = rospy.Publisher('agent_' + str(robot_i) + '/command' , parsian_robot_command, queue_size = 60) 
    # publisher topic is 'agent_i/command'
    rospy.init_node('atiyeh', anonymous = True) 
    # node name is 'atiyeh'
    prc = parsian_robot_command()
    prc.robot_id = robot_i
    rate = rospy.Rate(60)
    while not rospy.is_shutdown():
        t = rospy.get_time
        for i in range(0,dt):
            prc.wheelsspeed = True
            ii = float(i)
            #prc.wheel1, prc.wheel2 = 10 * math.sin(rospy.get_time()*0.01 * math.pi) , -10*math.cos(rospy.get_time()*0.01*math.pi)
            prc.wheel3, prc.wheel4 = 4.4 , 0
            prc.wheel1, prc.wheel2 = 0 , 0
            pub.publish(prc)
            rate.sleep()    
        
if __name__ == '__main__':
    try:
        wheelspeed()
    except rospy.ROSInterruptException:
        pass
    
    