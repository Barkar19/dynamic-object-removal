#include "imageprocesspipe.h"

ImageProcessPipe::ImageProcessPipe()
{

}

void ImageProcessPipe::appendFunction(ImageProcessPipe::EFunction type, vector<double> args)
{
    if ( type != FUNCTION_INVALID )
    {
        _aFunctions.push_back( type );
        _aArgs[ _aFunctions.size() - 1 ] = args;
    }
}

ImageProcessPipe::EFunction ImageProcessPipe::getFunctionAt(unsigned idx) const
{
    if( idx < _aFunctions.size() )
    {
        return _aFunctions[idx];
    }
    return FUNCTION_INVALID;
}

vector<double> ImageProcessPipe::getArgsAt(unsigned idx) const
{
    auto it = _aArgs.find( idx );
    if( it != _aArgs.end() )
    {
        return it->second;
    }
    return vector<double>();
}

unsigned ImageProcessPipe::getFunctionsCount() const
{
    return _aFunctions.size();
}
