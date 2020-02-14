<!--
 * @Author: Sauron Wu
 * @GitHub: wutianze
 * @Email: 1369130123qq@gmail.com
 * @Date: 2019-10-14 14:57:47
 * @LastEditors  : Sauron Wu
 * @LastEditTime : 2020-01-07 15:20:23
 * @Description: 
 -->
# What you will learn:
- How to collect training data.
- How to define your own controller.

# How to use
## Step 1:
Do some preparations. ini.sh will help you load the dpu.ko and set X11.
```shell
cd ~/Car
su
chmod +x ./ini.sh
./ini.sh
```

## Step2:
Compile the code and try.
```shell
make collect
./build/collect 50000 0.5 # 50000 means how many pictures you need to collect, 0.5 means the speed of your car
```
Now you will see a small window in your PC and you can use your keyboard to control your car:
- w: move forward and faster
- a: turn left
- d: turn right
- s: move slower
- t: start to save images until the number of pictures reach the threshold you set before, you can press it again to stop the save process until next press
- r: remove some images when you control your car in a wrong way and don't want to start all over. You need to input the number to delete in terminal, remember to change back to the cv window after finished and press `t` to collect.
- Esc: stop the whole process
The pictures and labels are saved in the directory `images`.

# Define your own controller
## 1. Modify the labels you want to save
In `src/control.h&control.cc`, you can see the current controller class PYNQZ2, the function `to_record` will return a string which will be written as labels. PYNQZ2 controller will store the throttle value and steer value of the car currently. You can replace the function content by your own to store other information.  
***Tip: The label file's format is csv which means values should be separated by ','***
## 2. Modify the keyboard settings
You can choose a more comfortable way to play the car. The function you need to change in this part is in `collect.cc/main`. You will see a `switch(c)` in `main`, read the code and I'm sure you can define your own settings quickly.  

# Pynq-Control API
1. void throttleSet(float)  
Set the throttle value of the car. The value range from -1.0 to 1.0. 0 means stop, > 0.0 means forward and < 0.0 means backward. The third led will be lighted.
2. void steerSet(float)  
Set the steer value of the car. The value range from -1.0 to 1.0. 0 means straight forward, > 0.0 means right and < 0.0 means left. The fourth led will be lighted.
3. void throttleChange(float)  
Add to the throttle value of the car. The max of input value is 1.0 and the min is -1.0. The first and third leds will be lighted.
4. void steerChange(float)  
Add to the steer value of the car. The max of input value is 1.0 and the min is -1.0. The second and fourth leds will be lighted.
5. PYNQZ2::Status* getStatus()  
Return the current Status of the car. It contains the current throttle and steer value of the car. You can use it like `PYNQZ2::Status* s = controller.getStatus(); float nowThrottle = s->throttleRate; float nowSteer = s->steerRate;`
6. string to_record()  
This function can generate a piece of info describing the current status of the car in .csv format. Example return: `0.1,0.5`, 0.1 is the steer value and 0.5 is the throttle value. It seems that the car is moving right forward now.
