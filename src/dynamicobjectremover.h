#ifndef DYNAMICOBJECTREMOVER_H
#define DYNAMICOBJECTREMOVER_H

#include <opencv2/core.hpp>
#include <vector>

using namespace std;
using namespace cv;

class DynamicObjectRemover
{
public:
    DynamicObjectRemover();

    cv::Mat reomveDynamicObjects(const vector<Mat>& images );
private:
    Vec3b pixelMedian(vector<cv::Mat> mats, int x, int y);
};

#endif // DYNAMICOBJECTREMOVER_H
