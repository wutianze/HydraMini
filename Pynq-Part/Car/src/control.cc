/*
 * @Author: Sauron Wu
 * @GitHub: wutianze
 * @Email: 1369130123qq@gmail.com
 * @Date: 2019-10-14 09:10:53
 * @LastEditors  : Sauron Wu
 * @LastEditTime : 2020-01-07 15:10:45
 * @Description: 
 */
#include"control.h"
#include<cmath>
#define THROTTLEINIT 500000
#define THROTTLEVAL 100000
#define THROTTLEZERO 140000
#define STEERINIT 2000000
#define STEERVAL 40000
#define STEERZERO 151000
// every time you start, make sure the steer is straight
PYNQZ2::PYNQZ2(){
    int fd = open("/dev/mem",O_RDWR);
    nowS = new PYNQZ2::Status();

    leds = (char*)mmap(0, 8, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x81210000);
    leds[4] = 0x00;
    leds[0] = 0x0F;

    throttle = (int*)mmap(0, 30, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x82800000);
    throttle[4] = 0b001010010110;
    throttle[0] = 0b011010010110;
    throttle[1] = THROTTLEINIT;
    throttleSet(0);

    steer = (int*)mmap(0, 30, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x82810000);
    steer[4] = 0b001010010110;
    steer[0] = 0b011010010110;
    steer[1] = STEERINIT;
    steerSet(0);
}
void PYNQZ2::throttleSet(float rate){
    //leds[0] = leds[0] & 0x04;
    if(abs(rate - nowS->throttleRate) < 0.001)return;
    if(rate > 1.0)rate = 1.0;
    else if(rate < -1.0)rate = -1.0;
    throttle[5] = THROTTLEZERO + rate * THROTTLEVAL;
    nowS->throttleRate = rate;
}

void PYNQZ2::steerSet(float rate){
    //leds[0] = leds[0] & 0x08;
    if(abs(rate - nowS->steerRate) < 0.001)return;
    if(rate > 1.0)rate = 1.0;
    else if(rate < -1.0)rate = -1.0;
    steer[5] = STEERZERO + rate * STEERVAL;
    nowS->steerRate = rate;
}

void PYNQZ2::throttleChange(float rate){
    //leds[0] = 0x01;
    throttleSet(nowS->throttleRate + rate);
}

void PYNQZ2::steerChange(float rate){
    //leds[0] = 0x02;
    steerSet(nowS->steerRate + rate);
}

PYNQZ2::Status* PYNQZ2::getStatus(){
    return nowS;
}

void PYNQZ2::setLeds(int num){
    leds[0] = num;
}

string PYNQZ2::to_record(){
    return to_string(nowS->steerRate) + ',' + to_string(nowS->throttleRate);
}

PYNQZ2::~PYNQZ2(){
    steerSet(0);
    throttleSet(0);
    throttle[0] = 0b000000010110;
    steer[0] = 0b000000010110;
    leds[0] = 0x00;
}
