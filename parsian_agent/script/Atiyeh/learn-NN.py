import tensorflow as tf
import numpy as np
#import matplotlib.pyplot as plt


######################## read data
matrix_in = np.loadtxt("in_new.txt")
matrix_out = np.loadtxt("out_new.txt")

######################## defining varialbles
v_in = tf.placeholder(tf.float32,[None, 5])
v_out = tf.placeholder(tf.float32, [None, 3])

######################## network layers
l1 = tf.layers.dense(v_in, 300, tf.nn.sigmoid) # check out
l2 = tf.layers.dense(l1, 100, tf.nn.sigmoid)
v_pred = tf.layers.dense(l2, 3) 
loss = tf.losses.mean_squared_error(v_out, v_pred)
optimizer = tf.train.GradientDescentOptimizer(learning_rate = 0.005)
train_op = optimizer.minimize(loss)

######################## session
sess = tf.Session()
sess.run(tf.global_variables_initializer())

for step in range(60000):
    #for i in range(5):
    _, l ,pred = sess.run([train_op, loss, v_pred],{v_in: matrix_in, v_out: matrix_out})
    if step % 10 == 0:
        print(str(l) + "  " + str(step) + str(pred)  + "\n")

#test = sess.run(w_pred, {v: matrix, w: 
#    np.transpose([-9.87921142578, 9.87921333313, -1.54955852032, 1.54955530167])})
#####print(str(test)