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

static void GuoHallIteration_optimized(cv::Mat& im, uchar table[2][256])
{
    cv::Mat marker = cv::Mat::zeros(im.size(), CV_8UC1);

	for (int iteration = 0; iteration < 2; iteration++)
	{
		for (int i = 1; i < im.rows-1; i++)
		{
			for (int j = 1; j < im.cols-1; j++)
			{
				if (im.at<uchar>(i, j))
				{
					marker.at<uchar>(i, j) = table[iteration] [im.at<uchar>(i - 1, j) + im.at<uchar>(i - 1, j + 1) * 2 + \
															  im.at<uchar>(i, j + 1) * 4 + im.at<uchar>(i + 1, j + 1) * 8 + \
															  im.at<uchar>(i + 1, j) * 16 + im.at<uchar>(i + 1, j - 1) * 32 + \
															  im.at<uchar>(i, j - 1) * 64 + im.at<uchar>(i-1, j-1) * 128];
				}
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

	static uchar look_up_table[2][256];
	static bool filled = false;
	
	if (!filled)
	{
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 256; j++)
			{
				uchar p2 = j & 1, p3 = j & 2, p4 = j & 4, p5 = j & 8, p6 = j & 16, \
					  p7 = j & 32, p8 = j & 64, p9 = j & 128;
			
				int C  = (!p2 & (p3 | p4)) + (!p4 & (p5 | p6)) + (!p6 & (p7 | p8)) + (!p8 & (p9 | p2));
				int N1 = (p9 | p2) + (p3 | p4) + (p5 | p6) + (p7 | p8);
				int N2 = (p2 | p3) + (p4 | p5) + (p6 | p7) + (p8 | p9);
				int N  = N1 < N2 ? N1 : N2;
				int m  = i == 0 ? ((p6 | p7 | !p9) & p8) : ((p2 | p3 | !p5) & p4);

				if (C == 1 && (N >= 2 && N <= 3) & (m == 0))
					look_up_table[i][j] = 1;
			}
		}
		filled = true;
	}

    do
    {
		GuoHallIteration_optimized(dst, look_up_table);
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
