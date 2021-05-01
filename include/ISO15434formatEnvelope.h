/*
 * ©2015 Symbol Technologies LLC. All rights reserved.
 */
#include <string>
#ifndef ISO15434FORMATENVELOPE_H
#define ISO15434FORMATENVELOPE_H
using namespace std;
class ISO15434formatEnvelope {
public:
    ISO15434formatEnvelope();
    ISO15434formatEnvelope(const ISO15434formatEnvelope& orig);
    ISO15434formatEnvelope(unsigned char* rawData,int length, int i);
    virtual ~ISO15434formatEnvelope();
    
    string getFileType();
    int getDataLength();
    int getDataIndex();
    bool getNext();
            
private:
    string fileType;   // File type name extracted from 09 envelope.
    int dataLength;
    int dataIndex;
    unsigned char* rawData;
    int index;
    int rawDataLength;
};

#endif /* ISO15434FORMATENVELOPE_H */

