#include "dynamicobjectremover.h"
#include <iostream>
#include <opencv2/opencv.hpp>

DynamicObjectRemover::DynamicObjectRemover()
{

}

cv::Mat DynamicObjectRemover::reomveDynamicObjects(const vector<Mat> &images)
{
    cv::Mat result(images.back().rows, images.back().cols, CV_8UC3, Scalar(128,0,128) );
    int i = 0;
    for( int x=0; x<result.rows; x++ )
    {
        for(int y=0; y<result.cols; y++)
        {
            result.at<Vec3b>(x,y) = pixelMedian( images, x,y);
//            cout << x << " " << y << endl;
            cout << double(i) / double(result.rows * result.cols * 1.0) << endl;
            i++;
        }
    }
    cvtColor( result, result, CV_HLS2BGR );
    return result;
}

Vec3b DynamicObjectRemover::pixelMedian( vector<cv::Mat> mats, int x, int y)
{
    Vec3b result;
    vector<vector<int>> channels(3, vector<int>(mats.size() ) );
    for ( unsigned i = 0; i < mats.size(); ++i )
    {
        const auto& mat = mats[i];
        const auto& pixel = mat.at<Vec3b>(x,y);
//        cvtColor( pixel, pixel, CV_BGR2HLS );
        channels[0][i] = pixel[0];
        channels[1][i] = pixel[1];
        channels[2][i] = pixel[2];
    }

    channels[0].erase( std::remove( channels[0].begin(), channels[0].end(), 0), channels[0].end());
    channels[1].erase( std::remove( channels[1].begin(), channels[1].end(), 0), channels[1].end());
    channels[2].erase( std::remove( channels[2].begin(), channels[2].end(), 0), channels[2].end());

    for( unsigned i = 0; i < channels.size(); ++i )
    {
        auto& c = channels[i];
        if ( c.size() )
        {
            const auto median_it = c.begin() + c.size() / 2;
            nth_element( c.begin(), median_it , c.end() );
            result[i] = *median_it;
        }
        else
        {
            result[i] = 0;
        }
    }
    return result;
}
