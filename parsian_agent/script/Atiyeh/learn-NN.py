import tensorflow as tf
import numpy as np
import matplotlib.pyplot as plt

######################## read data
matrix_out = np.loadtxt("outs.txt")
matrix_in = np.loadtxt("ins.txt")

######################## defining varialbles
v_in = tf.placeholder(tf.float32,[None, 3])
v_out = tf.placeholder(tf.float32, [None, 1])
vn_real , vf_real , vw_real = np.zeros((len(matrix_out[:,1]),1)) ,np.zeros((len(matrix_out[:,1]),1)),np.zeros((len(matrix_out[:,1]),1))

vn_real[:,0] =  (- matrix_in[:,0] * matrix_in[:,4] + matrix_in[:,1] * matrix_in[:,3]) * 0.5
vf_real[:,0] = (matrix_in[:,0] * matrix_in[:,3] + matrix_in[:,1] * matrix_in[:,4]) * 0.5
vw_real[:,0] = matrix_in[:,2]
v_real = np.concatenate((vf_real , vn_real , vw_real), axis = 1)
np.savetxt('velsReal.txt' , v_real , fmt = '%3.8f', delimiter= ' ')

######################## network layers
l1 = tf.layers.dense(v_in, 10, tf.nn.sigmoid) # check out
l2 = tf.layers.dense(l1, 10, tf.nn.sigmoid)
v_pred = tf.layers.dense(l2, 1) 
loss = tf.losses.mean_squared_error(v_out, v_pred)
optimizer = tf.train.GradientDescentOptimizer(learning_rate = 0.1)
train_op = optimizer.minimize(loss)

######################## session : train
sess = tf.Session()
sess.run(tf.global_variables_initializer())

for step in range(300):
    #for i in range(5):
    _, l ,pred = sess.run([train_op, loss, v_pred],{v_in: v_real , v_out: matrix_out[:,0:1]})
    if step % 10 == 0:
        print(str(l) + "  " + str(step) + str(pred)  + "\n")
        
np.savetxt('pred.txt' , pred , fmt = '%3.8f', delimiter= ' ')
pred_ = np.loadtxt("pred.txt")
plt.plot(pred_[0:1000])
plt.plot(matrix_out[0:1000,0])
plt.show()

######################## session : test
v_test = np.loadtxt("vTest.txt")
pred_test = sess.run(v_pred,{v_in : v_test})
np.savetxt('pred.txt' , pred_test , fmt = '%3.8f', delimiter= ' ')
pred_test = np.loadtxt("pred.txt")
plt.plot(pred_test[0:1000])
plt.plot(v_test[0:1000,0])
plt.show()