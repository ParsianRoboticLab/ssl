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
    GOTOPBOTH = 1
    GOBEHINDSMW = 2
    GOFORKICK = 3
    KICK = 4
    RECEIVE = 5
    RETREAT = 6
    STANDBY = 7
    CALCULATING = 8
    NONE = 9
    FINISHED = 10

class KickStat(Enum):
    ROBOT1KICKING = 1
    ROBOT2KICKING = 2
    NONE = 3
    ROBOT1RETREATING = 4
    ROBOT2RETREATING = 5


class ReceiveStat(Enum):
    ROBOT1RECING = 1
    ROBOT2RECING = 2
    NONE = 3

class KickProfiler():
    def __init__(self):

        signal.signal(signal.SIGINT, self.signal_handler)
        self.margin = 0                                     # 0.3                                  #GUI
        self.Y1 = 0                                         #3                                     #GUI
        self.Y2 = 0                                         #-3                                    #GUI
        self.X1 = 0                                         #-4.5                                  #GUI
        self.X2 = 0                                         #4.5                                   #GUI
        self.Y1M = 0                                        #self.Y1 - self.margin                 #GUI*
        self.Y2M = 0                                        #self.Y2 + self.margin                 #GUI*
        self.X1M = 0                                        #self.X1 + self.margin                 #GUI*
        self.X2M = 0                                        #self.X2 - self.margin                 #GUI*
        self.robotid1 = 0                                   #0                                     #GUI
        self.robotid2 = 0                                   #1                                     #GUI
        self.repeat = 0                                     #3                                     #GUI
        self.spinner = 0                                    #0                                     #GUI
        self.gui_debug = 0                                  #False                                 #GUI
        self.startingpoint1 = point.Point(0, 0)             #point.Point(0, -1.5)                  #GUI
        self.startingpoint2 = point.Point(0, 0)             #point.Point(2, +1.5 )                 #GUI
        self.robot1_restPos = point.Point(0, 0)             #point.Point(-4, -2.5)                 #GUI
        self.robot2_restPos = point.Point(0, 0)             #point.Point(-3.6, 2.5)                #GUI
        self.realspeedmax = 0                                                                      #GUI
        self.ischip = False                                                                        #GUI
        self.istest = False

        self.stepnum = 0
        self.robot1_overspeed = False
        self.robot2_overspeed = False
        self.startingkickspeed = 0
        self.endingkickspeed = 0
        self.current_speed = self.startingkickspeed

        self.robot1_vels = {}
        self.robot2_vels = {}
        self.positions1 = []
        self.positions2 = []

        self.last_speed1 = 1
        self.last_speed2 = 1
        self.speed_step = 0
        self.robot1_count = 1
        self.robot2_count = 1
        self.robot1_bad_count = 0
        self.robot2_bad_count = 0
        self.bad_flag = False
        self.calculatedone = False
        self.savingdone = False
        self.setupdone = False
        self.confcbonce = False
        self.startShoot = point.Point(0,0)
        self.endShoot = point.Point(0,0)
        self.m_wm = parsian_world_model()
        self.pos_count = 0
        self.my_robot1 = parsian_robot()
        self.my_robot2 = parsian_robot()
        self.state = State.STANDBY
        self.kickstat = KickStat.NONE
        self.receivestat = ReceiveStat.NONE
        self.velrecorder = []
        self.velrecflag = False
        self.wm_sub = rospy.Subscriber('world_model', parsian_world_model, self.wmCallback, queue_size=1,
                                  buff_size=2 ** 24)
        self.draw_pub = rospy.Publisher('draws', parsian_draw , queue_size=1, latch=True)
        self.srv = Server(kick_profilerConfig, self.cfg_callback)

    def resetvalues(self):

        if self.istest:
            self.startingkickspeed = 5
            self.endingkickspeed = 15
            self.current_speed = self.startingkickspeed
            self.speed_step = 1
        elif not self.istest:
            self.startingkickspeed = 400
            self.endingkickspeed = 1023
            self.current_speed = self.startingkickspeed
            self.speed_step = 100

        self.stepnum = 0
        self.last_speed1 = 1
        self.last_speed2 = 1
        self.robot1_count = 1
        self.robot2_count = 1
        self.robot1_bad_count = 0
        self.robot2_bad_count = 0
        self.bad_flag = False
        self.robot1_overspeed = False
        self.robot2_overspeed = False
        self.calculatedone = False
        self.savingdone = False
        self.setupdone = False
        self.confcbonce = False
        self.startShoot = point.Point(0,0)
        self.endShoot = point.Point(0,0)
        self.pos_count = 0
        self.velrecflag = False
        self.velrecorder = []
        self.robot1_vels = {}
        self.robot1_vels[self.current_speed] = []
        self.robot2_vels = {}
        self.robot2_vels[self.current_speed] = []
        self.positions1 = []
        self.positions2 = []
        return True





    def wmCallback(self, data):
        # type:(parsian_world_model) ->object
        #rospy.loginfo(self.state)
        self.m_wm = data
        self.getrobots(self.robotid1, self.robotid2)
        #starting the profile --> both robots to their starting points --> til they arrived their destination
        if self.state == State.GOTOPBOTH:
            if self.gotopoint(1, self.startingpoint1, self.setdirtorobot(2)) and self.gotopoint(2, self.startingpoint2, self.setdirtorobot(1)):
                self.state = State.GOBEHINDSMW
        #one robot should kick the ball --> go somwhere behind the ball(from the sit of other robot) --> till its arrived its destination
        elif self.state == State.GOBEHINDSMW:
            if self.gosomewherebehindball():
                self.state = State.GOFORKICK
        #the kicker should place itself exactly behind ball --> kickTask with no kick --> till it get realy close with ball
        elif self.state == State.GOFORKICK:
            if self.setkickerpos():
                self.state = State.KICK
        #time for kick --> kickTask --> till the ball get some vel and distance from the kicker
        elif self.state == State.KICK:
            if self.kick():
                self.state = State.RETREAT
        elif self.state == State.RETREAT:
            self.retreat()
            self.state = State.RECEIVE
        #time for receive the ball --> receiveTask --> till the ball vel equal to 0(approximately)
        elif self.state == State.RECEIVE:
        #the kicker gert back to its origin position --> just one gotopointAvoidTask --> no till condition
            if self.receive():
                self.state = State.CALCULATING
        #calculating the max vel of the ball --> no till condition
        elif self.state == State.CALCULATING:
            self.calculate()
        #to avoid multiple call on calculate after CALCULATE state immediately go to NONE state
        elif self.state == State.NONE:
            if self.calculatedone:
                self.calculatedone = False
                self.state = State.GOBEHINDSMW
        #both robots done their duties --> time for saving the profiles
        elif self.state == State.FINISHED:
            self.gotopoint(1, self.robot1_restPos, point.Point(0,1))
            self.gotopoint(2, self.robot2_restPos, point.Point(0,1))
            if not self.savingdone:
                self.savingdone = True
                rospy.loginfo("finished")
                self.save()
        #draw some datas in monitor if the flag is true
        if self.gui_debug:
            self.draw()
        #list the ball vels while the flag is True
        if self.velrecflag:
            self.velrecorder.append(math.hypot(data.ball.vel.x, data.ball.vel.y))
        #the ball outside the desired area --> both robots noAction --> till the ball come back
        if data.ball.pos.x > self.X2M or data.ball.pos.x < self.X1M or data.ball.pos.y > self.Y1M or data.ball.pos.y < self.Y2M:
            if self.state == State.RECEIVE or self.state == State.CALCULATING:
                if self.calculatedone:
                    self.state = State.STANDBY
                else:
                    self.calculate()
            if self.state == State.NONE:
                pass
            else:
                self.state = State.STANDBY
        if self.state == State.STANDBY:
            if self.standby():
                self.state = State.GOBEHINDSMW



    def cfg_callback(self, config, level):
        #self.confcbonce = True
        self.margin = config.Margin                                   # 0.3                                  #GUI
        self.Y1 = config.Upper_Boundry                                #3                                     #GUI
        self.Y2 = config.Lower_Boundry                                #-3                                    #GUI
        self.X1 = config.Left_Boundry                                 #-4.5                                  #GUI
        self.X2 = config.Right_Boundry                                #4.5                                   #GUI
        self.Y1M = self.Y1 - self.margin                                                              #GUI*
        self.Y2M = self.Y2 + self.margin                                                              #GUI*
        self.X1M = self.X1 + self.margin                                                              #GUI*
        self.X2M = self.X2 - self.margin                                                              #GUI*
        self.robotid1 = config.Robot1_id                              #0                                     #GUI
        self.robotid2 = config.Robot2_id                              #1                                     #GUI
        self.repeat = config.Repeat                                   #3                                     #GUI
        self.realspeedmax = config.Real_speed_max                     #0                                     #GUI
        self.spinner = config.Spinner                                 #0                                     #GUI
        self.gui_debug = config.GUI_Debug                             #False                                 #GUI
        self.ischip = config.Chip
        self.istest = config.Test
        if self.istest:
            self.startingkickspeed = 5
            self.endingkickspeed = 15
            self.current_speed = self.startingkickspeed
            self.startingkickspeed = 200
            self.speed_step = 1
        elif not self.istest:
            self.endingkickspeed = 1023
            self.current_speed = self.startingkickspeed
            self.speed_step = 100
        self.robot1_vels[self.current_speed] = []
        self.robot2_vels[self.current_speed] = []


        self.startingpoint1.x = config.Robot1_start_pos_x             #point.Point(0, -1.5)                  #GUI
        self.startingpoint1.y = config.Robot1_start_pos_y             #point.Point(0, -1.5)                  #GUI
        self.startingpoint2.x = config.Robot2_start_pos_x             #point.Point(2, +1.5 )                 #GUI
        self.startingpoint2.y = config.Robot2_start_pos_y             #point.Point(2, +1.5 )                 #GUI

        self.pos_count = 0
        self.positions1 = []
        self.positions2 = []
        self.getpositions(self.startingpoint1,self.startingpoint2)

        self.robot1_restPos.x = config.Robot1_rest_pos_x              #point.Point(-4, -2.5)                 #GUI
        self.robot1_restPos.y = config.Robot1_rest_pos_y              #point.Point(-4, -2.5)                 #GUI
        self.robot2_restPos.x = config.Robot2_rest_pos_x              #point.Point(-3.6, 2.5)                #GUI
        self.robot2_restPos.y = config.Robot2_rest_pos_y              #point.Point(-3.6, 2.5)                #GUI

        if config.Run:
            self.setupdone = True
        if not config.Run:
            if self.state == State.FINISHED:
                self.resetvalues()
            self.setupdone = False
            self.state = State.STANDBY
        return config



    #seperate the line joining the robot startingpoints and continue to the boarders, devide each robot distanse to borers by 10
    #and get some positions, as the shoot power rise up the robots distanse increases
    def getpositions(self, firstp, secondp):
        # type: (point.Point, point.Point) ->object
        if not self.istest:
            self.stepnum = (1000 - self.startingkickspeed)/self.speed_step + 2
        else:
            self.stepnum = (self.endingkickspeed - self.startingkickspeed)/self.speed_step + 1
        # x1 == x2
        if secondp.x == firstp.x:
            if (firstp.y > secondp.y):
               d1 = math.fabs(self.Y1M - firstp.y)
               d2 = math.fabs(secondp.y - self.Y2M)
               step1 = d1/self.stepnum
               step2 = d2/self.stepnum
               for i in range(self.stepnum):
                   self.positions1.append(point.Point(firstp.x, firstp.y + i * step1))
                   self.positions2.append(point.Point(secondp.x, secondp.y - i * step2))
                   #print('pos1.y = ', firstp.y + i * step1,'and pos2.y',secondp.y - i * step2)
            else:
               d1 = math.fabs(self.Y2M - firstp.y)
               d2 = math.fabs(secondp.y - self.Y1M)
               step1 = d1 / self.stepnum
               step2 = d2 / self.stepnum
               for i in range(self.stepnum):
                   self.positions1.append(point.Point(firstp.x, firstp.y - i * step1))
                   self.positions2.append(point.Point(secondp.x, secondp.y + i * step2))
                   # print('pos1.y = ', firstp.y - i * step1,'and pos2.y',secondp.y + i * step2)
        # y1 == y2
        elif firstp.y == secondp.y:
            if firstp.x < secondp.x:
               d1 = math.fabs(self.X1M - firstp.x)
               d2 = math.fabs(secondp.x - self.X2M)
               step1 = d1/self.stepnum
               step2 = d2/self.stepnum
               for i in range(self.stepnum):
                   self.positions1.append(point.Point(firstp.x - i * step1, firstp.y))
                   self.positions2.append(point.Point(secondp.x + i * step2, secondp.y))
                   #print('pos1.y = ', firstp.y + i * step1,'and pos2.y',secondp.y - i * step2)
            else:
               d1 = math.fabs(self.X2M - firstp.x)
               d2 = math.fabs(secondp.x - self.X1M)
               step1 = d1 / self.stepnum
               step2 = d2 / self.stepnum
               for i in range(self.stepnum):
                   self.positions1.append(point.Point(firstp.x  + i * step1, firstp.y))
                   self.positions2.append(point.Point(secondp.x  - i * step2, secondp.y))
                   # print('pos1.x = ', firstp.x + i * step1,'and pos2.x',secondp.x - i * step2)
        else:
            m = (secondp.y - firstp.y)/(secondp.x - firstp.x)
            b = secondp.y - m * secondp.x
            y_right = m*self.X2M + b
            y_left = m*self.X1M + b
            x_up = (self.Y1M - b)/m
            x_down = (self.Y2M - b)/m
            sols = []
            # filding the intersection points with margined-field
            if y_right <= self.Y1M and y_right >= self.Y2M:
                sols.append(point.Point(self.X2M, y_right))
            if y_left <= self.Y1M and y_left >= self.Y2M:
                sols.append(point.Point(self.X1M, y_left))
            if x_up < self.X2M and x_up > self.X1M:
                sols.append(point.Point(x_up,self.Y1M))
            if x_down < self.X2M and x_down > self.X1M:
                sols.append(point.Point(x_down,self.Y2M))
            #print(sols[0].x, sols[0].y, sols[1].x, sols[1].y)
            sol0_dist_first = sols[0].distance(firstp)
            sol0_dist_second = sols[0].distance(secondp)
            sol1_dist_first = sols[1].distance(firstp)
            sol1_dist_second = sols[1].distance(secondp)
            # step1 = 0
            # step2 = 0
            if sol0_dist_first < sol0_dist_second:
                step1 = sol0_dist_first/self.stepnum
                step2 = sol1_dist_second/self.stepnum
            else:
                step2 = sol0_dist_second / self.stepnum
                step1 = sol1_dist_first / self.stepnum
            #obtain positions for robot1
            delta = firstp.minus(secondp)
            udelta = delta.unitPoint()
            for i in range(self.stepnum):
                newPos = point.Point(0,0)
                newPos.x = firstp.x + i*step1*udelta.x
                newPos.y = firstp.y + i*step1*udelta.y
                self.positions1.append(newPos)
                # print('pos1.x = ', newPos.x, 'and pos1.y', newPos.y)
            #obtain positions for robot1
            delta = secondp.minus(firstp)
            udelta = delta.unitPoint()
            for i in range(self.stepnum):
                newPos = point.Point(0, 0)
                newPos.x = secondp.x + i * step2 * udelta.x
                newPos.y = secondp.y + i * step2 * udelta.y
                self.positions2.append(newPos)



    #get the robots from world model
    def getrobots(self, id1, id2):
        # type: (int, int) -> object
        for robot in self.m_wm.our:
            if robot.id == id1:
                self.my_robot1 = robot
            if robot.id == id2:
                self.my_robot2 = robot
        self.task_pub1 = rospy.Publisher('agent_' + str(self.my_robot1.id) + str('/task'), parsian_robot_task, queue_size=1,
                                                 latch=True)
        self.task_pub2 = rospy.Publisher('agent_' + str(self.my_robot2.id) + str('/task'), parsian_robot_task, queue_size=1,
                                                 latch=True)



    #send robot(id) to the point
    def gotopoint(self, id, point, dirpoint = point.Point(0, 1)):
        # type:(int, point.Point, point.Point) ->object
        if id == 1:
            current_task1 = parsian_robot_task()
            current_task1.select = parsian_robot_task.GOTOPOINTAVOID
            task1 = parsian_skill_gotoPointAvoid()
            task1.noAvoid = False
            task1.ballObstacleRadius = 0.3
            task1.base.lookAt.x = 5000
            task1.base.lookAt.y = 5000
            task1.base.maxVelocity = 0.1
            task1.base.targetPos.x = point.x
            task1.base.targetPos.y = point.y
            task1.base.targetDir.x = dirpoint.x #self.my_robot2.pos.x - self.my_robot1.pos.x
            task1.base.targetDir.y = dirpoint.y #self.my_robot2.pos.y - self.my_robot1.pos.y
            current_task1.gotoPointAvoidTask = task1
            self.task_pub1.publish(current_task1)
            if self.robotarrived(self.my_robot1, point):
                return  True
            else:
                return False

        if id == 2:
            current_task2 = parsian_robot_task()
            current_task2.select = parsian_robot_task.GOTOPOINTAVOID
            task2 = parsian_skill_gotoPointAvoid()
            task2.noAvoid = False
            task2.ballObstacleRadius = 0.3
            task2.base.lookAt.x = 5000
            task2.base.lookAt.y = 5000
            task2.base.maxVelocity = 0.1
            task2.base.targetPos.x = point.x
            task2.base.targetPos.y = point.y
            task2.base.targetDir.x = dirpoint.x #self.my_robot1.pos.x - self.my_robot2.pos.x
            task2.base.targetDir.y = dirpoint.y #self.my_robot1.pos.y - self.my_robot2.pos.y
            current_task2.gotoPointAvoidTask = task2
            self.task_pub2.publish(current_task2)
            if self.robotarrived(self.my_robot2, point):
                return  True
            else:
                return False



    # deside the kicker and send it somewhere behind the ball with gotopointAvoidtask(the -dont-kickTask doesn't do it with Aviod)
    def gosomewherebehindball(self):
        if not math.hypot(self.m_wm.ball.vel.x, self.m_wm.ball.vel.y) < 0.1:
            return False
        alpha = 0.3  # WTF: from 0.4
        target = point.Point(0, 0)
        if self.updatenearballid() == 1:
            r = point.Point(self.m_wm.ball.pos.x - self.my_robot2.pos.x,
                            self.m_wm.ball.pos.y - self.my_robot2.pos.y)
            r = r.unitPoint()
            r.x = r.x * alpha
            r.y = r.y * alpha
            target.x = self.m_wm.ball.pos.x + r.x
            target.y = self.m_wm.ball.pos.y + r.y
            self.gotopoint(1, target, self.setdirtorobot(2))
            if self.robotarrived(self.my_robot1, target):
                self.kickstat = KickStat.ROBOT1KICKING
                return True
            else:
                return False

        if self.updatenearballid() == 2:
            r = point.Point(self.m_wm.ball.pos.x - self.my_robot1.pos.x,
                            self.m_wm.ball.pos.y - self.my_robot1.pos.y)
            r = r.unitPoint()
            r.x = r.x * alpha
            r.y = r.y * alpha
            target.x = self.m_wm.ball.pos.x + r.x
            target.y = self.m_wm.ball.pos.y + r.y
            self.gotopoint(2, target, self.setdirtorobot(1))
            if self.robotarrived(self.my_robot2, target):
                self.kickstat = KickStat.ROBOT2KICKING
                return True
            else:
                return False



    #set a kickTask whith no kicking(-dont-kickTask)
    def setkickerpos(self):
        if self.kickstat == KickStat.ROBOT1KICKING:
            current_task1 = parsian_robot_task()
            current_task1.select = parsian_robot_task.KICK
            task1 = parsian_skill_kick()
            task1.dontKick = True
            task1.chip = False
            task1.target.x = self.my_robot2.pos.x
            task1.target.y = self.my_robot2.pos.y
            current_task1.kickTask = task1
            self.task_pub1.publish(current_task1)
            if self.robotarrived(self.my_robot1, point.Point(self.m_wm.ball.pos.x, self.m_wm.ball.pos.y), 0.2) and math.hypot(self.my_robot1.pos.x - self.my_robot2.pos.x ,self.my_robot1.pos.y - self.my_robot2.pos.y) >  math.hypot(self.m_wm.ball.pos.x - self.my_robot2.pos.x ,self.m_wm.ball.pos.y - self.my_robot2.pos.y):
                # prepare the other robot for reciveing
                current_task2 = parsian_robot_task()
                current_task2.select = parsian_robot_task.RECIVEPASS
                task2 = parsian_skill_receivePass()
                task2.receiveRadius = 1
                task2.target.x = self.startingpoint2.x
                task2.target.y = self.startingpoint2.y
                current_task2.receivePassTask = task2
                self.task_pub2.publish(current_task2)
                if self.robotarrived(self.my_robot2, self.startingpoint2, 0.2):
                    return True
                else:
                    return False
            else:
                return False

        if self.kickstat == KickStat.ROBOT2KICKING:
            current_task2 = parsian_robot_task()
            current_task2.select = parsian_robot_task.KICK
            task2 = parsian_skill_kick()
            task2.dontKick = True
            task2.chip = False
            task2.target.x = self.my_robot1.pos.x
            task2.target.y = self.my_robot1.pos.y
            current_task2.kickTask = task2
            self.task_pub2.publish(current_task2)
            if self.robotarrived(self.my_robot2, point.Point(self.m_wm.ball.pos.x, self.m_wm.ball.pos.y), 0.2) and math.hypot(self.my_robot1.pos.x - self.my_robot2.pos.x ,self.my_robot1.pos.y - self.my_robot2.pos.y) >  math.hypot(self.m_wm.ball.pos.x - self.my_robot1.pos.x ,self.m_wm.ball.pos.y - self.my_robot1.pos.y):
                # prepare the other robot for reciveing
                current_task1 = parsian_robot_task()
                current_task1.select = parsian_robot_task.RECIVEPASS
                task1 = parsian_skill_receivePass()
                task1.receiveRadius = 1
                task1.target.x = self.startingpoint1.x
                task1.target.y = self.startingpoint1.y
                current_task1.receivePassTask = task1
                self.task_pub1.publish(current_task1)
                if self.robotarrived(self.my_robot1, self.startingpoint1, 0.2):
                    return True
                else:
                    return False
            else:
                return False



    #set kick task for the kicker(decided in gosomewherebehindball),
    def kick(self):
        if self.kickstat == KickStat.ROBOT1KICKING:
            if not self.robotarrived(self.my_robot2, self.startingpoint2) and math.hypot(self.m_wm.ball.vel.x, self.m_wm.ball.vel.y) < 0.02:    #WTF:second cond must be not too
                return False
            self.velrecflag = True
            current_task1 = parsian_robot_task()
            current_task1.select = parsian_robot_task.KICK
            task1 = parsian_skill_kick()
            task1.chip = self.ischip
            task1.spin = self.spinner
            if not self.istest:
                task1.iskickchargetime = True
                task1.kickchargetime = self.current_speed
            else:
                task1.iskickchargetime = False
                task1.kickSpeed = self.current_speed
            task1.target.x = self.my_robot2.pos.x
            task1.target.y = self.my_robot2.pos.y
            current_task1.kickTask = task1
            self.task_pub1.publish(current_task1)
            if math.hypot(self.m_wm.ball.vel.x, self.m_wm.ball.vel.y) > 0.02 and math.hypot(self.m_wm.ball.pos.x - self.my_robot1.pos.x, self.m_wm.ball.pos.y - self.my_robot1.pos.y) > 0.4:
                # self.kick_start_time1 = time()
                self.kickstat = KickStat.ROBOT1RETREATING
                self.startShoot = point.Point(self.my_robot1.pos.x, self.my_robot1.pos.y)
                self.endShoot = point.Point(self.my_robot2.pos.x, self.my_robot2.pos.y)
                return True
            else:
                return False

        if self.kickstat == KickStat.ROBOT2KICKING:
            # if time() - self.kick_start_time2 < 2.0:
            #     return False
            if not self.robotarrived(self.my_robot1, self.startingpoint1)  and math.hypot(self.m_wm.ball.vel.x, self.m_wm.ball.vel.y) < 0.02:    #WTF:second cond must be not too
                return False
            self.velrecflag = True
            current_task2 = parsian_robot_task()
            current_task2.select = parsian_robot_task.KICK
            task2 = parsian_skill_kick()
            task2.chip = self.ischip
            task2.spin = self.spinner
            if not self.istest:
                task2.iskickchargetime = True
                task2.kickchargetime = self.current_speed
            else:
                task2.iskickchargetime = False
                task2.kickSpeed = self.current_speed
            task2.target.x = self.my_robot1.pos.x
            task2.target.y = self.my_robot1.pos.y
            current_task2.kickTask = task2
            self.task_pub2.publish(current_task2)
            if math.hypot(self.m_wm.ball.vel.x, self.m_wm.ball.vel.y) > 0.02 and math.hypot(self.m_wm.ball.pos.x - self.my_robot2.pos.x, self.m_wm.ball.pos.y - self.my_robot2.pos.y) > 0.4:
                # self.kick_start_time2 = time()
                self.kickstat = KickStat.ROBOT2RETREATING
                self.startShoot = point.Point(self.my_robot2.pos.x, self.my_robot2.pos.y)
                self.endShoot = point.Point(self.my_robot1.pos.x, self.my_robot1.pos.y)
                return True
            else:
                return False



    #just one gotopointAvoidTask for the kicker robot to its startingpoint
    def retreat(self):
        if self.kickstat == KickStat.ROBOT1RETREATING:
            self.gotopoint(1, self.startingpoint1, self.setdirtorobot(2))
            self.receivestat = ReceiveStat.ROBOT2RECING
        if self.kickstat == KickStat.ROBOT2RETREATING:
            self.gotopoint(2, self.startingpoint2, self.setdirtorobot(1))
            self.receivestat = ReceiveStat.ROBOT1RECING



    #set receivingTask for the receiver
    def receive(self):
        if self.receivestat == ReceiveStat.ROBOT1RECING:
            current_task1 = parsian_robot_task()
            current_task1.select = parsian_robot_task.RECIVEPASS
            task1 = parsian_skill_receivePass()
            task1.receiveRadius = 1
            task1.target.x = self.startingpoint1.x
            task1.target.y = self.startingpoint1.y
            current_task1.receivePassTask = task1
            self.task_pub1.publish(current_task1)
            if math.hypot(self.m_wm.ball.vel.x, self.m_wm.ball.vel.y) < 0.02 or self.m_wm.ball.pos.x > self.X2M or self.m_wm.ball.pos.x < self.X1M or self.m_wm.ball.pos.y > self.Y1M or self.m_wm.ball.pos.y < self.Y2M:
                current_task1 = parsian_robot_task()
                current_task1.select = parsian_robot_task.NOTASK
                task1 = parsian_skill_no()
                current_task1.noTask = task1
                self.task_pub1.publish(current_task1)
                time.sleep(1)
                return True
            else:
                return False

        if self.receivestat == ReceiveStat.ROBOT2RECING:
            current_task2 = parsian_robot_task()
            current_task2.select = parsian_robot_task.RECIVEPASS
            task2 = parsian_skill_receivePass()
            task2.receiveRadius = 1
            task2.target.x = self.startingpoint2.x
            task2.target.y = self.startingpoint2.y
            current_task2.receivePassTask = task2
            self.task_pub2.publish(current_task2)
            if math.hypot(self.m_wm.ball.vel.x, self.m_wm.ball.vel.y) < 0.02 or self.m_wm.ball.pos.x > self.X2M or self.m_wm.ball.pos.x < self.X1M or self.m_wm.ball.pos.y > self.Y1M or self.m_wm.ball.pos.y < self.Y2M:
                current_task2 = parsian_robot_task()
                current_task2.select = parsian_robot_task.NOTASK
                task2 = parsian_skill_no()
                current_task2.noTask = task2
                self.task_pub2.publish(current_task2)
                time.sleep(1)
                return True
            else:
                return False



    #calculate the max ball speed, if the desired repeated finished-->increase kickspeed(current_speed),
    # if allkickspeeds done-->go to FINISH state
    def calculate(self):
        if self.current_speed > self.endingkickspeed:
            self.state = State.FINISHED
            return 1
        self.state = State.NONE
        self.velrecflag = False
        self.velrecorder.sort()
        self.velrecorder.reverse()
        if self.kickstat == KickStat.ROBOT1RETREATING:
            if self.velrecorder[0] > self.last_speed1 and self.robot1_count <= self.repeat:
                rospy.loginfo('robot1 valid, current: %f ,speed: %f' % (self.velrecorder[0], self.current_speed))
                self.robot1_vels[self.current_speed].append(self.velrecorder[0])
                if self.velrecorder[0] > self.realspeedmax:
                    self.robot1_overspeed = True
                self.velrecorder[:] = []
                self.robot1_count += 1
            else:
                self.robot1_bad_count += 1
                rospy.loginfo('robot1 @@unvalid, current: %f ,speed: %f' %(self.velrecorder[0], self.current_speed))

        if self.kickstat == KickStat.ROBOT2RETREATING:
            if self.velrecorder[0] > self.last_speed2 and self.robot2_count <= self.repeat:
                rospy.loginfo('robot2 valid, current: %f ,speed: %f' %(self.velrecorder[0], self.current_speed))
                self.robot2_vels[self.current_speed].append(self.velrecorder[0])
                if self.velrecorder[0] > self.realspeedmax:
                    self.robot2_overspeed = True
                self.velrecorder[:] = []
                self.robot2_count += 1
            else:
                self.robot2_bad_count += 1
                rospy.loginfo('robot2 @@unvalid, current: %f,speed: %f' %(self.velrecorder[0], self.current_speed))

        if self.robot1_bad_count > 2 or self.robot2_bad_count > 2:
            self.bad_flag = True

        if self.robot1_count > self.repeat and self.robot2_count > self.repeat or self.bad_flag:
            rospy.loginfo("step up")
            if self.current_speed == 1000:      #WTF: i added this
                self.speed_step = 23
            if not self.bad_flag:
                rospy.loginfo(self.robot1_vels[self.current_speed]);
		if self.robot1_vels[self.current_speed]:
                    self.last_speed1 = self.robot1_vels[self.current_speed][0]
		if self.robot2_vels[self.current_speed]:
                    self.last_speed2 = self.robot2_vels[self.current_speed][0]
            self.bad_flag = False
            self.current_speed += self.speed_step

            if self.robot1_overspeed and self.robot2_overspeed and not self.ischip:
                self.state = State.FINISHED
                return 1
            if self.current_speed > self.endingkickspeed:
                self.state = State.FINISHED
                return 1
            self.pos_count += 1
            self.startingpoint1 = self.positions1[self.pos_count]
            self.startingpoint2 = self.positions2[self.pos_count]
            self.robot1_count = 1
            self.robot2_count = 1
            self.robot1_bad_count = 0
            self.robot2_bad_count = 0
            self.robot1_vels[self.current_speed] = []
            self.robot2_vels[self.current_speed] = []
        self.calculatedone = True



    #save the datas after profiling done
    def save(self):
        if self.ischip:
            prefix = "chip"
        else:
            prefix = "kick"
        if self.spinner > 0:
            file1 = open(path.abspath(rospkg.RosPack().get_path("parsian_agent")+ "/profiler_data/" + str(self.robotid1) + "_" +
                                           str(int(round(time.time()/10))) + "(spin_" + str(self.spinner) + ")_"+prefix+".profile"), "w+")
            file1.write(str(self.robot1_vels))
            file1.close()
            file2 = open(path.abspath(rospkg.RosPack().get_path("parsian_agent")+ "/profiler_data/" + str(self.robotid2) + "_" +
                                           str(int(round(time.time()/10))) + + "(spin_" + str(self.spinner) + ")_"+prefix+".profile"), "w+")
            file2.write(str(self.robot2_vels))
            file2.close()
        if self.spinner == 0:
            file1 = open(path.abspath(rospkg.RosPack().get_path("parsian_agent")+ "/profiler_data/" + str(self.robotid1) + "_" +
                                           str(int(round(time.time()/10))) + "_"+prefix+"(nospin).profile"), "w+")
            file1.write(str(self.robot1_vels))
            file1.close()
            file2 = open(path.abspath(rospkg.RosPack().get_path("parsian_agent")+ "/profiler_data/" + str(self.robotid2) + "_" +
                                           str(int(round(time.time()/10))) + "_"+prefix+"(nospin).profile"), "w+")
            file2.write(str(self.robot2_vels))
            file2.close()
        rospy.loginfo("saved!!")



    #for debuging purposes --> draw some datas on monitor
    def draw(self):
        drawer = parsian_draw()
        text = parsian_draw_text()
        text.value = "pos_count: " + str(self.pos_count)
        text.color.r = 255
        text.color.g = 0
        text.color.b = 0
        text.color.a = 0
        text.size = 30
        text.position.x = 0
        text.position.y = 0
        drawer.texts.append(text)
        for i in range(self.stepnum):
            cir = parsian_draw_circle()
            cir.circle.radius = 0.025
            cir.filled = True
            cir.color.r = 255
            cir.color.g = 0
            cir.color.b = 0
            cir.color.a = 0
            cir.startAng = 0
            cir.endAng = 360
            cir.circle.center.x = self.positions1[i].x
            cir.circle.center.y = self.positions1[i].y
            drawer.circles.append(cir)
        for i in range(self.stepnum):
            cir = parsian_draw_circle()
            cir.circle.radius = 0.025
            cir.filled = True
            cir.color.r = 255
            cir.color.g = 0
            cir.color.b = 0
            cir.color.a = 0
            cir.startAng = 0
            cir.endAng = 360
            cir.circle.center.x = self.positions2[i].x
            cir.circle.center.y = self.positions2[i].y
            drawer.circles.append(cir)
        if self.state == State.STANDBY:
            text = parsian_draw_text()
            text.value = 'STANDBY'
            text.color.r = 255
            text.color.g = 0
            text.color.b = 0
            text.color.a = 0
            text.size = 18
            text.position.x = self.X2M
            text.position.y = self.Y1 - self.margin/3
            drawer.texts.append(text)
        self.draw_pub.publish(drawer)



    #noTask for both robots
    def standby(self):
        current_task1 = parsian_robot_task()
        current_task1.select = parsian_robot_task.NOTASK
        task1 = parsian_skill_no()
        current_task1.noTask = task1
        self.task_pub1.publish(current_task1)
        current_task2 = parsian_robot_task()
        current_task2.select = parsian_robot_task.NOTASK
        task2 = parsian_skill_no()
        current_task2.noTask = task2
        self.task_pub2.publish(current_task2)
        if self.setupdone and not self.m_wm.ball.pos.x > self.X2M and not self.m_wm.ball.pos.x < self.X1M and not self.m_wm.ball.pos.y > self.Y1M and not self.m_wm.ball.pos.y < self.Y2M:
            return True
        return False



    #get the nearer to ball robot id(from the robots starting points not the robots themselves)
    def updatenearballid(self):
        robot1_ball = math.hypot(self.startingpoint1.x - self.m_wm.ball.pos.x, self.startingpoint1.y - self.m_wm.ball.pos.y)
        robot2_ball = math.hypot(self.startingpoint2.x - self.m_wm.ball.pos.x, self.startingpoint2.y - self.m_wm.ball.pos.y)
        if abs(robot1_ball - robot2_ball) < 0.05:
            return 1
        if robot1_ball > robot2_ball:
            return 2
        else:
            return 1



    # if the robots arrived too the target or not
    def robotarrived(self, robot, target, dist=0.1):
        # type:(parsian_robot, point.Point) ->object
        if math.hypot(robot.pos.x - target.x, robot.pos.y - target.y) < dist:
            return True
        else:
            return False



    # get the direction vector to the tarpos
    def setdir(self, myxpos, myypos, tarxpos, tarypos):
        # type:(float, float, float, float) ->point.Point
        return point.Point(tarxpos - myxpos, tarypos - myypos)



    # get the direction vector to the target robot
    def setdirtorobot(self, targetid):
        # type:(int) ->point.Point
        if targetid == 1:
            return point.Point(self.my_robot1.pos.x - self.my_robot2.pos.x,
                               self.my_robot1.pos.y - self.my_robot2.pos.y)
        if targetid == 2:
            return point.Point(self.my_robot2.pos.x - self.my_robot1.pos.x,
                               self.my_robot2.pos.y - self.my_robot1.pos.y)

    def signal_handler(self, signal, frame):
        print('You pressed Ctrl+C!')
        if not self.savingdone:
            ans = raw_input("save the results?(y,n)")
            if ans == "y":
                self.save()
                self.savingdone = True
        sys.exit(0)

if __name__ == '__main__':
    try:
        rospy.init_node('kick_profiler', anonymous=True)
        rospy.loginfo("kick_profiler is running")
        kick = KickProfiler()
        rospy.spin()
    except rospy.ROSInterruptException:
        pass
