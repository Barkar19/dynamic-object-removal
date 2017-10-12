#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <boost/filesystem.hpp>
#include <algorithm>

using namespace std;
using namespace cv;

Vec3b pixelMedian( vector<cv::Mat> mats, int x, int y)
{
    Vec3b result;
    vector<vector<int>> channels(3);
    for ( const auto& mat : mats )
    {
        channels[0].push_back( mat.at<Vec3b>(x,y)[0] );
        channels[1].push_back( mat.at<Vec3b>(x,y)[1] );
        channels[2].push_back( mat.at<Vec3b>(x,y)[2] );
    }
    for( unsigned i = 0; i < channels.size(); ++i )
    {
        auto& c = channels[i];
        const auto median_it = c.begin() + c.size() / 2;
        nth_element( c.begin(), median_it , c.end() );
        result[i] = *median_it;
    }
    return result;
}

int main( int argc, char** argv)
{

    boost::filesystem::path p(argv[1]);
    boost::filesystem::directory_iterator start(p);
    boost::filesystem::directory_iterator end;

    vector<cv::Mat> images;

    for( ; start != end; start++ )
    {
        const string file = start->path().string();
        images.push_back( imread( file ) );
        cout << file << endl;
    }

//    for( const auto& i : images )
//    {
////        imshow( "image", i);
//        waitKey(100);
//    }


    cv::Mat result(images.back().rows, images.back().cols, CV_8UC3, Scalar(128,0,128) );
    for( int x=0; x<result.rows; x++ )
    {
        for(int y=0; y<result.cols; y++)
        {
            result.at<Vec3b>(x,y) = pixelMedian( images, x,y);
//            cout << x << " " << y << endl;
        }
    }

//    imshow( "image", result);
    imwrite( string(argv[1]) + "out.jpg", result );
    waitKey();

    return 0;
}
