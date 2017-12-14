#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <opencv2/core.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace cv;

class ImageProvider
{
public:
    ImageProvider();

    vector<Mat> getImagesFromFolder( string folderName );
    vector<Mat> getImagesFromVideo( string videoName );
};

#endif // IMAGEPROVIDER_H
