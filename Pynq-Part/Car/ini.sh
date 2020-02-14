### 
# @Author: Sauron Wu
 # @GitHub: wutianze
 # @Email: 1369130123qq@gmail.com
 # @Date: 2019-10-14 14:51:04
 # @LastEditors: Sauron Wu
 # @LastEditTime: 2019-12-18 11:20:27
 # @Description: must run in su mode
 ###
insmod /home/xilinx/dpu.ko
xauth merge /home/xilinx/.Xauthority
rm -rf build
mkdir build
# Then run collcet in su mode, or you will get Gtk Warning: cannot open display.
