#include <gtest/gtest.h>

#include "skeleton_filter.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

using namespace cv;

int numberOfDifferentPixels(const cv::Mat &m1, const cv::Mat &m2)
{
    return countNonZero(m1 - m2);
}

int maxDifference(const cv::Mat &m1, const cv::Mat &m2)
{
    Mat diff;
    absdiff(m1, m2, diff);

    double max_diff;
    minMaxLoc(diff, 0, &max_diff);

    return (int)max_diff;
}

TEST(skeleton, cvtcolor_matches_opencv)
{
    // Arrange
    Mat bgr(5, 5, CV_8UC3);
    randu(bgr, Scalar::all(0), Scalar::all(255));

    // Act
    Mat result;
    ConvertColor_BGR2GRAY_BT709(bgr, result);

    // Assert
    Mat xyz;
    cvtColor(bgr, xyz, CV_BGR2XYZ);
    vector<Mat> planes;
    split(xyz, planes);
    Mat reference = planes[1];
    // std::cout << "Difference:\n" << reference - result << std::endl;
    EXPECT_EQ(0, numberOfDifferentPixels(reference, result));
}

TEST(skeleton, resize_matches_opencv)
{
    // Arrange
    Mat image(10, 10, CV_8UC1);
    randu(image, Scalar(0), Scalar(255));
    Size sz(image.cols / 2, image.rows / 2);

    // Act
    Mat result;
    ImageResize(image, result, sz);

    // Assert
    Mat reference;
    resize(image, reference, sz);
    // std::cout << "Difference:\n" << reference - result << std::endl;
    EXPECT_LT(maxDifference(reference, result), 2);
}
TEST(skeleton, 2_plus_2_equals_4)
{
	EXPECT_EQ(4, 2 + 2);
}
TEST(skeleton, convert_colour_size_matches)
{
	Mat image(10, 10, CV_8UC3);
	randu(image, Scalar::all(0), Scalar::all(255));
	Mat result;
	ConvertColor_BGR2GRAY_BT709(image, result);
	EXPECT_EQ(image.rows, result.rows);
	EXPECT_EQ(image.cols, result.cols);
}
TEST(skeleton, guohall_size_matches)
{
	Mat image(10, 10, CV_8UC1);
	randu(image, Scalar(0), Scalar(255));
	Mat result;
	GuoHallThinning(image, result);
	EXPECT_EQ(image.rows, result.rows);
	EXPECT_EQ(image.cols, result.cols);
}
TEST(skeleton, img_resize_argument_matches_size)
{
	// Arrange
	Mat image(10, 10, CV_8UC1);
	randu(image, Scalar(0), Scalar(255));
	Size sz(image.cols / 2, image.rows / 2);

	// Act
	Mat result;
	ImageResize(image, result, sz);
	EXPECT_EQ(result.rows, sz.height);
	EXPECT_EQ(result.cols, sz.width);
}
TEST(skeleton, resize_colour_matches)
{
	int n = 10, m = 10, val = 42;
	Mat image(n, m, CV_8UC1, Scalar(val));
	Mat cmp(n / 2, m / 2, CV_8UC1, Scalar(val));
	Size sz(n / 2, m / 2);

	Mat result;
	ImageResize(image, result, sz);
	Mat subtr = abs(cmp - result);
	
	EXPECT_EQ(0, countNonZero(subtr));
}
TEST(skeleton, convert_trichannel_colour_matches)
{
	Mat image(10, 10, CV_8UC3, Scalar::all(42));
	Mat result;
	ConvertColor_BGR2GRAY_BT709(image, result);

	uchar colour = result.at<uchar>(0, 0);
	Mat templ(10, 10, CV_8UC1, Scalar(colour));
	Mat diff = abs(result - templ);

	EXPECT_EQ(0, countNonZero(diff));
}