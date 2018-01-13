#ifndef DYNAMICOBJECTREMOVER_H
#define DYNAMICOBJECTREMOVER_H

#include <opencv2/core.hpp>
#include <vector>
#include <vector>

#include "imageprocesspipe.h"

using namespace std;
using namespace cv;

struct PixelDescriptor
{
    unsigned    id;
    Vec3b       vec;
    double      value;
    PixelDescriptor() = default;
    PixelDescriptor( unsigned i, Vec3b v, double d ):
        id(i),vec(v),value(d){}
};

typedef vector<PixelDescriptor> DescriptorsVector;

class DynamicObjectRemover
{
public:
    DynamicObjectRemover();

    cv::Mat reomveDynamicObjects(const vector<Mat>& images );

    void SetProcessPipe( ImageProcessPipe aPipe );

private:

    Vec3b processPixel(vector<cv::Mat> mats, int x, int y);

    void changeDescriptorsToChannelValue(DescriptorsVector& descriptors, unsigned channelId );
    void changeDescriptorsToLength(DescriptorsVector& descriptors, float minkowskiFactor);
    void changeDescriptorsToVectorDiffsValue( DescriptorsVector& descriptors, float minkowskiFactor );

    void reduceDescriptors(DescriptorsVector& descriptors, std::function<DescriptorsVector(DescriptorsVector)> fun, unsigned size );

    static pair<double, double> calculateStats(const DescriptorsVector &descriptors);
    static DescriptorsVector calculateDescriptorsMedian( DescriptorsVector descriptors );
    static DescriptorsVector calculateDescriptorsMean( DescriptorsVector descriptors );
    static DescriptorsVector removeOutliersDescriptors( DescriptorsVector descriptors, double factor );
    static DescriptorsVector removeSubvectors( DescriptorsVector descriptors, double factor );
    static DescriptorsVector findMostStableSubvector( DescriptorsVector descriptors, unsigned factor );

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

    ImageProcessPipe _oProcessPipe;

};

#endif // DYNAMICOBJECTREMOVER_H
