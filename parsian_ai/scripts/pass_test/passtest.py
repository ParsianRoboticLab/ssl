from parsian_msgs.msg import parsian_robot_task
from parsian_msgs.msg import parsian_world_model
from math import sqrt
from time import time

class Point(object):
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __init__(self,o):
        self.x = o.x
        self.y = o.y

    def norm(self):
        return sqrt(self.x ** 2 + self.y ** 2)

    def distance(self, other):
        dx = self.x - other.x
        dy = self.y - other.y
        return sqrt(dx**2 + dy**2)


class PassTest:
    def __init__(self):
        self.wm = parsian_world_model()
        self.recieive_point = Point(-3 ,-2)
        self.tasks = []
        self.bp_start_time = 0
        self.STATES = {"wait_for_receiver": 0, "ball_placement": 1, "kick": 2, "receive": 3}
        self.state = self.STATES["wait_for_receiver"]
        self.ids = {"receiver": 0, "passer": 1}
        self.knowlege = { "rec_dist_to_target": 5000,
                          "rec_dist_to_ball":   5000, "passer_dist_to_ball": 5000}

    def update_wm(self, wm):  # type: (parsian_world_model)
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

        print("state:  ", self.state)

        self.update_knowlege()

        if self.state == self.STATES["wait_for_receiver"]:

            if self.knowlege["rec_dist_to_target"] < .1:
                self.state = self.STATES["ball_placement"]
                self.bp_start_time = time()
            else:
                return

        if self.state == self.STATES["ball_placement"]:
            dif_time = time() - self.bp_start_time
            if Point(self.wm.ball.vel).norm() > .01:
                dif_time = 0
            print(dif_time)
            if dif_time > 3:
                self.state = self.STATES["kick"]
                self.save_pass_state()

        if self.state == self.STATES["kick"]:
            if self.kick_done():
               self.state = self.STATES["receive"]
            else:
                self.do_pass()

        if self.state == self.STATES["receive"]:
            self.check_pass_info()
            if self.recive_done():
                self.show_result()
                self.state = self.STATES["wait_for_receiver"]


    def update_knowlege(self):
        self.knowlege["rec_dist_to_target"] = self.recieive_point.distance(self.wm.our[self.ids["receiver"]].pos)
        self.knowlege["rec_dist_to_ball"] = Point(self.wm.ball.pos).distance(self.wm.our[self.ids["receiver"]].pos)
        self.knowlege["passer_dist_to_ball"] = Point(self.wm.ball.pos).distance(self.wm.our[self.ids["receiver"]].pos)


    def map_distance(self,dist):

        if dist < 2 :
            mapped_dist = 0
        elif dist < 3 :
            mapped_dist = 1
        elif dist < 3.5 :
            mapped_dist = 2
        elif dist < 4 :
            mapped_dist = 3
        elif dist < 4.5 :
            mapped_dist = 4
        elif dist < 5 :
            mapped_dist = 5
        elif dist < 6 :
            mapped_dist = 6
        else:
            mapped_dist = 7

        return mapped_dist