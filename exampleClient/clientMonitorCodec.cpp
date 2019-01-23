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
    string channelName;
    PvaClientChannelPtr pvaClientChannel;
    bool channelConnected;
    PvaClientPutPtr pvaClientPut;
    PVUByteArrayPtr pvValue; 
public:
    POINTER_DEFINITIONS(ClientCodec);
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
    
    virtual void channelStateChange(PvaClientChannelPtr const & channel, bool isConnected);
    void startMonitor(const string & channelName);
    void stopMonitor();
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

void ClientCodec::startMonitor(const string &channelName)
{
    if(!channelConnected) {
         cerr << "not connected to codecChannel\n";
         return;
    }
    PvaClientPutPtr pvaClientPut(pvaClientChannel->put("field(channelName,command.index)"));
    PvaClientPutDataPtr putData(pvaClientPut->getData());
    PVStructurePtr pv(putData->getPVStructure());
    pv->getSubField<PVInt>("command.index")->put(3);
    pv->getSubField<PVString>("channelName")->put(channelName);
    putData->getChangedBitSet()->set(0);
    pvaClientPut->put();
}


void ClientCodec::stopMonitor()
{
    if(!channelConnected) {
         cerr << "not connected to codecChannel\n";
         return;
    }
    PvaClientPutPtr pvaClientPut(pvaClientChannel->put("field(command.index)"));
    PvaClientPutDataPtr putData(pvaClientPut->getData());
    PVStructurePtr pv(putData->getPVStructure());
    pv->getSubField<PVInt>("command.index")->put(4);
    putData->getChangedBitSet()->set(0);
    pvaClientPut->put();
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
   
    cout << "_____clientMonitorCodec starting__\n";
    PvaClientPtr pvaClient(PvaClient::get("pva"));
    try {  
        if(debug) PvaClient::setDebug(true);
        ClientCodecPtr clientCodec(ClientCodec::create(pvaClient,codecChannelName));
        while(true) {
            cout << "enter one of: startMonitor stopMonitor exit\n";
            string str;
            getline(cin,str);
            if(str.compare("startMonitor")==0){
                 cout << "enter channelName\n";
                 getline(cin,str);
                 clientCodec->startMonitor(str);
                 continue;
            }
            if(str.compare("stopMonitor")==0){
                 clientCodec->stopMonitor();
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
