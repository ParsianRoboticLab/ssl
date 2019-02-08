#!/usr/bin/env python
# license removed for brevity

import rospy
import socket
from time import sleep
from GameControllerCommon import GameControllerCommon
from dynamic_reconfigure.server import Server
from parsian_protobuf_wrapper.cfg import refereeConfig

TEAM_NAME = 'Test Team'


class GameController():
    def __init__(self):
        ##variables
        self.registered = False
        self.isgoalieassigned = False
        self.goalie_id = 0
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
        self.srv = Server(refereeConfig, self.cfg_callback)

        # while True:
        #    result = s.recv(131072)
        #    result_msg = ssl_game_controller_team_pb2.ControllerToTeam()
        #    result_msg.ParseFromString(initiate_result[1:])
        #    print(result_msg)



    def cfg_callback(self, config, level):
        self.IP = config.refree_listen_ip
        self.PORT = config.refree_listen_port
        self.IP = '127.0.0.1' #TODO delete this
        #register
        self.register()
        self.assigngoalie()

        return config



    def register(self):
        try:
            self.socket.connect((self.IP, self.PORT))
        except:
            rospy.loginfo('cannot bind to the gamecontroller')
            sleep(0.5)
            self.register()
        ##getting initiate response
        initiate_result_msg = self.gc.readControllerToTeam(self.socket)
        ##send registration data
        registration_serialized = self.gc.teamSerializedRegistration(self.socket, TEAM_NAME, self.gc.token, self.privatekey)
        self.gc.sendSerializedMessage(self.socket, registration_serialized)
        ##getting registration result message
        registration_result_msg = self.gc.readControllerToTeam(self.socket)
        while self.gc.verification != 1:##NOT VERIFIED
            rospy.loginfo("error in registration message: " + registration_result_msg.controller_reply.reason)
            sleep(0.5)
            registration_serialized = self.gc.teamSerializedRegistration(self.socket, TEAM_NAME, self.gc.token, self.privatekey)
            self.gc.sendSerializedMessage(self.socket, registration_serialized)
            ##getting registration result message
            registration_result_msg = self.gc.readControllerToTeam(self.socket)

        rospy.loginfo("TEAM REGISTERED")
        self.registered = True


    def assigngoalie(self):
        ##send assigngoalie data
        assigngoalie_serialized = self.gc.teamSerializedAssignGoalie(self.socket, self.goalie_id, self.gc.token, self.privatekey)
        self.gc.sendSerializedMessage(self.socket, assigngoalie_serialized)
        ##getting assigngoalie result message
        assigngoalie_result_msg = self.gc.readControllerToTeam(self.socket)
        while self.gc.verification != 1:  ##NOT VERIFIED
            rospy.loginfo("error in goalie assignment message: " + assigngoalie_result_msg.controller_reply.reason)
            sleep(0.5)
            assigngoalie_serialized = self.gc.teamSerializedAssignGoalie(self.socket, self.goalie_id, self.gc.token,
                                                                    self.privatekey)
            self.gc.sendSerializedMessage(self.socket, assigngoalie_serialized)
            ##getting registration result message
            assigngoalie_result_msg = self.gc.readControllerToTeam(self.socket)

        rospy.loginfo("GOALIE ASSIGNED")
        self.isgoalieassigned = True



if __name__ == '__main__':
    try:
        rospy.init_node('game_controller', anonymous=True)
        rospy.loginfo("game_controller is running")
        control = GameController()
        rospy.spin()
    except rospy.ROSInterruptException:
        pass