/*
 * Temporal Filter.hpp
 *
 * 
 */

#ifndef __TEMPORAL_FILTER_HPP__
#define __TEMPORAL_FILTER_HPP__

#include <opencv2/opencv.hpp>

#include <algorithm>
#include <vector>

#include <thread>
#include <numeric>

namespace Inuchip
{
	class TemporalFast
	{
	public:
		TemporalFast(uint8_t mode = 3);
		cv::Mat process_frame(const cv::Mat& f, float alpha = 0.4f, uint16_t delta = 20);

	private:
		void temporal_smooth(cv::Mat& frame_data, cv::Mat& _last_frame_data, cv::Mat& history, float alpha, uint16_t delta);
		void calc_map(uint8_t mode);
		void setMode(uint8_t mode);

		size_t                  _width, _height;
		cv::Mat                 _last_frame;
		cv::Mat                  _history;
		std::array<uint8_t, 256> _LUT;
	};


	class TemporalFilter
	{
	public:
		/*
		* filter_length(8): how many previous frames are used for filter
		* bALL(false):      whether process every frame, false just invalid pixels
		* nThread(1):       how many threads are used
		*/
		TemporalFilter(int filter_length = 8, bool bAll = false, int nThread = 1);
		cv::Mat process_frame(const cv::Mat& frame);

	protected:
		void filter_worker(cv::Mat& imgDst, int nbegin, int nend);

		void init_median_lists(int size);
		cv::Mat mFrame;
		int mnFilterlength;
		int mnImgSize;
		bool mbInit;
		bool mbAll;
		int mnFrameid;
		int mnThreadNum;
		std::vector< std::vector<ushort> > mvHistoryLists;
	};

	
	class TemporalFilter_median_SLN
	{
	public:
		/*
		* filter_length(8): how many previous frames are used for filter
		* MaxInValidNum(5): set current pixel to invalid(0) is invalid data size is larger or equal than MaxInValidNum
		* th(2):			set threshold to remove outlier,default 2 
		* Mode(0):			mode 0 -> if current pixel is invalid, donot calc; 
		*					mode 1 -> conduct calculation anyway
		*/
		TemporalFilter_median_SLN(int filter_length = 8, int MaxInValidNum = 5, int th = 2, int Mode = 0);
		cv::Mat process_frame(const cv::Mat& frame);

	protected:
		void init_median_lists(int size);
		cv::Mat mFrame;
		int mnFilterlength;
		int mMaxInValidThres;
		int mnImgSize;
		bool mbInit;
		bool mbNotFirstLoop;
		int mMode;
		int mnFrameid;
		int mTH;
		std::vector< std::vector<ushort> > mvHistoryLists;
	};

}

#endif
