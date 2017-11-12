#include "imageprovider.h"

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace boost;
using namespace filesystem;
ImageProvider::ImageProvider()
{

}

vector<Mat> ImageProvider::getImagesFromFolder(string folderName)
{


    vector<cv::Mat> images;

    directory_iterator it{folderName};
    while (it != directory_iterator{})
    {
        if( is_regular_file( *it) )
        {
            const string file = it->path().string();
//            std::cout << file << endl;
            Mat img = imread( file );
            Size maxSize(1920,1080);
            if ( img.size().area() > maxSize.area() )
            {
                float ratio = sqrt( float(maxSize.area()) / img.size().area() );
                resize( img,img, Size(), ratio, ratio);
            }
            images.push_back( img );
        }
        it++;
    }
    return images;
}
