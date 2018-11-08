import time
import math
from point import Point
import rospy
import rospkg
from os import path
from parsian_msgs.msg import parsian_skill_gotoPointAvoid
from parsian_msgs.msg import parsian_robot_task
from parsian_msgs.msg import parsian_world_model
from parsian_msgs.msg import parsian_robot
from parsian_msgs.msg import parsian_robot_command

move_type = {"going": False, "coming_back": True}


class MotionProfiler:
    def __init__(self):

        self.__init_phase = 0
        self.__last_move_type = move_type["coming_back"]
        self.__ang_step = 0
        self.__dist_step = 0
        self.__start_pos = Point(0, 0)
        self.__end_pos = Point(0, 0)
        self.__robot_id = 0
        self.__max_vel = 0
        self.__result = dict()
        #        self.__result["max_vel"] = max_vel
        #        self.__result["robot_id"] = robot_id
        self.__current_key = ()
        self.__current_value = []
        self.__current_ang_step = 0
        self.__current_dist_step = -2
        self.__start_time = time.time()
        self.__path_angle = self.__start_pos.angle(self.__end_pos)
        self.__robot_command = parsian_robot_command()
        self.__current_task = parsian_robot_task()
        self.__current_task.select = parsian_robot_task.GOTOPOINTAVOID
        self.__wait_time = 0.0
        self.__task_start_time = 0.0
        self.__waiting_mode = False
        self.__isSaved = False
        self.__tasksAreFinished = False
        self.__doProfiling = False
        self.__updateTask()

    def reset(self, robot_id, start_pos, end_pos, **kw):
        # type: (int, Point, Point) -> object

        if 'init_phase' in kw.keys():
            self.__init_phase = kw['init_phase']
        else:
            self.__init_phase = 0

        if 'dist_step' in kw.keys():
            self.__dist_step = kw['dist_step']
        else:
            self.__dist_step = 2

        if 'ang_step' in kw.keys():
            self.__ang_step = kw['ang_step']
        else:
            self.__ang_step = 4

        if 'max_vel' in kw.keys():
            self.__max_vel = kw['max_vel']
        else:
            self.__max_vel = 4.5

        self.__last_move_type = move_type["coming_back"]
        self.__start_pos = start_pos
        self.__end_pos = end_pos
        self.__robot_id = robot_id
        self.__result = dict()
        #        self.__result["max_vel"] = max_vel
        #        self.__result["robot_id"] = robot_id
        self.__current_key = ()
        self.__current_value = []
        self.__current_ang_step = 0
        self.__current_dist_step = -2
        self.__start_time = time.time()
        self.__path_angle = self.__start_pos.angle(self.__end_pos)
        self.__robot_command = parsian_robot_command()
        self.__current_task = parsian_robot_task()
        self.__current_task.select = parsian_robot_task.GOTOPOINTAVOID
        self.__wait_time = 0.0
        self.__task_start_time = 0.0
        self.__waiting_mode = False
        self.__isSaved = False
        self.__tasksAreFinished = False
        self.__doProfiling = False
        self.__updateTask()

    def wmCallback(self, data):
        # type:(parsian_world_model)
        if self.__tasksAreFinished and not self.__doProfiling:
            if self.__isSaved:
                return
            else:
                self.__isSaved = True
                rospy.loginfo("saved")
                self.__addValueToKey()
                self.__saveResult()
                return

        if self.__doProfiling:
            self.doProfiling(data)

        my_robot = parsian_robot()
        my_robot.id = -1
        for robot in data.our:
            if robot.id == self.__robot_id:
                my_robot = robot

        if my_robot.id == -1:
            rospy.loginfo("robot " + str(self.__robot_id) + " losted!")
            return

        dis = Point(self.__current_task.gotoPointAvoidTask.base.targetPos.x,
                    self.__current_task.gotoPointAvoidTask.base.targetPos.y).distance(
            Point(my_robot.pos.x, my_robot.pos.y)
        )

        vel = math.hypot(my_robot.vel.x, my_robot.vel.y)

        if dis < .03 and vel < .1 and not self.__waiting_mode:
            self.__doProfiling = False
            self.__waiting_mode = True
            self.__wait_time = time.time()

        if self.__waiting_mode:
            if time.time() - self.__wait_time > .3:
                self.__waiting_mode = False
                self.__updateTask()

    def doProfiling(self, world_model):
        # type: (parsian_world_model) -> null

        my_robot = parsian_robot()
        my_robot.id = -1
        for robot in world_model.our:
            if robot.id == self.__robot_id:
                my_robot = robot

        if my_robot.id == -1:
            rospy.loginfo("robot not found")
            return

        vel_F = math.fabs(my_robot.vel.x * math.cos(self.__path_angle) + my_robot.vel.y * math.sin(self.__path_angle))

        dis = Point(self.__current_task.gotoPointAvoidTask.base.targetPos.x,
                    self.__current_task.gotoPointAvoidTask.base.targetPos.y).distance(
            Point(my_robot.pos.x, my_robot.pos.y))
        data = {
            "remain_dist": dis,
            "world_model": vel_F,
            "robot_command": math.hypot(self.__robot_command.vel_F, self.__robot_command.vel_N),
            "time": time.time() - self.__task_start_time
        }

        self.__current_value.append(data)

    def __saveResult(self):
        __log_file = open(path.abspath(rospkg.RosPack().get_path("parsian_agent")+ "/profiler_data/" + str(self.__robot_id) + "_" +
                                       str(int(round(time.time()/10))) + "_motion.profile"), "w+")
        __log_file.write(str(self.__result))
        __log_file.close()

    def __nextStep(self):
        if self.__current_dist_step == self.__dist_step:
            self.__current_dist_step = 0
            self.__current_ang_step += 1
        else:
            self.__current_dist_step += 1

    def __updateTask(self):
        task = parsian_skill_gotoPointAvoid()
        task.noAvoid = True
        task.base.lookAt.x = 5000
        task.base.lookAt.y = 5000
        task.base.maxVelocity = self.__max_vel

        self.__task_start_time = time.time()

        if self.__current_dist_step == -2:
            task.base.targetPos.x = self.__start_pos.x
            task.base.targetPos.y = self.__start_pos.y
            self.__nextStep()
        else:
            if self.__last_move_type == move_type["coming_back"]:
                self.__nextStep()
                task.base.targetPos.x = self.__start_pos.x + self.__start_pos.difX(
                    self.__end_pos) * self.__current_dist_step / float(self.__dist_step)
                task.base.targetPos.y = self.__start_pos.y + self.__start_pos.difY(
                    self.__end_pos) * self.__current_dist_step / float(self.__dist_step)
                task.base.targetDir.x = math.cos(self.__getTaskPhase())
                task.base.targetDir.y = math.sin(self.__getTaskPhase())
                self.__last_move_type = move_type["going"]
                if len(self.__current_key) is not 0:
                    self.__addValueToKey()
                    rospy.loginfo("done in " + str(time.time() - self.__task_start_time) + " s")
            else:  # "COMING BACK"
                task.base.targetPos.x = self.__start_pos.x
                task.base.targetPos.y = self.__start_pos.y
                task.base.targetDir.x = math.cos(self.__getTaskPhase())
                task.base.targetDir.y = math.sin(self.__getTaskPhase())
                self.__last_move_type = move_type["coming_back"]
                if self.__current_ang_step == self.__ang_step - 1 and self.__current_dist_step == self.__dist_step:
                    self.__tasksAreFinished = True
            if len(self.__current_key) is not 0:
                self.__addValueToKey()
                rospy.loginfo("done in " + str(time.time() - self.__task_start_time) + " s")

            if not self.__isSaved and self.__current_dist_step != 0:
                self.__addNewKey()
                self.__doProfiling = True

        self.__current_task.gotoPointAvoidTask = task

    def __getPhase(self):
        return math.pi * (self.__current_ang_step * 2 / float(self.__ang_step) + self.__init_phase / 180.0)\

    def __getTaskPhase(self):
        return self.__getPhase() + self.__path_angle

    def __addNewKey(self):
        if self.__last_move_type == move_type["going"]:
            self.__current_key = (
                self.__end_pos.distance(self.__start_pos) * self.__current_dist_step / self.__dist_step,
                self.__getPhase(), self.__path_angle)
        else:
            self.__current_key = (
                self.__end_pos.distance(self.__start_pos) * self.__current_dist_step / self.__dist_step,
                math.pi + self.__getPhase() - (0 if math.pi + self.__getPhase() < 2 * math.pi else 2 * math.pi),
                math.pi + self.__path_angle - (0 if math.pi + self.__path_angle < 2 * math.pi else 2 * math.pi))

    def __addValueToKey(self):
        self.__result[self.__current_key] = {"data": self.__current_value,
                                             "total_time": self.__current_value[-1]["time"]}
        rospy.loginfo(self.__current_key)
        self.__current_value = []
        self.__current_key = ()

    def set_robot_id(self, robot_id):
        self.__robot_id = robot_id

    def set_robot_command(self, robot_commaand):
        self.__robot_command = robot_commaand

    def get_task(self):
        return self.__current_task
