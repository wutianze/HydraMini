### 
# @Author: Sauron Wu
 # @GitHub: wutianze
 # @Email: 1369130123qq@gmail.com
 # @Date: 2019-09-25 15:32:42
 # @LastEditors  : Sauron Wu
 # @LastEditTime : 2019-12-19 10:58:55
 # @Description: 
 ###

rm -rf ./training_data_npz
python process_img.py

rm -rf ./model
python train.py



