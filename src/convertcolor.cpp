#include "skeleton_filter.hpp"

#if defined __SSSE3__  || (defined _MSC_VER && _MSC_VER >= 1500)
#  include "tmmintrin.h"
#  define HAVE_SSE
#endif

#include <string>
#include <sstream>

// Function for debug prints
template <typename T>
std::string __m128i_toString(const __m128i var)
{
    std::stringstream sstr;
    const T* values = (const T*) &var;
    if (sizeof(T) == 1)
    {
        for (unsigned int i = 0; i < sizeof(__m128i); i++)
        {
            sstr << (int) values[i] << " ";
        }
    }
    else
    {
        for (unsigned int i = 0; i < sizeof(__m128i) / sizeof(T); i++)
        {
            sstr << values[i] << " ";
        }
    }
    return sstr.str();
}

void ConvertColor_BGR2GRAY_BT709(const cv::Mat& src, cv::Mat& dst)
{
    CV_Assert(CV_8UC3 == src.type());
    cv::Size sz = src.size();
    dst.create(sz, CV_8UC1);

    const int bidx = 0;

    for (int y = 0; y < sz.height; y++)
    {
        const cv::Vec3b *psrc = src.ptr<cv::Vec3b>(y);
        uchar *pdst = dst.ptr<uchar>(y);

        for (int x = 0; x < sz.width; x++)
        {
            float color = 0.2126 * psrc[x][2-bidx] + 0.7152 * psrc[x][1] + 0.0722 * psrc[x][bidx];
            pdst[x] = (int)(color + 0.5);
        }
    }
}

void ConvertColor_BGR2GRAY_BT709_fpt(const cv::Mat& src, cv::Mat& dst)
{
    CV_Assert(CV_8UC3 == src.type());
    cv::Size sz = src.size();
    dst.create(sz, CV_8UC1);

    const int bidx = 0;

    int shift = 8;
    int bias  = 0;

    unsigned rw = (unsigned)(0.2126 * (1 << shift) + 0.5);
    unsigned gw = (unsigned)(0.7152 * (1 << shift) + 0.5);
    unsigned bw = (unsigned)(0.0722 * (1 << shift) + 0.5);

    for (int y = 0; y < sz.height; y++)
    {
        const cv::Vec3b *psrc = src.ptr<cv::Vec3b>(y);
        uchar *pdst = dst.ptr<uchar>(y);

        for (int x = 0; x < sz.width; x++)
        {

            pdst[x] = (rw * psrc[x][2] + gw * psrc[x][1] + bw * psrc[x][0] + (1<<(shift-1)) + bias) >> shift;

        }
    }
}

