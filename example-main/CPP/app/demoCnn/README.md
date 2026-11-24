# 中文版 (English Vestion at below)



# 函数说明
请参考 inuCNN.h，inuCNN.cpp, demoInuCnn.cpp 文件
|函数|参数|说明|
|-|-|-|
|| | |


# 使用说明
1. Params_CNN.yaml 文件中设置 inu_CNN_setting 的 OutputFormat
  
#ObjectDetection       - 0   /**< Object detection SSD fast but less accurate */
#Segmentation          - 1
#Classification        - 2
#FaceRecognition       - 3
#ObjectDetectionYoloV3 - 4 /**< Similar behavior as eObjectDetection but a different network(YoloV3) slower & accurate. */
#EnrollFace            - 5
#Yolact                - 6
#ObjectDetectionYoloV7 - 7
#PoseDetection         - 11
#HandDetection         - 12

0,4,7,11,12   test OK
but 1,2,3,5,6 not support now

# English Version:
