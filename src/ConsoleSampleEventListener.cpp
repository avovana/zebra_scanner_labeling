/*
 * ©2015 Symbol Technologies LLC. All rights reserved.
 */

#include "ConsoleSampleEventListener.h"
#include "ISO15434formatEnvelope.h"

/* standard C includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* standard template library includes */
#include <string>
#include <iostream>
#include <sstream>
#include <errno.h>
#include <cstring>
#include <time.h>
#include <QDebug>
#include "pugixml.hpp"

#include "dialog.h"


SampleEventListener::SampleEventListener(Dialog &dialog_) : dialog(dialog_)
{
    //barCodeEvent = barCodeEvent_;
}

SampleEventListener::~SampleEventListener()
{
	Close();
}

StatusID SampleEventListener::Open()
{
    StatusID status;
    ::Open(this, SCANNER_TYPE_ALL, &status);
    if(status != STATUS_OK)
    {
        return status;
    }

    std::string inXml = "<inArgs><cmdArgs><arg-int>6</arg-int><arg-int>1,2,4,8,16,32</arg-int></cmdArgs></inArgs>";
    std::string outXml;
    ::ExecCommand(CMD_REGISTER_FOR_EVENTS, inXml, outXml, &status);   
    return status;
}

void SampleEventListener::GetScanners()
{
    unsigned short count;
    vector<unsigned int> list;
    string outXml;
    StatusID eStatus;

    ::GetScanners(&count, &list, outXml, &eStatus);
    
     qDebug() << "================================" << endl;

    if ( eStatus != STATUS_OK )
    {
        qDebug() << "Get Scanners failed. Can't connect to the corescanner." << endl;
        return ;
    }

    qDebug() << "Get Scanners command success. " << endl << endl;
    qDebug() << "Scanner Count: " << count << endl;
    for(int x=0; x<count; x++)
    {
        qDebug() << "Scanner IDs : " << list[x] << endl;
    }
    qDebug() << "Out XML : " << outXml.data() << endl;
    qDebug() << "================================" << endl << endl;
}

void SampleEventListener::GetDeviceTopology()
{
    string inXml;
    string outXml;
    StatusID sId;
    
    qDebug() << "================================" << endl;
    
    ::ExecCommand(CMD_GET_DEVICE_TOPOLOGY, inXml, outXml, &sId);
    
    if(sId == STATUS_OK)
    {
        qDebug() << "Get Device Topology command success. " << endl << endl;
        qDebug() << "Out XML : " << outXml.data() << endl;
    }
    else
    {
        qDebug()<< "Get Device Topology command failed. Error code : " << sId <<endl;
    }    
    
    qDebug() << "================================" << endl << endl;
}

void SampleEventListener::GetAttribute()
{
    std::string scannerID = "";
    qDebug() << "Enter Scanner ID : " << endl;
    cin >> scannerID;
    qDebug() << "Enter attribute number or comma separated attribute numbers : " ;
    std::string attribute_number = "";
    cin >> attribute_number;

    std::string inXml = "<inArgs><scannerID>" + scannerID + 
                        "</scannerID><cmdArgs><arg-xml><attrib_list>" + 
                        attribute_number + "</attrib_list></arg-xml></cmdArgs></inArgs>";
          
    StatusID sId;
    std::string outXml;
    qDebug() << "In XML  : " << inXml.data() << endl << endl;
    ::ExecCommand(CMD_RSM_ATTR_GET, inXml, outXml, &sId);    
    
    qDebug() << "================================" << endl;
    if(sId == STATUS_OK)
    {
        qDebug() << "GetAttribute command success." << endl << endl;
         qDebug() << "Out XML : " << outXml.data() << endl;
    }
    else
    {
        qDebug() << "GetAttribute command failed. Error code : " << sId << endl;
    }
    qDebug() << "================================" << endl<< endl;
}

