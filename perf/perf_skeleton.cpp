#include "opencv_ptest/include/opencv2/ts/ts.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>

#include "skeleton_filter.hpp"

using namespace std;
using namespace perf;
using namespace cv;
using std::tr1::make_tuple;
using std::tr1::get;

//
// Test(s) for the ConvertColor_BGR2GRAY_BT709 function
//

// PERF_TEST(skeleton, ConvertColor_BGR2GRAY_BT709)
// {
//     Mat input = cv::imread("./bin/testdata/sla.png");
//
//     // Add code here
// }

//
// Test(s) for the ImageResize function
//

#define MAT_SIZES  ::perf::szVGA, ::perf::sz720p, ::perf::sz1080p

typedef perf::TestBaseWithParam<Size> Size_Only;

PERF_TEST_P(Size_Only, ImageResize, testing::Values(MAT_SIZES))
{
    Size sz = GetParam();
    Size sz_to(sz.width / 2, sz.height / 2);

    cv::Mat src(sz, CV_8UC1), dst(Size(sz_to), CV_8UC1);
    declare.in(src, WARMUP_RNG).out(dst);

    TEST_CYCLE()
    {
        ImageResize(src, dst, sz_to);
    }

    SANITY_CHECK(dst, 1 + 1e-6);
}

//
// Test(s) for the skeletonize function
//

 #define IMAGES testing::Values( std::string("./bin/testdata/sla.png"),\
                                 std::string("./bin/testdata/page.png"),\
                                 std::string("./bin/testdata/schedule.png") )

 typedef perf::TestBaseWithParam<std::string> ImageName;

 PERF_TEST_P(ImageName, skeletonize, IMAGES)
 {
     Mat input = cv::imread(GetParam());

     // Add code here

     declare.iterations(10);
     declare.time(300);

     Mat output;

     TEST_CYCLE()
     {
         skeletonize(input, output, false);
     }
     SANITY_CHECK(output,1 + 1e-6);
 }

//GuoHallThinning
#define IMAGES testing::Values( std::string("./bin/testdata/sla.png"),\
                                std::string("./bin/testdata/page.png"),\
                                std::string("./bin/testdata/schedule.png") )

typedef perf::TestBaseWithParam<std::string> ImageName;

PERF_TEST_P(ImageName, GuoHallThinning, IMAGES)
{
    Mat input = cv::imread(GetParam());

    // Add code here

    declare.iterations(10);
    declare.time(300);

    Mat output,gray_image;

    ConvertColor_BGR2GRAY_BT709(input, gray_image);

    // Downscale input image
    cv::Mat small_image;
    cv::Size small_size(input.cols / 1.5, input.rows / 1.5);
    ImageResize(gray_image, small_image, small_size);

    // Binarization and inversion
    cv::threshold(small_image, small_image, 128, 255, cv::THRESH_BINARY_INV);

    TEST_CYCLE()
    {
         GuoHallThinning(small_image, output);
    }
    SANITY_CHECK(output,1 + 1e-6);
}

//BGR2GRAY_BT709
#define IMAGES testing::Values( std::string("./bin/testdata/sla.png"),\
                                std::string("./bin/testdata/page.png"),\
                                std::string("./bin/testdata/schedule.png") )

typedef perf::TestBaseWithParam<std::string> ImageName;

PERF_TEST_P(ImageName, BGR2GRAY_BT709, IMAGES)
{
    Mat input = cv::imread(GetParam());

    // Add code here

    declare.iterations(10);
    declare.time(300);

    Mat output;

    TEST_CYCLE()
    {
         ConvertColor_BGR2GRAY_BT709(input, output);
    }
    SANITY_CHECK(output,1 + 1e-6);
}
