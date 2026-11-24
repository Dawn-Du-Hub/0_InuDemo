# 中文版 (English Vestion at below)



# 函数说明

## inuParams.h
|函数|参数|说明|
|-|-|-|
|InuParams|std::string |类构造函数，输入配置文件名|

### 使用说明
读取 app/cfg/Params.yaml 文件中的相关的模组设置，具体设置说明参考 Params.yaml 里说明

## inuSensor.h

|函数|参数|说明|
|-|-|-|
|InuSensor|InuParams *|类构造函数，输入是InuParams*|
|checkDeviceState|无|检测模组是否正常联接，只有正常联接后才能进行初始化。返回为 true 或者 false|
|DeviceInit|无|初始化模组|
|DeviceTerminate|无|关闭模组|
|getSensorTemperature|float&|获取模组sensor 温度|
|getRGBExposeParams||获取模组RGB sensor 的曝光参数；返回值在构造函数输入参数 InuParams 中|
|setRGBExposeParams||设置模组RGB sensor 的曝光参数；调整RGB曝光参数，可以调整自动曝光，手动曝光，数字增益，模拟增益，曝光时间，可以在 yaml 里设置，也可以实时调整 inuParams 里 inu_RGB_setting 值，然后调用此函数|
|getIRExposeParams||获取模组IR sensor 的曝光参数；返回值在构造函数输入参数 InuParams 中|
|setIRExposeParams||设置模组IR sensor 的曝光参数；调整RGB曝光参数，可以调整自动曝光，手动曝光，数字增益，模拟增益，曝光时间，可以在 yaml 里设置，也可以实时调整 inuParams 里 inu_Depth_setting 值，然后调用此函数|
|setProjector|iLevel（eOff，eHigh）|设置投射器状态|

### 使用说明

调用顺序为先类实例化，再调用checkDeviceState 返回为true，再调用 DeviceInit，退出时调用 DeviceTerminate。

## inuDataPath.h

### 使用说明
数据通道，模组callback出的数据，使用队列临时保存，方便上层应用读取，实例化后，作为参数传入需要的模块中。亦可使用其他的方式，例如共享指针等。
## inuRGB.h
|函数|参数|说明|
|-|-|-|
|InuRGB|InuSensor*，InuDataPath*|类构造函数，输入InuSensor*，InuDataPath*|
|StartStream|||
|StopStream|||
|getRGBErrorTimes||获取RGB数据连续出错的次数，当错误达到一定次数后，建议重新初始化获取数据|

### 使用说明
具体参考 demoInuRGB.cpp

模组使用过程中，有小概率发生数据丢失的情况，可以使用getRGBErrorTimes 获取数据丢失的次数，上层应用做相应的处理，例如可以重联，参考 demoInuReconnect.cpp ，亦可以对模组做重启。
## inuDepth.h
|函数|参数|说明|
|-|-|-|
|InuDepth|InuSensor*，InuDataPath*|类构造函数，输入InuSensor*，InuDataPath*|
|StartStream|||
|StopStream|||
|getDepthErrorTimes||获取Depth数据连续出错的次数，当错误达到一定次数后，建议重新初始化获取数据|
### 使用说明

## inuDepthFilter.h
inuDepthFilter 仅供 inuDepth 使用，主要针对获取Depth数据时，对一些滤波参数进行设置。

|函数|参数|说明|
|-|-|-|
|InuDepthFilter|InuSensor*|类构造函数，输入InuSensor*|
|setSDKTemporalFilter|int stable, int rapid|时域多帧滤波，参数需要预设置，可以在配置文件中设置，也可以在Depth 调用 StartStream 前设置|
|setSDKOutlierRemove|int percent, int dist|移除一些噪点，SDKBlob 已经有此功能，建议使用blob中功能|
|setSDKHoleFill|int radius|填充空洞，原则上不建议填充，SDKBlob 已经有此功能，建议使用blob中功能，可以实时设置|
|setSDKBlob|int mode, int maxsize, int maxheight, int threshold, int same|mode: 2-remove small patch,3-fill hole,4- 2+3;maxsize：small patch 占用最大像素值；maxheight:small patch 占用最大行数；threshold：视差值门限；same：视差值最大门限，小于此值可以视为1个blob； 可以实时设置|
|setDistance|uint32_t min_dist,uint32_t max_dist|(min,max)外的数据深度设置为0|
|setConfidence|uint8_t conf|深度值的置信值，每个像素点的深度值都有一个置信值，小于此设置的深度值会被置0|


