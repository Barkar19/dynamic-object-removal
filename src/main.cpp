/*
 * @file SURF_FlannMatcher
 * @brief SURF detector + descriptor + FLANN Matcher
 * @author A. Huaman
 */
#include <stdio.h>
#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"
#include "opencv2/xfeatures2d.hpp"


#include "imageprovider.h"
#include "imagematcher.h"
#include "dynamicobjectremover.h"

#include <boost/program_options.hpp>

using namespace std;
using namespace cv;
using namespace xfeatures2d;
using std::exception;

namespace po = boost::program_options;

int main(int argc, char **argv) {

    string input_path, output_path;
    try
    {

        po::options_description desc("Allowed options");
        desc.add_options()
        ("help,h", "produce help message")
        ("input,i", po::value<std::string>(), "set input source (directory or movie)")
        ("output,o", po::value<std::string>(), "set output destination (name of jpg)");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << "\n";
            return 0;
        }

        if (vm.count("input"))
        {
            input_path = vm["input"].as<string>() ;
            cout << "Input path was set to \""
                 << input_path << "\"\n";
        }
        else
        {
            cout << "Input path was not set!";
            return 0;
        }

        if (vm.count("output"))
        {
            output_path = vm["output"].as<string>() ;
            size_t pos = output_path.find_last_of(".");
            // make sure the poisition is valid
            if (pos == string::npos)
            {
                if( output_path[ output_path.size()-1 ] == '/' )
                {
                    output_path.pop_back();
                }
                output_path.append( ".jpg");
            }
            cout << "Output path was set to \""
                 << output_path << "\"\n";
        }
        else
        {
            output_path = input_path;
            size_t pos = output_path.find_last_of(".");
            // make sure the poisition is valid
            if (pos != string::npos)
            {
                output_path.replace( output_path.begin() + pos, output_path.end(), "_out.jpg");
            }
            else
            {
                if( output_path[ output_path.size()-1 ] == '/' )
                {
                    output_path.pop_back();
                }
                output_path.append( "_out.jpg");
            }
            cout << "Output was not set! Default: \""
                 << output_path << "\"\n";
        }

        if ( !boost::filesystem::exists( input_path ) )
        {
            std::cout << "Input path does not exist!" << std::endl;
            return 0;
        }

    }
    catch(exception& e)
    {
        cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...)
    {
        cerr << "Exception of unknown type!\n";
        return 1;
    }
    ImageProvider p;
    auto images = p.getImages( input_path );

    for( auto& img : images )
    {
        resize( img,img,Size(),0.35,0.35);
    //      imshow( "img", img);
    //      waitKey(100);
    }

  // Output image
    const ImageMatcher matcher;
    matcher.matchFrames( images[images.size() / 2], images );
    for ( auto r : images )
    {
        imshow( "img", r );
        waitKey(0);
    }

    DynamicObjectRemover r;
    Mat out = r.reomveDynamicObjects( images );

    imshow( "Result " + output_path , out);
    waitKey(0);

    try
    {
        imwrite( output_path, out);
    }
    catch ( cv::Exception e )
    {
        cerr << "Cannot write output file of given name. Wrong extention? Using default jpg";
        size_t pos = output_path.find_last_of(".");
        // make sure the poisition is valid
        if (pos != string::npos)
        {
            output_path.replace( output_path.begin() + pos, output_path.end(), ".jpg");
        }
        imwrite( output_path, out);
    }


//  matchImages(images);

  return 0;
}