void SampleEventListener::GetAttributeNext()
{
    std::string scannerID = "";
    qDebug() << "Enter Scanner ID : " << endl;
    cin >> scannerID;
    qDebug() << "Enter current attribute number : " ;
    std::string attribute_number = "";
    cin >> attribute_number;

    std::string inXml = "<inArgs><scannerID>" + scannerID + 
                        "</scannerID><cmdArgs><arg-xml><attrib_list>" + 
                        attribute_number + "</attrib_list></arg-xml></cmdArgs></inArgs>";    
    
    StatusID sId;
    std::string outXml;
    qDebug() << "In XML  : " << inXml.data() << endl << endl;
    ::ExecCommand(CMD_RSM_ATTR_GETNEXT, inXml, outXml, &sId);
    
    qDebug() << "================================" << endl;
    
    if(sId == STATUS_OK)
    {
        qDebug() << "GetAttributeNext command success. " << endl << endl;
         qDebug() << "Out XML : " << outXml.data() << endl;
    }
    else
    {
         qDebug() << "GetAttributeNext command failed. Error code : " << sId << endl;
    }
    
    qDebug() << "================================" << endl << endl;
}

void SampleEventListener::GetAllAttributes()
{    
    std::string scannerID = "";     
    
    qDebug() << "Enter scanner ID: ";
    cin >> scannerID;
    
    std::string inXml = "<inArgs><scannerID>"+scannerID+"</scannerID></inArgs>";
    StatusID sId;
    std::string outXml;
    qDebug() << "In XML  : " << inXml.data() << endl << endl;
    ::ExecCommand(CMD_RSM_ATTR_GETALL, inXml, outXml, &sId);
       
    qDebug() << "================================" << endl;
    if(sId == STATUS_OK)
    {
        qDebug() << "GetAllAttributes command success. " << endl << endl;
         qDebug() << "Out XML : " << outXml.data() << endl;
    }
    else
    {
         qDebug() << "GetAllAttributes command failed. Error code : " << sId << endl;
    }
    qDebug() << "================================" << endl << endl;
}



void SampleEventListener::SetAttribute()
{
    std::string scannerID = "";
    qDebug() << "Enter scanner ID : " << endl;
    cin >> scannerID;
    std::string attributeNumber = "";
    qDebug() << "Enter attribute number : " << endl;
    cin >> attributeNumber;
    std::string dataType = "";
    qDebug() << "Enter data type : " << endl;
    cin >> dataType;
    std::string attributeValue = "";
    qDebug() << "Enter attribute value : " << endl;
    cin >> attributeValue;
    
    std::string inXml = "<inArgs><scannerID>"+ scannerID +
                        "</scannerID><cmdArgs><arg-xml><attrib_list><attribute><id>" + attributeNumber + 
                        "</id><datatype>" + dataType + 
                        "</datatype><value>" + attributeValue + 
                        "</value></attribute></attrib_list></arg-xml></cmdArgs></inArgs>";
    
    StatusID sId;    
    std::string outXml;
    qDebug() << "In XML  : " << inXml.data() << endl << endl;
    ::ExecCommand(CMD_RSM_ATTR_SET, inXml, outXml, &sId);
    
    qDebug() << "================================" << endl;
    if(sId == STATUS_OK)
    {
        qDebug() << "SetAttribute command success. " << endl << endl;
    }
    else
    {
         qDebug() << "SetAttribute command failed. Error code : " << sId << endl;
    }
    
    qDebug() << "================================" << endl << endl;

}

