#!/usr/bin/env python
from __future__ import print_function
import rospy
from parsian_msgs.msg import parsian_world_model
from parsian_msgs.msg import parsian_robot_command
import math
import matplotlib.pyplot as plt
import numpy as np

# agent number and ...
gt, a, robot_i , omega = 0.0, 0.02, 4 ,0.5
dt = 10
vf_max , vn_max , vw_max = 1.2 , 1.2 , 100 #pub
rospy.init_node('atiyeh', anonymous = True)
t0 , i = rospy.get_time() , 0
outs = open("outputs.txt",'w')
ins = open("inputs.txt",'w')
vff ,vnn ,vww ,vxx ,vyy = np.zeros((10000,1)) ,np.zeros((10000,1)) ,np.zeros((10000,1)) ,np.zeros((10000,1)) ,np.zeros((10000,1))


def wheelspeed():
    global tt , t 
    pub = rospy.Publisher('agent_' + str(robot_i) + '/command' , parsian_robot_command, queue_size = 60) 
    # publisher topic is 'agent_i/command'
    rospy.init_node('atiyeh', anonymous = True) 
    # node name is 'atiyeh'
    prc = parsian_robot_command()
    prc.robot_id = robot_i
    rate = rospy.Rate(60)
    t = rospy.get_time()
    prc.vel_F , prc.vel_N , prc.vel_w =   0 , vf_max*math.sin((t-t0)*math.pi*omega) , vw_max*math.sin((t-t0)*math.pi*omega)
    #vn_max*math.sin((t-t0)*math.pi*omega) , 0
    pub.publish(prc)
    rate.sleep()
    #while not rospy.is_shutdown():
#    if rospy.get_time() - t0 < 2*dt : 
#        t = rospy.get_time()
#        prc.vel_F , prc.vel_N , prc.vel_w = vf_max*math.sin((t-t0)*math.pi*omega) , 0 , 0
#        pub.publish(prc)
#        rate.sleep()  

def callback0(data):
    global v_ang , v_x , v_y , dir_x , dir_y , vxx , vyy , i
    wm = data
    wheelspeed()
    for robot in wm.our:
        if robot.id == robot_i:
            r = robot
            #t = rospy.get_time()
            v_x , vxx[i] = r.vel.x , r.vel.x
            v_y , vyy[i] = r.vel.y , r.vel.y
            v_ang = r.angularVel
            dir_x = r.dir.x
            dir_y = r.dir.y
            
def callback1(data):
    global vf , vn , vw , tt ,t , v_ang , v_x , v_y , dir_x , dir_y , i , vff , vnn, vww
    tt = rospy.get_time()
    if int(10*tt) == int(t*10):
        vf , vff[i] = data.vel_F , data.vel_F
        vn , vnn[i] = data.vel_N , data.vel_N
        vw , vww[i] = data.vel_w , data.vel_w
        ins.write(str(vf) + " " + str(vn) + " " + str(vw/100.0) + "\n")
        outs.write(str(v_x) + " " + str(v_y) + " " + str(v_ang/100.0) + " " + str(dir_x) + " " + str(dir_y) + "\n")
        print(str(vff[i]) + " " + str(dir_x) + " " + str(dir_y) + " " + str(v_x) + str(v_y) + "\n")
        i = i + 1

def vels():
    rospy.init_node('atiyeh', anonymous = True)
    rospy.Subscriber('/world_model' , parsian_world_model , callback0)
    rospy.Subscriber('/agent_' + str(robot_i) +'/command', parsian_robot_command , callback1)
    rospy.spin()
    

if __name__ == '__main__':
    try:
        vels() #subs
        outs.close()
        ins.close()
        plt.plot(vff[0:i])
        plt.plot(vxx[0:i])
        plt.plot(vyy[0:i])
        plt.show()
    except rospy.ROSInterruptException:
        pass