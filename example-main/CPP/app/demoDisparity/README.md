# 中文版 (English Vestion at below)

注意：此功能如无特别需求，不建议使用，高精度数据可以在depth 中设置 channel:5, depthscale:10

# 函数说明
请参考 inuDepth.h，InuDepth.cpp,demoInuDisparity.cpp 文件

# 使用说明
1.Disparity 是直接输出视差数据，此数据未经过SDK做处理，需要用户程序根据视差计算距离的公式，由用户程序计算出深度，由于深度计算都是浮点计算，因此得到的深度自然而然的可以达到亚毫米的精度，这个在高精度的模组计算中是必须的。 
2.修改 Params.yaml inu_Depth_setting 设置中的 OutPutFormat 为 raw (1)。此时SDK的 PostProcessing 已经被禁止，SDK不会进行滤波。
3.如上所述，SDK不会对数据进行处理，因此，原始数据会尽可能多的保存细节，这也是静距离，高精度模组需要保留的。但是，此时，测量过程中产出的噪音也有可能会被保留下来，这是就需要使用第三方的滤波方法来对数据进行处理。



# English Version:
