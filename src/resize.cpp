#include "skeleton_filter.hpp"
#include <math.h>

void ImageResize(const cv::Mat &src, cv::Mat &dst, const cv::Size sz)
{
    CV_Assert(CV_8UC1 == src.type());
    cv::Size sz_src = src.size();
    dst.create(sz, src.type());

    const int src_rows = src.rows;
    const int src_cols = src.cols;

    const int dst_rows = sz.height;
    const int dst_cols = sz.width;

    for (int row = 0; row < dst_rows; row++)
    {
        uchar *ptr_dst = dst.ptr<uchar>(row);

        for (int col = 0; col < dst_cols; col++)
        {
            const float x = ((float)col + .5f) * sz_src.width  / sz.width  - .5f;
            const float y = ((float)row + .5f) * sz_src.height / sz.height - .5f;

            const int ix = (int)floor(x);
            const int iy = (int)floor(y);

            const int x1 = (ix < 0) ? 0 : ((ix >= src_cols) ? src_cols - 1 : ix);
            const int x2 = (ix < 0) ? 0 : ((ix >= src_cols - 1) ? src_cols - 1 : ix + 1);
            const int y1 = (iy < 0) ? 0 : ((iy >= src_rows) ? src_rows - 1 : iy);
            const int y2 = (iy < 0) ? 0 : ((iy >= src_rows - 1) ? src_rows - 1 : iy + 1);

            const uchar q11 = src.at<uchar>(y1, x1);
            const uchar q12 = src.at<uchar>(y2, x1);
            const uchar q21 = src.at<uchar>(y1, x2);
            const uchar q22 = src.at<uchar>(y2, x2);

            const int temp = ((x1 == x2) && (y1 == y2)) ? (int)q11 :
                             ( (x1 == x2) ? (int)(q11 * (y2 - y) + q22 * (y - y1)) :
                               ( (y1 == y2) ? (int)(q11 * (x2 - x) + q22 * (x - x1)) : 
                                 (int)(q11 * (x2 - x) * (y2 - y) + q21 * (x - x1) * (y2 - y) + q12 * (x2 - x) * (y - y1) + q22 * (x - x1) * (y - y1))));
            ptr_dst[col] = (temp < 0) ? 0 : ((temp > 255) ? 255 : (uchar)temp);
        }
    }
}

void ImageResize_optimized(const cv::Mat &src, cv::Mat &dst, const cv::Size sz)
{
    CV_Assert(CV_8UC1 == src.type());
    dst.create(sz, CV_8UC1);

    const int dst_rows = sz.height;
    const int dst_cols = sz.width;

    const float scale_x = (float)(src.cols) / dst_cols;
    const float scalar_x0 = 0.5f * scale_x  - 0.5f;
    const float scale_y = (float)(src.rows) / dst_rows;;
    const float scalar_y0 = 0.5f * scale_y  - 0.5f;

    // (col+0.5)*scale-0.5 < 0
    int last_negative_row = (int)(0.5f / scale_y - 0.5f);
    int last_negative_col = (int)(0.5f / scale_x - 0.5f);

    // Negative rows.
    const uchar* upper_row = src.ptr<uchar>(0);
    for (int row = 0; row < last_negative_row; ++row)
    {
        uchar* ptr_dst = dst.ptr<uchar>(row);
        const float y = row * scale_y + scalar_y0;
        
        // Negative cols.
        memset(ptr_dst, upper_row[0], last_negative_col);

        // Positive cols.
        for (int col = last_negative_col; col < dst_cols; ++col)
        {
            const float x = col * scale_x + scalar_x0;
            const int x1 = (int)x;
            const int x2 = x1 + 1;

            ptr_dst[col] = upper_row[x1] * (x2 - x) + upper_row[x2] * (x - x1);
        }
    }

    // Positive rows.
    for (int row = last_negative_row; row < dst_rows; row++)
    {
        uchar *ptr_dst = dst.ptr<uchar>(row);

        const float y = row * scale_y + scalar_y0;
        const int y1 = (int)y;
        const int y2 = y1 + 1;
        upper_row = src.ptr<uchar>(y1);
        const uchar* lower_row = src.ptr<uchar>(y2);

        // Negative cols.
        memset(ptr_dst,  upper_row[0] * (y2 - y) + lower_row[0] * (y - y1), last_negative_col);

        // Positive cols.
        for (int col = last_negative_col; col < dst_cols; ++col)
        {
            const float x = col * scale_x + scalar_x0;
            const int x1 = (int)x;
            const int x2 = x1 + 1;

            ptr_dst[col] = upper_row[x1] * (x2 - x) * (y2 - y) + upper_row[x2] * (x - x1) * (y2 - y) +
                           lower_row[x1] * (x2 - x) * (y - y1) + lower_row[x2] * (x - x1) * (y - y1);
        }
    }
}
