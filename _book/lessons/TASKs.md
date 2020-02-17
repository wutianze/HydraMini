<!--
 * @Author: your name
 * @Date: 2019-12-14 20:57:16
 * @LastEditTime: 2019-12-14 21:39:05
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /pynq_car/TASKs.md
 -->
- 任务1：
阅读src/collect.cc代码并编译执行（make collect），尝试修改键盘控制方法（main函数内switch部分）和保存的数据（storeImage函数），用你自己的方式保存1000-2000张训练图片，可以合作。
结果：很多的图片，train.csv记录的label
思考：尝试设计一种可以人为中断的保存方案（参考按键t的实现）。

- 任务2：
理解目前的图片预处理代码，尝试修改Host-Part/process_img.py :(img[40:,:])/255.0-0.5，修改后运行，看看是否会出错并尝试修复。请记住此处你自定义的预处理过程，之后会多处用到。
结果：顺利完成打包生成很多个.npz文件
思考：理解method参数的意义，尝试自己编写数据清洗代码并清洗自己搜集到的数据（找到原清洗代码并替换）。

- 任务3：
利用经过预处理之后的训练数据进行训练，请注意修改OUTPUT_NUM等参数。
结果：生成.h5文件，模型的误差在可接受范围内
思考：为什么现有代码中对label的处理是labels[i] = [(y[index][0]+1.)/2.]（提示，查看网络输出层的激活函数）；尝试修改网络结构并训练；阅读batch_generator代码，尝试解释训练数据送入的方式，讨论这和传统方式相比哪个好。

- 任务4：
将.h5文件转化为.pb文件，可以参考freeze_detect.sh脚本的内容，并观察decent_q inspect命令输出的结果。

- 任务5：
完成DNNDK的量化（参考dnndk-host/decent.sh）和编译（参考dnndk-host/compile.sh）的任务，请注意阅读graph_input_fn.py函数（需要你去指定输入图片文件夹并删除一些多余的处理过程，然后根据你之前的图片预处理过程来处理图片）

- 任务6：
在Pynq板子上编译运行run.cc（make run），注意修改KERNEL_CONV，CONV_INPUT_NODE "conv2d_1_convolution"，CONV_OUTPUT_NODE等参数，然后根据你之前的图片预处理过程去修改setInputImage函数
思考：理解run.cc使用的多进程模型并尝试加入自己的进程；观察小车运行结果，尝试优化。

- 附加任务：
完整走完手写数字识别流程，理解其中每一步都做了什么，撰写实验报告。
