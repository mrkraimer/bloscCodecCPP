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
#include <pv/convert.h>


using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;

typedef epicsGuard<epicsMutex> Guard;
typedef epicsGuardRelease<epicsMutex> UnGuard;

class ClientCodec;
typedef std::tr1::shared_ptr<ClientCodec> ClientCodecPtr;

class ClientCodec :
    public PvaClientChannelStateChangeRequester,
    public std::tr1::enable_shared_from_this<ClientCodec>
{
private:
    PvaClientPtr pvaClient;
    PvaClientChannelPtr pvaClientChannel;
    bool channelConnected;
    PvaClientPutPtr pvaClientPut;
    PvaClientPutGetPtr pvaClientPutGet;
    PVUByteArrayPtr pvValue; 
    string channelName;
public:
    POINTER_DEFINITIONS(ClientCodec);
private:
    ClientCodec(
         PvaClientPtr const &pvaClient
    )
    : pvaClient(pvaClient),
      channelConnected(false)
    {
    }
public:
    static ClientCodecPtr create(
        PvaClientPtr const &pvaClient
    )
    {
        ClientCodecPtr client(ClientCodecPtr(new ClientCodec(pvaClient)));
        return client;
    }
    
    virtual void channelStateChange(PvaClientChannelPtr const & channel, bool isConnected);
    void connect(const string & codecChannelName);
    void compress(const string & channelName);
    void decompress();


};

void ClientCodec::channelStateChange(PvaClientChannelPtr const & channel, bool isConnected)
    {
cout << "channelStateChange is Connected " << (isConnected ? "true" : "false") << "\n";
        channelConnected = isConnected;
    }

void ClientCodec::connect(const string & codecChannelName)
{
    pvaClientChannel = pvaClient->createChannel(codecChannelName);
    pvaClientChannel->setStateChangeRequester(shared_from_this());
    try {
        pvaClientChannel->connect();
    } catch (std::exception& e) {
        cerr << "exception " << e.what() << endl;
        return;
    }
}

void ClientCodec::compress(const string &channelName)
{
    if(!channelConnected) {
         cerr << "not connected to codecChannel\n";
         return;
    }
    this->channelName = channelName;
    PvaClientPutGetPtr pvaClientPutGet(
         pvaClientChannel->createPutGet("putField(channelName,command.index)getField(value)"));
    pvaClientPutGet->connect();
    PvaClientPutDataPtr putData(pvaClientPutGet->getPutData());
    PVStructurePtr pvData(putData->getPVStructure());
    pvData->getSubField<PVInt>("command.index")->put(1);
    pvData->getSubField<PVString>("channelName")->put(channelName);
    putData->getChangedBitSet()->set(0);
    pvaClientPutGet->putGet();
    PvaClientGetDataPtr getData(pvaClientPutGet->getGetData());
    pvValue = getData->getPVStructure()->getSubField<PVUByteArray>("value");
cout << "pvValue\n" << pvValue << "\n";
}

void ClientCodec::decompress()
{
    if(!channelConnected) {
         cerr << "not connected to codecChannel\n";
         return;
    }
    if(!pvValue) {
         cerr << "compress was not issued\n";
         return;
    }
    PvaClientPutGetPtr pvaClientPutGet(
         pvaClientChannel->createPutGet("putField(value,channelName,command.index)getField(value)"));
    pvaClientPutGet->connect();
    PvaClientPutDataPtr putData(pvaClientPutGet->getPutData());
    PVStructurePtr pvData(putData->getPVStructure());
    pvData->getSubField<PVInt>("command.index")->put(2);
    pvData->getSubField<PVString>("channelName")->put(channelName);
    PVUByteArrayPtr pvDest(pvData->getSubField<PVUByteArray>("value"));
    pvDest->copyUnchecked(*pvValue.get());
    putData->getChangedBitSet()->set(0);
    pvaClientPutGet->putGet();
}


int main(int argc,char *argv[])
{
    string argString("");
    string codecChannelName("bloscCodecRecord");
    string channelName("PVRdoubleArray");
    bool debug(false);
    int opt;
    while((opt = getopt(argc, argv, "hc:d:")) != -1) {
        switch(opt) {
            case 'c':
                codecChannelName = optarg;
                break;
            case 'd' :
               argString = optarg;
               if(argString=="true") debug = true;
               break;
            case 'h':
             cout << " -h -c codecChannelName - d debug  \n";
             cout << "default" << endl;
             cout << "-c " <<  codecChannelName
                  << " -d " << (debug ? "true" : "false")
                  << endl;           
                return 0;
            default:
                std::cerr<<"Unknown argument: "<<opt<<"\n";
                return -1;
        }
    }
   
    cout << "_____clietGetPutCodec starting__\n";
    PvaClientPtr pvaClient(PvaClient::get("pva"));
    try {  
        if(debug) PvaClient::setDebug(true);
        ClientCodecPtr clientCodec(ClientCodec::create(pvaClient));
        while(true) {
            cout << "enter one of: connect compress decompress exit\n";
            string str;
            getline(cin,str);
            if(str.compare("connect")==0){
                 clientCodec->connect(codecChannelName);
                 continue;
            }
            if(str.compare("compress")==0){
                 cout << "enter channelName\n";
                 getline(cin,str);
                 clientCodec->compress(str);
                 continue;
            }
            if(str.compare("decompress")==0){
                 clientCodec->decompress();
                 continue;
            }
            if(str.compare("exit")==0) break;
            cout << "illegal command\n";
        }

    } catch (std::exception& e) {
        cerr << "exception " << e.what() << endl;
        return 1;
    }
    return 0;
}
