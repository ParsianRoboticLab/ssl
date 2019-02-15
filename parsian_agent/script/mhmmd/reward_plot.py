import matplotlib.pyplot as plt
import numpy as np

f = open('/home/parsian-ai/data/reward1.txt', 'r')
f2 = open('/home/parsian-ai/data/save_percent1.txt', 'r')
rew_file = f.read()
percentL = f2.read()
plt.figure(figsize=[10,7])
reward = list(map(float, rew_file[1:-1].split(',')))
reward = reward[3:203]
# reward = [r for r in reward if r != 0]
percent = list(map(float, percentL[1:-1].split(',')))
percent = percent[3:203]
percent = [r*100 for r in percent]
average = [reward[0]]

for i in range(len(reward)):
    average.append(0.01 * reward[i] + 0.99 * average[-1])
plt.figure(1)

pltt = plt.plot(percent)
plt.setp(pltt, linewidth=3)
print(percent[-1])
plt.xticks(fontsize = 16)
plt.yticks(fontsize = 16)
plt.ylabel('Success Rate (%)',fontsize = 16)
plt.xlabel('Number of Trials',fontsize = 16)
plt.figure(figsize=[10,7])

plt.figure(2)


plt.ylabel('Reward Value',fontsize = 16)
plt.xlabel('Number of Epochs',fontsize = 16)
pltt = plt.plot(reward, label='Reward')
from matplotlib.font_manager import FontProperties
fontP = FontProperties()
fontP.set_size(16)

plt.setp(pltt, linewidth=3)

pltt = plt.plot(average,label='Average Reward')
# plt.legend()
plt.setp(pltt, linewidth=4,color = 'r')
plt.xticks(fontsize = 16)
plt.yticks(fontsize = 16)
plt.legend(loc=1, prop = fontP)
print(average[0])
print(average[-1])
# plt.yticks(range(-20,20))
plt.show()