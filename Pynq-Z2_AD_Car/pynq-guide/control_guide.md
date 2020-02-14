<!--
 * @Author: Sauron Wu
 * @GitHub: wutianze
 * @Email: 1369130123qq@gmail.com
 * @Date: 2019-12-24 15:55:57
 * @LastEditors  : Sauron Wu
 * @LastEditTime : 2019-12-24 16:02:28
 * @Description: How to set the hardware value.
 -->
# This is a simple guide, you will learn how to set the hardware.

- If you want to change the max speed value or max steer value, you can edit `src/control.cc`, see the codes below which shows the init value and zero(medium) value of the motors. You can change the zero value to make sure the motor doesn't move or the car runs straight; change the `THROTTLEVAL/STEERVAL` to make the car runs faster or makes a bigger turning. 
```c++
#define THROTTLEINIT 500000
#define THROTTLEVAL 60000
#define THROTTLEZERO 140000
#define STEERINIT 2000000
#define STEERVAL 40000
#define STEERZERO 151000
```
- Remember to run `make collect/run` to compile.