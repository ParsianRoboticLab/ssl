#!/usr/bin/env python3.5
import matplotlib.pyplot as plt
from math import pi
from sklearn import linear_model
import time
import os
X_values = []
Y_values = []
r = linear_model.LinearRegression()
os.chdir('../../profiler_data')
ls = [x for x in os.listdir() if x.endswith('motion.profile')]
for item in ls:
    with open(item) as file:
        f = eval(file.read())
        for (dist, phi, khab) in f:
            for point in f[(dist, phi, khab)]['data']:
                X_values.append([point['remain_dist'], point['world_model'], phi, khab])
                Y_values.append(f[(dist, phi, khab)]['total_time'] - point['time'])
                #print(str([point['remain_dist'], point['world_model'], phi, khab] )+ " : "+str(f[(dist, phi, khab)]['total_time'] - point['time']))

r.fit(X_values, Y_values)
with open(str(int(round(time.time() / 10))) + "motion.csv", "w+") as out:
    dist_step = 4   # 40
    max_dist = 4
    vel_step = 4    # 40
    max_vel = 4
    ang_step = 8    # 72

    out.write(str(dist_step) + " " + str(max_dist) + " " + str(vel_step) + " " + str(max_vel) + " " + str(ang_step) + "\n")

    for i in range(0, dist_step):
        for j in range(0, vel_step):
            for k in range(0, ang_step):
                for l in range(0, ang_step):
                    #print(str([i*max_dist/dist_step, j*max_vel/vel_step, k*2*pi/ang_step, round(l*pi*2/ang_step,2)])
                    #      + " : " + str(r.predict([[i*max_dist/dist_step, j*max_vel/vel_step, k*2*pi/ang_step,
                    #                                l*2*pi/ang_step]])))
                    out.write(str(i) + " " + str(j) + " " + str(k) + " " + str(l) + " " +
                              str(r.predict([[i * max_dist / dist_step, j * max_vel / vel_step, k * 2 * pi / ang_step,
                                              l * 2 * pi / ang_step]])[0]) + "\n")
    out.close()
