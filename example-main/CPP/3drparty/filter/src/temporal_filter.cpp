
#include "temporal_filter.h"
#include "omp.h"
using namespace std;
using namespace cv;

namespace Inuchip
{

	TemporalFast::TemporalFast(uint8_t mode)
	{
		setMode(mode);
	}

	void TemporalFast::setMode(uint8_t mode)
	{
		calc_map(mode);
		_last_frame.release();
		_history.release();
		_width = 0;
		_height = 0;
	}

	cv::Mat TemporalFast::process_frame(const cv::Mat& f, float alpha, uint16_t delta)
	{
		if (f.cols != _width || f.rows != _height)
		{
			_last_frame.release();
			_history.release();
			_width = f.cols;
			_height = f.rows;
			_last_frame = cv::Mat(_height, _width, CV_16UC1);
			_history = cv::Mat(_height, _width, CV_8UC1);
		}

		cv::Mat ret;
		f.copyTo(ret);
		temporal_smooth(ret, _last_frame, _history, alpha, delta);

		return ret;
	}


	void TemporalFast::temporal_smooth(cv::Mat& frame_data, cv::Mat& _last_frame_data, cv::Mat& history, float alpha, uint16_t delta)
	{

		uint16_t cur_val, prev_val;
		uint16_t height = frame_data.rows;
		uint16_t width = frame_data.cols;

		uint16_t* pCur = frame_data.ptr<ushort>(0);
		uint16_t* pPrev = _last_frame_data.ptr<ushort>(0);
		uint8_t* pHist = history.ptr<uint8_t>(0);
		// use history data to update 
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				int idx = i * width + j;
				cur_val = pCur[idx];
				prev_val = pPrev[idx];
				if (cur_val)
				{
					pHist[idx] <<= 1;
					if (!prev_val)
					{
						pPrev[idx] = cur_val;
						pHist[idx] = 1;
					}
					else
					{  // old and new val
						uint16_t diff = cur_val > prev_val ? (cur_val - prev_val) : (prev_val - cur_val);

						if (diff < delta)
						{  // old and new val agree
							pHist[idx] += 1;
							float filtered = alpha * cur_val + (1 - alpha) * prev_val;
							pCur[idx] = (uint16_t)filtered;
							pPrev[idx] = (uint16_t)filtered;
						}
						else
						{
							pPrev[idx] = cur_val;
							pHist[idx] = 1;
						}
					}
				}
				else
				{  // insert current value
					if (prev_val)
					{
						uint8_t hist = pHist[idx];
						if (_LUT[hist]) // current mask meet the combination, i.e., meet the criterion set by the mode
						{
							pCur[idx] = prev_val;
						}
					}
					pHist[idx] <<= 1;
				}
			}
		}
	}


	void TemporalFast::calc_map(uint8_t mode)
	{

		// valid flag stored in 8 bit data, from left to right stands for the oldest to the latest position
		_LUT.fill(0);

		for (size_t i = 0; i < _LUT.size(); i++)
		{
			uint8_t idx = i;
			if (mode == 1)
			{
				// always use prev result when current frame has no valid value and prev frame has.
				_LUT[i] = 1;
			}
			else if (mode == 2)
			{ // valid in one of the last two frames
				_LUT[i] = 1; 
				if ((uint8_t)(idx << 6) == 0)
					_LUT[i] = 0;
			}
			else if (mode == 3)
			{// valid in two of the last three frames
				_LUT[i] = 1;
				uint8_t tmp = (uint8_t)(idx << 5);
				idx = tmp >> 5;
				if (idx == 0 || idx == 1 || idx == 2 || idx == 4)
					_LUT[i] = 0;
			}
			else if (mode == 4)
			{// valid in two of the last four frames
				_LUT[i] = 1;
				uint8_t tmp = (uint8_t)(idx << 4);
				idx = tmp >> 4;
				if (idx == 0 || idx == 1 || idx == 2 || idx == 4 || idx == 8)
					_LUT[i] = 0;
			}
			else if (mode == 5)
			{// valid in two of the last five frames
				_LUT[i] = 1;
				if ((uint8_t)(idx << 3) == 0)
					_LUT[i] = 0;
			}
			else if (mode == 6)
			{// valid in two of the last eight frames
				if (i >= 1)
					_LUT[i] = 1;
			}
			else if (mode == 7) 
			{// valid in two of the last eight frames
				_LUT[i] = 1;
				if (idx == 0 || idx == 1 || idx == 2 || idx == 4 || idx == 8 || idx == 16 || idx == 32 || idx == 64 || idx == 128)
					_LUT[i] = 0;
			}
			else if (mode == 8) //  <--- all 1's
			{
				if (i == 255)
					_LUT[i] = 1;
			}
			else // all others, including 0, no persistance
			{

			}
		}

	}





	TemporalFilter::TemporalFilter(int filter_length, bool bAll, int nThread)
		: mnFilterlength(filter_length)
		, mnImgSize(0)
		, mbInit(false)
		, mbAll(bAll)
		, mnFrameid(0)
		, mnThreadNum(nThread)
	{ }


	void TemporalFilter::init_median_lists(int size)
	{
		this->mnImgSize = size;

		std::vector<ushort> vec(mnFilterlength, 0);
		mvHistoryLists = std::vector< std::vector<ushort> >(size, vec);

		for (auto vec : mvHistoryLists)
			for (auto pix : vec)
				pix = std::rand();

		mnFrameid = 0;
		mbInit = true;
	}

	void TemporalFilter::filter_worker(cv::Mat& imgDst, int nbegin, int nend)
	{
		cv::MatConstIterator_<ushort> bgr_it = this->mFrame.begin<ushort>() + nbegin,
			bgr_end = this->mFrame.begin<ushort>() + nend;// frame.end  <ushort>();

		std::vector<std::vector<ushort>>::iterator his_it = mvHistoryLists.begin() + nbegin;
		cv::MatIterator_<ushort> dst_it = imgDst.begin<ushort>() + nbegin;

		for (; bgr_it != bgr_end; bgr_it++, dst_it++, his_it++)
		{
			(*his_it)[mnFrameid] = *bgr_it;
			ushort cur = *bgr_it;

			if (cur > 0 && !mbAll)
			{
				*dst_it = cur;
				continue;
			}
			double sum = 0;
			int nItem = 0;
			for (int m = 0; m < (*his_it).size(); m++)
			{
				if ((*his_it)[m] > 0)
				{
					sum += (*his_it)[m];
					nItem++;
				}

			}
			if (nItem > 0)
			{
				sum /= nItem;
				*dst_it = (int)sum;
			}
			else
				*dst_it = cur;
		}
	}
	cv::Mat TemporalFilter::process_frame(const cv::Mat& frame)
	{

		// Initialize the filter if we haven't yet

		mFrame = frame;

		if (!mbInit)
			init_median_lists((int)frame.total());

		cv::Mat retDst(mFrame.size(), mFrame.type());

		if (mnThreadNum == 1)
		{
			filter_worker(retDst, 0, mnImgSize);
		}
		else
		{
			std::vector<std::thread*> vThreads;
			int nInter = mnImgSize / mnThreadNum;
			int b0 = 0;
			int b1 = nInter;
			for (int i = 0; i < mnThreadNum; i++)
			{
				std::thread* th1 = new std::thread(&TemporalFilter::filter_worker, this, std::ref(retDst), b0, b1);
				vThreads.push_back(th1);
				b0 = b1;
				b1 = b1 + nInter;
				if (i == mnThreadNum - 2)
					b1 = mnImgSize;
			}
			for (int i = 0; i < mnThreadNum; i++)
			{
				vThreads[i]->join();
			}
			for (int i = 0; i < mnThreadNum; i++)
			{
				delete vThreads[i];
			}
		}

		mnFrameid = (mnFrameid + 1) % mnFilterlength;

		return retDst;
	}



	TemporalFilter_median_SLN::TemporalFilter_median_SLN(int filter_length, int maxInValidNum, int TH, int mode)
		: mnFilterlength(filter_length)
		, mMaxInValidThres(maxInValidNum)
		, mMode(mode)
		, mnImgSize(0)
		, mbInit(false)
		, mbNotFirstLoop(false)
		, mnFrameid(0)
		, mTH(TH)
	{ }


	void TemporalFilter_median_SLN::init_median_lists(int size)
	{
		this->mnImgSize = size;

		std::vector<ushort> vec(mnFilterlength, 0);
		mvHistoryLists = std::vector< std::vector<ushort> >(size, vec);

		for (auto vec : mvHistoryLists)
			for (auto pix : vec)
				pix = 0;

		mnFrameid = 0;
		mbInit = true;
	}


	cv::Mat TemporalFilter_median_SLN::process_frame(const cv::Mat& frame)
	{
		// set linear threshold, for depth ~500, the thres is 5
		int scaleTH = 5;
		int scaleRange = 500;

		mFrame = frame;
		cv::Mat retDst = cv::Mat::zeros(mFrame.rows, mFrame.cols, mFrame.type());

		// Initialize the filter if we haven't yet
		if (!mbInit)
			init_median_lists((int)frame.total());

		ushort* ptrDep = mFrame.ptr<ushort>(0);
		ushort* ptrDst = retDst.ptr<ushort>(0);
		std::vector<ushort>* ptrHist = &(mvHistoryLists[0]);

		for (int i = 0; i < frame.total(); i++)
		{
			ushort cur = *(ptrDep + i);
			(*(ptrHist + i))[mnFrameid] = cur;
			if (mMode == 0 || !mbNotFirstLoop)
			{
				if (cur == 0)
				{
					*(ptrDst + i) = 0;
					continue;
				}
			}
			int InValidNum = count((ptrHist + i)->begin(), (ptrHist + i)->end(), 0);

			if (mbNotFirstLoop && InValidNum >= mMaxInValidThres)
			{
				*(ptrDst + i) = 0;
				continue;
			}

			int cnt = mnFilterlength - InValidNum;
			if (cnt <= 2)
			{
				continue;
			}

			// calc mean
			int total = 0;
			for (int j = 0; j < mnFilterlength; j++)
			{
				total += (int)(*(ptrHist + i))[j];
			}
			int mean = (total - cur) / (cnt - 1);

			// calc std
			int std = 0;
			for (int j = 0; j < mnFilterlength; j++)
			{
				if (j == mnFrameid || (*(ptrHist + i))[j] == 0)
					continue;
				std += ((*(ptrHist + i))[j] - mean) * ((*(ptrHist + i))[j] - mean);
			}
			int sigma = sqrt(std) / (cnt - 1);

			int thres2 = scaleTH * mean / scaleRange;
			if (sigma > thres2) // return if sigma is too large, i.e., the depth is not stable
			{
				continue;
			}

			int diff = cur > mean ? cur - mean : mean - cur; // deal with case when sigma is very small
			if (sigma <= thres2 && diff <= thres2)
			{
				*(ptrDst + i) = (uint16_t)(total / cnt);
				continue;
			}

			// deal with the case when sigma can be used as threshold
			if ((int)cur< (mean - sigma * mTH) || (int)cur >(mean + sigma * mTH))
			{
				*(ptrDst + i) = 0;
			}
			else
			{
				*(ptrDst + i) = (uint16_t)(total / cnt);
			}
		}

		mnFrameid = (mnFrameid + 1) % mnFilterlength;

		// min_th works only after first round
		if (mbNotFirstLoop == false)
		{
			if (mnFrameid == mnFilterlength - 1)
			{
				mbNotFirstLoop = true;
			}
		}
		return retDst;
	}
}