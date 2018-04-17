#!/usr/bin/env python
from rosgraph_msgs.msg import Log
from parsian_tools.cfg import blackBoardConfig
from dynamic_reconfigure.server import Server
import sys
import os
import rospy
import re
names = ["D_ERROR", "D_GAME", "D_EXPERIMENT", "D_DEBUG", "D_NADIA", "D_MAHI", "D_ALI", "D_MHMMD", "D_FATEME", "D_AHZ",
         "D_AMIN", "D_PARSA", "D_HAMED", "D_SEPEHR","D_KK", "D_HOSSEIN", "D_ATOUSA"]

class BlackBoardConsole:
    def __init__(self):
        rospy.init_node('blackBoard', anonymous=True)
        if len(sys.argv) > 1:
            self.mode = 0 if int(sys.argv[1]) != 1 else 1
        else:
            self.mode = 0
        print(self.mode)
        self.data = []
        self.config = blackBoardConfig
        self.srv = Server(blackBoardConfig, self.cfg_callback)
        rospy.Subscriber('/rosout', Log,
                         self.callback, queue_size=1, buff_size=2 ** 24)
        rospy.spin()

    def callback(self, msg):
        # type: (Log) -> null
        my_level = msg.level
        if my_level is Log.DEBUG:
            node_name_msg = msg.msg.split("::")
            if len(node_name_msg) != 3:
                return
            node = node_name_msg[0].split(".")[1]
            namespace = node_name_msg[1]
            debug_msg = node_name_msg[2]
            if self.config[names[int(namespace)]] is False:
                return

            if self.config[node] is False and self.config["parsian"] is False:
                return

            if self.mode == 1:
                k_v_pair = debug_msg.split(" = ")
                if k_v_pair[0] == "":
                    new_key_pair = re.split(':|=', k_v_pair[1])
                    if len(new_key_pair) >= 2:
                        k_v_pair[0] = ""
                        for i in range(len(new_key_pair) - 1):
                            k_v_pair[0] += new_key_pair[i]
                        k_v_pair[1] = new_key_pair[-1]
                    else:
                        return
                change_index = self.add_data(k_v_pair[0], k_v_pair[1])
                if change_index != -2:
                    if change_index == -1:
                        change_index += len(self.data)
                    self.clear_screen(change_index)
                    self.preview_data(change_index)
                if change_index == -1:
                    self.print_there(0, 20 + len(self.data), "new data( "+str(len(self.data))+" )" + msg.msg)
            else:
                print(debug_msg[3:])


    def print_there(self, x, y, text):
        sys.stdout.write("\x1b7\x1b[%d;%df%s\x1b8" % (y, x, text))
        sys.stdout.flush()

    def clear_screen(self, row):
        rows, columns = os.popen('stty size', 'r').read().split()
        start = len(self.data[row][0])+3
        self.print_there(start, row + 1, " " * (int(columns)-start))

    def preview_data(self, row):
        self.print_there(0, row+1, self.data[row][0] + " = " + self.data[row][1])

    def add_data(self, var_name, new_data):
        change_index = -1
        for index in range(len(self.data)):
            if self.data[index][0] == var_name:
                if self.data[index][1] == new_data:
                    return -2
                else:
                    change_index = index
                    break

        if change_index == -1:
            self.data.append((var_name, new_data))
        else:
            self.data[change_index] = (var_name, new_data)
        return change_index

    def cfg_callback(self, config, level):
        self.config = config
        return config


if __name__ == '__main__':
    try:
        BlackBoardConsole()
    except rospy.ROSInterruptException:
        pass
