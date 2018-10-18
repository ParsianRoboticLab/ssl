from parsian_msgs.srv import parsian_log_player, parsian_log_playerRequest, parsian_log_playerResponse
from parsian_msgs.msg import parsian_draw_buffer
import rospy


class Player():
    def __init__(self):
        self.current = rospy.Time()
        self.fps = 60
        self.loop = False
        self.l_start = 0
        self.l_end = -1
        self.wm_history = []
        self.wm_index = -1

    def play(self, time):
        pass

    def get_frame(self):
        db = parsian_draw_buffer()
        wm = None
        if self.wm_index < len(self.wm_history) - 1:
            self.wm_index += 1
            wm = self.wm_history[self.wm_index]
        db.wm = wm
        return db

    def add_history(self, history):
        self.wm_history = history
        self.wm_index = -1

    def add_logfile(self, logfile):
        pass

    def parse_req(self, req):
        res = parsian_log_playerResponse()
        res.ok = True
        res.fps = self.fps = req.fps
        res.loop = self.loop = req.loop
        res.l_end = self.l_end = req.l_end
        res.l_start = self.l_start = req.l_start
        return res