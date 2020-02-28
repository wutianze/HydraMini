'''
@Author: Sauron Wu
@GitHub: wutianze
@Email: 1369130123qq@gmail.com
@Date: 2019-09-20 14:23:08
@LastEditors: Please set LastEditors
@LastEditTime: 2020-02-28 10:42:31
@Description: 
'''
import keras
#import tensorflow
import sys
import os
import h5py
import numpy as np
import glob
import random
import csv
import cv2
from sklearn.model_selection import train_test_split
from keras.models import Sequential
from keras.layers import Lambda, Conv2D, MaxPooling2D, Dropout, Dense, Flatten, Cropping2D,BatchNormalization
from keras.models import load_model, Model, Input
from keras.callbacks import ModelCheckpoint, EarlyStopping, TensorBoard, ReduceLROnPlateau
from keras.optimizers import Adam, SGD
from keras.utils import plot_model
import argparse

#os.environ["TF_CPP_MIN_LOG_LEVEL"] = "2"
np.random.seed(0)
IMAGE_SHAPE = [120,160,3]# will be set autonomously
global OUTPUT_NUM
OUTPUT_NUM = 2# need to be set by --output_num
global CUT_SIZE
CUT_SIZE = 40
global READ_PATH
READ_PATH ="./images"

# step1,load data
def load_data():
    with open(READ_PATH+"/train.csv") as f:
        files = list(csv.reader(f))
        single_image = cv2.imread(READ_PATH+'/'+files[0][0])
        IMAGE_SHAPE[0] = single_image.shape[0]-CUT_SIZE
        IMAGE_SHAPE[1] = single_image.shape[1]
        IMAGE_SHAPE[2] = single_image.shape[2]
    print("IMAGE_SHAPE:")
    print(IMAGE_SHAPE)
    print("ORIGINAL_LABEL_NUM:%d"%(len(files[0])-1))
    cut = int(len(files)*0.8)
    return files[0:cut], files[cut:]

# step2 Build Model
def build_model(keep_prob,model_path):
    #if os.path.exists(model_path+"/model.h5"):
    #    model = load_model(model_path+"/model.h5")
    #    return model
    print("start compile")
    model = Sequential()
    model.add(Conv2D(24, (5, 5), strides=(2, 2), activation="relu",input_shape=(IMAGE_SHAPE[0],IMAGE_SHAPE[1],IMAGE_SHAPE[2])))
    #model.add(BatchNormalization())
    model.add(Dropout(keep_prob))
    model.add(Conv2D(32, (5, 5), strides=(2, 2), activation="relu"))
    model.add(Dropout(keep_prob))
    model.add(Conv2D(64, (5, 5), strides=(2, 2), activation="relu"))
    model.add(Dropout(keep_prob))
    model.add(Conv2D(64, (3, 3), strides=(1,1), activation="relu"))
    model.add(Dropout(keep_prob))
    model.add(Conv2D(64, (3, 3), strides=(1,1), activation="relu"))    
    model.add(Dropout(keep_prob))
    model.add(Flatten())
    model.add(Dense(100, activation="relu"))
    model.add(Dropout(keep_prob))
    model.add(Dense(50, activation="relu"))
    model.add(Dropout(keep_prob))
    model.add(Dense(OUTPUT_NUM,activation="relu"))
    #model.add(Dense(OUTPUT_NUM,activation='softmax'))
    model.summary()
    plot_model(model, to_file="model.png",show_shapes=True)
    return model

