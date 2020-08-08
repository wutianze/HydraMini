/*
 * @Author: Sauron Wu
 * @GitHub: wutianze
 * @Email: 1369130123qq@gmail.com
 * @Date: 2019-09-19 12:44:06
 * @LastEditors  : Sauron Wu
 * @LastEditTime : 2020-01-07 15:11:45
 * @Description: 
 */
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cassert>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <dnndk/dnndk.h>
#include <opencv2/opencv.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>
#include <fstream>
#include <sstream>
#include "safe_queue.h"
#include <mutex>
#include <thread>
#include "control.h"
#include<csignal>
#include "httplib.h"
using namespace cv;
using namespace std;
using namespace std::chrono;
using namespace httplib;

//whether to use opencv, set by user
string mode;

#define NNCONTROL 0
#define CVCONTROL 1
// commander indicates the car is controlled by AI or opencv currently
mutex commanderLock;
mutex exitLock;
mutex timeLock;
bool EXIT = false;
int commander = NNCONTROL;
clock_t timeSet;
safe_queue<Mat> takenImages;
struct steer_throttle{
    steer_throttle(){};
    steer_throttle(float s, float t):steer(s),throttle(t){};
    float steer;
    float throttle;
};
safe_queue<steer_throttle> generatedSteer_Throttles;
//vector<string> kinds = {"left", "forward", "right"};
vector<string> kinds = {"steer","throttle"};
float runSpeed = -1;
int client;

#define KERNEL_CONV "testModel"
#define CONV_INPUT_NODE "conv2d_1_convolution"
#define CONV_OUTPUT_NODE "dense_3_MatMul"
//#define OUTPUT_NUM 1

#define CUT_SIZE 40
#define STORESIZE_WIDTH 160
#define STORESIZE_HEIGHT 120

#define TASKNUM 1
//#define SHOWTIME
#ifdef SHOWTIME
#define _T(func)                                                              \
    {                                                                         \
        auto _start = system_clock::now();                                    \
        func;                                                                 \
        auto _end = system_clock::now();                                      \
        auto duration = (duration_cast<microseconds>(_end - _start)).count(); \
        string tmp = #func;                                                   \
        tmp = tmp.substr(0, tmp.find('('));                                   \
        cout << "[TimeTest]" << left << setw(30) << tmp;                      \
        cout << left << setw(10) << duration << "us" << endl;                 \
    }
#else
#define _T(func) func;
#endif

void sig_handler(int sig){
cout<<"SIGTSTP resceived\n";
if(sig == SIGTSTP){
	exitLock.lock();
	EXIT = true;
	exitLock.unlock();
}
}

// the input image is in BGR format(opencv default), and the '/255 - 0.5' must be the same when you 
// train the model, don't forget to multiply the scale finally
void setInputImage(DPUTask *task, const char* inNode, const cv::Mat& image) {
    DPUTensor* in = dpuGetInputTensor(task, inNode);
    float scale = dpuGetTensorScale(in);
    int8_t* data = dpuGetTensorAddress(in);
    for(int i = 0; i < 3; ++i) {
        for(int j = 0; j < image.rows; ++j) {
            for(int k = 0; k < image.cols; ++k) {
               data[j*image.rows*3+k*3+i] = (float(image.at<Vec3b>(j,k)[i])/255.0 - 0.5)*scale;
            }
        }
    }
}

int topKind(const float *d, int size)
{
    assert(d && size > 0);
    int result = 0;
    float maxx = d[0];
    //d[1]=d[1] -0.1;
    for (auto i = 0; i < size; ++i)
    {
        if (d[i] > maxx)
        {
            maxx = d[i];
            result = i;
        }
    }
    cout<<"result:"<<d[0]<<", "<<d[1]<<", "<<d[2]<<", \n";
    return result;
}

