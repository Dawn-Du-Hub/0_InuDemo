#ifndef _DEPTH2RGB_H_
#define _DEPTH2RGB_H_

#include <memory>
#include<opencv2/opencv.hpp>
cv::Mat Short2RGB(const cv::Mat& imDepth,int maxDepth = 4000);
cv::Mat Merge2RGB(const cv::Mat& imRgb, const cv::Mat& imDepth, int maxDepth = 4000);

#endif // !_DEPTH2RGB_H_
