# 中文版 (English Vestion at below)



# 函数说明
请参考 inuDepth.h，space_filter.h,temporal_filter.h,inuDepth.cpp, demoInuDepth3partyFilter.cpp 文件


# 使用说明
此demo示例如何使用第三方的滤波函数对深度数据进行滤波。主要有2类滤波，空间滤波和时间滤波。
主要的滤波提供：
|函数|参数|说明|
|-|-|-|
|process_outlier_remove||滤除噪点数据|
|process_hole_fill||此函数可以填充噪点数据滤除后留下的空洞|
|process_passthrough||滤除距离之外的数据，Z是深度值，x，y 是像素值|
|process_edge_preserve||保边滤波，在数据平滑的同时保留边缘信息|
|TemporalFast::process_frame||使用多帧数据平均，滤除单帧突变数据，但是会造成一定的拖尾|

可以看出，这里使用的滤波函数和SDK里提供的滤波功能有很多相似功能，主要是为有自己滤波算法的用户提供一个使用第三方滤波算法示例。用户可以使用GPU或其他并行算法进行加速，此时可以关闭SDK的滤波。

# English Version:
