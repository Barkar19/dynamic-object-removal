#include "imagematcher.h"
#include <limits>


ImageMatcher::ImageMatcher()
{

}

vector<Mat> ImageMatcher::matchTo(Mat dst, vector<Mat> input) const
{
    int i = 0;
    auto currentSize = dst.size();
    int counter = 0;
    for ( const auto& img : input )
    {
        auto r = unifyCoordinates( img, dst );
        dst = r.second;
        cerr << "IMAGE MATCHED TO " << ++i ;
        cerr << dst.size() << endl;
        if ( currentSize == dst.size() )
        {
            if( counter == input.size() / 3 )
                break;
            else
                counter++;
        }
        else
        {
            currentSize = dst.size();
            counter = 0;
        }
    }
    i = input.size();
    vector<Mat> result;
    for ( const auto& img : input )
    {
        auto r = unifyCoordinates( img, dst );
        result.push_back( r.first );
        cerr << "IMAGE ADDED " << --i ;
    }
    return result;
}

Mat ImageMatcher::getHomography( const Mat & src, const Mat & dst ) const
{
    Mat img_1,img_2;
    cvtColor( src, img_1, CV_BGR2GRAY );
    cvtColor( dst, img_2, CV_BGR2GRAY );

    //-- Step 1: Detect the keypoints
//    Ptr<BRISK> detector = BRISK::create();
//    Ptr<KAZE> detector = KAZE::create();
    Ptr<AKAZE> detector = AKAZE::create();
//    Ptr<ORB> detector = ORB::create();
//    Ptr<SIFT> detector = SIFT::create();
//    Ptr<SURF> detector = SURF::create();

    std::vector<KeyPoint> keypoints_1, keypoints_2;
    Mat descriptors_1, descriptors_2;

//    if(descriptors_1.type()!=CV_32F) {
//        descriptors_1.convertTo(descriptors_1, CV_32F);
//    }

//    if(descriptors_2.type()!=CV_32F) {
//        descriptors_2.convertTo(descriptors_2, CV_32F);
//    }
    detector->detect(img_1, keypoints_1);
    detector->detect(img_2, keypoints_2);

    detector->compute(img_1, keypoints_1, descriptors_1);
    detector->compute(img_2, keypoints_2, descriptors_2);


    //-- Step 2: Matching descriptor vectors using FLANN matcher
    BFMatcher matcher/*(new flann::LshIndexParams(20, 10, 2))*/;
    std::vector< std::vector<DMatch> > matches;
//    matcher.match(descriptors_1, descriptors_2, matches);
    matcher.knnMatch(descriptors_1, descriptors_2, matches, 2);

    std::vector<DMatch> good_matches;
    for ( auto& m : matches )
    {
        if( m.size() == 2 && m[0].distance < m[1].distance * 1.75 )
        {
            good_matches.push_back( DMatch( m[0].queryIdx, m[0].trainIdx, m[0].distance ));
        }
    }



    sort(good_matches.begin(), good_matches.end(),
         [](const DMatch &a, const DMatch &b) -> bool {
           return a.distance < b.distance;
         });

//    good_matches = std::vector<DMatch>( good_matches.begin(), good_matches.begin() + ( good_matches.size() > 300 ? 300 : good_matches.size() ) );


//    good_matches.push_back( matches[0] );
//    good_matches.push_back( matches[1] );
//    good_matches.push_back( matches[2] );
//    good_matches.push_back( matches[3] );

//    for (int i = 4; i < matches.size(); i++)
//    {
//        bool addMatch = true;
////        for( const auto m : good_matches )
////        {
//////            cerr << "\nDIST: " <<norm(keypoints_1[ m.queryIdx ].pt - keypoints_1[ matches[i].queryIdx ].pt );
////            if ( norm(keypoints_1[ m.queryIdx ].pt - keypoints_1[ matches[i].queryIdx ].pt ) < 50 )
////            {
////                addMatch = false;
////                break;
////            }
////        }
//        if ( addMatch )
//        {
////            cout << "DIST: " << matches[i].distance << "\n";
//            if ( matches[i].distance <= 2 * matches[0].distance)
//                good_matches.push_back(matches[i]);
////            if( good_matches.size() == 4)
////            {
////                break;
////            }
//        }
//    }
//    cerr << "SIZE: " << good_matches.size();
//    Mat img_matches;
//    drawMatches(img_1, keypoints_1, img_2, keypoints_2, vector<DMatch>( matches.begin(), matches.begin() + 20), img_matches,
//                Scalar::all(-1), Scalar::all(-1), vector<char>(),
//                DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
//    //-- Show detected matches
//    imshow("Matches", img_matches);
//    waitKey(0);
    vector<Point2f> source_points, destination_points;

    for (size_t i = 0; i < good_matches.size(); i++) {
      Point2f src_p = keypoints_1[good_matches[i].queryIdx].pt;
      Point2f dst_p = keypoints_2[good_matches[i].trainIdx].pt;

      source_points.push_back(src_p);
      destination_points.push_back(dst_p);
    }

    Mat result;
    if ( false )
    {
        result = getPerspectiveTransform(source_points, destination_points);
    }
    else if ( true )
    {
        result =  findHomography(source_points, destination_points, noArray(), CV_RANSAC, 4.0 );
    }
    else
    {
        // TO DO??? NOT WORKING
        result = estimateRigidTransform(source_points,destination_points,false);

        // extend rigid transformation to use perspectiveTransform:
        cv::Mat H = cv::Mat(3,3,result.type());
        H.at<double>(0,0) = 1.0;//result.at<double>(0,0);
        H.at<double>(0,1) = 0.0;//result.at<double>(0,1);
        H.at<double>(0,2) = result.at<double>(0,2);

        H.at<double>(1,0) = 0.0;//result.at<double>(1,0);
        H.at<double>(1,1) = 1.0;//result.at<double>(1,1);
        H.at<double>(1,2) = 0.0;//result.at<double>(1,2);

        H.at<double>(2,0) = 0.0;
        H.at<double>(2,1) = 0.0;
        H.at<double>(2,2) = 1.0;
        result = H;
    }
    return result;
}

