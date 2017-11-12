#ifndef IMAGEMATCHER_H
#define IMAGEMATCHER_H

#include "opencv2/opencv.hpp"
#include "opencv2/xfeatures2d.hpp"

using namespace std;
using namespace cv;
using namespace xfeatures2d;

class ImageMatcher
{
public:
    ImageMatcher();

public:
    vector<Mat> matchTo( Mat dst, vector<Mat> input ) const;
    std::pair<Mat, Mat> unifyCoordinates(Mat src, Mat dst) const;
private:
    Mat getHomography(Mat img_1, Mat img_2) const;


};

#endif // IMAGEMATCHER_H
