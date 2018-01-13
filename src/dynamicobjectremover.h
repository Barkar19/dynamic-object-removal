#ifndef DYNAMICOBJECTREMOVER_H
#define DYNAMICOBJECTREMOVER_H

#include <opencv2/core.hpp>
#include <vector>
#include <vector>

using namespace std;
using namespace cv;

class DynamicObjectRemover
{
public:
    DynamicObjectRemover();

    cv::Mat reomveDynamicObjects(const vector<Mat>& images );

private:
    Vec3b vectorMedian(vector<cv::Mat> mats, int x, int y);
    Vec3b pixelMedian(vector<cv::Mat> mats, int x, int y);
    Vec3b differentialPixel(vector<cv::Mat> mats, int x, int y);
    Vec3b pixelMedianOfMedians(vector<cv::Mat> mats, int x, int y);

    static Vec3b medianVectorLength(vector<Vec3b> v);
    static Vec3b medianChannel(vector<Vec3b> v);
    static Vec3b meanVectorLength(vector<cv::Vec3b> v);

    static Vec3b medianVector(vector<cv::Vec3b> v);
    static Vec3b meanChannel( vector<cv::Vec3b> v);

    vector<Vec3b> reduce(vector<Vec3b> vec, std::function<vector<Vec3b>(vector<Vec3b>)> fun, int size);
    vector<Vec3b> reduce(vector<cv::Vec3b> vec, std::function<Vec3b (vector<cv::Vec3b>)> fun, int size);
    static vector<Vec3b> removeOutliers(vector<cv::Vec3b> vecs, double factor);
    static vector<Vec3b> removeStdev(vector<cv::Vec3b> vecs, double factor);
    static vector<Vec3b> stableStdev(vector<cv::Vec3b> vecs, unsigned size);
};

#endif // DYNAMICOBJECTREMOVER_H
