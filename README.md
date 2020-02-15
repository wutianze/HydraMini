<!--
 * @Author: Sauron Wu
 * @GitHub: wutianze
 * @Email: 1369130123qq@gmail.com
 * @Date: 2019-09-03 16:28:15
 * @LastEditors  : Sauron Wu
 * @LastEditTime : 2020-01-19 13:05:04
 * @Description: 
 -->
# Build your own Autonomous Driving Car
## Introduction
Enjoy the fun of your own autonomous driving car with the support of DPU， feel free to try your own AI model and new ideas.

## Material
Download the image we provided [here](https://pan.baidu.com/s/1gOJaoJJ8z2jf-BaLklID3Q). To understand the whole project well, you can read the paper we published in FPT2019 [here](https://easychair.org/publications/preprint/GMvL). And here is one simple guide written in Chinese, [Xilinx HydraMini试玩教程](https://blog.csdn.net/qq_25762163/article/details/103591878)

## Preparations
- The host pc should be ubuntu 16.04, you can refer to [this](https://github.com/wutianze/dnndk3.0-pynqz2) to build an environment for DNNDK_v3.0 using tensorflow. If you want to run the simulator, refer to [Autonomous-Driving-Simulator](https://github.com/wutianze/Autonomous-Driving-Simulator) to set the Unity3d environment.
- Power the Pynq-Z2 in your car and run `nmtui` to connect the car to your switch(make sure your PC connected to the same).
- Read the guides.

## Learning Steps
1. Read `pynq-guide/collect_guide.md` to learn how to use your keyboard for car controlling. And then collect some images for training.
2. Read `host-guide/preprocess_train.md` and preprocess the collected images, then use the preprocessed images to train the model.
3. Read `host-guide/quantize_compile.md` and do quantization and compilation to the model.
4. Read `pynq-guide/run_guide.md` and run the autonomous driving car.


