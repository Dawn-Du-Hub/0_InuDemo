#include "depth2rgb.h"


const unsigned char BLut[256] =
{

	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,  64,
	68,  72,  76,  80,  84,  88,  92,  96, 100, 104, 108, 112, 116, 120, 124, 128,
	132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176 ,180, 184, 188, 192,
	196, 200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	252, 248, 244, 240, 236, 232, 228, 224, 220, 216, 212, 208, 204, 200, 196, 192,
	188, 184, 180, 176, 172, 168, 164, 160, 156, 152, 148, 144, 140, 136, 132, 128,
};
const unsigned char GLut[256] =
{
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,  64,
	68,  72,  76,  80,  84,  88,  92,  96, 100, 104, 108, 112, 116, 120, 124, 128,
	132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188, 192,
	196, 200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	252, 248, 244, 240, 236, 232, 228, 224, 220, 216, 212, 208, 204, 200, 196, 192,
	188, 184, 180, 176, 172, 168, 164, 160, 156, 152, 148, 144, 140, 136, 132, 128,
	124, 120, 116, 112, 108, 104, 100,  96,  92,  88,  84,  80,  76,  72,  68,  64,
	60,  56,  52,  48,  44,  40,  36,  32,  28,  24,  20,  16,  12,   8,   4,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};
const unsigned char RLut[256] =
{
	0, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188, 192,
	196, 200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	252, 248, 244, 240, 236, 232, 228, 224, 220, 216, 212, 208, 204, 200, 196, 192,
	188, 184, 180, 176, 172, 168, 164, 160, 156, 152, 148, 144, 140, 136, 132, 128,
	124, 120, 116, 112, 108, 104, 100,  96,  92,  88,  84,  80,  76,  72,  68,  64,
	60,  56,  52,  48,  44,  40,  36,  32,  28,  24,  20,  16,  12,   8,   4,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};
cv::Mat Short2RGB(const cv::Mat& imIn, int maxDepth)
{
	cv::Mat imOut(imIn.size(), CV_8UC3);
	//cv::Mat imOut;
	//cvtColor(imIn, imOut, COLOR_GRAY2BGR);
	//int maxDepth = *max_element(imIn.begin<uint16_t>(), imIn.end<uint16_t>());
	//int minDepth = *min_element(imIn.begin<uint16_t>(), imIn.end<uint16_t>());
	//int maxDepth = 40000;//4000
	int minDepth = 0;
	float nomrmalizeFactor = (maxDepth - minDepth);

	for (int r = 0; r < imIn.rows; r++)
	{
		const ushort* in_ptr = imIn.ptr<ushort>(r);
		cv::Vec3b* out_ptr = imOut.ptr<cv::Vec3b>(r);
		for (int c = 0; c < imIn.cols; c++)
		{
			int noramlizedZ = int(255.0f * (in_ptr[c] - minDepth) / nomrmalizeFactor + 0.5f);
			out_ptr[c][0] = BLut[noramlizedZ];
			out_ptr[c][1] = GLut[noramlizedZ];
			out_ptr[c][2] = RLut[noramlizedZ];

		}
	}

	return imOut;
}

std::shared_ptr<cv::Mat> Merge2RGB2(const cv::Mat& imGray, const cv::Mat& imDepth)
{
	int maxDepth = 6000;//4000
	int minDepth = 0;
	float nomrmalizeFactor = (maxDepth - minDepth);
	float factor = 255.0f / 65535;
	//cv::Mat tmp;
	//imgGray.convertTo(tmp, CV_8UC1, 255.0 / 65535, 0.5);
	std::shared_ptr<cv::Mat> imOut = std::make_shared<cv::Mat>(imGray.size(), CV_8UC3);
	for (int r = 0; r < imGray.rows; r++)
	{
		cv::Vec3b* out_ptr = imOut->ptr<cv::Vec3b>(r);
		for (int c = 0; c < imGray.cols; c++)
		{
			uint16_t udep = imDepth.at<uint16_t>(r, c);
			uint16_t ugray = imGray.at<uint16_t>(r, c);
			ugray = int(ugray * factor);
			udep = int(255 * (udep - minDepth) / nomrmalizeFactor) ;
			uint16_t ucom = udep * 0.5 + ugray * 0.5;
			out_ptr[c][0] = ugray;
			out_ptr[c][1] = ugray;// imGray.at<uint8_t>(r, c);
			if (udep > 1)
				out_ptr[c][2] = 255-udep;
			else
				out_ptr[c][2] = ugray;

		}
	}

	return imOut;

}

cv::Mat Merge2RGB(const cv::Mat& imRgb, const cv::Mat& imDepth,int maxDepth)
{
	cv::Mat imgDepthShowNorm;
	imDepth.convertTo(imgDepthShowNorm, CV_8UC1, 255.0f / maxDepth, 0);

	cv::Mat_<uint8_t>::iterator it = imgDepthShowNorm.begin<uint8_t>();
	for (; it != imgDepthShowNorm.end< uint8_t >(); it++)
	{
		if (*it != 0) *it = 255 - *it;
	}
	std::vector<cv::Mat> channels;
	split(imRgb, channels);
	channels[2] = imgDepthShowNorm;
	cv::Mat imgShow;
	cv::merge(channels, imgShow);

	return imgShow;

}