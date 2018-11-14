import matplotlib.pyplot as plt
import numpy as np




matrix_out = np.loadtxt("outs.txt")
matrix_in = np.loadtxt("ins.txt")
vn_real =  (- matrix_in[:,0] * matrix_in[:,4] + matrix_in[:,1] * matrix_in[:,3]) * 0.5
vf_real = (matrix_in[:,0] * matrix_in[:,3] + matrix_in[:,1] * matrix_in[:,4]) * 0.5
vn = matrix_out[:,1]
lin = np.zeros(len(vn))
plt.plot(vn[0:4000])
plt.plot(vn_real[4000:7000])
plt.plot(lin[4000:7000])
plt.plot(vf_real[4000:7000])
plt.show()

v_real = np.loadtxt("velsReal.txt")
plt.plot(matrix_out[0:1000,0])
plt.plot(v_real[0:1000,0])
#plt.plot(v_real[7000:,0])
plt.show()

############################## create test data:
vn_test = np.zeros((4000,1))
for i in range(0,1000):
    vn_test[i,0] = 0.0012 * i
    
for i in range(0,2000):
    vn_test[i+1000,0] = 1.2
    
for i in range(0,1000):
    vn_test[i+3000,0] = 0.0012 * (1000-i)
    
vf_test , vw_test = np.zeros((4000,1)) , np.zeros((4000,1)) 
v_test = np.concatenate((vf_test , vn_test , vw_test), axis = 1)
plt.plot(vn_test)
plt.show()

np.savetxt('vTest.txt' , v_test , fmt = '%3.8f', delimiter= ' ')
##############################
vn_test = np.zeros((4000,1))
vf_test = np.zeros((4000,1))
vw_test = np.zeros((4000,1))
vf_test[:,0] = np.loadtxt("pred_vf.txt")
vn_test[:,0] = np.loadtxt("pred_vn.txt")
vw_test[:,0] = np.loadtxt("pred_vw.txt")
v_real = np.concatenate((vf_test , vn_test , vw_test), axis = 1)
np.savetxt('velsTEST.txt' , v_real , fmt = '%3.8f', delimiter= ' ')
