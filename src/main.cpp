#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <boost/filesystem.hpp>
#include <algorithm>

#include "dynamicobjectremover.h"
#include "imageprovider.h"

using namespace std;
using namespace cv;



int main( int argc, char** argv)
{
    ImageProvider provider;
    DynamicObjectRemover remover;

    Mat out = remover.reomveDynamicObjects( provider.getImagesFromFolder( argv[1] ) );

    imwrite( string(argv[1]) + "_out.jpg", out );
    return 0;
}
