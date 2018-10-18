from parsian_msgs.srv import parsian_logger, parsian_loggerRequest, parsian_loggerResponse
import rosbag

MAX_DRAW_HISTORY = 600
MAX_WM_HISTORY = 600


class Logger:

    def __init__(self):
        self.mode = parsian_loggerRequest.LIVE
        self.log_path = ''
        self.wm_history = []
        self.draw_history = {}

    def parse_req(self, req):
        res = parsian_loggerResponse()

        if req.mode == self.mode:
            res.ok = False
            res.mode = self.mode
            return res

        if req.mode == parsian_loggerRequest.JUST_LIVE:
            self.save_log()
            self.clear_history()
            res.filename = self.log_path
        elif req.mode == parsian_loggerRequest.LIVE:
            self.save_log()
        elif req.mode == parsian_loggerRequest.HISTORY:
            self.pause_log()
            res.metadata.append('wm: ' + str(len(self.wm_history)))
            [res.metadata.append(str(k) + ': ' + str(len(self.draw_history[k]))) for k in self.draw_history.keys()]

        elif req.mode == parsian_loggerRequest.RECORD:
            pass
        elif req.mode == parsian_loggerRequest.LOAD:
            pass

        self.mode = res.mode = req.mode
        res.ok = True
        return res

    def pause_log(self):
        pass

    def save_log(self):
        pass

    def clear_history(self):
        pass

    def add_wm_to_history(self, wm):
        self.wm_history.append(wm)
        if len(self.wm_history) > MAX_WM_HISTORY:
            self.wm_history.pop(0)

    def add_draw_to_history(self, draw):
        if draw.node not in self.draw_history.keys():
            self.draw_history[draw.node] = []
        self.draw_history[draw.node].append(draw.draws)
        if len(self.draw_history[draw.node]) > MAX_DRAW_HISTORY:
            self.draw_history[draw.node].pop(0)
