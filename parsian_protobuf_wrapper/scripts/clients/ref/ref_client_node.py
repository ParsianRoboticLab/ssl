#!/usr/bin/python
import rospy
from dynamic_reconfigure import server
from parsian_msgs.msg import parsian_world_model
from parsian_msgs.msg import parsian_draws
from parsian_msgs.msg import parsian_draw_buffer
from rqt_parsian_gui.cfg import drawbufferConfig
import socket
import struct
import sys
import parsian_protobuf_wrapper.ssl_referee_pb2 as ref_proto
import parsian_protobuf_wrapper.cfg.refereeConfig as cfg
import clients.ref.ref_client


def CommandName(c):
    return referee_pb2._SSL_REFEREE_COMMAND.values_by_number[c.command].name

def StageName(c):
    return referee_pb2._SSL_REFEREE_STAGE.values_by_number[c.stage].name


if __name__ == "__main__":
    rospy.init_node('game_ref_node', anonymous=True)

    # multicast_group = str(cfg.extract_params('refree_multicast_ip'))
    # server_address = ('', int(cfg.extract_params('refree_multicast_port')))
    multicast_group = '224.5.23.2'
    server_address = '10003'

    # Create the socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # Bind to the server address
    sock.bind(server_address)

    # Tell the operating system to add the socket to the multicast group
    # on all interfaces.
    group = socket.inet_aton(multicast_group)
    mreq = struct.pack('4sL', group, socket.INADDR_ANY)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    ref_msg = ref_proto.Referee()
    while rospy.is_shutdown():
        data, address = sock.recvfrom(1024)
        ref_msg.ParseFromString(data)
        print('%d: %s' % (ref_msg.packet_timestamp,
                          ref_msg.command))
        print(CommandName(ref_msg) + " " + StageName(ref_msg))
        print(ref_msg.yellow.name + " (yellow) vs. " + ref_msg.blue.name + " (blue)")
