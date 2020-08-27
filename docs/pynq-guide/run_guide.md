<!--
 * @Author: Sauron Wu
 * @GitHub: wutianze
 * @Email: 1369130123qq@gmail.com
 * @Date: 2019-10-15 09:17:19
 * @LastEditors  : Sauron Wu
 * @LastEditTime : 2019-12-24 11:43:44
 * @Description: 
 -->
# What you will learn:
- How to run the autonomous car.

# What you need to modify according to your own model:
1. Variables:
   - `KERNEL_CONV` is the `net_name` parameter in `compile.sh`, `CONV_INPUT_NODE & CONV_OUTPUT_NODE` are the `input_nodes & output_nodes` in `quant.sh`.
   - `CUT_SIZE` is the number of lines to cut in initial images(be the same in `process_img.py`). `STORESIZE_WIDTH & STORESIZE_HEIGHT` are the size of taken images. The final input size is `STORESIZE_WIDTH` \* (`STORESIZE_HEIGHT` - `CUT_SIZE`).
2. Functions:
   - `setInputImage()`: Replace the image preprocess with your own. Now the preprocess is:
   ```c++
   data[j*image.rows*3+k*3+i] = (float(image.at<Vec3b>(j,k)[i])/255.0 - 0.5)*scale;
   ```
   - `topKind()`: It will use the results of softmax function and output the final kind, you only need it if you use classification model
   - `runCV()`: This function is independent of DPU usage, you can add your cv control in it.
   - If you use regression model, you don't need to use `topKind()` and softmax function, you should get the model output directly and add it in `addSteer()` or `addSteer_Throttle` instead of `addCommand()` and use it in `run_steer()` or `run_steer_throttle()` instead of `run_command()`.
3. Output handle:
   The quantity of the model output is influenced by many factors. You can adjust the output to make the car run well. For example, if the car has a tendency to turn left, you can increase the steer value to make it normal.

# Run the car
```shell
cd ~/Car
make run
./init.sh # instead run: insmod /home/xilinx/dpu.ko
./build/run n 0.5 # Usage of this exe: ./car c/n 0.5(run speed)
```
The first parameter means the run mode, `n` for only using neural network, `c` for using both neural network and computer vision functions. The second parameter is used unless the model outputs throttle value directly.

# References
[libn2cube API](https://www.xilinx.com/support/documentation/sw_manuals/ai_inference/v1_5/ug1327-dnndk-user-guide.pdf)
