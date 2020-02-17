<!--
 * @Author: Sauron Wu
 * @GitHub: wutianze
 * @Email: 1369130123qq@gmail.com
 * @Date: 2019-10-14 14:57:47
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-02-17 20:33:43
 * @Description: 
 -->
# What you will learn:
- How to collect training data.
- How to define your own controller.

## Step 1:
Do some preparations. `ini.sh` will help you load the dpu.ko and set X11.
```shell
cd ~/Car
su
chmod +x ./ini.sh
./ini.sh
```
You can also run the commands in `ini.sh` step by step:
```
su
insmod /home/xilinx/dpu.ko # load dpu.ko
xauth merge /home/xilinx/.Xauthority # X11 config
rm -rf build # clear exist directory
mkdir build # build compile directory
```

## Step2:
Compile the code and try.
```shell
make collect
rm -rf images
.build/collect 20000 0.5 1 # 20000 means how many pictures you need to collect, 0.5 means the speed of your car, 1 means whether to show the pictures in your host(0 recommended which means not show)
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
*Every time before you want to collect images, you should remove `images` directory first*

# Next
Now you have collected the data for later training. Next, you will use host pc to train the network.
