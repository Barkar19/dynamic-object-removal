#include "dynamicobjectremover.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <functional>

DynamicObjectRemover::DynamicObjectRemover()
{

}

cv::Mat DynamicObjectRemover::reomveDynamicObjects(const vector<Mat> &images)
{
    cv::Mat result(images.back().rows, images.back().cols, CV_8UC3, Scalar(128,0,128) );
    int i = 0;
    for( int x=0; x<result.rows; x++ )
    {
        for(int y=0; y<result.cols; y++)
        {
            result.at<Vec3b>(x,y) = processPixel(images, x,y);
//            cout << x << " " << y << endl;
            cout << "\r[ " << setw(5) << right << std::fixed  << std::setprecision(1) << 100.0 * double(i) / (result.rows * result.cols) << "% ]"<< std::flush;
            i++;
        }
    }
    cout << "\r[ " << setw(5) << right << std::fixed  << std::setprecision(1) << 100.0L  << "% ]"<< std::flush;
    return result;
}

void DynamicObjectRemover::SetProcessPipe(ImageProcessPipe aPipe)
{
    _oProcessPipe = aPipe;
}

Vec3b DynamicObjectRemover::processPixel(vector<Mat> mats, int x, int y)
{
    DescriptorsVector vec( mats.size() );
    for ( unsigned i = 0; i < mats.size(); ++i )
    {
        const auto& mat = mats[i];
        const auto& v = mat.at<Vec3b>(x,y);
        vec[i] = PixelDescriptor( i, v, v[0] + v[1] + v[2]);
    }
    using namespace std::placeholders;

    std::remove_if( vec.begin(), vec.end(), []( const PixelDescriptor& p)->bool
    {
        return p.value <= 0.0001;
    });
    if( vec.empty() )
        return Vec3b();

    for( unsigned i = 0; i < _oProcessPipe.getFunctionsCount(); ++i )
    {
        ImageProcessPipe::EFunction currentFunction = _oProcessPipe.getFunctionAt(i);
        std::function<DescriptorsVector(DescriptorsVector)> binding;
        switch ( currentFunction )
        {
            case ImageProcessPipe::FUNCTION_CHANNEL:
            {
                changeDescriptorsToChannelValue( vec, _oProcessPipe.getArgsAt(i)[1]);
                continue;
            }
            case ImageProcessPipe::FUNCTION_LENGTH:
            {
                changeDescriptorsToLength( vec, _oProcessPipe.getArgsAt(i)[1]);
                continue;
            }
            case ImageProcessPipe::FUNCTION_VECTOR:
            {
                changeDescriptorsToVectorDiffsValue( vec, _oProcessPipe.getArgsAt(i)[1]);
                continue;
            }
            case ImageProcessPipe::FUNCTION_MEDIAN:
            {
                binding = calculateDescriptorsMedian;
                break;
            }
            case ImageProcessPipe::FUNCTION_MEAN:
            {
                binding = calculateDescriptorsMean;
                break;
            }
            case ImageProcessPipe::FUNCTION_OUTLIERS:
            {
                binding = std::bind( removeOutliersDescriptors, _1, _oProcessPipe.getArgsAt(i)[1]);
                break;
            }
            case ImageProcessPipe::FUNCTION_REMOVE_UNSTABLE:
            {
                binding = std::bind( removeSubvectors, _1, _oProcessPipe.getArgsAt(i)[1]);
                break;
            }
            case ImageProcessPipe::FUNCTION_MOST_STABLE:
            {
                binding = std::bind( findMostStableSubvector, _1, _oProcessPipe.getArgsAt(i)[1]);
                break;
            }
            case ImageProcessPipe::FUNCTION_INVALID:
            {
                continue;
            }

        }
        if( vec.empty() )
            break;
        unsigned size = _oProcessPipe.getArgsAt(i)[0];
        if ( size == -1 )
            size = vec.size();

        reduceDescriptors( vec, binding, size);
    }
    if( vec.empty() )
        return Vec3b(255,255,255);
    return vec.front().vec;
}

void DynamicObjectRemover::changeDescriptorsToChannelValue(DescriptorsVector &descriptors, unsigned channelId)
{
    for ( auto& pixel : descriptors )
    {
        pixel.value = pixel.vec[channelId] / 255.0;
    }
}

void DynamicObjectRemover::changeDescriptorsToLength(DescriptorsVector &descriptors, float minkowskiFactor)
{
    for ( auto& pixel : descriptors )
    {
        pixel.value = std::pow( std::pow( pixel.vec[0] / 255.0, minkowskiFactor ) +
                                std::pow( pixel.vec[1] / 255.0, minkowskiFactor ) +
                                std::pow( pixel.vec[2] / 255.0, minkowskiFactor ) , 1.0 / minkowskiFactor );
    }
}

void DynamicObjectRemover::changeDescriptorsToVectorDiffsValue(DescriptorsVector &descriptors, float minkowskiFactor)
{
    for ( auto& src : descriptors )
    {
        double sum = 0.0;
        for( auto& dst : descriptors )
        {
            sum += std::pow(    std::pow( std::fabs(src.vec[0] - dst.vec[0]) / 255.0, minkowskiFactor ) +
                                std::pow( std::fabs(src.vec[1] - dst.vec[1]) / 255.0, minkowskiFactor ) +
                                std::pow( std::fabs(src.vec[2] - dst.vec[2]) / 255.0, minkowskiFactor ) , 1.0 / minkowskiFactor );
        }
        src.value = sum;
    }
}

void DynamicObjectRemover::reduceDescriptors(DescriptorsVector &descriptors, std::function<DescriptorsVector (DescriptorsVector)> fun, unsigned size)
{
    DescriptorsVector result;
    result.reserve( descriptors.size() );

    unsigned begin = 0;
    for ( ; (begin + size) < descriptors.size(); begin += size )
    {
        DescriptorsVector output = fun( DescriptorsVector(descriptors.begin() + begin, descriptors.begin() + begin + size) );
        result.insert( result.end(), output.begin(), output.end() );
    }
    DescriptorsVector output = fun( DescriptorsVector(descriptors.begin() + begin, descriptors.end()) );
    result.insert( result.end(), output.begin(), output.end() );

    descriptors = result;
}

DescriptorsVector DynamicObjectRemover::calculateDescriptorsMedian(DescriptorsVector descriptors)
{
    size_t n = descriptors.size() / 2;
    nth_element(descriptors.begin(), descriptors.begin()+n, descriptors.end(),
               [](const PixelDescriptor& a,const PixelDescriptor& b)->bool
               {
                   return (a.value < b.value);
               });

    return {descriptors[n]};
}

DescriptorsVector DynamicObjectRemover::calculateDescriptorsMean(DescriptorsVector descriptors)
{
    double mean = 0.0;
    for( auto& pixel : descriptors )
    {
        mean += pixel.value;
    }
    mean /= descriptors.size();

    auto it = descriptors.begin();
    double min = numeric_limits<double>::max();
    auto best = it;
    for ( ; it != descriptors.end(); it++ )
    {
        double tmp = fabs( it->value - mean);
        if ( tmp < min )
        {
            min = tmp;
            best = it;
        }

    }
    return { *best };
}

pair<double,double> DynamicObjectRemover::calculateStats( const DescriptorsVector& descriptors )
{
    double sum = 0.0;
    std::for_each(std::begin(descriptors), std::end(descriptors),
                  [&]( const PixelDescriptor pixel)
                        {
                            sum += pixel.value;
                        });
    double mean =  sum / descriptors.size();

    double accum = 0.0;
    std::for_each (std::begin(descriptors), std::end(descriptors),
                   [&]( const PixelDescriptor pixel)
                         {
                             accum += (pixel.value - mean) * (pixel.value - mean);
                         });

    double stdev = sqrt(accum / (descriptors.size()-1));
    return pair<double,double>( mean, stdev );
}

DescriptorsVector DynamicObjectRemover::removeOutliersDescriptors(DescriptorsVector descriptors, double factor)
{
    const auto stats = calculateStats( descriptors );

    const double mean = stats.first;
    const double stdev = stats.second;
    DescriptorsVector result;
    result.reserve( descriptors.size() );
    for( unsigned i = 0; i < descriptors.size(); ++i )
    {
        if( std::fabs(descriptors[i].value - mean) <= factor * stdev )
            result.push_back( descriptors[i] );
    }
    return result;
}

DescriptorsVector DynamicObjectRemover::removeSubvectors(DescriptorsVector descriptors, double factor)
{
    auto stats = calculateStats( descriptors );
    const double mean = stats.first;
    const double stdev = stats.second;

    return (stdev / mean > factor) ? DescriptorsVector() : descriptors;
}

DescriptorsVector DynamicObjectRemover::findMostStableSubvector(DescriptorsVector descriptors, unsigned size)
{
    if( descriptors.empty() )
        return DescriptorsVector();
    vector<pair<pair<int,int>, double>> results;
    int begin = 0;
    for ( ; (begin + size) < descriptors.size(); begin += size )
    {
        DescriptorsVector sample(descriptors.begin() + begin, descriptors.begin() + begin + size);
        const auto stats = calculateStats( sample );
        const double stdev = stats.second;

        results.push_back( pair<pair<int,int>,double>(pair<int,int>(begin, begin+size), stdev));

    }
    {
        DescriptorsVector sample( descriptors.begin() + begin, descriptors.end() );
        const auto stats = calculateStats( sample );
        const double stdev = stats.second;

        results.push_back( pair<pair<int,int>,double>(pair<int,int>(begin, begin+size), stdev));

    }

    auto element = min_element( results.begin(), results.end(),
          [](const pair<pair<int,int>,double>& a,const pair<pair<int,int>,double>& b)->bool
            {
                return a.second < b.second;
            }
        );
    pair<int,int> final = (*element).first;
    return DescriptorsVector( descriptors.begin() + final.first, descriptors.begin() + final.second);
}



Vec3b DynamicObjectRemover::differentialPixel( vector<cv::Mat> mats, int x, int y )
{
    Vec3b result;
    vector<vector<int>> channels(3, vector<int>(mats.size() ) );
    for ( unsigned i = 0; i < mats.size(); ++i )
    {
        const auto& mat = mats[i];
        const auto& pixel = mat.at<Vec3b>(x,y);
//        cvtColor( pixel, pixel, CV_BGR2HLS );
        channels[0][i] = int(sqrt(((pixel[0]*pixel[0]) + (pixel[1]*pixel[1]) + (pixel[2]*pixel[2]))));
        channels[1][i] = int(sqrt(((pixel[0]*pixel[0]) + (pixel[1]*pixel[1]) + (pixel[2]*pixel[2]))));
        channels[2][i] = int(sqrt(((pixel[0]*pixel[0]) + (pixel[1]*pixel[1]) + (pixel[2]*pixel[2]))));
    }
    vector<vector<int>> diffs(3, vector<int>(mats.size()-1 ) );
    for ( unsigned i =0; i < mats.size() - 1; ++i )
    {
        diffs[0][i] = std::abs(channels[0][i+1] - channels[0][i]);
        diffs[1][i] = std::abs(channels[1][i+1] - channels[1][i]);
        diffs[2][i] = std::abs(channels[2][i+1] - channels[2][i]);
    }

    vector<vector<vector<int>>> groups(3);
    groups[0].push_back( vector<int>() );
    groups[1].push_back( vector<int>() );
    groups[2].push_back( vector<int>() );
    for ( unsigned i =0; i < mats.size() - 1; ++i )
    {
        groups[0].back().push_back( channels[0][i] );
        if ( diffs[0][i] > 200 )
        {
            groups[0].push_back( vector<int>() );
        }
        groups[1].back().push_back( channels[1][i] );
        if ( diffs[1][i] > 200 )
        {
            groups[1].push_back( vector<int>() );
        }
        groups[2].back().push_back( channels[2][i] );
        if ( diffs[2][i] > 200 )
        {
            groups[2].push_back( vector<int>() );
        }
    }
    groups[0].back().push_back( channels[0].back() );
    groups[1].back().push_back( channels[1].back() );
    groups[2].back().push_back( channels[2].back() );

    for ( unsigned ch = 0; ch < 3; ch++ )
    {
        auto stdev_fun = [&](vector<int> v)->pair<int, double>
        {
            double sum = std::accumulate(v.begin(), v.end(), 0.0);
            double mean = sum / v.size();

            double sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
            double stdev = std::sqrt(sq_sum / v.size() - mean * mean);
            return pair<int,double>(v.size(), stdev );
        };
        vector<pair<int,double>> stats;
        for ( auto& g : groups[ch] )
        {
            if( g.size() != 1 )
                stats.push_back( stdev_fun(g) );
        }
        auto element = min_element( stats.begin(), stats.end(),
                     [](const pair<int,double>& a,const pair<int,double>& b)->bool
                    {
                        return (a.second < b.second);
                    });
        int idx = 0;
        for( int i = 0; i < element - stats.begin(); ++i)
        {
            idx += groups[ch][i].size() - 1;
        }
        result = mats[idx].at<Vec3b>(x,y);
    }

    return result;
}


Vec3b DynamicObjectRemover::vectorMedian( vector<cv::Mat> mats, int x, int y)
{
    vector<Vec3b> pixels;
    for ( unsigned i = 0; i < mats.size(); ++i )
    {
        const auto& mat = mats[i];
        Vec3b tmp = mat.at<Vec3b>(x,y);
        if ( cv::norm( tmp, NORM_L1 ) > 0 )
        {
            pixels.push_back( tmp );
        }
    }
    if (pixels.empty() )
        return Vec3b();

//    Mat samples(pixels.size(), 3, CV_32F);
//    for( int i = 0; i < pixels.size(); i++)
//        for( int z = 0; z < 3; z++)
//          samples.at<float>(i, z) = pixels[i][z];

//    int clusterCount = 3;
//    Mat labels;
//    int attempts = 5;
//    Mat centers;
//    kmeans(Mat(pixels), clusterCount, labels, TermCriteria(CV_TERMCRIT_ITER, 10,0), attempts, KMEANS_PP_CENTERS, centers );


    vector<pair<int,double>> distances( pixels.size() );
    for ( unsigned i = 0; i < pixels.size(); ++i )
    {
        double sum = 0.0L;
        for ( auto p : pixels )
        {
//            cout << p <<endl;
            auto tmp = pixels[i] - p;
            sum += cv::norm( tmp, NORM_L1);
        }
//        sum = cv::norm( pixels[i], NORM_INF);
        distances[i] = pair<int,double>(i, sum);
//        cout << "i = " << i << "; sum = " << sum << endl;
    }

    sort( distances.begin(), distances.end(),
          [](const pair<int,double>& a,const pair<int,double>& b)->bool
            {
                return a.second < b.second;
            }
        );
    const auto median = *( distances.begin() + distances.size() / 2);
//    nth_element( distances.begin(), median , distances.end() );

//    cout << "ID = " << distances[distances.size() / 2].first<< endl;

    return pixels[ median.first ];

}

Vec3b DynamicObjectRemover::medianVector( vector<cv::Vec3b> v )
{
//    vector<double> lengths( v.size() );
//    for( int i = 0; i < v.size(); ++i )
//    {
//        lengths[i] = cv::norm( v[i], NORM_L2 );
//    }
    vector<double> values(v.size(), 0);

    for( unsigned i = 0; i < v.size(); ++i )
    {
        for ( unsigned j = 0; j < v.size(); ++j  )
        {
            auto tmp = v[i] - v[j];
            values[i] += cv::norm( tmp, NORM_L1);
        }
    }
    vector<double> copy( values );
    const auto median_it = values.begin() + values.size() / 2;
    nth_element( values.begin(), median_it , values.end() );
    for ( unsigned i = 0; i < copy.size(); ++i)
    {
        if ( copy[i] == *median_it )
            return v[i];
    }
    return v[0];
}

Vec3b DynamicObjectRemover::medianVectorLength( vector<cv::Vec3b> v )
{
    vector<double> lengths( v.size() );
    for( unsigned i = 0; i < v.size(); ++i )
    {
        lengths[i] = norm( v[i], NORM_L2 );
    }
    vector<double> copy( lengths );
    const auto median_it = lengths.begin() + lengths.size() / 2;
    nth_element( lengths.begin(), median_it , lengths.end() );
    for ( unsigned i = 0; i < copy.size(); ++i)
    {
        if ( copy[i] == *median_it )
            return v[i];
    }
    return v[0];
}

Vec3b DynamicObjectRemover::medianChannel( vector<cv::Vec3b> v )
{
    Vec3b result;
    vector<vector<int>> channels(3, vector<int>(v.size() ) );
    for ( unsigned i = 0; i < v.size(); ++i )
    {
        const auto& pixel = v[i];
        channels[0][i] = pixel[0];
        channels[1][i] = pixel[1];
        channels[2][i] = pixel[2];
    }
    for( unsigned i = 0; i < channels.size(); ++i )
    {
        auto& c = channels[i];
        if ( c.size() )
        {
            const auto median_it = c.begin() + c.size() / 2;
            nth_element( c.begin(), median_it , c.end() );
            result[i] = *median_it;
        }
    }
    return result;
}

Vec3b DynamicObjectRemover::meanChannel( vector<cv::Vec3b> v )
{
    double ch[] = {0.0, 0.0, 0.0};
    for ( auto element : v )
    {
        ch[0] += element[0];
        ch[1] += element[1];
        ch[2] += element[2];
    }
    return Vec3b(ch[0]/v.size(), ch[1]/v.size(), ch[2]/v.size());
}

Vec3b DynamicObjectRemover::meanVectorLength( vector<cv::Vec3b> v )
{
//    double ch[] = {0.0, 0.0, 0.0};
//    for ( auto element : v )
//    {
//        ch[0] += element[0];
//        ch[1] += element[1];
//        ch[2] += element[2];
//    }
//    return Vec3b(ch[0]/v.size(), ch[1]/v.size(), ch[2]/v.size());
    vector<double> lengths( v.size() );
    double mean = 0.0;
    for( unsigned i = 0; i < v.size(); ++i )
    {
        lengths[i] = norm( v[i], NORM_L2 );
        mean += lengths[i];
    }
    mean /= lengths.size();
    vector<double> copy(lengths);
    sort( lengths.begin(), lengths.end() );
    int i = 0;
    double best_length = 0.0;
    while( true )
    {
        if ( lengths[i] >= mean )
            best_length = lengths[i];
            break;
        ++i;
    }
    for ( unsigned i = 0; i < copy.size(); ++i)
    {
        if( copy[i] - best_length <= 1 )
        {
            return v[i];
        }
    }
    return v[0];
}

vector<Vec3b> DynamicObjectRemover::removeOutliers( vector<cv::Vec3b> vecs, double factor )
{
    vector<double> v( vecs.size() );
    for( unsigned i = 0; i < vecs.size(); ++i )
    {
        v[i] = norm( vecs[i], NORM_L2 );
    }
    double sum = std::accumulate(std::begin(v), std::end(v), 0.0);
    double m =  sum / v.size();

    double accum = 0.0;
    std::for_each (std::begin(v), std::end(v), [&](const double d) {
        accum += (d - m) * (d - m);
    });

    double stdev = sqrt(accum / (v.size()-1));

    vector<Vec3b> result;
    for( unsigned i = 0; i < vecs.size(); ++i )
    {
        if( v[i] <= m + factor * stdev )
            result.push_back( vecs[i] );
    }
//    cout << stdev << " Ruduced by " << vecs.size() - result.size() << endl;
    return result;
}

vector<Vec3b> DynamicObjectRemover::removeStdev( vector<cv::Vec3b> vecs, double factor )
{
    vector<double> v( vecs.size() );
    for( unsigned i = 0; i < vecs.size(); ++i )
    {
        v[i] = norm( vecs[i], NORM_L2 );
    }
    double sum = std::accumulate(std::begin(v), std::end(v), 0.0);
    double mean =  sum / v.size();

    double accum = 0.0;
    std::for_each (std::begin(v), std::end(v), [&](const double d) {
        accum += (d - mean) * (d - mean);
    });

    double stdev = sqrt(accum / (v.size()-1));

//    if ( stdev / mean > factor ) cout << "REMOVED! " << vecs.size() <<"std = "<< stdev / mean << endl;
    return (stdev / mean > factor) ? vector<Vec3b>() : vecs;
}

vector<Vec3b> DynamicObjectRemover::stableStdev( vector<cv::Vec3b> vecs, unsigned size )
{
    if( vecs.empty() )
        return vector<Vec3b>();
    vector<pair<pair<int,int>, double>> results;
    int begin = 0;
    for ( ; (begin + size) < vecs.size(); begin += size )
    {
        vector<Vec3b> sample_vecs(vecs.begin() + begin, vecs.begin() + begin + size);

        sample_vecs = removeOutliers( sample_vecs, 0.5 );

        vector<double> sample( sample_vecs.size() );
        for( unsigned i = 0; i < sample_vecs.size(); ++i )
        {
            sample[i] = norm( sample_vecs[i], NORM_L2 );
        }

        double sum = std::accumulate(std::begin(sample), std::end(sample), 0.0);
        double m =  sum / sample.size();

        double accum = 0.0;
        std::for_each (std::begin(sample), std::end(sample), [&](const double d) {
            accum += (d - m) * (d - m);
        });

        double stdev = sqrt(accum / (sample.size()-1));

        results.push_back( pair<pair<int,int>,double>(pair<int,int>(begin, begin+size), stdev));

    }
    {
        vector<Vec3b> sample_vecs(vecs.begin() + begin, vecs.begin() + begin + size-1);

        sample_vecs = removeOutliers( sample_vecs, 0.5 );

        vector<double> sample( sample_vecs.size() );
        for( unsigned i = 0; i < sample_vecs.size(); ++i )
        {
            sample[i] = norm( sample_vecs[i], NORM_L2 );
        }

        double sum = std::accumulate(std::begin(sample), std::end(sample), 0.0);
        double m =  sum / sample.size();

        double accum = 0.0;
        std::for_each (std::begin(sample), std::end(sample), [&](const double d) {
            accum += (d - m) * (d - m);
        });

        double stdev = sqrt(accum / (sample.size()-1));
        results.push_back( pair<pair<int,int>,double>(pair<int,int>(begin, vecs.size()),stdev));
    }

    sort( results.begin(), results.end(),
          [](const pair<pair<int,int>,double>& a,const pair<pair<int,int>,double>& b)->bool
            {
                return a.second < b.second;
            }
        );

//    for ( auto r : results )
//    {
//        cout << r.first.first << " -> " << r.first.second << "( " << r.second <<endl;
//    }

    pair<int,int> final = results.front().first;
    return vector<Vec3b>( vecs.begin() + final.first, vecs.begin() + final.second);

//    vector<double> v( vecs.size() );
//    for( int i = 0; i < vecs.size(); ++i )
//    {
//        v[i] = norm( vecs[i], NORM_L2 );
//    }
//    double sum = std::accumulate(v.begin(), v.end(), 0.0);
//    double mean = sum / v.size();

//    double sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
//    double stdev = std::sqrt(sq_sum / v.size() - mean * mean);

//    if ( stdev / mean > factor ) cout << "REMOVED! " << vecs.size() <<"std = "<< stdev / mean << endl;
//    return (stdev / mean > factor) ? vector<Vec3b>() : vecs;
}


