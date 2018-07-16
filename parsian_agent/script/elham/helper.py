import rospy
from std_msgs.msg import String

from parsian_msgs.msg import parsian_world_model, parsian_skill_gotoPointAvoid, parsian_robot
from parsian_msgs.msg import parsian_robot_task
from parsian_msgs.msg import parsian_skill_gotoPoint
from parsian_msgs.msg import vector2D
import random
import math

wm = parsian_world_model()
d = vector2D(1 , -1)
newResult = 0
t = 0
sumResult = 0
a = []
lb = []
g = [0 , 0 , 0]
num = 0
lbt = []
sumTime = 10000000
gt = 10000000
turn = 0

def GTPA(pub):
    global wm , d
    if not wm.our:
        return 0
    for robot in wm.our:
        if robot.id == 0:
            r = robot
    task = parsian_robot_task()
    task.select = parsian_robot_task.GOTOPOINTAVOID
    setTask = parsian_skill_gotoPointAvoid()  # type: parsian_skill_gotoPointAvoid
    setTask.diveMode = False
    setTask.base.targetPos = r.pos
    setTask.base.lookAt = vector2D(5000 , 5000)
    setTask.base.targetDir = d
    task.gotoPointAvoidTask = setTask
    pub.publish(task)

def NA(pub):
    task = parsian_robot_task()
    task.select = parsian_robot_task.NOTASK
    pub.publish(task)

def wmCallback(data):
    global wm
    wm = data

def done():
    global wm , d
    if not wm.our:
        return 0
    for robot in wm.our:
        if robot.id == 0:
            r = robot
    x = r.dir.x * (2 / (r.dir.x ** 2 + r.dir.y ** 2)) ** (1/2.0)
    y = r.dir.y * (2 / (r.dir.x ** 2 + r.dir.y ** 2)) ** (1/2.0)
    if abs(x - d.x) < 0.2 and abs(y - d.y) < 0.2 and abs(r.vel.x) + abs(r.vel.y) + abs(r.angularVel) < 0.01:
        return 1
    return 0

def initPSO():
    global a , num , lb , lbt , g
    for x in range(0 , 10):
        for y in range(-10 , 10):
            for z in range(-10 , 10):
                a[num] = [x , y ,z]
                lb[num] = [x , y , z]
                lbt[num] = 100000000
                num += 1
    g = [0 , 0 , 0] #midonam eshtebahe vali dorostesh sakhte:)

def go(pub , n):
    global lb , a , g , d , t , sumTime
    t += 1
    if done():
        PID = open("PID.txt", "w")
        NA(pub)
        for x in range(0 , 3):
            a[n][x] = a[n][x] + random.uniform(0 , 1) * (lb[n][x] - a[n][x]) + random.uniform(0 , 1) * (g[x] - a[n][x])
            PID.write(str(a[n][x]))
            PID.write(str(' '))
        if d == vector2D(1, 1):
            sumTime = t
            t = 0
            d = vector2D(1, -1)
        elif d == vector2D(1, -1):
            d = vector2D(-1, -1)
        elif d == vector2D(-1, -1):
            d = vector2D(-1, 1)
        elif d == vector2D(-1, 1):
            d = vector2D(1, 1)
        PID.close()
        return;
    GTPA(pub)


def PSO(pub):
    global sumTime , lbt , lb , a , gt , g , num , turn
    go(pub , turn)
    if t == 0:
        if sumTime < lbt[turn]:
            lbt[turn] = sumTime
            lb[turn] = a[turn]
        if sumTime < gt:
            g = a[turn]
            gt = sumTime
        turn += 1
        turn %= num

if __name__ == "__main__":
    if 1:
        print("Right!")
    else:
        print("Helper not working")