std::pair<Mat,Mat> ImageMatcher::unifyCoordinates(Mat src, Mat dst) const
{
    Mat transf = getHomography(src, dst);
    int src_w = src.cols;
    int src_h = src.rows;
    float data[] = { 0, src_w, src_w, 0,
                     0, 0, src_h, src_h,
                     1, 1, 1, 1};
    transf.convertTo( transf, CV_32F);

    Mat lin_homg_pts = Mat( 3,4, CV_32F, data );

    Mat top_left_transf = transf * lin_homg_pts.col(0);
    top_left_transf /= top_left_transf.at<float>(0,2);

    Mat top_right_transf = transf * lin_homg_pts.col(1);
    top_right_transf /= top_right_transf.at<float>(0,2);

    Mat bottom_left_transf = transf * lin_homg_pts.col(2);
    bottom_left_transf /= bottom_left_transf.at<float>(0,2);

    Mat bottom_right_transf = transf * lin_homg_pts.col(3);
    bottom_right_transf /= bottom_right_transf.at<float>(0,2);

    vector<float> x_coords = {  top_left_transf.at<float>(0,0),
                                top_right_transf.at<float>(0,0),
                                bottom_left_transf.at<float>(0,0),
                                bottom_right_transf.at<float>(0,0)};

    vector<float> y_coords = {  top_left_transf.at<float>(0,1),
                                top_right_transf.at<float>(0,1),
                                bottom_left_transf.at<float>(0,1),
                                bottom_right_transf.at<float>(0,1)};

    float minX = *std::min_element( x_coords.begin(), x_coords.end() );
    float maxX = *std::max_element( x_coords.begin(), x_coords.end() );
    float minY = *std::min_element( y_coords.begin(), y_coords.end() );
    float maxY = *std::max_element( y_coords.begin(), y_coords.end() );

//    cout << "minX = " << minX << endl;
//    cout << "maxX = " << maxX << endl;
//    cout << "minY = " << minY << endl;
//    cout << "maxY = " << maxY << endl;

    Size new_size( std::floor( std::max( maxX, float(dst.cols)) - std::min( float(0.0), minX) ),
                   std::floor( std::max( maxY, float(dst.rows)) - std::min( float(0.0), minY) ) );

//    cout << "size: " << new_size << endl;
//    cout << "src.cols = " << dst.cols << endl;
//    cout << "dst.rows = " << dst.rows << endl;

    float data1[] = { 1.0, 0, 0,
                     0, 1, 0,
                     0, 0, 1};
    Mat new_transf( 3,3, CV_32FC1, data1);

    int anchorX = 0, anchorY = 0;
    if ( minX < 0)
    {
        anchorX = -minX;
        new_transf.at<float>(0,2) += anchorX;
    }
    if ( minY < 0)
    {
        anchorY = -minY;
        new_transf.at<float>(1,2) += anchorY;
    }

//    cout << "\n\nT1" << transf;
//    cout << "\n\nT2" << new_transf;
    new_transf = new_transf *  transf;
    new_transf /= new_transf.at<float>(2,2);

//    cout << "size: " << new_size << endl;
//    cout << "anchorX = " << anchorX << endl;
//    cout << "anchorY = " << anchorY << endl;
//    cout << "dst.cols + anchorX = " << dst.cols + anchorX << endl;
//    cout << "dst.rows + anchorY = " << dst.rows + anchorY << endl;
//    cout << "dst.cols = " << dst.cols << endl;
//    cout << "dst.rows = " << dst.rows << endl;


    Mat output_dst = Mat::zeros( new_size, src.type() );
    dst.copyTo( output_dst(cv::Rect(anchorX,anchorY,dst.cols,dst.rows)));

    Mat output_src;
    warpPerspective( src, output_src, new_transf, new_size);

    return std::pair<Mat,Mat>( output_src, output_dst );
}

