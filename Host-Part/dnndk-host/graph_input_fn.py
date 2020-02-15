'''
@Author: sauron
@Date: 2019-10-29 15:33:55
@LastEditTime : 2020-01-07 11:59:29
@LastEditors  : Sauron Wu
@Description: In User Settings Edit
@FilePath: /pynq_car/Host-Part/dnndk-host/graph_input_fn.py
'''
import cv2
#from PIL import Image
import os
import numpy as np
import glob
CONV_INPUT = "conv2d_1_input"
calib_batch_size = 50
def calib_input(iter):
    training_data = glob.glob("../training_data_npz/*.npz")
    images = []
    with np.load(training_data[iter%len(training_data)]) as data:
        img_num_in_npz = len(data['train_imgs'])
        for index in range(0, calib_batch_size):
            images.append(data['train_imgs'][index%img_num_in_npz])
    return {CONV_INPUT: images}