#define COMMANDMAXLEN 3
void addSteer_Throttle(steer_throttle tmpC){
    timeLock.lock();
    clock_t now = clock();
    if(now < timeSet){
        timeLock.unlock();
        return;
    }else{
    	//cout<<"FPS:"<<CLOCKS_PER_SEC/float(now-timeSet)<<endl;
        timeSet = now;
    }
    timeLock.unlock();
    int nowSize = generatedSteer_Throttles.size();
    if( nowSize >= COMMANDMAXLEN){
        if(generatedSteer_Throttles.try_pop())generatedSteer_Throttles.push(tmpC);
        return;
    }else{
        generatedSteer_Throttles.push(tmpC);
    }
}

void run_model(DPUTask* task){
    cout<<"Run Model\n";
    int channel = kinds.size();
    vector<float> smRes(channel);

    Mat tmpImage;
    while (1)
    {
	    exitLock.lock();
	    if(EXIT)break;
	    else{
	        exitLock.unlock();
	    }
        commanderLock.lock();
        if(commander == CVCONTROL){
            commanderLock.unlock();
            continue;
        }
        commanderLock.unlock();
        if(!takenImages.try_pop(tmpImage))continue;
	//takenImages.wait_and_pop(tmpImage);
        _T(setInputImage(task, CONV_INPUT_NODE, tmpImage));
        //dpuSetInputImage2(task,CONV_INPUT_NODE, tmpImage);
        _T(dpuRunTask(task));
        float scale = dpuGetOutputTensorScale(task, CONV_OUTPUT_NODE);
        int8_t* modelRes = dpuGetTensorAddress(dpuGetOutputTensor(task, CONV_OUTPUT_NODE));
        float steer = modelRes[0] * scale * 2 - 1.0;
        float throttle = modelRes[1] * scale * 2 - 1.0;
	    //_T(dpuRunSoftmax(modelRes, smRes.data(), channel, 1, scale));
	cout<<"output steer:"<<steer<<" throttle:"<<throttle<<endl;  

    //you should edit the following code according to your model's performance
	float changeV = steer - 0.08;
        if(changeV > 0){
	changeV = changeV + 0.1;
	}	
	if(changeV < 0){
		changeV = changeV - 0.4;
	}

    steer_throttle tmpC;
    tmpC.steer = changeV;
    if(runSpeed < 0>){
        tmpC.throttle = throttle;
    }else{
        tmpC.throttle = runSpeed;
    }

    addSteer_Throttle(tmpC);
    //addCommand(topKind(smRes.data(), channel));        
    }
    exitLock.unlock();
    cout<<"Run Model Exit\n";
}

int cv_al1(Mat image){
    return 0;
}

void run_cv(){
    cout<<"Run CV\n";
    Mat tmpImage;
    while(true){
    	exitLock.lock();
    	if(EXIT)break;
    	else{
    	exitLock.unlock();
    	}
        if(!takenImages.try_pop(tmpImage))continue;
        int tmpCommand = cv_al1(tmpImage);
        if(tmpCommand == 0)continue;
        commanderLock.lock();
        if(commander == CVCONTROL){
            commanderLock.unlock();
            continue;
        }
        commanderLock.unlock();
    }
    exitLock.unlock();
    cout<<"Run CV Exit\n";
}

