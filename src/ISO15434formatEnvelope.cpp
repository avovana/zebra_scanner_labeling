/*
 * ©2015 Symbol Technologies LLC. All rights reserved.
 */

#include <iostream>
#include "ISO15434formatEnvelope.h"
#include <cstdio>
#include <stdlib.h>
ISO15434formatEnvelope::ISO15434formatEnvelope() {
}

ISO15434formatEnvelope::ISO15434formatEnvelope(const ISO15434formatEnvelope& orig) {
}

ISO15434formatEnvelope::ISO15434formatEnvelope(unsigned char* rawData, int length, int i) {
    this->rawData = rawData;
    this->index = i;
    this->rawDataLength = length;
}

ISO15434formatEnvelope::~ISO15434formatEnvelope() {
}

int ISO15434formatEnvelope::getDataIndex(){
    return dataIndex;
}
int ISO15434formatEnvelope::getDataLength(){
    return dataLength;
}

string ISO15434formatEnvelope::getFileType(){
    return fileType;
}

int indexOf(unsigned char* rawData, int length, int startIndex, unsigned char element){

    int index = -1;
    for(int i=startIndex;i<length;i++){
        if(element == rawData[i]){
            index = i;
            break;
        }
    }
    return index;
}

string getString(unsigned char* rawData,int startIndex, int endIndex){
    int numElements = endIndex-startIndex;
    char buffer [numElements];
    int j=0;
    for(int i=startIndex;i<endIndex;i++){
        sprintf (&buffer[j++], "%c", rawData[i]);
    }
    string str(buffer);
    return str;
}

/*
 * Get the next ISO15434 envelope, if it exists. Returns true if another envelope exists, false otherwise
 */
bool ISO15434formatEnvelope::getNext(){
    const unsigned char ISO_EOT = 0x04;  // ISO15454 Message Trailer Character
    const unsigned char ISO_RS = 0x1E;  // ISO15454 Format Trailer Character
    const unsigned char ISO_GS = 0x1D;  // ISO15454 Data Element Separator
    // Get the next format envelope of interest. Right now we only want '09' (binary data) and 
    // all others will be skipped if possible [probably want to print this event during testing].
    // Return true if an 09 envelope is found, false otherwise 
    bool gotAn09 = false;
    do
    {
        if (rawData[index] == ISO_EOT) return false; // now more envelopes here
        
        string strIndicator = getString(rawData,index,index+2);
        // Convert the format indicator to a number and validate
        int indicator =  atoi (strIndicator.c_str());

        switch (indicator)
        {
            case 1:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                // These formats terminate with an ISO_RS, so we can skip them
                index = indexOf(rawData,rawDataLength,index,ISO_RS);
                if (index < 0)
                    return false;
                index++; // Skip past the ISO_RS
                break;
            case 9:
                // This is what we want! Let's get out of this loop
                gotAn09 = true;
                break;
            case 2:
            default:
                // These formats can not be skipped, either they are Reserved for future use 
                // (0, 10-11, 12-99) or format 2 (Transportation) which is the only format in this message
                return false;
        } // switch
    } while (!gotAn09);

     // Hey, we finally got a Binary data format envelope, now we can process it
    int curIndex = index + 2; // Point past the two characters of the format indicator
    int searchIndex;

    // The next character better be a ISO_GS
    if (rawData[curIndex] != ISO_GS)
        return false;

    // Extract the file type name (ttt...t)

    searchIndex = indexOf(rawData,rawDataLength,++curIndex,ISO_GS);
    fileType = getString(rawData,curIndex,searchIndex);

    // Dump the compression technique name (ccc...c)
    curIndex = searchIndex + 1;
    searchIndex = indexOf(rawData,rawDataLength,curIndex,ISO_GS);

    // Extract and convert the number of bytes (nnn...n)
    curIndex = searchIndex + 1;
    searchIndex = indexOf(rawData,rawDataLength,curIndex,ISO_GS);
    string cnt = getString(rawData,curIndex,searchIndex);

    // Update the variables for the data
    dataLength = atoi (cnt.c_str());
    dataIndex = searchIndex + 1;

    // Final check - the character after the data should be ISO_RS
    if (rawData[dataIndex + dataLength] != ISO_RS)
        return false;
    index = dataIndex + dataLength + 1; // setup for next time
    return true; // found a valid entry if we got to here.
}