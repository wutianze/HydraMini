/*
 * @Author: Sauron Wu
 * @GitHub: wutianze
 * @Email: 1369130123qq@gmail.com
 * @Date: 2019-09-19 12:44:06
 * @LastEditors  : Sauron Wu
 * @LastEditTime : 2019-12-24 16:40:57
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
#include <string>
#include <vector>
#include <thread>
#include "control.h"
#include<csignal>
#include "cv_lane.h"
#include "runYolo.h"
using namespace cv;
using namespace std;
using namespace std::chrono;

//#define DEBUG_MODE

#define NNCONTROL 0
#define CVCONTROL 1

//camera parameters: img size
#define TAKE_SIZE_WIDTH 640
#define TAKE_SIZE_HEIGHT 480 


// commander indicates the car is controlled by AI or opencv currently
mutex commanderLock;
mutex exitLock;
mutex timeLock;
bool EXIT = false;
int commander = CVCONTROL;
clock_t timeSet;
safe_queue<Mat> takenImages;
struct steer_throttle_command{
    steer_throttle_command(){};
    steer_throttle_command(float s, float t):steer(s),throttle(t){};
    float steer;
    float throttle;
};
safe_queue<steer_throttle_command> generatedCommands;

// the tasks created to run yolo, now the number can only be one
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

#define COMMANDMAXLEN 3
void addCommand(steer_throttle_command tmpC){
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
    int nowSize = generatedCommands.size();
    if( nowSize >= COMMANDMAXLEN){
        if(generatedCommands.try_pop())generatedCommands.push(tmpC);
        return;
    }else{
        generatedCommands.push(tmpC);
    }
}
void avoid(){
    cout<<"run avoid\n";
    steer_throttle_command tmpC;
    
    //1. turn right
    tmpC.steer = 1;
    tmpC.throttle = -1.0;
    addCommand(tmpC);
    std::this_thread::sleep_for(std::chrono::milliseconds(4200));

    //2. turn left
    tmpC.steer = -1;
    tmpC.throttle = -1.0;
    addCommand(tmpC);
    std::this_thread::sleep_for(std::chrono::milliseconds(6000));

    //2. straight to cross the obstacle
    tmpC.steer = 0;
    tmpC.throttle = -1.0;
    addCommand(tmpC);
    std::this_thread::sleep_for(std::chrono::milliseconds(700));

    //2. turn left
    tmpC.steer = -1;
    tmpC.throttle = -1.0;
    addCommand(tmpC);
    std::this_thread::sleep_for(std::chrono::milliseconds(4900));

    //2. turn right
    tmpC.steer = 1;
    tmpC.throttle = -1.0;
    addCommand(tmpC);
    std::this_thread::sleep_for(std::chrono::milliseconds(3400));

    //2. straight again
    tmpC.steer = 0;
    tmpC.throttle = -1.0;
    addCommand(tmpC);
    //std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

// turn left in cross road
void small_left(){
cout<<"turn left in crossroad(small)\n";
steer_throttle_command tmpC;
tmpC.steer = 0;
addCommand(tmpC);
std::this_thread::sleep_for(std::chrono::milliseconds(1000));
tmpC.steer = -1;
addCommand(tmpC);
std::this_thread::sleep_for(std::chrono::milliseconds(10000));
tmpC.steer = 0;
addCommand(tmpC);
}

// turn right in cross road
void big_right(){
cout<<"turn right in crossroad(big)\n";
steer_throttle_command tmpC;
tmpC.steer = 0;
tmpC.throttle = -1;
addCommand(tmpC);
std::this_thread::sleep_for(std::chrono::milliseconds(6000));
tmpC.steer = 1;
addCommand(tmpC);
std::this_thread::sleep_for(std::chrono::milliseconds(8000));
tmpC.steer = 0;
addCommand(tmpC);
}

// if not green, then is red or yellow which means stop
#define iLowH_green 40
#define iHighH_green 90
#define iLowS 90
#define iHighS 255
#define iLowV 90
#define iHighV 255
//0 is green
int get_light(Mat img){
    Mat imgHSV;
    vector<Mat> hsvSplit;
    cvtColor(img,imgHSV,COLOR_BGR2HSV);
  
    //转化成直方图均衡化
    split(imgHSV,hsvSplit);
    equalizeHist(hsvSplit[2],hsvSplit[2]);
    merge(hsvSplit,imgHSV);
    Mat imgThresholded;
    //确定颜色显示的范围
    inRange(imgHSV, Scalar(iLowH_green, iLowS, iLowV), Scalar(iHighH_green, iHighS,iHighV),imgThresholded);
    //去除噪点
    Mat element = getStructuringElement(MORPH_RECT,Size(5,5));
    morphologyEx(imgThresholded,imgThresholded,MORPH_OPEN,element);
     //连接连通域
    morphologyEx(imgThresholded, imgThresholded, MORPH_CLOSE, element);
    int count_green = 0;
for(int nrow=0;nrow<imgThresholded.rows;nrow++)
    {
        uchar*data=imgThresholded.ptr<uchar>(nrow);
        for(int ncol=0;ncol<imgThresholded.cols*imgThresholded.channels();ncol++)
        {
            count_green++;
        }
    }
    if(count_green >(imgThresholded.rows*imgThresholded.cols*imgThresholded.channels())/5)return 0;
    return 1;
}

//----------fpt rules
// last_command: -1:no command, 0:avoid, 1:stop, 2:back to cv
int last_command = -1;
int pre_last_command = -1;

#define IGNORE_OB_X  0.3
#define IGNORE_OB_AREA 0.2
#define SEE_OBSTACLE 2

#define PERSON_OUT_X 0.1
#define IGNORE_PER_AREA 0.1
#define SEE_PER 2

#define IGNORE_SIDEWALK_AREA 0.3
#define SEE_SIDEWALK 2

#define IGNORE_LIGHT_AREA 0.3
//-------------------

void box_handler(vector<vector<float>>&res, Mat&img){
    //person_place means where is the person, -1:no person;0:in the road;1:out of the road
    int person_place = -1;
    bool has_ob = false;
    bool has_side = false;
    //0 is green, 1 is yellow or red
    int light = -1;

    //first find things
    for(int i=0;i<res.size();i++){
        switch(int(res[i][4])){
            //person
            case 0:{
                if(res[i][2]*res[i][3] < IGNORE_PER_AREA){
                    continue;
                }
                if(res[i][0]-res[i][2]/2 < PERSON_OUT_X){
                    person_place = 1;
                }
                else{
                    person_place = 0;
                }
                break;
            }
            //obstacles
            case 1:case 2:case 3:{
                if(res[i][0]-res[i][2]/2 > IGNORE_OB_X || res[i][2] * res[i][3] < IGNORE_OB_AREA){
                    continue;
                }
                has_ob = true;
                break;
            }
            //sidewalk
            case 4:{
                if(res[i][2]*res[i][3] < IGNORE_PER_AREA){
                    continue;
                }
                has_side = true;
                break;
            }
            //lights
            case 5:{
                if(res[i][2]*res[i][3] < IGNORE_PER_AREA){
                    continue;
                }
                light = get_light(img(Rect((res[i][0]-res[i][2]/2)*TAKE_SIZE_WIDTH,(res[i][1]-res[i][3]/2)*TAKE_SIZE_HEIGHT,res[i][2]*TAKE_SIZE_WIDTH,res[i][3]*TAKE_SIZE_HEIGHT)));
                break;
            }
        }
    }

    int now_command = -1;
    //second generate last_command
    if(has_ob){
        now_command = 0;
    }else if(person_place == 0){
        now_command = 1;
    }else if(person_place == 1){
        if(has_side){
            switch(light){
                case -1:case 1:{//if the light is not green or no light,stop
                    now_command = 1;
                    break;
                }
                case 0:{//if the light is green and the people is not in the road, keep running
                    now_command = 2;
                    break;
                }
            }
        }else{//no side, people not in road, run(shouldn't have light)
            switch(light){
                case 1:{//if the light is not green stop
                    now_command = 1;
                    break;
                }
                case 0:case -1:{//if the light is green and the people is not in the road, keep running
                    now_command = 2;
                    break;
                }
            }
        }
    }else if(person_place == -1){
        switch(light){
                //no ob no person no light just use cv
                case -1:case 0:{
                    now_command = 2;
                    break;
                }
                case 1:{
                    now_command = 1;
                    break;
                }
            }
    }
    bool change = false;
    if(last_command == now_command && last_command != pre_last_command){
        change = true;
    }
    if(change){
        switch(now_command){
            case 0:{
                commanderLock.lock();
                commander = NNCONTROL;
                commanderLock.unlock();
                avoid();
                commanderLock.lock();
                commander = CVCONTROL;
                commanderLock.unlock();
                break;
            }
            case 1:{
                commanderLock.lock();
                commander = NNCONTROL;
                commanderLock.unlock();
                steer_throttle_command tmpC;
                tmpC.steer = 0;
                tmpC.throttle = 0;
                addCommand(tmpC);
                break;
            }
            case 2:{
                commanderLock.lock();
                commander = CVCONTROL;
                commanderLock.unlock();
                break;
            }
            default:{
                commanderLock.lock();
                commander = CVCONTROL;
                commanderLock.unlock();
                break;
            }
        }
    }
    pre_last_command = last_command;
    last_command = now_command;

}

void run_model(DPUTask* task){
    cout<<"Run Model\n";
    int sh = dpuGetInputTensorHeight(task, INPUTNODE);
    int sw = dpuGetInputTensorWidth(task, INPUTNODE);

    Mat tmpImage;
    while (1)
    {
	    exitLock.lock();
	    if(EXIT)break;
	    else{
	        exitLock.unlock();
	    }
        
        if(!takenImages.try_pop(tmpImage))continue;
	    set_input_image(task, tmpImage,INPUTNODE);
        dpuRunTask(task);
	    vector<vector<float>> res = deal(task, tmpImage, sw, sh);
        //imshow("yolo-v3", img);
        //waitKey(0);	    //takenImages.wait_and_pop(tmpImage);
        box_handler(res,tmpImage);
    }
    exitLock.unlock();
    cout<<"Run Model Exit\n";
}

#define STRAIGHT_LEFT_K -1.1
#define STRAIGHT_RIGHT_K 1.1
#define L_TOO_LEFT 85
#define K_RANGE 0.2
void run_cv(){
    cout<<"Run CV\n";
    Mat tmpImage;
    CvSize frame_size = cvSize(LANE_DET_WIDTH, LANE_DET_HEIGHT/2);
    IplImage *temp_frame = cvCreateImage(frame_size, IPL_DEPTH_8U, 3);
	IplImage *grey = cvCreateImage(frame_size, IPL_DEPTH_8U, 1);
	IplImage *edges = cvCreateImage(frame_size, IPL_DEPTH_8U, 1);
	CvMemStorage* houghStorage = cvCreateMemStorage(0);

    while(true){
    	exitLock.lock();
    	if(EXIT)break;
    	else{
    	exitLock.unlock();
    	}
        commanderLock.lock();
        if(commander == NNCONTROL){
            commanderLock.unlock();
            continue;
        }
        commanderLock.unlock();
        if(!takenImages.try_pop(tmpImage))continue;
        IplImage frame_get = IplImage(tmpImage);
        int canFind = find_lane(&frame_get,temp_frame,grey,edges,houghStorage);
        
        steer_throttle_command tmpC;
        if(canFind == 1){//cannot find left but can find right
            tmpC.steer = -0.4;
		/*float tmpK = laneR.k.get();
	    float tmpB = laneR.b.get();
	    if(tmpK > STRAIGHT_RIGHT_K + K_RANGE|| tmpK < 0)tmpC.steer = 0.5;
	    else if(tmpK < STRAIGHT_RIGHT_K - K_RANGE && tmpK > 0)tmpC.steer = float(STRAIGHT_RIGHT_K - K_RANGE - tmpK)/float(STRAIGHT_RIGHT_K - K_RANGE);
    	    //if((72- tmpB)/tmpK < 140)tmpC.steer = -0.4;
	    cout<<"laneR k:"<<tmpK<<endl;
	    //cout<<"laneR b:"<<laneR.b.get()<<",k:"<<laneR.k.get()<<";steer:"<<tmpC.steer<<"\r\033[k";
	    */
        }else if(canFind == 3){
	    tmpC.steer = -0.4;
    	    //cout<<"cannot find any so steer:"<<tmpC.steer<<"\r\033[k";
	}else{
            float tmpB = laneL.b.get();
    	    float tmpK = laneL.k.get();
	    if(tmpK > STRAIGHT_LEFT_K + K_RANGE && tmpK < 0)tmpC.steer = float(STRAIGHT_LEFT_K + K_RANGE - tmpK)/float(STRAIGHT_LEFT_K + K_RANGE);
            else if(tmpK < STRAIGHT_LEFT_K - K_RANGE || tmpK > 0)tmpC.steer = -0.5;
	    if(tmpB > L_TOO_LEFT)tmpC.steer += 0.3;
	    cout<<"laneL k:"<<tmpK<<" b:"<<tmpB<<endl;
    	    //if(tmpK > 10 && -tmpB/tmpK > TOO_LEFT)tmpC.steer = 
    	    //cout<<"laneL b:"<<laneL.b.get()<<",k:"<<laneL.k.get()<<"; laneR b:"<<laneR.b.get()<<",k:"<<laneR.k.get()<<";steer:"<<tmpC.steer<<"\r\033[k";
        }
	
	cout<<"steer:"<<setprecision(4)<<tmpC.steer<<endl;

    tmpC.throttle = -1.0;
    addCommand(tmpC);

    }
    exitLock.unlock();
    cvReleaseMemStorage(&houghStorage);

	cvReleaseImage(&grey);
	cvReleaseImage(&edges);
	cvReleaseImage(&temp_frame);
    cout<<"Run CV Exit\n";
}

