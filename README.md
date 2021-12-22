<!--
 * @Author: Sauron Wu
 * @GitHub: wutianze
 * @Email: 1369130123qq@gmail.com
 * @Date: 2019-09-03 16:28:15
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-02-17 20:36:49
 * @Description: 
 -->
# Build your own Autonomous Driving Car
## Introduction
Enjoy the fun of your own autonomous driving car with the support of DPU， feel free to try your own AI model and new ideas.

## Material
Download the image we provided [here](https://pan.baidu.com/s/1gOJaoJJ8z2jf-BaLklID3Q). To understand the whole project well, you can read the paper we published in FPT2019 [here](https://easychair.org/publications/preprint/GMvL) or the paper in MetroCAD2020 [here](https://ieeexplore.ieee.org/abstract/document/9138641). And here is one simple guide written in Chinese, [Xilinx HydraMini试玩教程](https://blog.csdn.net/qq_25762163/article/details/103591878)

## Preparations
- The host pc should be ubuntu 16.04, you can refer to [this](https://github.com/wutianze/dnndk3.0-pynqz2) to build an environment for DNNDK_v3.0 using tensorflow. If you want to run the simulator, refer to [Autonomous-Driving-Simulator](https://github.com/wutianze/Autonomous-Driving-Simulator) to set the Unity3d environment.
- Power the Pynq-Z2 in your car and run `nmtui` to connect the car to your switch(make sure your PC connected to the same).
- Read the guides in gitbook [HydraMini Gitbook](https://1369130123.gitbook.io/hydramini/).

## Learning Steps
1. Read `Pynq Part/Collect Data` to learn how to use your keyboard for car controlling. And then collect some images for training.
2. Read `Host Part/Preprocess & Train` and preprocess the collected images, then use the preprocessed images to train the model.
3. Read `Host Part/Quantize & Compile` and do quantization and compilation to the model.
4. Read `Pynq Part/Run the Car` and run the autonomous driving car.
5. If you want to try Autonomous Driving Simulator based on Unity3D, please refer to `Virtual Part`.


