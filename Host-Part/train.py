'''
@Author: Sauron Wu
@GitHub: wutianze
@Email: 1369130123qq@gmail.com
@Date: 2019-09-20 14:23:08
@LastEditors  : Please set LastEditors
@LastEditTime : 2020-01-19 10:57:07
@Description: 
'''
import keras
import tensorflow
import sys
import os
import h5py
import numpy as np
import glob
import random
from sklearn.model_selection import train_test_split
from keras.models import Sequential
from keras.layers import Lambda, Conv2D, MaxPooling2D, Dropout, Dense, Flatten, Cropping2D,BatchNormalization
from keras.models import load_model, Model, Input
from keras.callbacks import ModelCheckpoint, EarlyStopping, TensorBoard, ReduceLROnPlateau
from keras.optimizers import Adam, SGD
from keras.utils import plot_model
import argparse

np.random.seed(0)
IMAGE_SHAPE = [120,160,3]# will be set autonomously
global OUTPUT_NUM
OUTPUT_NUM = 2# need to be set by --output_num
CHUNK_SIZE = 256
ORIGINAL_LABEL_NUM = 1# will be set autonomously
# step1,load data
def load_data(read_path):
    training_data = glob.glob(read_path + '/*.npz')
    # match all the files return as list
    print("Match Completed")
    print("Total: %d Round"%len(training_data))

    # if no data, exit
    if not training_data:
        print("No training data in directory, exit")
        sys.exit()
    single_npz = training_data[0]
    with np.load(single_npz) as data:
        single_image = data['train_imgs'][0]
        IMAGE_SHAPE[0] = single_image.shape[0]
        IMAGE_SHAPE[1] = single_image.shape[1]
        IMAGE_SHAPE[2] = single_image.shape[2]
        global ORIGINAL_LABEL_NUM
        ORIGINAL_LABEL_NUM = data['train_labels'][0].shape[0]
    print("IMAGE_SHAPE:")
    print(IMAGE_SHAPE)
    print("ORIGINAL_LABEL_NUM:")
    print(ORIGINAL_LABEL_NUM)
    cut = int(len(training_data)*0.8)
    return training_data[0:cut], training_data[cut:]

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
                        validation_steps=(len(valid_list)*CHUNK_SIZE)/batch_size,
                        callbacks=[tensorboard, checkpoint, early_stop, reduce_lr],
                        verbose=2)

# step4
def batch_generator(name_list, batch_size):
    i = 0
    while True:
        # load
        image_array = np.zeros((1, IMAGE_SHAPE[0], IMAGE_SHAPE[1], IMAGE_SHAPE[2]),'float')
        label_array = np.zeros((1, ORIGINAL_LABEL_NUM), 'float')
        # every time read <=10 pack and shuffle
        for read_num in range(10):
            single_npz = name_list[random.randint(0,len(name_list)-1)]
        
            with np.load(single_npz) as data:
            #print(data.keys())
                train_temp = data['train_imgs']
                train_labels_temp = data['train_labels']
                image_array = np.vstack((image_array, train_temp)) 
                label_array = np.vstack((label_array, train_labels_temp))
        X = image_array[1:, :]
        y = label_array[1:, :]

        images = np.zeros([batch_size, IMAGE_SHAPE[0], IMAGE_SHAPE[1], IMAGE_SHAPE[2]])
        labels = np.zeros([batch_size, OUTPUT_NUM])
        #for index in np.random.permutation(X.shape[0]):
        for index in np.random.permutation(X.shape[0]):
            images[i] = X[index]
            #if y[index][0] < 0:
            #    labels[i] = [1.,0.,0.]
            #elif y[index][0] > 0:
            #    labels[i] = [0.,0.,1.]
            #else:
            #    labels[i] = [0.,1.,0.]
            if OUTPUT_NUM == 1:
                labels[i] = [(y[index][0]+1.)/2.]
            elif OUTPUT_NUM ==2:
                labels[i] = [(y[index][0]+1.)/2.,(y[index][1]+1.)/2.]
            #print(labels[i])
            i += 1
            if i == batch_size:
                i = 0
                yield (images, labels)

def main(model_path, read_path):

    print('-'*30)
    print('parameters')
    print('-'*30)

    keep_prob = 0.1
    # learning_rate must be smaller than 0.0001
    learning_rate = 0.0001
    nb_epoch = 100
    samples_per_epoch = 3000
    batch_size = 30

    print('keep_prob = ', keep_prob)
    print('learning_rate = ', learning_rate)
    print('nb_epoch = ', nb_epoch)
    print('samples_per_epoch = ', samples_per_epoch)
    print('batch_size = ', batch_size)
    print('-' * 30)

    # Start loading data
    train_list, valid_list = load_data(read_path)
    print("Load Data Finished")

    # compile the model
    model = build_model(keep_prob,model_path)
    print("build finished")

    # train model
    train_model(model, learning_rate, nb_epoch, samples_per_epoch, batch_size, train_list, valid_list,model_path)
    print("Model Train Finished")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='prediction server')
    parser.add_argument('--model', type=str, help='model dir', default="./model")
    parser.add_argument('--read', type=str, help='npz store dir', default="./training_data_npz")
    parser.add_argument('--output_num', type=int, help='the output num of the model not the original label num', default=2)
    args = parser.parse_args()
    OUTPUT_NUM = args.output_num
    print("OUTPUT_NUM:%d"%OUTPUT_NUM)
    main(args.model,args.read)
