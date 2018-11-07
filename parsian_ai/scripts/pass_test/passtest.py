from parsian_msgs.msg import parsian_robot_task
from parsian_msgs.msg import parsian_world_model
from math import sqrt
from time import time
from math import exp
from pprint import pprint
from math import asin
robot_radius = .1


class Point:
    def __init__(self, x=5000, y=5000):
        self.x = x
        self.y = y

    def init(self, o):
        self.x = o.x
        self.y = o.y
        return self

    def norm(self):
        return sqrt(self.x ** 2 + self.y ** 2)

    def distance(self, other):
        dx = self.x - other.x
        dy = self.y - other.y
        return sqrt(dx ** 2 + dy ** 2)

    def __add__(self, other):
        return Point(self.x + other.x, self.y + other.y)

    def __sub__(self, other):
        return Point(self.x - other.x, self.y - other.y)

    def cross(self, p):
        return abs(self.x * p.y - self.y * p.x)

    def __mul__(self, other):
        return Point(self.x * other, self.y * other)

    def distance_to_line(self, p0, p1):
        return (p1 - p0).cross(self - p1) / (p1 - p0).norm()


class PassTest:
    def __init__(self):
        self.wm = parsian_world_model()
        self.recieive_point = Point(-3, -2)
        self.tasks = []
        self.bp_start_time = 0
        self.STATES = {"wait_for_receiver": 0, "ball_placement": 1, "kick": 2, "receive": 3}
        self.state = self.STATES["wait_for_receiver"]
        self.ids = {"receiver": 0, "passer": 1}
        self.knowlege = {"rec_dist_to_target": 5000, "ball_dist_to_target": 5000,
                         "rec_dist_to_ball": 5000, "passer_dist_to_ball": 5000}
        self.pass_informations = {}
        self.current_pass_key = None
        self.speed_step = 8
        self.vel_queue_size = 5
        self.ball_vel_queue = []

    def update_wm(self, wm):  # type: (parsian_world_model) -> None
        self.wm = wm

        self.exe()

    def get_tasks(self):
        return self.tasks

    def choose_rec_pass(self):
        changed = {"receiver": True, "passer": True}
        our_len = len(self.wm.our)
        if our_len < 2:
            return False

        for i in range(our_len):
            if self.ids["receiver"] == self.wm.our[i].id:
                changed["receiver"] = False

        for i in range(our_len):
            if self.ids["passer"] == self.wm.our[i].id:
                changed["passer"] = False

        if changed["receiver"]:
            for i in range(our_len):
                if self.wm.our[i].id != self.ids["passer"]:
                    self.ids["receiver"] = self.wm.our[i].id
                    break

        if changed["passer"]:
            for i in range(our_len):
                if self.wm.our[i].id != self.ids["receiver"]:
                    self.ids["passer"] = self.wm.our[i].id
                    break
        return True

    def exe(self):
        if not self.choose_rec_pass():
            print("cant find two robots")
            return

        # print("state:  ", self.state)

        self.update_knowlege()

        no_task = parsian_robot_task()
        no_task.select = no_task.NOTASK

        rec_task = parsian_robot_task()
        rec_task.select = rec_task.RECIVEPASS
        rec_task.receivePassTask.target.x = self.recieive_point.x
        rec_task.receivePassTask.target.y = self.recieive_point.y
        rec_task.receivePassTask.receiveRadius = 1

        self.tasks = []
        self.tasks.append({
            "id": self.ids["passer"],
            "msg": no_task
        })
        self.tasks.append({
            "id": self.ids["receiver"],
            "msg": rec_task
        })

        if self.state == self.STATES["wait_for_receiver"]:

            if self.knowlege["rec_dist_to_target"] < .1:
                self.state = self.STATES["ball_placement"]
                self.bp_start_time = time()
            else:
                return

        if self.state == self.STATES["ball_placement"]:
            dif_time = time() - self.bp_start_time
            if Point().init(self.wm.ball.vel).norm() > .01:
                dif_time = 0
            # print(dif_time)
            if dif_time > 3:
                self.state = self.STATES["kick"]
                self.save_pass_state()
                print(self.current_pass_key)

        if self.state == self.STATES["kick"]:
            if self.kick_done():
                self.state = self.STATES["receive"]
            else:
                self.do_pass()

        if self.state == self.STATES["receive"]:
            self.check_pass_info()
            if self.receive_done():
                self.show_result()
                self.state = self.STATES["wait_for_receiver"]

    def update_knowlege(self):
        self.knowlege["rec_dist_to_target"] = self.recieive_point.distance(self.wm.our[self.ids["receiver"]].pos)
        self.knowlege["rec_dist_to_ball"] = Point().init(self.wm.ball.pos).distance(self.wm.our[self.ids["receiver"]].pos)
        self.knowlege["passer_dist_to_ball"] = Point().init(self.wm.ball.pos).distance(self.wm.our[self.ids["passer"]].pos)
        self.knowlege["ball_dist_to_target"] = self.recieive_point.distance(self.wm.ball.pos)

        self.ball_vel_queue.append(Point().init(self.wm.ball.vel).norm())
        if len(self.ball_vel_queue) > self.vel_queue_size:
            self.ball_vel_queue.pop(0)

    def map_distance(self, dist):
        if dist < 2:
            mapped_dist = 0
        elif dist < 3:
            mapped_dist = 1
        elif dist < 3.5:
            mapped_dist = 2
        elif dist < 4:
            mapped_dist = 3
        elif dist < 4.5:
            mapped_dist = 4
        elif dist < 5:
            mapped_dist = 5
        elif dist < 6:
            mapped_dist = 6
        else:
            mapped_dist = 7

        return mapped_dist

    def save_pass_state(self):
        self.current_pass_key = self.map_distance(self.knowlege["ball_dist_to_target"])
        if self.current_pass_key not in self.pass_informations :
            self.pass_informations[self.current_pass_key] = []

        cur_step = len(self.pass_informations[self.current_pass_key])
        if cur_step < self.speed_step:
            self.pass_informations[self.current_pass_key].append(
                {
                    "step": 10 / self.speed_step * cur_step,
                    "dist": self.knowlege["ball_dist_to_target"]
                }
            )

    def kick_done(self):
        if sum(self.ball_vel_queue) / len(self.ball_vel_queue) > .05:
            deviation = self.recieive_point.distance_to_line(Point().init(self.wm.ball.pos), Point().init(self.wm.ball.vel))
            deviation_eval = abs(asin(deviation/self.knowlege["ball_dist_to_target"])) / 3.1415 * 180
            self.pass_informations[self.current_pass_key][-1].update(
                {
                    "deviation": deviation_eval
                }
            )
            print("kick Done")
            return True
        return False

    def do_pass(self):
        task = parsian_robot_task()
        task.select = task.KICK
        if self.current_pass_key in self.pass_informations:
            cur_step = len(self.pass_informations[self.current_pass_key])
        else:
            cur_step = 0
        task.kickTask.kickSpeed = 10 * (cur_step+1) / self.speed_step
        task.kickTask.kickchargetime = True
        task.kickTask.target.x = self.recieive_point.x
        task.kickTask.target.y = self.recieive_point.y

        self.tasks[0]= {
            "id": self.ids["passer"],
            "msg": task
        }

    def check_pass_info(self):
        if self.knowlege["rec_dist_to_ball"] < robot_radius + .03:
            rec_pos = Point().init(self.wm.our[self.ids["receiver"]].pos)
            rec_dir = Point().init(self.wm.our[self.ids["receiver"]].dir)
            dist_to_fak = Point().init(self.wm.ball.pos).distance_to_line(
                rec_pos, rec_pos + rec_dir
            )
            self.pass_informations[self.current_pass_key][-1].update(
                {
                    "dist_to_fak_eval": 1 - dist_to_fak / robot_radius
                }
            )
            print("received.. dist: ", self.knowlege["rec_dist_to_ball"])

    def show_result(self):
        pprint(self.pass_informations)
        print("\n --------- \n")

    def receive_done(self):
        ave_bal_vel = sum(self.ball_vel_queue) / len(self.ball_vel_queue)
        if ave_bal_vel < .1:
            print("receive done")
            return True
        else:
            return False
