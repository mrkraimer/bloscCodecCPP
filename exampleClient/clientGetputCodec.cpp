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
    string codecChannelName;
    PvaClientChannelPtr pvaClientChannel;
    bool channelConnected;
    PvaClientPutPtr pvaClientPut;
    PvaClientPutGetPtr pvaClientPutGet;
    PVUByteArrayPtr pvValue; 
    string channelName;
public:
    POINTER_DEFINITIONS(ClientCodec);
    virtual void channelStateChange(PvaClientChannelPtr const & channel, bool isConnected);
private:
    ClientCodec(
         PvaClientPtr const &pvaClient,
         const string & codecChannelName
    )
    : pvaClient(pvaClient),
      codecChannelName(codecChannelName),
      channelConnected(false)
    {
    }
    void connect();
public:
    static ClientCodecPtr create(
        PvaClientPtr const &pvaClient,
        const string & codecChannelName
    )
    {
        ClientCodecPtr client(ClientCodecPtr(new ClientCodec(pvaClient,codecChannelName)));
        client->connect();
        return client;
    }
    void compress(const string & channelName);
    void decompress();
};

void ClientCodec::channelStateChange(PvaClientChannelPtr const & channel, bool isConnected)
    {
cout << "channelStateChange is Connected " << (isConnected ? "true" : "false") << "\n";
        channelConnected = isConnected;
    }

void ClientCodec::connect()
{
    pvaClientChannel = pvaClient->createChannel(codecChannelName);
    pvaClientChannel->setStateChangeRequester(shared_from_this());
    pvaClientChannel->connect();
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
        ClientCodecPtr clientCodec(ClientCodec::create(pvaClient,codecChannelName));
        while(true) {
            cout << "enter one of: compress decompress exit\n";
            string str;
            getline(cin,str);
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
