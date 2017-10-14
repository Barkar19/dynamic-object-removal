#include "imageprovider.h"

#include <opencv2/highgui.hpp>

ImageProvider::ImageProvider()
{

}

vector<Mat> ImageProvider::getImagesFromFolder(string folderName)
{
    boost::filesystem::path p( folderName );
    boost::filesystem::directory_iterator start(p);
    boost::filesystem::directory_iterator end;

    vector<cv::Mat> images;

    for( ; start != end; start++ )
    {
        const string file = start->path().string();
        images.push_back( imread( file ) );
    }
    return images;
}
