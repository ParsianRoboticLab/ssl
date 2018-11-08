#!/usr/bin/env python
# created by kian and hamid in parsian ai

import rospy
import random
from enum import Enum
from parsian_msgs.msg import parsian_robot_task
from parsian_msgs.msg import vector2D
from parsian_msgs.msg import parsian_robot_command
from parsian_msgs.msg import parsian_skill_gotoPointAvoid
from parsian_msgs.msg import parsian_skill_no
from parsian_msgs.msg import parsian_world_model
from parsian_msgs.msg import parsian_robot
from parsian_msgs.msg import parsian_skill_receivePass
from parsian_msgs.msg import parsian_skill_kick
from point import Point
import math
import point
from parsian_msgs.msg import parsian_robot
from time import time



class States(Enum):
    POSITIONING = 0
    KICK = 1

class Kick_Plan():

    def __init__(self):
        self.wm = parsian_world_model()
        self.state = States.POSITIONING

        self.kicker = parsian_robot
        self.random_ids = []
        self.timer = time()

        self.generate_initial_positions = True
        self.initial_random_poses = []


    def execute(self, _wm, isforcestart):
        self.wm = _wm
        rospy.loginfo(isforcestart)
        if self.state == States.POSITIONING:
            if self.positioning():
                self.state = States.KICK

        if self.state == States.KICK:
            if isforcestart:
                self.kick()



        if self.ball_out_of_field():
            return True

        if self.out_of_time():
            self.state = States.POSITIONING
            return True

        return False

    def positioning(self):

        if self.generate_initial_positions:
            for robot in self.wm.our:
                self.initial_random_poses.append([ point.Point(random.uniform(0,4.5), random.uniform(-3, 3)), robot.id, False])
            self.generate_initial_positions = False

        for robot_struct in self.initial_random_poses:
            if self.gotopoint(robot_struct[1], robot_struct[0]):
                robot_struct[2] = True

        for robot_struct in self.initial_random_poses:
            if robot_struct[2] == False:
                return False
        return True

    def kick(self):

        self.kicker = self.findnearesttoball()
        task_pub2 = rospy.Publisher('agent_' + str(self.kicker.id) + str('/task'), parsian_robot_task,
                                    queue_size=1,
                                    latch=True)
        current_task = parsian_robot_task()
        current_task.select = parsian_robot_task.KICK
        task = parsian_skill_kick()
        task.chip = False
        task.spin = random.randint(0, 6)
        # task2.iskickchargetime = True
        # task2.kickchargetime = self.current_speed
        task.kickSpeed = 700  ##
        task.target.x = 6
        task.target.y = 0
        current_task.kickTask = task
        task_pub2.publish(current_task)

        self.random_pos_in_random_region()




    def ball_out_of_field(self):
        return self.wm.ball.pos.x > 6 or self.wm.ball.pos.x < -6 or self.wm.ball.pos.y > 4.5 or self.wm.ball.pos.y < -4.5

    def out_of_time(self):
        if time() - self.timer > 200:
            return True


    def gotopoint(self, id, point, dirpoint = point.Point(0, 1)):
        # type:(int, point.Point, point.Point) ->object
        task_pub = rospy.Publisher('agent_' + str(id) + str('/task'), parsian_robot_task,
                                         queue_size=1,
                                         latch=True)
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
        task_pub.publish(current_task1)
        if self.robotarrived(id, point):
            return  True
        else:
            return False

    # if the robots arrived too the target or not
    def robotarrived(self, id, target, dist=0.1):
        # type:(int, point.Point) ->object
        robot = parsian_robot
        for mrobot in self.wm.our:
            if mrobot.id == id:
                robot = mrobot
        if math.hypot(robot.pos.x - target.x, robot.pos.y - target.y) < dist:
            return True
        else:
            return False

    def findnearesttoball(self):
        near = parsian_robot
        dist = 10000
        for robot in self.wm.our:
            if self.dist(robot.pos, self.wm.ball.pos) < dist:
                near = robot
                dist = self.dist(robot.pos, self.wm.ball.pos)
        return near

    def dist(self, firstV2, secondV2):
        # type:(vector2D, vector2D) ->float
        return math.hypot(firstV2.x - secondV2.x, firstV2.y - secondV2.y)

    def random_pos_in_random_region(self):
        for robot_struct in self.initial_random_poses:
            if robot_struct[1] != self.kicker.id :
                if robot_struct[2]:
                    robot_struct[2] = self.gotopoint(robot_struct[1], point.Point(robot_struct[0].x + random.uniform(-3, 3) , robot_struct[0].y + random.uniform(-3, 3)))

    def resetplan(self):
        self.state = States.POSITIONING

        self.kicker = parsian_robot
        self.random_ids = []
        self.timer = time()

        self.generate_initial_positions = True
        self.initial_random_poses = []

        for robot in self.wm.our:
            task_pub2 = rospy.Publisher('agent_' + str(robot.id) + str('/task'), parsian_robot_task,
                                        queue_size=1,
                                        latch=True)
            current_task3 = parsian_robot_task()
            current_task3.select = parsian_robot_task.NOTASK
            task3 = parsian_skill_no()
            current_task3.noTask = task3
            task_pub2.publish(current_task3)