# step3 Train Model
def train_model(model, learning_rate, nb_epoch, samples_per_epoch,
                batch_size, train_list, valid_list, model_path):
    if not os.path.exists(model_path+'/'):
        os.mkdir(model_path+'/')

    #checkpoint = ModelCheckpoint(model_path+'/model-{epoch:03d}.h5',
    checkpoint = ModelCheckpoint(model_path+'/model.h5',
                                 monitor='val_loss',
                                 verbose=0,
                                 save_best_only=True,
                                 mode='min')
    early_stop = EarlyStopping(monitor='val_loss', min_delta=.0005, patience=20,
                               verbose=1, mode='min')
    tensorboard = TensorBoard(log_dir='./logs', histogram_freq=0, batch_size=20, write_graph=True,write_grads=True,
                              write_images=True, embeddings_freq=0, embeddings_layer_names=None,
                              embeddings_metadata=None)
    reduce_lr = ReduceLROnPlateau(monitor='loss', factor=0.1, patience=10, verbose=0, mode='min', min_delta=1e-5,cooldown=3, min_lr=0)

    
    #model.compile(loss='categorical_crossentropy', optimizer=keras.optimizers.Adam(lr=learning_rate), metrics=['accuracy'])
    model.compile(optimizer=keras.optimizers.Adam(lr=learning_rate),loss='mean_squared_error') # for congression model
    
    model.fit_generator(batch_generator(train_list, batch_size),
                        steps_per_epoch=samples_per_epoch/batch_size,
                        epochs = nb_epoch,
                        max_queue_size=1,
                        validation_data=batch_generator(valid_list, batch_size),
                        validation_steps=len(valid_list)/batch_size,
                        callbacks=[tensorboard, checkpoint, early_stop, reduce_lr],
                        verbose=2)

# step4
def batch_generator(name_list, batch_size):
    while True:
        images = np.zeros([batch_size, IMAGE_SHAPE[0], IMAGE_SHAPE[1], IMAGE_SHAPE[2]])
        labels = np.zeros([batch_size, OUTPUT_NUM])
        #for index in np.random.permutation(X.shape[0]):
        i = 0
        for index in np.random.permutation(len(name_list)):
            # this line defines how images are processed, should be same as that in graph_input_fn
            #print(READ_PATH+'/'+name_list[index][0])
            images[i] = cv2.imread(READ_PATH+'/'+name_list[index][0])[CUT_SIZE:,:]/255.0-0.5
            if OUTPUT_NUM == 1:
                labels[i] = [(name_list[index][1]+1.)/2.]
            elif OUTPUT_NUM ==2:
                labels[i] = [(float(name_list[index][1])+1.)/2.,(float(name_list[index][2])+1.)/2.]
            #print(name_list[index][2])
            #print(labels[i])
            i += 1
            if i == batch_size:
                i = 0
                yield (images, labels)

def main(model_path,nb_epoch):

    print("Start loading data list from:"+READ_PATH)
    train_list, valid_list = load_data()
    total_number_img = len(train_list) + len(valid_list)
    print("total images number is:%d"%(total_number_img))
    print("Load Data list Finished")

    print('-'*30)
    print('parameters')
    print('-'*30)

    keep_prob = 0.1
    # learning_rate must be smaller than 0.0001
    learning_rate = 0.0001
    batch_size = 30
    samples_per_epoch = int(total_number_img / batch_size)

    print('keep_prob = ', keep_prob)
    print('learning_rate = ', learning_rate)
    print('nb_epoch = ', nb_epoch)
    print('samples_per_epoch = ', samples_per_epoch)
    print('batch_size = ', batch_size)
    print('-' * 30)


    # compile the model
    model = build_model(keep_prob,model_path)
    print("build finished")

    # train model
    train_model(model, learning_rate, nb_epoch, samples_per_epoch, batch_size, train_list, valid_list,model_path)
    print("Model Train Finished")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='prediction server')
    parser.add_argument('--model', type=str, help='model dir', default="./model")
    parser.add_argument('--read', type=str, help='image dir', default="./images")
    parser.add_argument('--output_num', type=int, help='the output num of the model not the original label num, now we support 1 or 2', default=2)
    parser.add_argument('--cut_head_size', type=int, help='top head of the image to cut', default=40)
    parser.add_argument('--epoch', type=int, help='the number of epoch', default=20)
    args = parser.parse_args()
    OUTPUT_NUM = args.output_num
    CUT_SIZE = args.cut_head_size
    READ_PATH = args.read
    print("OUTPUT_NUM:%d"%OUTPUT_NUM)
    print("CUT_SIZE:%d"%CUT_SIZE)
    main(args.model,args.epoch)