### 使用说明
Depth 和 DepthFilter 是一起使用的，要用来获取每个像素点的深度值。获取的深度值不可避免的会受到各种因数的影响，存在噪音干扰的问题。因此，芯片内部做了多种滤波算法。
这些算法可以设置感兴趣的深度范围，如：setDistance；
可以保留高可信度的数据，如：setConfidence；
也可以对深度数据做一些时域，空域的滤波，滤除噪音数据和对数据进行增强，如：setSDKTemporalFilter，setSDKOutlierRemove，setSDKHoleFill，setSDKBlob。

这些滤波参数可以通过params.yaml 文件预设值，也可以通过调用API函数在运行时设置。 有些滤波函数要预设置值，如： OutlierRemove,TemporalFilter, 有些可以在运行时实时改变，如：HoleFill，blob,setDistance，setConfidence。

其中，最常见和最优先的滤波就是 blob, 使用默认的参数就可以满足了绝大部分的场景。

同时，对深度数据有自己滤波处理的用户，也可以不用SDK提供的滤波方法，直接获取原始的深度数据，使用自己的滤波函数进行处理。 具体可参考 3drparty 中的 Filter 目录，此处给出了一些算法实现滤波的参考。
## inuIR.h
|函数|参数|说明|
|-|-|-|
|InuIR|InuSensor*，InuDataPath*|类构造函数，输入InuSensor*，InuDataPath*|
|StartStream|||
|StopStream|||
|getIRErrorTimes||获取IR数据连续出错的次数，当错误达到一定次数后，建议重新初始化获取数据|
### 使用说明

## inuPCL.h
|函数|参数|说明|
|-|-|-|
|InuPCL|InuSensor*，InuDataPath*|类构造函数，输入InuSensor*，InuDataPath*|
|StartStream|||
|StopStream|||
|getPCLErrorTimes||获取PCL数据连续出错的次数，当错误达到一定次数后，建议重新初始化获取数据|
### 使用说明

PCL 是芯片提供的原始点云数据，需要使用第三方的点云滤波算法获取更高质量的数据。

## inuIMU.h
|函数|参数|说明|
|-|-|-|
|InuIMU|InuSensor*，InuDataPath*|类构造函数，输入InuSensor*，InuDataPath*|
|StartStream|||
|StopStream|||
|getIMUErrorTimes||获取IMU数据连续出错的次数，当错误达到一定次数后，建议重新初始化获取数据|
### 使用说明

IMU 提供加速度和角速度数据。

## inuTempera.h
|函数|参数|说明|
|-|-|-|
|InuTempera|InuSensor*，InuDataPath*|类构造函数，输入InuSensor*，InuDataPath*|
|StartStream|||
|StopStream|||
|getTemperaErrorTimes||获取温度（sensor1,sensor2,CPU）数据连续出错的次数，当错误达到一定次数后，建议重新初始化获取数据|
### 使用说明

Tempera 提供Sensor1,Sensor2,CPU的温度数据。
## inuStatus.h
|函数|参数|说明|
|-|-|-|
|setSensorStatus|int status|0: stop 1:start|
|setRGBStatus|int status|0: stop 1:start|
|setDepthStatus|int status|0: stop 1:start|
|setIRStatus|int status|0: stop 1:start|
|getSensorStatus||返回状态|
### 使用说明
主要用来记录模块是否已经开启，在重联时使用

# English Version:
