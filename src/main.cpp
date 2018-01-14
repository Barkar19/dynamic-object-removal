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
#include "xmlsettings.h"

#include <boost/program_options.hpp>

using namespace std;
using namespace cv;
using namespace xfeatures2d;
using std::exception;

namespace po = boost::program_options;

int main(int argc, char **argv)
{
    string input_path, output_path, config_path;
    try
    {

        po::options_description desc("Allowed options");
        desc.add_options()
        ("help,h", "produce help message")
        ("input,i", po::value<std::string>(), "set input source (directory or movie)")
        ("output,o", po::value<std::string>(), "set output destination (name of image)")
        ("config,c", po::value<std::string>(), "config file path")
        ("verbose,v", "show progress");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (!vm.count("verbose")) {
            std::cout.setstate(std::ios_base::failbit);
        }

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
            cerr << "Input path was not set!";
            return 0;
        }

        if (vm.count("config"))
        {
            config_path = vm["config"].as<string>() ;
            cout << "Config file path was set to \""
                 << config_path << "\"\n";
        }
        else
        {
            cerr << "Config path was not set!";
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
            std::cerr << "Input path does not exist!" << std::endl;
            return 0;
        }
        if ( !boost::filesystem::exists( config_path ) )
        {
            std::cerr << "Config path does not exist!" << std::endl;
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

    // 1. Read settings file
    cout << "Reading settings file...\n";
    XmlSettings settings;
    settings.readSettings( config_path );
    cout << "DONE.\n";

    // 2. Read images
    // WARNING! Reading source has resizing implemented due to lack of memory...
    cout << "Reading source files...\n";
    ImageProvider p;
    auto images = p.getImages( input_path );
    cout << "\nDONE.\n";

    // 3. Preprocessing
    // 3.1. Resize
    if ( settings.dResizeValue > 0 )
    {
        for( auto& img : images )
        {
            const double v = settings.dResizeValue;
            resize( img,img,Size(), v, v);
        }
    }

    // 3.2. Matching
    if( settings.bMatch )
    {
        const ImageMatcher matcher;
        matcher.matchFrames( images[images.size() / 2], images );
    }


    // 3.3. Change colorspace
    const bool hlsColorspace = (settings.getColorspace() == "HLS");
    if ( hlsColorspace )
    {
        for ( auto& i : images )
        {
            cvtColor( i,i, CV_BGR2HLS);
        }
    }


    // 4. Processing
    cout << "Removing objects...\n";
    DynamicObjectRemover r;
    r.SetProcessPipe( settings.getProcessPipe() );
    Mat out = r.reomveDynamicObjects( images );
    cout << "\nDONE.\n";



    // 3.3. Change back colorspace
    if ( hlsColorspace )
    {
        cvtColor(out, out, CV_HLS2BGR);
    }
//    imshow( "Result " + output_path , out);
//    waitKey(0);


    // 5. Save output
    try
    {
        imwrite( output_path, out);
    }
    catch ( cv::Exception e )
    {
        cerr << "Cannot write output file of given name. Wrong extention? Using default jpg";
        size_t pos = output_path.find_last_of(".");
        if (pos != string::npos)
        {
            output_path.replace( output_path.begin() + pos, output_path.end(), ".jpg");
        }
        imwrite( output_path, out);
    }
  return 0;
}
