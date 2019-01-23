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
     size_t nelements = std::stoul (cstr,nullptr,0);
     
     vector<string> values(nelements);
     cout << "first element\n";
     getline(cin,input);
     values[0] = input;
     cout << "last element\n";
     getline(cin,input);
     values[nelements-1] = input;
     cout << "other elements\n";
     getline(cin,input);
     values[1] = input;
     for(size_t i=1; i< (nelements-1); ++i) {
        values[i] = values[1];
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
            cout << "enter exit or return\n";
            string str;
            getline(cin,str);
            if(str.compare("exit")==0) break;
            clientPutArray->put();
        } catch (std::exception& e) {
            cerr << "exception " << e.what() << endl;
        }
    }
    return 0;
}
