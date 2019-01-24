/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 */

/* Author: Marty Kraimer */

#include <iostream>
#include <sstream>
#include <epicsGetopt.h>
#include <epicsGuard.h>
#include <pv/pvaClient.h>
#include <epicsThread.h>
#include <pv/event.h>
#include <pv/timeStamp.h>


using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;

typedef epicsGuard<epicsMutex> Guard;
typedef epicsGuardRelease<epicsMutex> UnGuard;

class ClientPutArray;
typedef std::tr1::shared_ptr<ClientPutArray> ClientPutArrayPtr;

class ClientPutArray :
    public std::tr1::enable_shared_from_this<ClientPutArray>
{
private:
    PvaClientPtr pvaClient;
public:
    POINTER_DEFINITIONS(ClientPutArray);
private:
    ClientPutArray(
         PvaClientPtr const &pvaClient
    )
    : pvaClient(pvaClient) {}
public:
    static ClientPutArrayPtr create(
        PvaClientPtr const &pvaClient
    )
    {
        ClientPutArrayPtr client(ClientPutArrayPtr(new ClientPutArray(pvaClient)));
        return client;
    }
    void put();
};

void ClientPutArray::put()
{
     cout << "channelName\n";
     string channelName;
     getline(cin,channelName);
     PvaClientPutPtr pvaClientPut(pvaClient->channel(channelName)->createPut());
     pvaClientPut->connect();
     PvaClientPutDataPtr putData(pvaClientPut->getData());
     cout << "number elements\n";
     string input;
     getline(cin,input);
     const char * cstr(input.c_str());
     int nelements = std::stoul (cstr,nullptr,0);
     if(nelements<0) nelements = 0;
     vector<string> values(nelements);
     int start = 0;
     int repeat = 0;
     int maxvalue = 0;
     if(nelements>1) {
         string input;
         cout << "first element\n";
         getline(cin,input);
         const char * cstr(input.c_str());
         start = std::stoul (cstr,nullptr,0);
         if(start<0) start = 0;
         cout << "number of times to repeat same number\n";
         getline(cin,input);
         const char * cstr1(input.c_str());
         repeat = std::stoul (cstr1,nullptr,0);
         if(repeat<0) repeat = 0;
         cout << "max element value\n";
         getline(cin,input);
         const char * cstr2(input.c_str());
         maxvalue = std::stoul (cstr2,nullptr,0);
         if(maxvalue < start+10) maxvalue = start + 10;
     }
     int index = 0;
     int value = start;
     while(true) {
        if(index>=nelements) break;
        stringstream ss;
        ss << value;
        values[index] = ss.str();
        if((index + repeat)>=nelements) repeat = nelements -index -1;
        for(int ind=0; ind<repeat; ++ind) {
            values[index+ind+1] = values[index];
        }
        index += repeat +1;
        value += 1;
        if(value>maxvalue) value = start;
     }
     PVScalarArrayPtr pvScalarArray(putData->getPVStructure()->getSubField<PVScalarArray>("value"));
     pvScalarArray->setLength(nelements);
     putData->putStringArray(values);  
     pvaClientPut->put();
}

int main(int argc,char *argv[])
{
    cout << "_____clienPutArray starting__\n";
    PvaClientPtr pvaClient(PvaClient::get("pva"));
    ClientPutArrayPtr clientPutArray(ClientPutArray::create(pvaClient));
    while(true) {
        try {
            cout << "enter put or exit or return\n";
            string str;
            getline(cin,str);
            if(str.compare("put")==0) {
                clientPutArray->put();
            } else if(str.compare("exit")==0) break;
            
        } catch (std::exception& e) {
            cerr << "exception " << e.what() << endl;
        }
    }
    return 0;
}
