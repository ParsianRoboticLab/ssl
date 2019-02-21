import os
import socket
import rsa
import rospy
from time import sleep
from parsian_protobuf_wrapper import ssl_game_controller_team_pb2
#import ssl_game_controller_team_pb2
import varint


class GameControllerCommon:
    def __init__(self):
        self.token = ''
        self.verification = False
        self.status = False

    def readPrivateKey(self, filename):
        if not os.path.isfile(filename):
            return False, 'empty'
        else:
            privatefile = open(filename)
            keydata = privatefile.read()
            privatekey = rsa.PrivateKey.load_pkcs1(keydata, 'PEM')
            return True, privatekey

    def readControllerToTeam(self, socket):
        # type:(socket.socket) ->(object, object)
        result = socket.recv(131072)
        result_msg = ssl_game_controller_team_pb2.ControllerToTeam()
        result_msg.ParseFromString(result[1:])
        ##ControllerReply
        isControllerReply = False
        if result_msg.HasField("controller_reply"):
            isControllerReply = True
            verification_tmp = -1
            if result_msg.controller_reply.HasField("verification"):
                verification_tmp = result_msg.controller_reply.verification
            if verification_tmp == 1:
                self.verification = True
            else:
                self.verification = False
            if result_msg.controller_reply.HasField("next_token"):
                self.token = result_msg.controller_reply.next_token
            else:
                rospy.loginfo("NO TOKEN RECEIVED")
            status_tmp = -1
            if result_msg.controller_reply.HasField("status_code"):
                status_tmp = result_msg.controller_reply.status_code
            if status_tmp == 1:
                self.status = True
            else:
                self.status = False
        ##AdvantageChoice = Do Nothing just wait for ControllerReply
        else:
            self.readControllerToTeam(socket)

        #rospy.loginfo(result_msg)
        return (result_msg, isControllerReply)

    def teamSerializedRegistration(self, socket, teamname, token, privatekey):
        # type:(socket.socket, str, str, rsa.privatekey) ->object
        registration = ssl_game_controller_team_pb2.TeamRegistration()
        registration.team_name = teamname
        registration.signature.token = token
        registration.signature.pkcs1v15 = bytes()
        ##creating signature
        serialized = registration.SerializeToString()
        hash = rsa.compute_hash(serialized, 'SHA-256')
        signature = rsa.sign_hash(hash, privatekey, 'SHA-256')
        registration.signature.pkcs1v15 = signature
        ##sending registration
        serializedmessage = registration.SerializeToString()
        return serializedmessage

    def teamSerializedAssignGoalie(self, socket, goalie_id, token, privatekey):
        # type:(socket.socket, int, str, rsa.privatekey) ->object
        assigngoalie = ssl_game_controller_team_pb2.TeamToController()
        assigngoalie.desired_keeper = goalie_id
        assigngoalie.signature.token = token
        assigngoalie.signature.pkcs1v15 = bytes()
        ##creating signature
        serialized = assigngoalie.SerializeToString()
        hash = rsa.compute_hash(serialized, 'SHA-256')
        signature = rsa.sign_hash(hash, privatekey, 'SHA-256')
        assigngoalie.signature.pkcs1v15 = signature
        ##sending registration
        serializedmessage = assigngoalie.SerializeToString()
        return serializedmessage

    def teamSerializedsubstitute(self, socket, token, privatekey):
        # type:(socket.socket, str, rsa.privatekey) ->object
        substitute = ssl_game_controller_team_pb2.TeamToController()
        substitute.substitute_bot = True
        substitute.signature.token = token
        substitute.signature.pkcs1v15 = bytes()
        ##creating signature
        serialized = substitute.SerializeToString()
        hash = rsa.compute_hash(serialized, 'SHA-256')
        signature = rsa.sign_hash(hash, privatekey, 'SHA-256')
        substitute.signature.pkcs1v15 = signature
        ##sending registration
        serializedmessage = substitute.SerializeToString()
        return serializedmessage

    def sendSerializedMessage(self, socket, serializedmsg, repeat):
        # type:(socket.socket, str, bool) ->object
        size = len(serializedmsg)
        data = varint.encode(size) + serializedmsg
        try:
            socket.send(data)
            return True
        except:
            print("cannot send data to the gamecontroller")
            if repeat:
                sleep(0.5)
                self.sendSerializedMessage(socket, serializedmsg, repeat)