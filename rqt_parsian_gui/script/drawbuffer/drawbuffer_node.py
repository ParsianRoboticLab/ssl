#!/usr/bin/python
import rospy
from parsian_msgs.msg import parsian_world_model
from parsian_msgs.msg import parsian_draws
from parsian_msgs.msg import parsian_draw_buffer
from parsian_msgs.srv import parsian_logger, parsian_loggerRequest, parsian_loggerResponse
from parsian_msgs.srv import parsian_log_player

import logger
import drawbuffer

db = drawbuffer.DrawBuffer()
pub = rospy.Publisher('/buffer_draws', parsian_draw_buffer, queue_size=1, latch=True)

lgr = logger.Logger()


def wm_cb(wm):
    print "wm"
    db.wm = wm
    if lgr.mode == parsian_loggerRequest.LIVE or lgr.mode == parsian_loggerRequest.RECORD:
        lgr.add_wm_to_history(wm)


def draw_cb(draw):
    db.add_draw(draw)
    if lgr.mode == parsian_loggerRequest.LIVE or lgr.mode == parsian_loggerRequest.RECORD:
        lgr.add_draw_to_history(draw)


def timer_cb(time):
    out = lgr.plr.get_frame()
    if (lgr.mode == parsian_loggerRequest.HISTORY or lgr.mode == parsian_loggerRequest.LOAD) and out:
        pub.publish(lgr.plr.get_frame())
    else:
        pub.publish(db.get_msg())


def player_timer_cb(time):
    frame = lgr.plr.play(time)
    if frame:
        pub.publish(frame)


def clean_cb(time):
    for k in db.update:
        if db.update[k] is False:
            db.draw[k] = []
        db.update[k] = False


def logger_cb(req):
    global lgr
    return lgr.parse_req(req)


def log_player_cb(req):
    global lgr
    return lgr.plr.parse_req(req)


if __name__ == "__main__":
    rospy.init_node('drawbuffer_node', anonymous=True)
    wm_sub = rospy.Subscriber('/world_model', parsian_world_model, wm_cb, queue_size=1, buff_size=2 ** 24)
    draw_sub = rospy.Subscriber('/draws', parsian_draws, draw_cb, queue_size=1, buff_size=2 ** 24)
    timer = rospy.Timer(rospy.Duration(secs=0, nsecs=16000000), timer_cb)
    player_timer = rospy.Timer(rospy.Duration(secs=0, nsecs=1000000), player_timer_cb)
    clean = rospy.Timer(rospy.Duration(secs=1, nsecs=0), clean_cb)
    logger_server = rospy.Service('/logger', parsian_logger, logger_cb)
    player_server = rospy.Service('/log_player', parsian_log_player, log_player_cb)
    rospy.spin()