void SampleEventListener::SetAttributeStore()
{
    std::string scannerID = "";
    qDebug() << "Enter scanner ID : " << endl;
    cin >> scannerID;
    std::string attributeNumber = "";
    qDebug() << "Enter attribute number : " << endl;
    cin >> attributeNumber;
    std::string dataType = "";
    qDebug() << "Enter data type : " << endl;
    cin >> dataType;
    std::string attributeValue = "";
    qDebug() << "Enter attribute value : " << endl;
    cin >> attributeValue;    

    std::string inXml = "<inArgs><scannerID>"+ scannerID +
                        "</scannerID><cmdArgs><arg-xml><attrib_list><attribute><id>" + attributeNumber + 
                        "</id><datatype>" + dataType + 
                        "</datatype><value>" + attributeValue + 
                        "</value></attribute></attrib_list></arg-xml></cmdArgs></inArgs>";
    
    StatusID sId;    
    std::string outXml;
    qDebug() << "In XML  : " << inXml.data() << endl << endl;
    ::ExecCommand(CMD_RSM_ATTR_STORE, inXml, outXml, &sId);
   
    qDebug() << "================================" << endl;
    if(sId == STATUS_OK)
    {
        qDebug() << "StoreAttribute command success. " << endl << endl;
    }
    else
    {
         qDebug() << "StoreAttribute command failed. Error code : " << sId << endl;
    }    
    qDebug() << "================================" << endl << endl;
}

void SampleEventListener::SetZeroWeight()
{
    std::string inXml = "<inArgs><scannerID>1</scannerID><cmdArgs><arg-xml><attrib_list><attribute><id>6019</id><datatype>X</datatype><value>0</value></attribute></attrib_list></arg-xml></cmdArgs></inArgs>";

    StatusID sId;
    std::string outXml;
    qDebug() << "In XML  : " << inXml.data() << endl << endl;
    ::ExecCommand(CMD_RSM_ATTR_SET, inXml, outXml, &sId);
      
    qDebug() << "================================" << endl;
    if(sId == STATUS_OK)
    {
        qDebug() << "SetZeroWeight command success. " << endl << endl;
    }
    else
    {
         qDebug() << "SetZeroWeight command failed. Error code : " << sId << endl;
    } 
    qDebug() << "================================" << endl << endl;
}

//Event EventListener for get Get Version number of Core scanner
void SampleEventListener::GetVersion() {
    
    std::string inXml = "<inArgs></inArgs>";   
    StatusID sId;
    std::string outXml;
    qDebug() << "In XML  : " << inXml.data() << endl << endl;
    ::ExecCommand(CMD_GET_VERSION, inXml, outXml, &sId);
       
    qDebug() << "================================" << endl;
    if(sId == STATUS_OK)
    {
        qDebug() << "GetVersion command success. " << endl << endl;
         qDebug() << "Out XML : " << outXml.data() << endl;
    }
    else
    {
         qDebug() << "GetVersion command failed. Error code : " << sId << endl;
    } 
    qDebug() << "================================" << endl << endl;
}

void SampleEventListener::Close()
{
    StatusID status;
    ::Close(0, &status);
    qDebug() << "close" << endl;

}

string getStringFromRawData(unsigned char* rawData,int startIndex, int endIndex){
    int numElements = endIndex-startIndex;
    char buffer [numElements];
    int j=0;
    for(int i=startIndex;i<endIndex;i++){
        sprintf (&buffer[j++], "%c", rawData[i]);
    }
    string str(buffer);
    return str;
}
void SampleEventListener::OnImageEvent( short eventType, int size, short imageFormat, char* sfimageData, int dataLength, std::string& pScannerData )
{
    qDebug() << "OnImageEvent" << endl;
}

