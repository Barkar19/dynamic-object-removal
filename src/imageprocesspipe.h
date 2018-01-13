#ifndef IMAGEPROCESSPIPE_H
#define IMAGEPROCESSPIPE_H

#include <vector>
#include <map>

using namespace std;

class ImageProcessPipe
{
public:

    enum EFunction
    {
        FUNCTION_INVALID,
        FUNCTION_MEDIAN,
        FUNCTION_MEAN,
        FUNCTION_STDEV,
        FUNCTION_CHANNEL,
        FUNCTION_LENGTH,
        FUNCTION_VECTOR,
    };
    ImageProcessPipe();

    void appendFunction( EFunction type, vector<double> args );
    EFunction getFunctionAt( unsigned idx ) const;
    vector<double> getArgsAt( unsigned idx ) const;

    unsigned getFunctionsCount() const;

private:

    vector< EFunction > _aFunctions;
    map< unsigned, vector<double>> _aArgs;
};

#endif // IMAGEPROCESSPIPE_H
