#!/usr/bin/env python
# license removed for brevity

import rospy
from parsian_msgs.srv import *
import docWatch
import time


class getPlan:
    def __init__(self):

        rospy.init_node('plan_server')
        self.s1 = rospy.Service('update_plans', parsian_update_plans, self.handle_gui_plan_request)
        self.s2 = rospy.Service('get_plans', plan_service, self.handle_plan_request)
        self.__w = docWatch.Watcher()
        self.response = parsian_update_plansResponse()
        self.__w.run()

    def handle_gui_plan_request(self, req):
        # type:(parsian_update_plansRequest) -> req

        print("---> request:")
        print(req)
        received = req.newPlans
        if '' in received:
            received.remove('')
        if len(received) > 0:
            print ("response to gui...... update plans")
            self.response.allPlans = self.__w.update_master_active(received, req.index, req.isMaster, req.isActive)
            return self.response
        else:
            print ("response to gui...... return all plans")
            self.response.allPlans = self.__w.get_all_plans()
            return self.response

    def handle_plan_request(self, req):
        # type: (plan_serviceRequest) -> req
        print("REQUEST:: #players: " + str(req.plan_req.playersNum) + " game mode: " + str(req.plan_req.gameMode) + "\n")
        t = int(round(time.time() * 1000000))
        response = plan_serviceResponse()
        out = self.__w.choose_plan(req.plan_req.playersNum, req.plan_req.gameMode, req.plan_req.ballPos.x, req.plan_req.ballPos.y)
        if out is not None:
            t2 = int(round(time.time() * 1000000)) - t
            response.the_plan = out
            response.time_us = t2
            print ("time: " + str(t2) + " ms")
            return response
        else:
            print ("No plan sent :(")


if __name__ == '__main__':
    g = getPlan()
    rospy.spin()