FrameMatch ImageMatcher::getFrameMatch(const Mat & src,const Mat & dst) const
{
//    imshow("src", src);
//    imshow("dst", dst);
//    waitKey(0);
    Mat transf = getHomography(src, dst);
    int src_w = src.cols;
    int src_h = src.rows;
    float data[] = { 0, src_w, src_w, 0,
                     0, 0, src_h, src_h,
                     1, 1, 1, 1};
    transf.convertTo( transf, CV_32F);

    Mat lin_homg_pts = Mat( 3,4, CV_32F, data );

    Mat top_left_transf = transf * lin_homg_pts.col(0);
    top_left_transf /= top_left_transf.at<float>(0,2);

    Mat top_right_transf = transf * lin_homg_pts.col(1);
    top_right_transf /= top_right_transf.at<float>(0,2);

    Mat bottom_left_transf = transf * lin_homg_pts.col(2);
    bottom_left_transf /= bottom_left_transf.at<float>(0,2);

    Mat bottom_right_transf = transf * lin_homg_pts.col(3);
    bottom_right_transf /= bottom_right_transf.at<float>(0,2);

    FrameMatch result;

    result.src_coords.top_left = Point2f( top_left_transf.at<float>(0,0),
                               top_left_transf.at<float>(0,1));
    result.src_coords.top_right = Point2f( top_right_transf.at<float>(0,0),
                               top_right_transf.at<float>(0,1));
    result.src_coords.bottom_left = Point2f( bottom_left_transf.at<float>(0,0),
                               bottom_left_transf.at<float>(0,1));
    result.src_coords.bottom_right = Point2f( bottom_right_transf.at<float>(0,0),
                               bottom_right_transf.at<float>(0,1));

    result.homography = transf;


    return result;
}

