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
- How to use DPU to accelerate inference process.
- How to make changes to run part code according to your own model.
- How to run the autonomous car.

# DPU usage
## What is DPU?
The Xilinx® Deep Learning Processor Unit (DPU) is a programmable engine optimized for convolutional
neural networks. The unit includes a high performance scheduler module, a hybrid computing array
module, an instruction fetch unit module, and a global memory pool module. The DPU uses a
specialized instruction set, which allows for the efficient implementation of many convolutional neural
networks.

## Library libn2cube usage:
- Library libn2cube is the DNNDK core library. It implements the functionality of DPU loader, and encapsulates the system calls to invoke the DPU driver for DPU Task scheduling, monitoring, profiling, and resources management. 
- Simple guide of libn2cube
  1. The information you need to know after the generation of .elf using the host part of DNNDK-v3.0(If you don't know what it is, please refer to the Host part guide of this project) is:
     -  kernel name: KERNEL_CONV
     -  input node(s): CONV_INPUT_NODE
     -  output node(s): CONV_OUTPUT_NODE  
  2. Each DPU kernel supported by the DPU has a corresponding ELF object file with a name that is the same as the kernel name prefixed by dpu_ with extension .elf. So before compiling your code, you should put the .elf files into model directory and add their names to MODEL variable in Makefile.  
  3. The sample flow of using DPU kernel:  
    ```c++
    int main(void) {
    /* DPU Kernels/Tasks for running ResNet-50 */
    DPUKernel* kernelConv;
    DPUTask* taskConv
    /* Attach to DPU driver and prepare for running */
    dpuOpen()
    /* Create DPU Kernels for CONV Nodes */
    kernelConv = dpuLoadKernel(KERNEL_CONV)
    /* Create DPU Tasks for CONV Nodes*/
    taskConv = dpuCreateTask(kernelConv, 0)
    /* Run CONV Kernel*/
    Mat image = imread(baseImagePath + imageName);
    dpuSetInputImage2(taskConv, CONV_INPUT_NODE, image);
    dpuRunTask(taskConv)
    /* Get inference result */
    float scale = dpuGetOutputTensorScale(taskConv, CONV_OUTPUT_NODE);
    modelRes = dpuGetTensorAddress(dpuGetOutputTensor(taskConv, CONV_OUTPUT_NODE));
    dpuRunSoftmax(modelRes, smRes.data(), channel, 1, scale)
    /* Destroy DPU Tasks & release resources */
    dpuDestroyTask(taskConv)
    /* Destroy DPU Kernel & release resources */
    dpuDestroyKernel(kernelConv)
    /* Detach DPU driver & release resources */
    dpuClose();
    return 0
    ```
  4. The `main()` operations include:
     - Call `dpuOpen()` to open the DPU device.
     - Call `dpuLoadKernel()` to load the DPU kernel reset50_0.
     - Call `dpuCreateTask()` to create task for each DPU kernel.
     - Call `dpuDestroyKernel()` and `dpuDestroyTask()` to destroy DPU kernel and task.
     - Call `dpuClose()` to close the DPU device.
  5. Run CONV Kernel include:
     - Get the image for processing and set it as input
     - Run the task using `dpuRunTask()`
     - Get the output of the kernel and Run Softmax in it if needed
     - If your model is regression model, you can just use the result without running softmax like  
    ```c++
    float* res = float*(smRes.data()); float final_res = res[0] + res[1]
    ``` 
## What you need to modify according to your own model:
1. Variables:
   - `KERNEL_CONV, CONV_INPUT_NODE, CONV_OUTPUT_NODE`
   - `vector<string> kinds`: if you use classification model, you need to define your own categories
2. Functions:
   - `setInputImage()`: Replace it with your own image pre-process way
   - `topKind()`: It will use the results of softmax function and output the final kind, you only need it if you use classification model
   - `runCV()`: This function is independent of DPU usage, you can add your cv control here
   - If you use regression model, you don't need to use `topKind()` and softmax function, you should get the model output directly and add it in `addSteer()` instead of `addCommand()` and use it in `run_steer()` instead of `run_command()`.

## Run the car
```shell
cd ~/Car
make run
./build/run n 0.5 # Usage of this exe: ./car c/n 0.5(run speed)
```
# References
[libn2cube API](https://www.xilinx.com/support/documentation/sw_manuals/ai_inference/v1_5/ug1327-dnndk-user-guide.pdf)
