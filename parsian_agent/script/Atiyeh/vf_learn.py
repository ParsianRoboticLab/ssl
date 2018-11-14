import tensorflow as tf
import numpy as np
import matplotlib.pyplot as plt

######################## read data
matrix_out = np.loadtxt("outs.txt")
matrix_in = np.loadtxt("ins.txt")

######################## defining varialbles
BATCH_SIZE = 256
v_in = tf.placeholder(tf.float32,[None, 3])
v_out = tf.placeholder(tf.float32, [None, 1])
vn_real = np.zeros((len(matrix_out[:,1]),1))
vf_real = np.zeros((len(matrix_out[:,1]),1))
vw_real = np.zeros((len(matrix_out[:,1]),1))
vn_real[:,0] =  (- matrix_in[:,0] * matrix_in[:,4] + matrix_in[:,1] * matrix_in[:,3]) * 0.5
vf_real[:,0] = (matrix_in[:,0] * matrix_in[:,3] + matrix_in[:,1] * matrix_in[:,4]) * 0.5
vw_real[:,0] = matrix_in[:,2]
v_real = np.concatenate((vf_real , vn_real , vw_real), axis = 1)
np.savetxt('velsReal.txt' , v_real , fmt = '%3.8f', delimiter= ' ')
vf_pub = matrix_out[:,0:1]
######################## network layers
l1 = tf.layers.dense(v_in, 8, tf.nn.sigmoid) # check out
l2 = tf.layers.dense(l1, 8, tf.nn.sigmoid)
v_pred = tf.layers.dense(l2, 1) 
loss = tf.losses.mean_squared_error(v_out, v_pred)
optimizer = tf.train.GradientDescentOptimizer(learning_rate = 0.05)
train_op = optimizer.minimize(loss)

######################## session : train
sess = tf.Session()
sess.run(tf.global_variables_initializer())
for step in range(30000):
    index = np.random.randint(0, vf_real.shape[0], BATCH_SIZE)
    _, l ,pred = sess.run([train_op, loss, v_pred],{v_in: v_real[index,:] , v_out: vf_pub[index,:]})
    if step % 10 == 0:
        print(str(l) + "  " + str(step)  + "\n")
        
######################## session : test
v_test = np.loadtxt("vTest.txt")
pred_test = sess.run(v_pred,{v_in : v_test})
np.savetxt('pred_vf.txt' , pred_test , fmt = '%3.8f', delimiter= ' ')
pred_test = np.loadtxt("pred_vf.txt")
plt.plot(pred_test[0:4000])
plt.plot(v_test[0:4000,0])
plt.show()

######################## session : test 2 
v_test = v_real
pred_test = sess.run(v_pred,{v_in : v_test})
np.savetxt('pred.txt' , pred_test , fmt = '%3.8f', delimiter= ' ')
pred_test = np.loadtxt("pred.txt")
plt.plot(vf_pub[5500:8500])
plt.plot(pred_test[5500:8500])
plt.show()