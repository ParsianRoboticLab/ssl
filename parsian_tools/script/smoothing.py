#!/usr/bin/env python
from parsian_msgs.msg import ssl_vision_detection
from parsian_msgs.msg import ssl_vision_detection_ball
from filterpy.kalman import KalmanFilter
import numpy as np
import rospy

s = .016
kalman_gain = 0


class vector:
    def __init__(self):
        self.x = 0
        self.y = 0


class Latency:
    def __init__(self):
        self.queue = []
        rospy.init_node('latency', anonymous=True)

        self.kalman = KalmanFilter(dim_x=6, dim_z=2)
        self.init()

        self.pos = vector()
        self.vel = vector()
        self.acc = vector()

        self.last_pos = vector()
        self.last_vel = vector()
        self.last_acc = vector()

        rospy.Subscriber('/vision_detection', ssl_vision_detection,
                         self.visionCallback, queue_size=1, buff_size=2 ** 24)
        rospy.spin()

    def copy(self, x, y):
        self.pos.x = x
        self.pos.y = y

        self.last_vel.x = self.vel.x
        self.last_vel.y = self.vel.y

        self.last_acc.x = self.acc.x
        self.last_acc.y = self.acc.y

    def get_data(self, x, y):
        self.copy(x, y)
        self.vel.x = (self.pos.x - self.last_pos.x) * 60
        self.vel.y = (self.pos.y - self.last_pos.y) * 60

        self.acc.x = (self.vel.x - self.last_vel.x) * 60
        self.acc.y = (self.vel.y - self.last_vel.y) * 60

        return np.array([[self.pos.x, self.pos.y], [self.vel.x, self.vel.y], [self.acc.x, self.acc.y]]).T

    def init(self):
        self.kalman.F = np.array([[1, 0, s, 0, s ** 2 / 2, 0],
                                  [0, 1, 0, s, 0, s ** 2 / 2],
                                  [0, 0, 1, 0, s, 0],
                                  [0, 0, 0, 1, 0, s],
                                  [0, 0, 0, 0, kalman_gain, 0],
                                  [0, 0, 0, 0, 0, kalman_gain]])

        self.kalman.H = np.array([[1, 0, 0, 0, 0, 0],
                                  [0, 1, 0, 0, 0, 0]])
        self.kalman.P *= .05
        self.kalman.R = np.array([[.005, 0],
                                  [0, .005]])
        # self.kalman.Q = Q_discrete_white_noise(dim=6, dt=0.1, var=0.13)

    def visionCallback(self, data):
        # type: (ssl_vision_detection)
        if len(data.balls) > 0:
            ball = data.balls[0]  # type:ssl_vision_detection_ball
            self.kalman.predict()
            self.kalman.update(np.array([ball.pos.x, ball.pos.y]))
            print self.kalman.x


if __name__ == '__main__':
    try:
        Latency()
    except rospy.ROSInterruptException:
        pass
