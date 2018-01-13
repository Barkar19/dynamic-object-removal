#include "xmlsettings.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <boost/algorithm/string.hpp>

XmlSettings::XmlSettings()
{

}

void XmlSettings::readSettings(string path)
{
    file<> file(path.c_str());
    xml_document<> doc;
    doc.parse<0>(file.data());

//    xml_document<> doc;
//    std::ifstream file( path );
//    std::stringstream buffer;
//    buffer << file.rdbuf();
//    file.close();
//    std::string content(buffer.str());
//    doc.parse<0>(&content[0]);

    xml_node<> *pRoot = doc.first_node();

    if( pRoot )
    {
        xml_node<>* pColorNode = pRoot->first_node("colorspace");
        if( pColorNode )
        {
            auto pAttr = pColorNode->first_attribute("name");
            if( pAttr )
            {
                _strColorspace = pAttr->value();
            }
            else
            {
                _strColorspace = "bgr";
            }
        }
        else
        {
            _strColorspace = "bgr";
        }
        for(xml_node<> *pFunctionNode = pRoot->first_node("function"); pFunctionNode; pFunctionNode=pFunctionNode->next_sibling())
        {
            auto functionType = readFunctionName( pFunctionNode );
            double functionInputSize = readFunctionInputSize( pFunctionNode );
            double functionParameter = readFunctionParameter( pFunctionNode );
            _oPipe.appendFunction( functionType, {functionInputSize, functionParameter});
        }
    }
    return;
}

ImageProcessPipe XmlSettings::getProcessPipe()
{
    return _oPipe;
}

string XmlSettings::getColorspace()
{
    return boost::to_upper_copy<std::string>(_strColorspace);
}

ImageProcessPipe::EFunction XmlSettings::readFunctionName( xml_node<> * pNode )
{
    xml_attribute<> *pAttr = pNode->first_attribute( "name" );
    string type = pAttr->value();
    ImageProcessPipe::EFunction functionType;
    if ( type == "median" )
    {
        functionType = ImageProcessPipe::FUNCTION_MEDIAN;
    }
    else if ( type == "mean" )
    {
        functionType = ImageProcessPipe::FUNCTION_MEAN;
    }
    else if ( type == "stdev" )
    {
        functionType = ImageProcessPipe::FUNCTION_STDEV;
    }
    else if ( type == "channel" )
    {
        functionType = ImageProcessPipe::FUNCTION_CHANNEL;
    }
    else if ( type == "length" )
    {
        functionType = ImageProcessPipe::FUNCTION_LENGTH;
    }
    else if ( type == "vector" )
    {
        functionType = ImageProcessPipe::FUNCTION_VECTOR;
    }
    else
    {
        functionType = ImageProcessPipe::FUNCTION_INVALID;
    }
    return functionType;
}

double XmlSettings::readAttr(xml_attribute<> *pAttr, double defValue)
{
    double value;
    if( pAttr )
    {
        value = stod( pAttr->value() );
    }
    else
    {
        value = defValue;
    }
    return value;
}

double XmlSettings::readFunctionInputSize(xml_node<> *pNode)
{
    xml_attribute<> *pAttr = pNode->first_attribute( "size" );
    return readAttr( pAttr, -1 );
}

double XmlSettings::readFunctionParameter(xml_node<> *pNode)
{
    xml_attribute<> *pAttr = pNode->first_attribute( "parameter" );
    return readAttr( pAttr, 1.0 );
}
