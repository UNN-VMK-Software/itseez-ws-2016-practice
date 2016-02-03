#include "skeleton_filter.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <iostream>

static void GuoHallIteration(cv::Mat& im, int iter)
{
    cv::Mat marker = cv::Mat::zeros(im.size(), CV_8UC1);

    for (int i = 1; i < im.rows-1; i++)
    {
        for (int j = 1; j < im.cols-1; j++)
        {
            uchar p2 = im.at<uchar>(i-1, j);
            uchar p3 = im.at<uchar>(i-1, j+1);
            uchar p4 = im.at<uchar>(i, j+1);
            uchar p5 = im.at<uchar>(i+1, j+1);
            uchar p6 = im.at<uchar>(i+1, j);
            uchar p7 = im.at<uchar>(i+1, j-1);
            uchar p8 = im.at<uchar>(i, j-1);
            uchar p9 = im.at<uchar>(i-1, j-1);

            int C  = (!p2 & (p3 | p4)) + (!p4 & (p5 | p6)) +
                     (!p6 & (p7 | p8)) + (!p8 & (p9 | p2));
            int N1 = (p9 | p2) + (p3 | p4) + (p5 | p6) + (p7 | p8);
            int N2 = (p2 | p3) + (p4 | p5) + (p6 | p7) + (p8 | p9);
            int N  = N1 < N2 ? N1 : N2;
            int m  = iter == 0 ? ((p6 | p7 | !p9) & p8) : ((p2 | p3 | !p5) & p4);

            if (C == 1 && (N >= 2 && N <= 3) & (m == 0))
                marker.at<uchar>(i,j) = 1;
        }
    }

    im &= ~marker;
}

void GuoHallThinning(const cv::Mat& src, cv::Mat& dst)
{
    CV_Assert(CV_8UC1 == src.type());

    dst = src / 255;

    cv::Mat prev = cv::Mat::zeros(src.size(), CV_8UC1);
    cv::Mat diff;

    do
    {
        GuoHallIteration(dst, 0);
        GuoHallIteration(dst, 1);
        cv::absdiff(dst, prev, diff);
        dst.copyTo(prev);
    }
    while (cv::countNonZero(diff) > 0);

    dst *= 255;
}

//
// Place optimized version here
//

static std::vector<uchar> MakeGuoHallTable(int iter)
{
	uchar p2;
	uchar p3;
	uchar p4;
	uchar p5;
	uchar p6;
	uchar p7;
	uchar p8;
	uchar p9;
	char c;
	std::vector<uchar> table(256, 0);
	//std::cout << table.size();
	
	
	for (int code = 0; code < 256; code++)
	{
		p2 = code & 1;
		p3 = code & 2;
		p4 = code & 4;
		p5 = code & 8;
		p6 = code & 16;
		p7 = code & 32;
		p8 = code & 64;
		p9 = code & 128;

		p2 == 0 ? p2 = 0 : p2 = 1;
		p3 == 0 ? p3 = 0 : p3 = 1;
		p4 == 0 ? p4 = 0 : p4 = 1;
		p5 == 0 ? p5 = 0 : p5 = 1;
		p6 == 0 ? p6 = 0 : p6 = 1;
		p7 == 0 ? p7 = 0 : p7 = 1;
		p8 == 0 ? p8 = 0 : p8 = 1;
		p9 == 0 ? p9 = 0 : p9 = 1;


		int C  = (!p2 & (p3 | p4)) + (!p4 & (p5 | p6)) +
					(!p6 & (p7 | p8)) + (!p8 & (p9 | p2));
		int N1 = (p9 | p2) + (p3 | p4) + (p5 | p6) + (p7 | p8);
		int N2 = (p2 | p3) + (p4 | p5) + (p6 | p7) + (p8 | p9);
		int N  = N1 < N2 ? N1 : N2;
		int m  = iter == 0 ? ((p6 | p7 | !p9) & p8) : ((p2 | p3 | !p5) & p4);

		if (C == 1 && (N >= 2 && N <= 3) & (m == 0))
			table[code] = 1;
	}
	//std::cout << iter;
	/*if (iter > 0)
	{
		for (auto i: table)
			std::cout << static_cast<int> (i) << " ";
		std::cin >> c;
	}*/
	return table;
}

static void GuoHallIteration_optimized(cv::Mat& im, int iter)
{
    cv::Mat marker = cv::Mat::zeros(im.size(), CV_8UC1);
	std::vector<uchar> table = MakeGuoHallTable(iter);
	
    
	for (int i = 1; i < im.rows-1; i++)
    {
        for (int j = 1; j < im.cols-1; j++)
        {
			if (im.at<uchar>(i, j) != 0)
			{
				char c;
				uchar p2 = im.at<uchar>(i-1, j);
				uchar p3 = im.at<uchar>(i-1, j+1);
				uchar p4 = im.at<uchar>(i, j+1);
				uchar p5 = im.at<uchar>(i+1, j+1);
				uchar p6 = im.at<uchar>(i+1, j);
				uchar p7 = im.at<uchar>(i+1, j-1);
				uchar p8 = im.at<uchar>(i, j-1);
				uchar p9 = im.at<uchar>(i-1, j-1);
				
				uchar code = p2 * 1 +
					   p3 * 2 +
					   p4 * 4 +
					   p5 * 8 +
					   p6 * 16 +
					   p7 * 32 +
					   p8 * 64 +
					   p9 * 128;
				marker.at<uchar>(i,j) = table[code];
			}
        }
    }

    im &= ~marker;
}

void GuoHallThinning_optimized(const cv::Mat& src, cv::Mat& dst)
{
    CV_Assert(CV_8UC1 == src.type());

    dst = src / 255;

    cv::Mat prev = cv::Mat::zeros(src.size(), CV_8UC1);
    cv::Mat diff;

    do
    {
        GuoHallIteration_optimized(dst, 0);
        GuoHallIteration_optimized(dst, 1);
        cv::absdiff(dst, prev, diff);
        dst.copyTo(prev);
    }
    while (cv::countNonZero(diff) > 0);

    dst *= 255;
}

//
// Sample performance report
//
//           Name of Test               base          1           2          1          2
//                                                                           vs         vs
//                                                                          base       base
//                                                                       (x-factor) (x-factor)
// Thinning::Size_Only::640x480      333.442 ms  216.775 ms  142.484 ms     1.54       2.34
// Thinning::Size_Only::1280x720     822.569 ms  468.958 ms  359.877 ms     1.75       2.29
// Thinning::Size_Only::1920x1080    2438.715 ms 1402.072 ms 1126.428 ms    1.74       2.16
