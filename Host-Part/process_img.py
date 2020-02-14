'''
@Author: Sauron Wu
@GitHub: wutianze
@Email: 1369130123qq@gmail.com
@Date: 2019-09-20 14:23:08
@LastEditors  : Sauron Wu
@LastEditTime : 2020-01-17 15:09:01
@Description: 
'''
import os
import numpy as np
from time import time
import math
import csv
import argparse
import random
import cv2
#from PIL import Image

CHUNK_SIZE = 256
IMAGE_SHAPE = [120,160,3]
global CUT_SIZE
CUT_SIZE = 40
global OUTPUT_NUM
OUTPUT_NUM = 1# will be set autonomously, the label value may not be all used when training the model, so the number of OUTPUT_NUM here is more than OUTPUT_NUM in train.py
def image_handle(img):
    #print("img:")
    #print(img)
    #image = np.asarray(img)
    #img.reshape((img.shape[0],img.shape[1],img.shape[2]))
    return (img[CUT_SIZE:,:])/255.0-0.5

def process_img(img_path, key):
    image_array = cv2.imread(img_path)
    label_array = []
    for k in key:
        label_array.append(float(k)) 

    image_array = image_handle(image_array)
    image_array = np.expand_dims(image_array, axis=0)
    return (image_array, label_array)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='prediction server')
    parser.add_argument('--path', type=str, help='images dir', default="./images")
    parser.add_argument('--store', type=str, help='npz store dir', default="./training_data_npz")
    parser.add_argument('--method', type=int, help='whether to reduce some categories\' number, 0 for true', default=1)
    parser.add_argument('--filter_size', type=int, help='size of smallest file', default=1000)
    parser.add_argument('--cut_head_size', type=int, help='top head of the image to cut', default=40)
    
    args = parser.parse_args()
    CUT_SIZE = args.cut_head_size
    print("CUT_SIZE is:%d"%CUT_SIZE)    
    path = args.path
    names = []
    keys = {}
    with open(path+"/train.csv") as f:
        files = list(csv.reader(f))
        for row in files:
            if (not(os.path.exists(path+'/'+row[0]))) or os.path.getsize(path+'/'+row[0]) < args.filter_size:
                continue
            
            if args.method == 0:
                if float(row[1]) == 0: # this should be set according to your training data
                    randNum = random.randint(0,10)
                    # delete some data randomly, bigger of the threshold number means more data in this category will be ignored
                    if randNum < 5:
                        continue
            names.append(row[0])
            tmp = []
            for i in range(1,len(row)):
                tmp.append(float(row[i]))
            keys[row[0]] = tmp
    image_for_shape = cv2.imread(path+'/'+names[0])
    IMAGE_SHAPE[0] = image_for_shape.shape[0]-CUT_SIZE
    IMAGE_SHAPE[1] = image_for_shape.shape[1]
    IMAGE_SHAPE[2] = image_for_shape.shape[2]
    OUTPUT_NUM = len(files[0]) - 1
    print("LABEL NUM is:%d"%OUTPUT_NUM)    
    turns = int(math.ceil(len(names) / CHUNK_SIZE))      
    print("number of files: {}".format(len(names)))
    print("turns: {}".format(turns))

    for turn in range(0, turns):
        train_labels = np.zeros((1, OUTPUT_NUM), 'int')           # initialize labels
        train_imgs = np.zeros([1, IMAGE_SHAPE[0],IMAGE_SHAPE[1],IMAGE_SHAPE[2]])            # initialize image array

        CHUNK_files = names[turn * CHUNK_SIZE: (turn + 1) * CHUNK_SIZE] # get one chunk
        print("number of CHUNK files: {}".format(len(CHUNK_files)))
        print("Turn Now:%d"%turn)
        for file in CHUNK_files:
            if not os.path.isdir(file) and file[len(file) - 3:len(file)] == 'jpg':
                key = keys[file]
                image_array, label_array = process_img(path + "/" + file,key)
                train_imgs = np.vstack((train_imgs, image_array))
                train_labels = np.vstack((train_labels, label_array))

        # delete the initial all-0 array
        train_imgs = train_imgs[1:, :]
        train_labels = train_labels[1:, :]
        file_name = str(int(time()))
        directory = args.store

        if not os.path.exists(directory):
            os.makedirs(directory)
        try:
            np.savez(directory + '/' + file_name + '.npz', train_imgs=train_imgs, train_labels=train_labels)
        except IOError as e:
            print(e)

