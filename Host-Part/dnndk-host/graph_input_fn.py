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
def image_handle(img):
    return (img[40:,:])/255.0-0.5
CONV_INPUT = "conv2d_1_input"
calib_batch_size = 50
def calib_input(iter):
  images = []
  path = "../images/"
  files = os.listdir(path)
  for index in range(0, calib_batch_size):
      if files[iter*calib_batch_size+index] == "train.csv" or (not os.path.exists(path+files[iter*calib_batch_size + index]) or os.path.getsize(path + files[iter*calib_batch_size + index])<1000):# it will ignore the images which are too small
          continue
      image = image_handle(cv2.imread(path + files[iter*calib_batch_size + index]))
      images.append(image)
  return {CONV_INPUT: images}

