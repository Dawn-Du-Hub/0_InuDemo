# 中文版 (English Vestion at below)



# 函数说明
请参考 inuDepth.h，inuDepth.cpp, demoInuDepthPCL.cpp 文件
|函数|参数|说明|
|-|-|-|
|pub_pcl|const cv::Mat &dep, const cv::Mat &imgWeb,int fidx,bool bOrder,int rinter,int cinter,int frameID|dep: 深度数据<br> imgWeb: RGB数据<br> bOrder: 有序点云<br> rinter: 行抽取数据<br> cinter: 列抽取数据<br> frameID: 帧号<br>|


# 使用说明
1. 此demo示例如何从深度数据重建点云数据。
2. 通过深度坐标可以依照三角函数计算出 X，Y 方向上的坐标。主要用到光心和焦距2个数据，深度数据可以以右IR镜头为坐标中心，也可以对齐到RGB，以RGB镜头为坐标中心，因此，光心和焦距也要使用IR和RGB坐标。



# English Version:
