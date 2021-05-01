/*
 * ©2015 Symbol Technologies LLC. All rights reserved.
 */

#ifndef SAMPLEEVENTLISTENER_H_
#define SAMPLEEVENTLISTENER_H_



#include "CsIEventListenerXml.h"
#include "CsUserDefs.h"
#include "CsBarcodeTypes.h"
#include "Cslibcorescanner_xml.h"

/* standard C includes */
#include <stdio.h>
#include <stdlib.h>

/* standard template library includes */
#include <string>
#include <iostream>
#include <sstream>
#include <functional>

using namespace std;

class Dialog;

class SampleEventListener : public IEventListenerXml
{
public:
    explicit SampleEventListener(Dialog &dialog); //std::function<void(const Foo&, int)>
	virtual ~SampleEventListener();

    virtual void OnImageEvent( short eventType, int size, short imageFormat, char* sfimageData, int dataLength, std::string& pScannerData );
    virtual void OnVideoEvent( short eventType, int size, char* sfvideoData, int dataLength, std::string& pScannerData );
    virtual void OnBarcodeEvent( short eventType, std::string& pscanData );
    virtual void OnPNPEvent( short eventType, std::string ppnpData );
    virtual void OnCommandResponseEvent( short status, std::string& prspData );
    virtual void OnScannerNotification( short notificationType, std::string& pScannerData );
    virtual void OnIOEvent( short type, unsigned char data );
    virtual void OnScanRMDEvent( short eventType, std::string& prmdData );
    virtual void OnDisconnect();
    virtual void OnBinaryDataEvent( short eventType, int size, short dataFormat, unsigned char* sfBinaryData, std::string&  pScannerData);


    StatusID Open();
    void GetScanners();
    void GetAttribute();
    void GetAttributeNext();
    void GetAllAttributes();
    void SetAttribute();
    void SetAttributeStore();
    void SetZeroWeight();
    void Close();
    
    void RebootScanner();
    void ExecuteActionCommand(CmdOpcode opCode);
    void GetDeviceTopology();
    void FirmwareUpdate();
    void FirmwareUpdateFromPlugin();
    void StartNewFirmware();
    void AbortFirmwareUpdate();
    void GetVersion();

    Dialog &dialog;
    std::string get_decode_data(std::string outXml);
    std::vector<std::string> stringTokernize(std::string inStr, char cDelim);
};


#endif /* SAMPLEEVENTLISTENER_H_ */
