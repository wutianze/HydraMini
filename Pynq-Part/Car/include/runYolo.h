/*
 * @Author: Sauron Wu
 * @GitHub: wutianze
 * @Email: 1369130123qq@gmail.com
 * @Date: 2019-11-20 13:47:36
 * @LastEditors: Sauron Wu
 * @LastEditTime: 2019-11-25 14:26:59
 * @Description: 
 */
#ifndef RUNYOLO_H
#define RUNYOLO_H
#include <algorithm>
#include <vector>
#include <atomic>
#include <queue>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <mutex>
#include <zconf.h>
#include <thread>
#include <sys/stat.h>
#include <dirent.h>
#include <iomanip>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <math.h>
#include <arm_neon.h>
#include <opencv2/opencv.hpp>
#include <dnndk/n2cube.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;
using namespace std::chrono;

//confidence and threshold
#define CONF 0.5
#define NMS_THRE  0.1
//dpu kernel info
#define YOLOKERNEL "testModel"
#define INPUTNODE "conv2d_1_convolution"

extern vector<string>outputs_node;
//yolo parameters
extern const int classification;
//categories should be the same as predefined_classes.txt
extern vector<string>categories;
extern const int anchor;
extern vector<float> biases;

vector<vector<float>> deal(DPUTask* task, Mat& img, int sw, int sh);
void set_input_image(DPUTask* task, const Mat& img, const char* nodename);
#endif