<!--
 * @Author: Sauron Wu
 * @GitHub: wutianze
 * @Email: 1369130123qq@gmail.com
 * @Date: 2019-12-24 15:55:57
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-02-17 20:33:54
 * @Description: How to set the hardware value.
 -->
# Guide For Developer
Show you the detail of HydraMini in Pynq-Part including keyboard control, motor value set and DPU c++ library usage.

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

# Motor Value Set
1. If you want to change the max speed value or max steer value, you can edit `src/control.cc`, see the codes below which shows the init value and zero(medium) value of the motors. You can change the zero value to make sure the motor doesn't move or the car runs straight; change the `THROTTLEVAL/STEERVAL` to make the car runs faster or makes a bigger turning. 
```c++
#define THROTTLEINIT 500000
#define THROTTLEVAL 60000
#define THROTTLEZERO 140000
#define STEERINIT 2000000
#define STEERVAL 40000
#define STEERZERO 151000
```
2. Remember to run `make collect/run` to compile.

# DPU c++ library libn2cube usage:
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
    /* modelRes is the start address of the model's output */
    int8_t* modelRes = dpuGetTensorAddress(dpuGetOutputTensor(taskConv, CONV_OUTPUT_NODE));
    /* the true value should * scale */
    float result1 = modelRes[0] * scale;

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