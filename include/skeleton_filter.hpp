#pragma once

#include "opencv2/core/core.hpp"

// Macros for time measurements
#if 1
    #define TS(name) int64 t_##name = cv::getTickCount()
	#define TE(name) #name << ';' << std::setprecision(3) << std::fixed << 1000.f * ((cv::getTickCount() - t_##name) / cv::getTickFrequency()) << ';'
#else
    #define TS(name)
    #define TE(name)
#endif

// Pipeline
void skeletonize(const cv::Mat& input, cv::Mat& output, bool save_images);

// Internal functions
void ConvertColor_BGR2GRAY_BT709(const cv::Mat& src, cv::Mat& dst);
void ImageResize(const cv::Mat &src, cv::Mat &dst, const cv::Size sz);
void GuoHallThinning(const cv::Mat& src, cv::Mat& dst);

// Optimized versions
void GuoHallThinning_optimized(const cv::Mat& src, cv::Mat& dst);
