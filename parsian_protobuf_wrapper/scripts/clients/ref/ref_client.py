import socket
import struct
import sys
import parsian_protobuf_wrapper.ssl_referee_pb2

multicast_group = '224.5.23.1'
server_address = ('', 10003)

# Create the socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind to the server address
sock.bind(server_address)

# Tell the operating system to add the socket to the multicast group
# on all interfaces.
group = socket.inet_aton(multicast_group)
mreq = struct.pack('4sL', group, socket.INADDR_ANY)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

def CommandName(c):
    return referee_pb2._SSL_REFEREE_COMMAND.values_by_number[c.command].name

def StageName(c):
    return referee_pb2._SSL_REFEREE_STAGE.values_by_number[c.stage].name

# Receive/respond loop
ref_msg = referee_pb2.SSL_Referee()
while True:
    data, address = sock.recvfrom(1024)
    ref_msg.ParseFromString(data)
    print('%d: %s' % (ref_msg.packet_timestamp,
                      ref_msg.command))
    print(CommandName(ref_msg) + " " + StageName(ref_msg))
    print(ref_msg.yellow.name + " (yellow) vs. " + ref_msg.blue.name + " (blue)")

