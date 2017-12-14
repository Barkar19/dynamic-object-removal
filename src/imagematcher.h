#ifndef IMAGEMATCHER_H
#define IMAGEMATCHER_H

#include "opencv2/opencv.hpp"
#include "opencv2/xfeatures2d.hpp"

using namespace std;
using namespace cv;
using namespace xfeatures2d;

struct FrameCoordinates
{
    Point2f top_left;
    Point2f top_right;
    Point2f bottom_left;
    Point2f bottom_right;
};

struct FrameMatch
{
    Mat homography; // homography to dst coordinates
    FrameCoordinates src_coords; // 4 cornes of original frame
    unsigned srcID;  // id of frame
};

class ImageMatcher
{
public:
    ImageMatcher();

public:
    vector<Mat> matchTo(Mat dst, vector<Mat> input ) const;
    std::pair<Mat, Mat> unifyCoordinates(Mat src, Mat dst) const;
    void matchFrames(const Mat &dst, vector<Mat> &input ) const;

private:
    FrameMatch getFrameMatch(const Mat &src, const Mat &dst) const;

    vector<Point2f> unifyFrameMatches( vector<FrameMatch> & input ) const;

    Mat getHomography(const Mat &img_1, const Mat &dst) const;

    float getMinX(const FrameMatch &match) const;
    float getMaxX(const FrameMatch &match) const;
    float getMinY(const FrameMatch &match) const;
    float getMaxY(const FrameMatch &match) const;

};

#endif // IMAGEMATCHER_H
