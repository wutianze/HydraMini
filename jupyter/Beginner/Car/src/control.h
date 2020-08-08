/*
 * @Author: Sauron Wu
 * @GitHub: wutianze
 * @Email: 1369130123qq@gmail.com
 * @Date: 2019-10-14 09:10:53
 * @LastEditors: Sauron Wu
 * @LastEditTime: 2019-10-22 11:37:40
 * @Description: 
 */
#include<stdio.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<string>
using namespace std;
class Controller{
    public: 
    virtual string to_record() = 0;
};
class PYNQZ2: public Controller{
    private:
    int* throttle;
    int* steer;
    char* leds;
    
    public:
    struct Status{
        float throttleRate;
        float steerRate;
    };
    Status* nowS;
    PYNQZ2();
    ~PYNQZ2();

    void throttleSet(float);
    void steerSet(float);

    void throttleChange(float);
    void steerChange(float);

    void setLeds(int);

    PYNQZ2::Status* getStatus();

    string to_record();
};