vector<Vec3b> DynamicObjectRemover::reduce( vector<cv::Vec3b> vec, std::function<vector<Vec3b>(vector<cv::Vec3b>)> fun, int size)
{
    vector<Vec3b> result;
    unsigned begin = 0;
    for ( ; (begin + size) < vec.size(); begin += size )
    {
        vector<Vec3b> output = fun( vector<Vec3b>(vec.begin() + begin, vec.begin() + begin + size) );
        result.reserve( result.size() + output.size() );
        result.insert( result.end(), output.begin(), output.end() );
    }
    vector<Vec3b> output = fun( vector<Vec3b>(vec.begin() + begin, vec.end()) );
    result.reserve( result.size() + output.size() );
    result.insert( result.end(), output.begin(), output.end() );
    return result;
}

vector<Vec3b> DynamicObjectRemover::reduce( vector<cv::Vec3b> vec, std::function<Vec3b(vector<cv::Vec3b>)> fun, int size)
{
    vector<Vec3b> result;
    result.reserve( vec.size() / size);
    unsigned begin = 0;
    for ( ; (begin + size) < vec.size(); begin += size )
    {
        result.push_back(fun( vector<Vec3b>(vec.begin() + begin, vec.begin() + begin + size) ));
    }
    result.push_back(fun( vector<Vec3b>(vec.begin() + begin, vec.end())));
    return result;
}

// zamienić na vectorki zwracanie
// zrobić metody, które według jakiegoś kryterium wypierdalają część wartości
// random_shuffle
// removing outliers
// removing whole / part if stdev > min
// histograms???

Vec3b DynamicObjectRemover::pixelMedianOfMedians( vector<cv::Mat> mats, int x, int y)
{
    vector<Vec3b> vec( mats.size() );
    for ( unsigned i = 0; i < mats.size(); ++i )
    {
        const auto& mat = mats[i];
        vec[i] = mat.at<Vec3b>(x,y);
    }
    using namespace std::placeholders;
    auto remOutliersbyFactor = std::bind( removeOutliers, _1, 1);
    auto remStdevByFactor = std::bind( removeStdev, _1, 0.25);
    auto stableStdevByFactor10 = std::bind( stableStdev, _1,11);
    auto stableStdevByFactor50 = std::bind( stableStdev, _1, 90);

    return reduce( vec, medianChannel, mats.size() + 1 )[0];
    return reduce( reduce( vec, stableStdevByFactor10, 190), medianChannel, mats.size() + 1)[0];
    return reduce( reduce( reduce( vec, stableStdevByFactor50, 3000), stableStdevByFactor10, 3000), medianChannel, mats.size() + 1)[0];
//     return reduce( reduce( reduce( reduce( vec, remStdevByFactor, 50), stableStdevByFactor, 1000), medianChannel, 20), medianVector, mats.size() + 1)[0];
    // BEST so far...
//     return reduce( reduce( reduce( vec, medianChannel, 30), medianVectorLength, 4), medianVector, mats.size() + 1)[0];
}


Vec3b DynamicObjectRemover::pixelMedian( vector<cv::Mat> mats, int x, int y)
{
    Vec3b result;
    vector<vector<int>> channels(3, vector<int>(mats.size() ) );
    for ( unsigned i = 0; i < mats.size(); ++i )
    {
        const auto& mat = mats[i];
        const auto& pixel = mat.at<Vec3b>(x,y);
//        cvtColor( pixel, pixel, CV_BGR2HLS );
        channels[0][i] = pixel[0];
        channels[1][i] = pixel[1];
        channels[2][i] = pixel[2];
    }

    channels[0].erase( std::remove( channels[0].begin(), channels[0].end(), 0), channels[0].end());
    channels[1].erase( std::remove( channels[1].begin(), channels[1].end(), 0), channels[1].end());
    channels[2].erase( std::remove( channels[2].begin(), channels[2].end(), 0), channels[2].end());

    for( unsigned i = 0; i < channels.size(); ++i )
    {
        auto& c = channels[i];
        if ( c.size() )
        {
            const auto median_it = c.begin() + c.size() / 2;
            nth_element( c.begin(), median_it , c.end() );
            result[i] = *median_it;
        }
        else
        {
            result[i] = 0;
        }
    }
    return result;
}
