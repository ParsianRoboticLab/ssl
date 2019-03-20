#!/usr/bin/env python

import rospy
import rospkg
import os
from docWatch import Watcher
from watchdog.observers import Observer
from parsian_msgs.srv import plan_service



class PlanServer:
    def __init__(self):
        self.path = os.path.join(rospkg.RosPack().get_path("parsian_ai"), "plans")#/home/kian/parsian_ws/src/parsian_ssl/parsian_ai/plans
        self.watcher = Watcher(self.path)

        self.ai_service = rospy.Service('get_plans', plan_service, self.handle_ai_request)

        self.observer = Observer()
        self.observer.schedule(self.watcher, self.path, recursive=True)
        self.observer.start()

    def handle_ai_request(self, req):
        res = self.watcher.choose_plan(req)
        return res




if __name__ == '__main__':
    try:
        rospy.init_node('plan_server', anonymous=True)
        rospy.loginfo("plan_server is running")
        planServer = PlanServer()
        rospy.spin()
    except rospy.ROSInterruptException:
        pass