void SampleEventListener::OnBinaryDataEvent( short eventType, int size, short dataFormat, unsigned char* sfBinaryData, std::string&  pScannerData)
{    
     // Constants for ISO15434 messages
        const unsigned char ISO_RS = 0x1E;  // ISO15454 Format Trailer Character
//        const unsigned char ISO_GS = 0x1D;  // ISO15454 Data Element Separator
        const unsigned char ISO_EOT = 0x04;  // ISO15454 Message Trailer Character
        const unsigned char MSG_EASYCAP = 0;     // ISO15451 Message DocCap message number
        FILE *imgFile; 
        qDebug() << "OnBinaryDataEvent" << endl;
		
        int packetLength = (sfBinaryData[0] << 24) |
                        (sfBinaryData[1] << 16) |
                        (sfBinaryData[2] << 8) |
                        sfBinaryData[3];
        
        if(packetLength+4!= size)
        {
            qDebug() << "Incorrect packet size\n" << endl;
        }
        else
        {
            if(sfBinaryData[4]!= MSG_EASYCAP)
            {
                qDebug() << "Incorrect Msg type\n" << endl;
            }
            else
            {
                // ISO15434 Envelope: is message header correct?
                if ((sfBinaryData[5] != '[') || (sfBinaryData[6] != ')') || (sfBinaryData[7] != '>') || (sfBinaryData[8] != ISO_RS))
                {
                    qDebug() << "Incorrect message header\n" << endl;
                }
                else
                {
                     // ISO15434 Envelope: is message header correct?
                    if (sfBinaryData[size - 1] != ISO_EOT)
                    {
                        qDebug() << "Incorrect message header ISO_EOT\n" << endl;
                    }
                    else
                    {
                        qDebug() << "Correct packet received \n" << endl;
                        ISO15434formatEnvelope *anEnvelope = new ISO15434formatEnvelope(sfBinaryData,size, 9);
                        while (anEnvelope->getNext())
                        {
                            string fileType = anEnvelope->getFileType();

                            if (fileType == "BarCode")
                            {
                                string decodeData=getStringFromRawData(sfBinaryData,anEnvelope->getDataIndex(),anEnvelope->getDataIndex()+anEnvelope->getDataLength());
                                qDebug() << "Barcode received " << endl;
                                qDebug() << " Data type = "<<  (int)decodeData.at(0) << endl;
                                qDebug() << " Data = "<< decodeData.substr(1,anEnvelope->getDataLength()).data()<< endl;
                            }else{
                                char filename[200];
                                string fileFormat = anEnvelope->getFileType();
                                time_t t = time(0);   // get time now
                                tm* now = localtime(&t);
                                sprintf(filename,"ZIMG-%d%d%d%d%d%d%s%s", now->tm_year + 1900, now->tm_mon + 1,now->tm_mday,now->tm_hour,now->tm_min,now->tm_sec,".",fileFormat.c_str());
                                imgFile = fopen(filename, "w");
                                if (!imgFile)
                                {
                                    qDebug() << "Unable to open file " << filename<< endl;
                                    continue;
                                }
                                fwrite(&sfBinaryData[anEnvelope->getDataIndex()]+6,sizeof(unsigned char),anEnvelope->getDataLength()+6,imgFile);
                                fflush(imgFile);
                                fclose (imgFile);
                                qDebug() << "ImageData saved in " <<filename<< endl;
                            }
                        }
                    }
                }
                
            }
        }
}

void SampleEventListener::OnVideoEvent( short eventType, int size, char* sfvideoData, int dataLength, std::string& pScannerData )
{
    qDebug() << "OnVideoEvent" << endl;
}

void SampleEventListener::OnPNPEvent( short eventType, std::string ppnpData )
{
    string str;
    if (eventType == SCANNER_ATTACHED) {
        qDebug() << "Scanner attached" << endl;
        str = ppnpData;
    } else if (eventType == SCANNER_DETACHED) {
        qDebug() << "Scanner detached" << endl;
        str =  ppnpData;
    } else {
        str = " UNKNOWN PNP Event ";
    }
    qDebug() << str.data() << endl;
}

void SampleEventListener::OnCommandResponseEvent( short status, std::string& prspData )
{
    qDebug() << endl << "Scanner data: " << prspData.data() << endl;
    qDebug() << "OnCommandResponseEvent" << endl;
    qDebug() << prspData.data() << endl;
}

void SampleEventListener::OnScannerNotification( short notificationType, std::string& pScannerData )
{
    qDebug() << endl << "Scanner event data: " << pScannerData.data() << endl;
    qDebug() << "OnScannerNotification" << endl;
}

