#!/usr/bin/env python
from rosgraph_msgs.msg import Log
import sys
import os
import rospy
from time import time


class BlackBoardConsole:
    def __init__(self):
        rospy.init_node('latency', anonymous=True)
        self.print_index = 1
        if len(sys.argv) > 1:
            self.limt_preview_time = int(sys.argv[1])
            self.dont_apply_time_limit = False
        else:
            self.limt_preview_time = 0
            self.dont_apply_time_limit = True
        self.data = {}
        self.clear_screen()
        rospy.Subscriber('/rosout', Log,
                         self.callback, queue_size=1, buff_size=2 ** 24)
        rospy.spin()

    def callback(self, msg):
        # type: (Log) -> null
        k_v_pair = msg.msg.split(" = ")
        my_level = msg.level
        if my_level is Log.DEBUG:
            is_changed = self.add_data(k_v_pair[0], k_v_pair[1])
            if is_changed:
                self.clear_screen()
                self.preview_data()

    def print_there(self, x, y, text):
        sys.stdout.write("\x1b7\x1b[%d;%df%s\x1b8" % (y, x, text))
        sys.stdout.flush()

    def clear_screen(self):
        rows, columns = os.popen('stty size', 'r').read().split()

        for i in range(int(rows)):
            self.print_there(0, i, " " * int(columns))

    def add_data(self, key, new_data):
        is_changed = True
        if key in self.data:
            if self.data[key][0] == new_data:
                is_changed = False
        self.data[key] = (new_data, time())
        return is_changed

    def preview_data(self):
        self.print_index = 1
        for x in self.data:
            if time() - self.data[x][1] < self.limt_preview_time or self.dont_apply_time_limit:
                self.print_there(0, self.print_index, x + "  =  " + self.data[x][0])
                self.print_index += 1

if __name__ == '__main__':
    try:
        BlackBoardConsole()
    except rospy.ROSInterruptException:
        pass
