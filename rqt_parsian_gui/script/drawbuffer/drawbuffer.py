from parsian_msgs.msg import parsian_world_model
from parsian_msgs.msg import parsian_draw_buffer
from parsian_msgs.msg import parsian_draw
from parsian_msgs.msg import parsian_draws


class DrawBuffer:
    def __init__(self):
        self.wm = parsian_world_model()
        self.draw = {}
        self.update = {}
        self.blacklist = []

    def add_draw(self, draws):
        self.draw[draws.node] = draws.draws
        self.update[draws.node] = True

    def get_msg(self):
        db = parsian_draw_buffer()
        db.wm = self.wm
        db.draws.node = 'drawbuffer_node'
        s = []
        for d in self.draw.values():
            for f in d:
                s.append(f)
        db.draws.draws = s
        return db