void SampleEventListener::OnIOEvent( short type, unsigned char data )
{
    qDebug() << "OnIOEvent" << endl;
}

void SampleEventListener::OnScanRMDEvent( short eventType, std::string& prmdData )
{
    qDebug() << "OnScanRMDEvent" << endl;
        qDebug() << "Out XML " << endl;
        qDebug() << prmdData.data() << endl;
}

void SampleEventListener::OnDisconnect()
{
    qDebug() << "OnDisconnect" << endl;
}

void SampleEventListener::OnBarcodeEvent(short int eventType, std::string & pscanData)
{
    qDebug() << "Barcode Detected" << endl;
    qDebug() << "Out XML" << endl;
    QString str = QString::fromUtf8(pscanData.c_str());
    qDebug() << str << endl;

    std::string decoded_bar_code = get_decode_data(pscanData);


    cout << "bar_code:    " << decoded_bar_code << endl;
    dialog.barCodeEvent(decoded_bar_code);
}

// tokernize string for a given delimiter //
static char tokernize_buffer[ 256];
std::vector<std::string> SampleEventListener::stringTokernize(std::string inStr,char cDelim ) {
    cout << __PRETTY_FUNCTION__ << " start =======================" << endl;
    std::vector<std::string> _return;

    int iLength = inStr.size();
    int iCurrentIndex = 0;
    int iCurrentOutputIndex =0;
    while ( iCurrentIndex < iLength ){
        char cCurrentChar = inStr[iCurrentIndex++];
        if (cDelim == cCurrentChar  ){
            if ( iCurrentOutputIndex == 0 )
            {
                // just ignore//
            }else {
                tokernize_buffer[iCurrentOutputIndex] =0;
                _return .push_back( tokernize_buffer);
                iCurrentOutputIndex= 0;
            }
        }else{
            tokernize_buffer[iCurrentOutputIndex++] = (char)cCurrentChar;
        }
    }

    if ( iCurrentOutputIndex > 0){
        tokernize_buffer[iCurrentOutputIndex] = 0;
        _return.push_back(tokernize_buffer);
    }

    cout << __PRETTY_FUNCTION__ << " end =======================" << endl;
    return _return;
}

static char buffer_decode_data [ 1024*256] ; // hope we don't have barcodes beyond 256 KB of length. //
std::string SampleEventListener::get_decode_data(std::string outXml) {
    cout << __PRETTY_FUNCTION__ << " start =======================" << endl;
    // :TODO:
    pugi::xml_document doc;
    pugi::xml_parse_result parse_result = doc.load_string( outXml.c_str());
    if ( pugi::status_ok != parse_result.status)
    {
        cout << "loading outXml to pugi failed.";
        return "";
    }

    pugi::xml_node nodeDataLabel = doc.child("outArgs").child("arg-xml").
            child("scandata").child("datalabel");
    const char * cstrDataLabel = nodeDataLabel.child_value();
    int iLength = strlen( cstrDataLabel);

    if (iLength < 1)
    {
        cout << "get_decode_data :: outXml->datalabel has no data";
    }

    std::vector<std::string> vecStrTokernized = stringTokernize(cstrDataLabel, ' ');
    std::vector<std::string>::iterator itr;
    int iIndexOutput = 0;
    int cCurrentChar ;
    for( itr = vecStrTokernized.begin(); itr != vecStrTokernized.end();itr++){
        std::string strCurrentToken = *itr;
        int iRet = sscanf( strCurrentToken.c_str(), "%x" , &cCurrentChar);
        if ( iRet == 0){
            sscanf( strCurrentToken.c_str() , "%x" , &cCurrentChar);
        }
        buffer_decode_data[iIndexOutput++] = (char) cCurrentChar;
    }

    buffer_decode_data[iIndexOutput] = 0;

    cout << __PRETTY_FUNCTION__ << " end =======================" << endl;

    return buffer_decode_data;
}


void SampleEventListener::RebootScanner()
{
    
}