void ConvertColor_BGR2GRAY_BT709_simd(const cv::Mat& src, cv::Mat& dst)
{
    CV_Assert(CV_8UC3 == src.type());
    cv::Size sz = src.size();
    dst.create(sz, CV_8UC1);

#ifdef HAVE_SSE
    __m128i ssse3_blue_indices_0  = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, 12,  9,  6,  3,  0);
    __m128i ssse3_blue_indices_1  = _mm_set_epi8(-1, -1, -1, -1, -1, 14, 11,  8,  5,  2, -1, -1, -1, -1, -1, -1);
    __m128i ssse3_blue_indices_2  = _mm_set_epi8(13, 10,  7,  4,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    __m128i ssse3_green_indices_0 = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 13, 10,  7,  4,  1);
    __m128i ssse3_green_indices_1 = _mm_set_epi8(-1, -1, -1, -1, -1, 15, 12,  9,  6,  3,  0, -1, -1, -1, -1, -1);
    __m128i ssse3_green_indices_2 = _mm_set_epi8(14, 11,  8,  5,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    __m128i ssse3_red_indices_0   = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, 11,  8,  5,  2);
    __m128i ssse3_red_indices_1   = _mm_set_epi8(-1, -1, -1, -1, -1, -1, 13, 10,  7,  4,  1, -1, -1, -1, -1, -1);
    __m128i ssse3_red_indices_2   = _mm_set_epi8(15, 12,  9,  6,  3,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    __m128i red_coeff   = _mm_set1_epi16(54);
    __m128i green_coeff = _mm_set1_epi16(183);
    __m128i blue_coeff  = _mm_set1_epi16(19); // 19 instead of 18 because the sum of 3 coeffs must be == 256
    __m128i bias = _mm_set1_epi16(128);
    __m128i zero = _mm_setzero_si128();

    __m128i red_lo = _mm_setzero_si128();
    __m128i red_hi = _mm_setzero_si128();

    __m128i green_lo = _mm_setzero_si128();
    __m128i green_hi = _mm_setzero_si128();

    __m128i blue_lo = _mm_setzero_si128();
    __m128i blue_hi = _mm_setzero_si128();

    __m128i gray_lo = _mm_setzero_si128();
    __m128i gray_hi = _mm_setzero_si128();
#endif

    for (int y = 0; y < sz.height; y++)
    {
        const uchar *psrc = src.ptr<uchar>(y);
        uchar *pdst = dst.ptr<uchar>(y);

        int x = 0;

#ifdef HAVE_SSE
        // Here is 16 times unrolled loop for vector processing
        for (; x <= sz.width - 16; x += 16)
        {
            __m128i chunk0 = _mm_loadu_si128((const __m128i*)(psrc + x*3));
            __m128i chunk1 = _mm_loadu_si128((const __m128i*)(psrc + x*3 + 16));
            __m128i chunk2 = _mm_loadu_si128((const __m128i*)(psrc + x*3 + 32));

            __m128i red = _mm_or_si128(_mm_or_si128(_mm_shuffle_epi8(chunk0, ssse3_red_indices_0),
                                                    _mm_shuffle_epi8(chunk1, ssse3_red_indices_1)),
                                                    _mm_shuffle_epi8(chunk2, ssse3_red_indices_2));


            __m128i green = _mm_or_si128(_mm_or_si128(_mm_shuffle_epi8(chunk0, ssse3_green_indices_0),
                                                    _mm_shuffle_epi8(chunk1, ssse3_green_indices_1)),
                                                    _mm_shuffle_epi8(chunk2, ssse3_green_indices_2));


            __m128i blue = _mm_or_si128(_mm_or_si128(_mm_shuffle_epi8(chunk0, ssse3_blue_indices_0),
                                                    _mm_shuffle_epi8(chunk1, ssse3_blue_indices_1)),
                                                    _mm_shuffle_epi8(chunk2, ssse3_blue_indices_2));


            red_lo = _mm_unpacklo_epi8(red,zero);
            red_hi = _mm_unpackhi_epi8(red,zero);

            green_lo = _mm_unpacklo_epi8(green,zero);
            green_hi = _mm_unpackhi_epi8(green,zero);

            blue_lo = _mm_unpacklo_epi8(blue,zero);
            blue_hi = _mm_unpackhi_epi8(blue,zero);

            red_lo = _mm_mullo_epi16(red_lo, red_coeff);
            green_lo = _mm_mullo_epi16(green_lo, green_coeff);
            blue_lo = _mm_mullo_epi16(blue_lo, blue_coeff);

            red_hi = _mm_mullo_epi16(red_hi, red_coeff);
            green_hi = _mm_mullo_epi16(green_hi, green_coeff);
            blue_hi = _mm_mullo_epi16(blue_hi, blue_coeff);

            gray_lo = _mm_add_epi16(_mm_add_epi16(red_lo,green_lo),blue_lo);
            gray_hi = _mm_add_epi16(_mm_add_epi16(red_hi,green_hi),blue_hi);

            gray_lo = _mm_srli_epi16(gray_lo,8);
            gray_hi = _mm_srli_epi16(gray_hi,8);

            __m128i gray_packed  = _mm_packus_epi16(gray_lo,gray_hi);


            _mm_storeu_si128((__m128i*)(pdst + x),gray_packed );
        }
#endif

        // Process leftover pixels
        for (; x < sz.width; x++)
        {
            float color = 0.2126 * psrc[3 * x + 2] + 0.7152 * psrc[3 * x + 1] + 0.0722 * psrc[3 * x];
            pdst[x] = (int)(color + 0.5);
        }
    }

}


//Geometric mean

//             Name of Test                 base     base      base
//                                          bgr      bgr       bgr
//                                          fpt      intr      intr
//                                                              vs
//                                                             base
//                                                             bgr
//                                                             fpt
//                                                          (x-factor)
//ConvertColor_simd::Size_Only::640x480   0.807 ms 0.195 ms    4.13
//ConvertColor_simd::Size_Only::1280x720  2.501 ms 0.705 ms    3.55
//ConvertColor_simd::Size_Only::1920x1080 5.689 ms 1.713 ms    3.32
