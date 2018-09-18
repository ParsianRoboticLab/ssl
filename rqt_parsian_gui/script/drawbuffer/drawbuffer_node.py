import rospy
from dynamic_reconfigure import server
from parsian_msgs.msg import parsian_world_model
from parsian_msgs.msg import parsian_draws
from parsian_msgs.msg import parsian_draw_buffer
from rqt_parsian_gui.cfg import drawbufferConfig

import drawbuffer

db = drawbuffer.DrawBuffer()
pub = rospy.Publisher('/buffer_draws', parsian_draw_buffer, queue_size=1, latch=True)


def wm_cb(wm):
    print "wm"
    db.wm = wm


def draw_cb(draw):
    db.add_draw(draw)


def timer_cb(time):
    pub.publish(db.get_msg())


def clean_cb(time):
    for k in db.update:
        if db.update[k] is False:
            db.draw[k] = []
        db.update[k] = False


# def config_cb(config, level):
#     print config


if __name__ == "__main__":
    rospy.init_node('drawbuffer_node', anonymous=True)
    wm_sub = rospy.Subscriber('/world_model', parsian_world_model, wm_cb, queue_size=1, buff_size=2 ** 24)
    draw_sub = rospy.Subscriber('/draws', parsian_draws, draw_cb, queue_size=1, buff_size=2 ** 24)
    timer = rospy.Timer(rospy.Duration(secs=0, nsecs=16000000), timer_cb)
    clean = rospy.Timer(rospy.Duration(secs=1, nsecs=0), clean_cb)
    # srv = server.Server(drawbufferConfig, config_cb)
    rospy.spin()