void SampleEventListener::FirmwareUpdate()
{
    std::string inXml;
    std::string outXml;
    std::string datFilePath; 
    StatusID sId;
    std::string scannerID;
    std::string bulkOption;
    
    qDebug() << "Enter Scanner ID : " << endl;
    std::cin >> scannerID;
    qDebug() << "Enter Firmware DAT file path: " << endl;
    std::cin >> datFilePath;
    qDebug() << "Enter USB communication mode 1=hid, 2=bulk : ";
    std::cin >> bulkOption;
    
    inXml = "<inArgs><scannerID>" + scannerID + "</scannerID><cmdArgs><arg-string>" + datFilePath + "</arg-string><arg-int>" + bulkOption + "</arg-int></cmdArgs></inArgs>";
    qDebug() << "In XML  : " << inXml.data() << endl << endl;
    ::ExecCommand(CMD_DEVICE_UPDATE_FIRMWARE, inXml, outXml, &sId);
        
    qDebug() << "================================" << endl;
    if(sId == STATUS_OK)
    {
        qDebug() << "Firmware Update command success. " << endl << endl;
    }
    else
    {
         qDebug() << "Firmware Update command failed. Error code : " << sId << endl;
    } 
    qDebug() << "================================" << endl << endl;
}

void SampleEventListener::FirmwareUpdateFromPlugin()
{
     std::string inXml;
    std::string outXml;
    std::string pluginFilePath=""; 
    StatusID sId;
    std::string scannerID;
    std::string bulkOption;
    
    qDebug() << "Enter Scanner ID : " << endl;
    std::cin >> scannerID;
    qDebug() << "Enter Firmware Plug-in file path: " << endl;
    //std::cin >>  pluginFilePath;

    while ( pluginFilePath.size() < 4 ){
        std::getline(std::cin, pluginFilePath);
    }
    
    if ( !(pluginFilePath.substr(pluginFilePath.find_last_of(".")+ 1) == "SCNPLG") ){
        qDebug() << "Please Enter a file with extension .SCNPLG." << endl << endl;
        return;
    }
    
    qDebug() << "Enter USB communication mode 1=hid, 2=bulk : ";
    std::cin >> bulkOption;
    
    inXml = "<inArgs><scannerID>" + scannerID + "</scannerID><cmdArgs><arg-string>" + pluginFilePath + "</arg-string><arg-int>" + bulkOption + "</arg-int></cmdArgs></inArgs>";
    qDebug() << "In XML  : " << inXml.data() << endl << endl;
    ::ExecCommand(CMD_DEVICE_UPDATE_FIRMWARE_FROM_PLUGIN, inXml, outXml, &sId);
    
    qDebug() << "================================" << endl;
    if(sId == STATUS_OK)
    {
        qDebug() << "FirmwareUpdate From Plug-in command success. " << endl << endl;
    }
    else
    {
         qDebug() << "FirmwareUpdate From Plug-in command failed. Error code : " << sId << endl;
    } 
    qDebug() << "================================" << endl << endl;
}

void SampleEventListener::StartNewFirmware()
{
    std::string inXml;
    std::string outXml;
    StatusID sId;
    std::string scannerID;
    
    qDebug() << "Enter Scanner ID : " << endl;
    std::cin >> scannerID;
       
    inXml = "<inArgs><scannerID>" + scannerID + "</scannerID></inArgs>"; 
    qDebug() << "In XML  : " << inXml.data() << endl << endl;
    ::ExecCommand(CMD_START_NEW_FIRMWARE, inXml, outXml, &sId);
    
    qDebug() << "================================" << endl;
    if(sId == STATUS_OK)
    {
        qDebug() << "Start New Firmware command success. " << endl << endl;
    }
    else
    {
         qDebug() << "Start New Firmware command failed. Error code : " << sId << endl;
    } 
    qDebug() << "================================" << endl << endl;
}

