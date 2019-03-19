#!/usr/bin/env python

import rospy
import os
from watchdog.events import FileSystemEventHandler




class Watcher(FileSystemEventHandler):
    patterns = ['*.json']
    def __init__(self, path):
        #initiate
        self.path = path
        rospy.Timer(rospy.Duration(0.5), self.my_callback)
        self.is_newEvent_happend = True

        #update_plans
        self.all_jsons_root = []#all files with json extension
        self.all_ignoredjsons_root = []#all ignored files with json extension
        self.all_desiredjson_root = []#all json files that are not in ignore file


    def on_any_event(self, event):
        self.is_newEvent_happend = True

    ##update all plans every 0.5 sec if a new event happend in directory
    def my_callback(self, event):
        if not self.is_newEvent_happend:
            return
        self.is_newEvent_happend = False

        self.get_all_jsons_root()

        self.get_all_ignoredjsons_root()

        self.get_all_desiredjson_root()


    def get_all_jsons_root(self):
        self.all_jsons_root = []
        for root, dirs, files in os.walk(self.path, topdown=False):
            for name in files:
                if name.lower().endswith(".json"):
                    self.all_jsons_root.append(os.path.join(root, name))

    def get_all_ignoredjsons_root(self):
        os.chdir(self.path)
        self.all_ignoredjsons_root = []
        if not os.path.exists("plans.ignore") and not os.path.isfile("plans.ignore"):
            rospy.loginfo("plans.ignore not found")
            return
        clear_comment_lines = []#ignore all commented parts
        with open("plans.ignore") as file:
            for line in file:
                if line.find('#') == 0:
                    continue
                elif line.find('#') > 0:
                    clear_comment_lines.append(line[0: line.find('#')].rstrip())
                else:
                    clear_comment_lines.append(line.rstrip())

        for i in range(len(clear_comment_lines)):
            if clear_comment_lines[i].startswith('/'):
                clear_comment_lines[i] = clear_comment_lines[i][1:]


        for line in clear_comment_lines:
            os.chdir(self.path)
            if os.path.isfile(line) and line.lower().endswith(".json"):
                self.all_ignoredjsons_root.append(os.path.join(self.path, line))
            elif os.path.isdir(os.path.join(self.path, line)):
                for root, dirs, files in os.walk(os.path.join(self.path, line), topdown=False):
                    for name in files:
                        if name.lower().endswith(".json"):
                            self.all_ignoredjsons_root.append(os.path.join(root, name))

    def get_all_desiredjson_root(self):
        pass