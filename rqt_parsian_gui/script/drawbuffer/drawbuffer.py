from parsian_msgs.msg import parsian_world_model
from parsian_msgs.msg import parsian_draw

class DrawBuffer:

    def __init__(self):
        self.wm = parsian_world_model()
        self.draw = []

    def execute(self):
        pass