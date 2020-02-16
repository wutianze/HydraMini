<!--
 * @Author: Sauron Wu
 * @GitHub: wutianze
 * @Email: 1369130123qq@gmail.com
 * @Date: 2019-10-16 10:23:58
 * @LastEditors  : Sauron Wu
 * @LastEditTime : 2020-01-10 11:40:29
 * @Description: 
 -->
# Notice
If you want to make changes to the simulator, at least you should be familiar with C#, Python3, Unity3d and know some knowledge about how to build a network server. Before you start, I recommend that you go to [Tawn Kramer's project](https://github.com/tawnkramer/sdsandbox) first and read his documents and code.

# Project structure guide 
1. The most important script is `Car.cs & ICar.cs`, you should read the files carefully and understand how the car works.
2. The Control commands can be changed in `JoystickCarControl.cs`
3. The training data style can be changed in `Logger.cs`
4. You should enable the hidden buttons in `Canvas/Panel` Menu like ManualTraingData
5. The tcp control can be changed in `Script/tcp/TcpCarHandler.cs`, and you need to change the `predict_server.py` to communicate with the car correctly.
6. I recommend to use PID function to generate training data, you can define your own PID control in `PIDController.cs`.
7. You can read road info and build it by setting the `Do Build Road & Do Load Script Path` and indicating the `Path To Load`.
8. Set the MainCamera's `ifFollow` to decide whether to watch following the car. 

# Car.cs
- `RequestThrottle() & RequestSteering()` are two main functions which are used to control the car.
- You can change `maxTorque` to control the power of accelerator and `maxSpeed` to control the max Speed of the car. Remember the `Throttle` value doesn't mean the speed of car but the accelerator's force, you can use `car.GetVelocity().magnitude` to get the speed of the car, more information about WheelCollider and velocity please read the documents of Unity3d.

# JoystickCarControl.cs
- The `GetAxis()` function will get the Axis value in Unity3d, the x,y values will be used to calculate the steering and throttle values. These two values will be sent to the car and control the car's direction and speed.
- The Axis control of Unity3d is performed by keyboard, 'w,a,s,d' is used to change the value, you can try driving the car using Joystick control to see how it works. Also, you can define your own control functions, you can read the code in the comments for reference. Remember, the functions you use to send info to the car must exist in `Car.cs`. Take `RequestCommand() & GetComNow()` in `Car.cs` as an example.
 
# Logger.cs
- Now we define our Pynq-Style record structure in `Logger.cs`. You can define your own styles of record by the following steps:
    1. Define a boolean value to claim your style, when you use one style, you should make sure that the other styles' boolean values must be false. For example, `public bool MyStyle = true`.
    2. In `Awake()`, define your outputFilename, the file will record all your info costumed.
    3. In `Update()`, refer to PynqStyle and write your own record format. Usually, you can record the status of the car or the command you send to the car.
    4. Implement your image name generating function like `GetPynqStyleImageFilename()` and use the function in `SaveCamSensor()` which will save the images taken from the camera.
- You can add your own Get* functions in `Car.cs` and use them to get the info you want to record.

# TcpCarHandler.cs & predict_server.py
- Read `OnControlsRecv() & OnPynqControlsRecv()`, you can add your own messages just the same and define your own handlers. Pay attention that the json's key value must match what you send from `predict_server.py` .
- Read `SendTelemetry()`, the information in this function will be sent to `predict_server.py`, now we will only use the field `image` as the input of our model, you can add whatever you like in `SendTelemetry()` and handle them in `predict_server.py`.
- Read `PynqSimMsgHandler` in `predict_server.py`, it's easy for you to build your own handler. The data received from Unity3d is processed in `on_telemetry()`, the control message is sent to the car through `send_control*()`.

# PIDController.cs
- We all know that the quantity of training data is key to a good trained model. However, if your Joystick controller code is bull shit or you are not a good game player, it's hard to generate good training data. But since we are the master in virtual world, we can generate the training data automatically.
- Read `Update()` and find where it uses `RequestThrottle() & RequestSteering()` then you will understand how the PID controller works, and will be able to define your own functions.

# More
The simulator has more features for you to explore, you can build your own terrain, road, obstacles etc. You can checkout to the dev branch of original sdsandbox to learn more.