import parsian_msgs
import rospy
from dynamic_reconfigure import server
from parsian_msgs.msg import parsian_world_model
from parsian_msgs.msg import parsian_draw
import drawbuffer

db = drawbuffer.DrawBuffer()
pub = rospy.Publisher('/buffer_draws', parsian_draw, queue_size=1000)


def wm_cb(wm):
    db.wm = wm


def draw_cb(draw):
    db.draw.append(draw)


def timer_cb():
    pass


if __name__ == "__main__":
    rospy.init_node('drawbuffer_node', anonymous=True)
    wm_sub = rospy.Subscriber('/world_model', parsian_world_model, wm_cb, queue_size=1000)
    draw_sub = rospy.Subscriber('/draws', parsian_draw, draw_cb, queue_size=1000)
    timer = rospy.Timer(rospy.Duration(secs=0, nsecs=20000000), timer_cb)
    srv = server.Server()
    rospy.spin()