#define IMAGEMAXLEN 10 
void run_camera(){
    cout<<"Run Camera\n";
    VideoCapture cap(0);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, TAKE_SIZE_WIDTH);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, TAKE_SIZE_HEIGHT);
    Mat image;
    while(true){
	exitLock.lock();
	if(EXIT)break;
	else{
	exitLock.unlock();
	}
        cap >> image;
        int nowSize = takenImages.size();
        if(nowSize >= IMAGEMAXLEN){
            if(takenImages.try_pop())takenImages.push(image.rowRange(40,image.rows).clone());
        }else{
            takenImages.push(image.rowRange(40,image.rows).clone());
        }
    }
    exitLock.unlock();
    cout<<"Run Camera Exit\n";
    cap.release();
}


void run_command(){
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
        steer_throttle_command tmpS;
        if(!generatedCommands.try_pop(tmpS))continue;
    	//cout<<tmpS.steer<<endl;
	controller.steerSet(tmpS.steer);
        controller.throttleSet(tmpS.throttle);
    }
    exitLock.unlock();
    controller.steerSet(0);
    controller.throttleSet(0);
    cout<<"Run Steer Exit\n";
    }


int main(int argc, char **argv)
{
     if (argc != 1) {
        cout << "Usage of this exe: ./car"<< endl;
        return -1;
      }

    signal(SIGTSTP,sig_handler);
    /*
    dpuOpen();
    DPUKernel *kernelConv = dpuLoadKernel(YOLOKERNEL);
    vector<DPUTask*> tasks(TASKNUM);
    generate(tasks.begin(),tasks.end(),std::bind(dpuCreateTask,kernelConv,0));    
    */
    vector<thread> threads;
    threads.push_back(thread(run_command));
    threads.push_back(thread(run_camera));
    threads.push_back(thread(run_cv));
 
    /*
    for(int i=0;i<TASKNUM;i++){
    	threads.push_back(thread(run_model,tasks[i]));
    }
*/
    //threads.push_back(thread(big_right));
    
    for(int i = 0; i < threads.size(); i++){
        threads[i].join();
        cout<<"one exit:"<<i<<endl;
    }
/*
    for_each(tasks.begin(),tasks.end(),dpuDestroyTask);

    dpuDestroyKernel(kernelConv);
    dpuClose();
*/
    return 0;
}
