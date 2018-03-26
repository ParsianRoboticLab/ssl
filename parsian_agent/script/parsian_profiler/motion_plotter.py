#!/usr/bin/env python3.5
import matplotlib.pyplot as plt
from math import pi
import os

command = []
world_model = []
remain_dist = []
time = []
os.chdir('../../profiler_data')
ls = [x for x in os.listdir() if x.endswith('motion.profile')]
for item in ls:
    with open(item) as file:
        f = eval(file.read())
        for (dist, phi, khab) in f:
            for point in f[(dist, phi, khab)]['data']:
                command.append(point['robot_command'])
                world_model.append(point['world_model'] * 1.364894903)
                remain_dist.append(point['remain_dist'])
                time.append(point['time'])
            plt.figure()
            plt.subplot(1, 1, 1)
            plt.xlabel(r'time')
            plt.title(str(round(phi / pi * 180)) + "  " + str(round(180 * khab / pi, 2)))
            wm = plt.plot(time, world_model, 'ro', color='blue', label="world_model vel")
            #plt.plot(time, world_model, 'k--')
            com = plt.plot(time, command, 'ro', color='red', label="command vel")
            #plt.plot(time, command, 'k--')
            rem = plt.plot(time, remain_dist, 'ro', color='green', label="remain dist")
            #plt.plot(time, remain_dist, 'k--')

            plt.legend(bbox_to_anchor=(1, 1),
                       bbox_transform=plt.gcf().transFigure)
            plt.grid(True)
            plt.savefig(str((round(dist, 2), round(180 * phi / pi, 2), round(180 * khab / pi, 2))) + '.png')
            plt.clf()
            time.clear()
            remain_dist.clear()
            command.clear()
            world_model.clear()

