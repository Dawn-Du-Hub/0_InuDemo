# 中文版 (English Vestion at below)



# 函数说明
请参考 inuDepth.h，inuDepth.cpp, demoInuDepth.cpp 文件
|函数|参数|说明|
|-|-|-|
|setDistance|uint32_t min_dist,uint32_t max_dist|滤除深度值在 （min_dist,max_dist）外的数据|
|setConfidence|uint8_t conf|原始深度数据本身有一个可信值，提高这个confidence 设置，会滤除可信值低于此设置的深度数据|

# 使用说明
1. Params.yaml 文件中的 FPS，Resolution 2 个参数，可以设置和RGB不一样的值。
2. OutputFormat 一般设置为0， 直接输出深度值，如果设置为1，输出是2幅IR图的视差值。当输出视差值时，PostProcessing 设置无效。
3. PostProcessing 是SDK中对深度数据进行滤波的算法，按照bit位累加设置：
   1 Registration： 深度图对齐RGB图
   2 Blob: 滤除噪音数据 （测试方法，遮住模组镜头，如果此时深度图中干净，没有噪音数据，说明起效。）
   4 Temporal: 使用多帧数据进行平滑滤波
  


# English Version:
