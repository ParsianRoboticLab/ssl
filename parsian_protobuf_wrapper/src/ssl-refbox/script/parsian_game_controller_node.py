#!/usr/bin/env python
# license removed for brevity

import rospy
import socket
from time import sleep
from GameControllerCommon import GameControllerCommon
from dynamic_reconfigure.server import Server
from parsian_protobuf_wrapper.cfg import refereeConfig
from parsian_ai.cfg import aiConfig
from parsian_msgs.msg import parsian_draws
from parsian_msgs.msg import parsian_draw
from parsian_msgs.msg import parsian_draw_text
from parsian_msgs.msg import parsian_world_model

TEAM_NAME = 'Test Team'


class GameController():
    def __init__(self):

        ##variables
        self.registered = False
        self.goalie_id = -1
        self.isFirstGoalieAssignment = True
        self.gc = GameControllerCommon()
        ##load rsa key
        is_privatekey_exist, self.privatekey = self.gc.readPrivateKey(TEAM_NAME + '.key.pem')
        if not is_privatekey_exist:
            rospy.loginfo('COULDNT FIND ANY PRIVATE KEY')
            exit(0)

        ##socket
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        #dynamic reconfigure
        self.IP = '127.0.0.1'
        self.PORT = 10008
        self.srv_net = Server(refereeConfig, self.cfg_callback_net, "/refbox")
        self.srv_ai = Server(aiConfig, self.cfg_callback_ai, "/ai_node")

        #draw
        self.draw_pub = rospy.Publisher('/draws', parsian_draws, queue_size=1, latch=True)
        self.wm_sub = rospy.Subscriber('world_model', parsian_world_model, self.wmCallback, queue_size=1,
                                       buff_size=2 ** 24)



    def cfg_callback_net(self, config, level):
        self.IP = config.refree_listen_ip
        self.PORT = config.refree_listen_port
        self.IP = '127.0.0.1' #TODO delete this
        #register
        self.register()

        return config

    def cfg_callback_ai(self, config, level):
        if self.goalie_id != config.Goalie:
            self.goalie_id = config.Goalie
            if self.registered:
                self.assigngoalie()

        return config


    def register(self):

        self.registered = False
        self.isgoalieassigned = False
        self.socket.close()
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.socket.connect((self.IP, self.PORT))
        except:
            rospy.loginfo('cannot bind to the gamecontroller')
            return
            #sleep(0.5)
            #self.register()
        ##getting initiate response
        initiate_result_msg, iscontrollerreply = self.gc.readControllerToTeam(self.socket)
        if not self.gc.verification:
            rospy.loginfo('problem in getting the initiate response from controller')
            return
            #sleep(0.5)
            #self.register()
        ##send registration data
        registration_serialized = self.gc.teamSerializedRegistration(self.socket, TEAM_NAME, self.gc.token, self.privatekey)
        self.gc.sendSerializedMessage(self.socket, registration_serialized, False)
        ##getting registration result message
        registration_result_msg, iscontrollerreply = self.gc.readControllerToTeam(self.socket)
        if self.gc.verification != 1:##NOT VERIFIED
            rospy.loginfo("error in registration message: " + registration_result_msg.controller_reply.reason)
            return
            #sleep(0.5)
            #self.register()

        rospy.loginfo("TEAM REGISTERED")
        self.registered = True
        if not self.isgoalieassigned:
            self.assigngoalie()


    def assigngoalie(self):
        self.isgoalieassigned = False
        ##send assigngoalie data
        assigngoalie_serialized = self.gc.teamSerializedAssignGoalie(self.socket, self.goalie_id, self.gc.token, self.privatekey)
        self.gc.sendSerializedMessage(self.socket, assigngoalie_serialized, False)
        ##getting assigngoalie result message
        assigngoalie_result_msg, iscontrollerreply = self.gc.readControllerToTeam(self.socket)
        if not self.gc.status:  ##NOT VERIFIED
            rospy.loginfo("error in goalie assignment message: " + assigngoalie_result_msg.controller_reply.reason)
            return

        rospy.loginfo("GOALIE ASSIGNED")
        self.isgoalieassigned = True


    def wmCallback(self, data):
        #type:(parsian_world_model)
        if self.registered and self.isgoalieassigned:
            return
        draws = parsian_draws()
        drawText1 = parsian_draw()
        drawText2 = parsian_draw()
        if not self.registered:
            drawText1.text = "TEAM NOT REGISTERED TO REFEREE"
            drawText1.primary.x = 0
            drawText1.primary.y = 4.3
            drawText1.size = 30
            drawText1.color.r = 1
            drawText1.color.g = 0
            drawText1.color.b = 0
            drawText1.type = drawText1.TEXT
            draws.draws.append(drawText1)
        if not self.isgoalieassigned:
            drawText2.text = "GOALIE NOT ASSIGNED TO REFEREE"
            drawText2.primary.x = 0
            drawText2.primary.y = 3.8
            drawText2.size = 30
            drawText2.color.r = 1
            drawText2.color.g = 0
            drawText2.color.b = 0
            drawText2.type = drawText2.TEXT
            draws.draws.append(drawText2)
        self.draw_pub.publish(draws)





if __name__ == '__main__':
    try:
        rospy.init_node('game_controller', anonymous=True)
        rospy.loginfo("game_controller is running")
        control = GameController()
        rospy.spin()
    except rospy.ROSInterruptException:
        pass