void ImageMatcher::matchFrames(const Mat & dst, vector<Mat>& input) const
{
    cout << "Calculating transformation matrices...\n";
    vector< FrameMatch > frameMatches( input.size() );
    for ( unsigned i = 0; i < input.size(); ++i )
    {
        frameMatches[i] = getFrameMatch(input[i], dst);
        frameMatches[i].srcID = i;
        cout << "\r[ " << setw(5) << right << std::fixed  << std::setprecision(1) << 100 * double(i) / input.size() << "% ]"<< std::flush;
    }
    cout << "\r[ " << setw(5) << right << std::fixed  << std::setprecision(1) << 100.0 << "% ]"<< std::flush;
    cout << "\nDONE.\n";
    auto output =  unifyFrameMatches( frameMatches );

    Size imageSize( std::floor( std::max( output[1].x, float(dst.cols)) - std::min( float(0.0), output[0].x) ),
                   std::floor( std::max( output[1].y, float(dst.rows)) - std::min( float(0.0), output[0].y) ) );

    cout << "Transforming images...\n";
    for ( const FrameMatch & m : frameMatches )
    {
        Mat transf_in;
        warpPerspective( input[m.srcID], transf_in, m.homography, imageSize);
        input[m.srcID] = transf_in;
        cout << "\r[ " << setw(5) << right << std::fixed  << std::setprecision(1) << 100 * double(m.srcID) / input.size() << "% ]"<< std::flush;
    }
    cout << "\r[ " << setw(5) << right << std::fixed  << std::setprecision(1) << 100.0 << "% ]"<< std::flush;
    cout << "\nDONE.\n";
}

float ImageMatcher::getMinX( const FrameMatch& match ) const
{
    return std::min( {match.src_coords.top_left.x,
                      match.src_coords.top_right.x,
                      match.src_coords.bottom_left.x,
                      match.src_coords.bottom_right.x});
}

float ImageMatcher::getMaxX( const FrameMatch& match ) const
{
    return std::max( {match.src_coords.top_left.x,
                      match.src_coords.top_right.x,
                      match.src_coords.bottom_left.x,
                      match.src_coords.bottom_right.x});
}

float ImageMatcher::getMinY( const FrameMatch& match ) const
{
    return std::min( {match.src_coords.top_left.y,
                      match.src_coords.top_right.y,
                      match.src_coords.bottom_left.y,
                      match.src_coords.bottom_right.y});
}
float ImageMatcher::getMaxY( const FrameMatch& match ) const
{
    return std::max( {match.src_coords.top_left.y,
                      match.src_coords.top_right.y,
                      match.src_coords.bottom_left.y,
                      match.src_coords.bottom_right.y});
}


/* Unifies all frame matches to positive X and Y cooridnates,
 * returns Size of new image, and offset of dst image
 */
vector<Point2f> ImageMatcher::unifyFrameMatches(vector<FrameMatch> &matches) const
{
    float minX = numeric_limits<float>::max();
    float minY = numeric_limits<float>::max();
    float maxX = numeric_limits<float>::min();
    float maxY = numeric_limits<float>::min();


    for ( const auto & m : matches )
    {
        float tmpMinX = getMinX( m );
        float tmpMaxX = getMaxX( m );
        float tmpMinY = getMinY( m );
        float tmpMaxY = getMaxY( m );

        minX = std::min( minX, tmpMinX);
        maxX = std::max( maxX, tmpMaxX);
        minY = std::min( minY, tmpMinY);
        maxY = std::max( maxY, tmpMaxY);
    }

    float data1[] = { 1.0, 0, 0,
                     0, 1, 0,
                     0, 0, 1};
    Mat new_transf( 3,3, CV_32FC1, data1);

    int anchorX = 0, anchorY = 0;
    if ( minX < 0)
    {
        anchorX = -minX;
        new_transf.at<float>(0,2) += anchorX;
    }
    if ( minY < 0)
    {
        anchorY = -minY;
        new_transf.at<float>(1,2) += anchorY;
    }

    for ( auto & m : matches )
    {
        m.homography = new_transf * m.homography;
        m.homography /= m.homography.at<float>(2,2);
    }
    return { Point2f( minX, minY), Point2f( maxX, maxY), Point2f( anchorX, anchorY )};
}