void SampleEventListener::AbortFirmwareUpdate()
{
    std::string inXml;
    std::string outXml;
    StatusID sId;
    std::string scannerID;    
   
    qDebug() << "Enter Scanner ID : " << endl;
    std::cin >> scannerID;
    
    inXml = "<inArgs><scannerID>" + scannerID + "</scannerID></inArgs>";
    qDebug() << "In XML  : " << inXml.data() << endl << endl;
    ::ExecCommand(CMD_DEVICE_ABORT_UPDATE_FIRMWARE, inXml, outXml, &sId);
    
    qDebug() << "================================" << endl;
    if(sId == STATUS_OK)
    {
        qDebug() << "Abort Firmware Update command success. " << endl << endl;
    }
    else
    {
         qDebug() << "Abort Firmware Update command failed. Error code : " << sId << endl;
    } 
    qDebug() << "================================" << endl << endl;
}

/**
 * Method to execute action attribute related commands.
 * added to v1.3.0 release.
 * @param opCode
 */
void SampleEventListener::ExecuteActionCommand(CmdOpcode opCode)
{
    std::string scannerID = "";
    std::string ledNumber = "";
    std::string beeperCode = "";
    std::string inXml;
    std::string commandName = "";
           
    switch (opCode)
    {
        case CMD_DEVICE_LED_ON:
        case CMD_DEVICE_LED_OFF:
        {
            commandName = "Device LED ON/OFF command ";
            qDebug() << "Enter scanner ID : ";
            cin >> scannerID;
            qDebug() << "Enter LED number : ";
            cin >> ledNumber;

            inXml = "<inArgs><scannerID>" + scannerID + 
                                "</scannerID><cmdArgs><arg-int>" + ledNumber +
                                "</arg-int></cmdArgs></inArgs>";
            break;
        }
        case CMD_DEVICE_BEEP_CONTROL:
        {
            commandName = "Beep command ";
            qDebug() << "Enter scanner ID : ";
            cin >> scannerID;
            qDebug() << "Enter Beeper code : ";
            cin >> beeperCode;

            inXml = "<inArgs><scannerID>" + scannerID + 
                                "</scannerID><cmdArgs><arg-int>" + beeperCode +
                                "</arg-int></cmdArgs></inArgs>";
            break;
        }
        case CMD_DEVICE_SCAN_ENABLE:
        case CMD_DEVICE_SCAN_DISABLE:
        {
            commandName = "Device Scan Enable/Disable command ";
            qDebug() << "Enter scanner ID : ";
            cin >> scannerID;
            inXml = "<inArgs><scannerID>" + scannerID + "</scannerID></inArgs>";
            break;
        }
        case CMD_REBOOT_SCANNER:
        {
            commandName = "Reboot scanner command ";
            qDebug() << "Enter scanner ID : ";
            cin >> scannerID;
            inXml = "<inArgs><scannerID>" + scannerID + "</scannerID></inArgs>";
            break;
        }
        case CMD_DEVICE_AIM_ON:
        case CMD_DEVICE_AIM_OFF:
        {
            commandName = "Device AIM ON/OFF command ";
            qDebug() << "Enter scanner ID : ";
            cin >> scannerID;
            inXml = "<inArgs><scannerID>" + scannerID + "</scannerID></inArgs>";
            break;
        }
        default:
        {
            commandName = "Default Action command ";
            qDebug() << "Enter scanner ID : ";
            cin >> scannerID;
            inXml = "<inArgs><scannerID>" + scannerID + "</scannerID></inArgs>";
            break;
        }
    }
      
    StatusID sId;
    std::string outXml;
    qDebug() << "In XML  : " << inXml.data() << endl << endl;
    ::ExecCommand(opCode, inXml, outXml, &sId);

    qDebug() << "================================" << endl;
    if(sId == STATUS_OK)
    {
        qDebug() << commandName.data() << "success. " << endl << endl;
    }
    else
    {
         qDebug() << commandName.data() << "failed. Error code : " << sId << endl;
    } 
    qDebug() << "================================" << endl << endl;
}