#define IMAGEMAXLEN 10 
void run_camera(){
    cout<<"Run Camera\n";
    //VideoCapture cap(0);
    //cap.set(CV_CAP_PROP_FRAME_WIDTH, STORESIZE_WIDTH);
    //cap.set(CV_CAP_PROP_FRAME_HEIGHT, STORESIZE_HEIGHT);
    Mat image,resizeImage;
    while(true){
	exitLock.lock();
	if(EXIT)break;
	else{
	exitLock.unlock();
	}
        //cap >> image;
        image = imread("a.jpg",CV_LOAD_IMAGE_COLOR);
        resize(image,resizeImage,Size(STORESIZE_WIDTH,STORESIZE_HEIGHT));
        int nowSize = takenImages.size();
        if(nowSize >= IMAGEMAXLEN){
            if(takenImages.try_pop())takenImages.push(resizeImage.rowRange(CUT_SIZE,resizeImage.rows).clone());
        }else{
            takenImages.push(resizeImage.rowRange(CUT_SIZE,resizeImage.rows).clone());
        }
    }
    exitLock.unlock();
    cout<<"Run Camera Exit\n";
    cap.release();
}
void run_steer_throttle(){
    cout<<"Run Command\n";
    PYNQZ2 controller = PYNQZ2();
    //controller.throttleSet(runSpeed);
    while(true){
	exitLock.lock();
	if(EXIT){
		break;
		controller.steerSet(0.0);
		controller.throttleSet(0.0);
	}
	else{
	exitLock.unlock();
	}
        steer_throttle tmpS;
        if(!generatedSteer_Throttles.try_pop(tmpS))continue;
    	//cout<<tmpS.steer<<endl;
	    controller.steerSet(tmpS.steer);
        controller.throttleSet(tmpS.throttle);
    }
    exitLock.unlock();
    controller.steerSet(0);
    controller.throttleSet(0);
    cout<<"Run Command Exit\n";
    }

void run_steer(){
    cout<<"Run Steer\n";
    PYNQZ2 controller = PYNQZ2();
    controller.throttleSet(runSpeed);
    while(true){
	exitLock.lock();
	if(EXIT){
		break;
		controller.steerSet(0.0);
		controller.throttleSet(0.0);
	}
	else{
	exitLock.unlock();
	}
        steer_throttle tmpS;
        if(!generatedSteer_Throttles.try_pop(tmpS))continue;
    		controller.steerSet(tmpS.steer);
    }
    exitLock.unlock();
    cout<<"Run Steer Exit\n";
    }

void run_server(){
    cout<<"Run Server\n";
  Server svr;

  svr.Get("/st", [](const Request& req, Response& res) {
        steer_throttle tmpS;
        while(!generatedSteer_Throttles.try_front(tmpS)){}
    res.set_content("{\"steer\":"+to_string(tmpS.steer)+",\"throttle\":"+to_string(tmpS.throttle)+"}", "application/json");
  });
//TODO
svr.Post("/multipart", [&](const auto& req, auto& res) {
    auto size = req.files.size();
    auto ret = req.has_file("name1");
    const auto& file = req.get_file_value("name1");
    //file.filename;
    // file.content_type;
    // file.content;
                });

  svr.Get("/stop", [&](const Request& req, Response& res) {
    svr.stop();
  });

  svr.listen("0.0.0.0", 9090);
}

int main(int argc, char **argv)
{
     if (argc < 2) {
        cout << "Usage of this exe: ./car c/n/s 0.5(run speed)"<< endl;
        return -1;
      }

    signal(SIGTSTP,sig_handler);
    // n means just use ml, c means use ml & cv, s means use ml and build server
    mode = argv[1];
    if(argc >2)runSpeed = atof(argv[2]);

    /* The main procress of using DPU kernel begin. */
    DPUKernel *kernelConv;

    dpuOpen();
    kernelConv = dpuLoadKernel(KERNEL_CONV);
    vector<DPUTask*> tasks(TASKNUM);
    generate(tasks.begin(),tasks.end(),std::bind(dpuCreateTask,kernelConv,0));    
    vector<thread> threads;
    for(int i=0;i<TASKNUM;i++){
    	threads.push_back(thread(run_model,tasks[i]));
    }
    //threads.push_back(thread(run_steer_throttle));
    //threads.push_back(thread(run_steer));
    threads.push_back(thread(run_camera));
    if(mode[0]=='c'){
    threads.push_back(thread(run_cv));
    }
    if(mode[0]=='s')threads.push_back(thread(run_server));
    for(int i = 0; i < threads.size(); i++){
        threads[i].join();
        cout<<"one exit:"<<i<<endl;
    }

    for_each(tasks.begin(),tasks.end(),dpuDestroyTask);

    dpuDestroyKernel(kernelConv);
    dpuClose();
    return 0;
}
