/*
 * @file SURF_FlannMatcher
 * @brief SURF detector + descriptor + FLANN Matcher
 * @author A. Huaman
 */
#include <stdio.h>
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/xfeatures2d.hpp"

#include "imageprovider.h"
#include "imagematcher.h"
#include "dynamicobjectremover.h"
using namespace std;
using namespace cv;
using namespace xfeatures2d;

void matchImages( vector<Mat> input )
{

    for( int i = 0; i < input.size(); ++i )
        for( int j = 0; j < input.size(); ++j )
        {
            if( i != j )
            {
                const ImageMatcher matcher;
                auto r = matcher.unifyCoordinates(input[j], input[i] );
                Mat out;
                addWeighted(r.first,0.5,r.second,0.5,0.0,out );
//                cvtColor(r.first,r.first,CV_BGR2GRAY);
//                cvtColor(r.second,r.second,CV_BGR2GRAY);

//                out = abs(r.first - r.second);
//                threshold(out,out,100,255,THRESH_BINARY);
//                resize(out, out, Size(), 0.2,0.2);
                imshow("One", out);

    //            homography = getHomography( input[i], input[j] );
    //            r = unifyCoordinates(input[i], input[j], homography);
    ////            addWeighted(r.first,0.5,r.second,0.5,0.0,out );
    //            out = r.first - r.second;
    //            resize(out, out, Size(), 0.2,0.2);
    //            imshow("Two", out);
                waitKey(0);

            }

        }
}

int main(int argc, char **argv) {

    string inputName = "barcelona_port.mkv";
  ImageProvider p;
  auto images = p.getImagesFromVideo( inputName );

//  for( auto& img : images )
//  {
//      resize( img,img,Size(),0.5,0.5);
////      imshow( "img", img);
////      waitKey(100);
//  }

//  // Output image
//    const ImageMatcher matcher;

//    matcher.matchFrames( images[images.size() / 2], images );
////    for ( auto r : images )
////    {
////        imshow( "img", r );
////        waitKey(0);
////    }

    DynamicObjectRemover r;
    Mat out = r.reomveDynamicObjects( images );

    imshow( "img", out);
    waitKey(0);

    imwrite( inputName + "_out.jpg", out);


//  matchImages(images);

  return 0;
}
