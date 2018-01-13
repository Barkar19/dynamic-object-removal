#ifndef XMLSETTINGS_H
#define XMLSETTINGS_H

#include <string>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "imageprocesspipe.h"

using namespace std;
using namespace rapidxml;

class XmlSettings
{
public:
    XmlSettings();

    void readSettings( string path );
    ImageProcessPipe getProcessPipe();

    string getColorspace();
private:
    double readAttr(xml_attribute<> *pAttr, double defValue);

    ImageProcessPipe::EFunction readFunctionName(xml_node<> *pNode);
    double readFunctionInputSize(xml_node<> *pNode);
    double readFunctionParameter(xml_node<> *pNode);
    double readFunctionType(xml_node<> *pNode);
    double readFunctionTypeValue(xml_node<> *pNode);

    ImageProcessPipe _oPipe;
    string _strColorspace;
};

#endif // XMLSETTINGS_H
