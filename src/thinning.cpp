#include "skeleton_filter.hpp"
#include <opencv2/imgproc/imgproc.hpp>

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

static void GuoHallIteration_optimized(cv::Mat& im, int iter)
{
    cv::Mat marker = cv::Mat::zeros(im.size(), CV_8UC1);

	int code;
	int encode;

	uchar LUT[256];

	uchar p2 = 0;
	uchar p3 = 0;
	uchar p4 = 0;
	uchar p5 = 0;
	uchar p6 = 0;
	uchar p7 = 0;
	uchar p8 = 0;
	uchar p9 = 0;

	for (int i = 0; i < 256; i++)
	{
		code = i;

		p2 = (code & 1);

		if ((code & 2) - 1 < 0) p3 = 0;
		else p3 = 1;

		if ((code & 4) - 3 < 0) p4 = 0;
		else p4 = 1;

		if ((code & 8) - 7 < 0) p5 = 0;
		else p5 = 1;

		if ((code & 16) - 15 < 0) p6 = 0;
		else p6 = 1;

		if ((code & 32) - 31 < 0) p7 = 0;
		else p7 = 1;

		if ((code & 64) - 63 < 0) p8 = 0;
		else p8 = 1;

		if ((code & 128) - 127 < 0) p9 = 0;
		else p9 = 1;


		int C  = (!p2 & (p3 | p4)) + (!p4 & (p5 | p6)) +
						 (!p6 & (p7 | p8)) + (!p8 & (p9 | p2));
		int N1 = (p9 | p2) + (p3 | p4) + (p5 | p6) + (p7 | p8);
		int N2 = (p2 | p3) + (p4 | p5) + (p6 | p7) + (p8 | p9);
		int N  = N1 < N2 ? N1 : N2;
		int m  = iter == 0 ? ((p6 | p7 | !p9) & p8) : ((p2 | p3 | !p5) & p4);

		if (C == 1 && (N >= 2 && N <= 3) & (m == 0))
			LUT[i] = 1;
		else
			LUT[i] = 0;
	}
	
	int x = 0;
    for (int i = 1; i < im.rows-1; i++)
    {
        for (int j = 1; j < im.cols-1; j++)
        {
			if (im.at<uchar>(i,j) != 0)
			{
				uchar q2 = im.at<uchar>(i-1, j);
				uchar q3 = im.at<uchar>(i-1, j+1);
				uchar q4 = im.at<uchar>(i, j+1);
				uchar q5 = im.at<uchar>(i+1, j+1);
				uchar q6 = im.at<uchar>(i+1, j);
				uchar q7 = im.at<uchar>(i+1, j-1);
				uchar q8 = im.at<uchar>(i, j-1);
				uchar q9 = im.at<uchar>(i-1, j-1);

				encode = q2*1 + q3*2 + q4*4 + q5*8 + q6*16 + q7*32 + q8*64 + q9*128;
				
				if (encode != 255)
					x = x + 1;
				
				marker.at<uchar>(i,j) = LUT[encode]; 
				
				/*
				int C  = (!p2 & (p3 | p4)) + (!p4 & (p5 | p6)) +
						 (!p6 & (p7 | p8)) + (!p8 & (p9 | p2));
				int N1 = (p9 | p2) + (p3 | p4) + (p5 | p6) + (p7 | p8);
				int N2 = (p2 | p3) + (p4 | p5) + (p6 | p7) + (p8 | p9);
				int N  = N1 < N2 ? N1 : N2;
				int m  = iter == 0 ? ((p6 | p7 | !p9) & p8) : ((p2 | p3 | !p5) & p4);
				

				if (C == 1 && (N >= 2 && N <= 3) & (m == 0))
					marker.at<uchar>(i,j) = 1;
				*/
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
