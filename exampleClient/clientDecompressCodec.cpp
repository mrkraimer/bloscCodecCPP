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
#include <epicsThread.h>
#include <pv/pvaClient.h>
#include <pv/event.h>
#include <pv/timeStamp.h>
#include <pv/pvEnumerated.h>
#include <pv/bloscCodec.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;
using namespace epics::codec;

typedef epicsGuard<epicsMutex> Guard;
typedef epicsGuardRelease<epicsMutex> UnGuard;

class ClientCodec;
typedef std::tr1::shared_ptr<ClientCodec> ClientCodecPtr;

class ClientCodec :
    public PvaClientChannelStateChangeRequester,
    public PvaClientMonitorRequester,
    public epicsThreadRunable,
    public std::tr1::enable_shared_from_this<ClientCodec>
{
private:
    PvaClientPtr pvaClient;
    string codecChannelName;
    string channelName;
    PvaClientChannelPtr pvaClientChannel;
    bool channelConnected;
    bool monitorConnected;
    bool runIsActive;
public:
    POINTER_DEFINITIONS(ClientCodec);
    
private:
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
    void start(const string & channelName);
    void stop();
    virtual void run();
    virtual void monitorConnect(Status const & status,
        PvaClientMonitorPtr const & monitor,StructureConstPtr const & structure);
    virtual void event(PvaClientMonitorPtr const & monitor);
    Event runStop;
private:
    std::auto_ptr<epicsThread> thread;
    ClientCodec(
         PvaClientPtr const &pvaClient,
         const string & codecChannelName
    )
    : pvaClient(pvaClient),
      codecChannelName(codecChannelName),
      channelConnected(false),
      monitorConnected(false),
      runIsActive(false)
    {
    }
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

void ClientCodec::run()
{
   PvaClientMonitorPtr pvaClientMonitor(pvaClientChannel->createMonitor(""));
   pvaClientMonitor->setRequester(shared_from_this());
   pvaClientMonitor->issueConnect();
   runStop.wait();
   pvaClientMonitor->stop();
}

void ClientCodec::stop()
{
    if(!runIsActive) return;
    runStop.signal();
    runIsActive = false;
}


void ClientCodec::monitorConnect(Status const & status,
    PvaClientMonitorPtr const & monitor,StructureConstPtr const & structure)
{
    cout << "monitorConnect " << channelName << " status " << status << endl;
    if(!status.isOK()) return;
    monitor->start();
}
    
void ClientCodec::event(PvaClientMonitorPtr const & monitor)
{
    while(monitor->poll()) {
        PvaClientMonitorDataPtr monitorData = monitor->getData();
        PVStructurePtr pvStructure(monitorData->getPVStructure());
        bool doit = true;
        if(pvStructure->getSubField<PVInt>("command.index")->get()!=1) doit = false;
        string channelName = pvStructure->getSubField<PVString>("channelName")->get();
        if(channelName.compare(this->channelName)!=0) doit = false;
        if(doit)
        {
            BloscCodecPtr codec(BloscCodec::create());
            FieldCreatePtr fieldCreate = getFieldCreate();
            PVDataCreatePtr pvDataCreate = getPVDataCreate();
            int elementScalarType = pvStructure->getSubField<PVInt>("elementScalarType")->get();
            StructureConstPtr  topStructure = fieldCreate->createFieldBuilder()->
                addArray("value",(ScalarType)elementScalarType)->
            createStructure();
            PVStructurePtr pvStructureDest = pvDataCreate->createPVStructure(topStructure);
            PVUByteArrayPtr pvSource = pvStructure->getSubField<PVUByteArray>("value");
            PVScalarArrayPtr pvDest = pvStructureDest->getSubField<PVScalarArray>("value");
            PVStructurePtr pvBloscArg = pvStructure->getSubField<PVStructure>("bloscArgs");
            bool result = codec->decompressBlosc(pvSource,pvDest,pvBloscArg);
            cout << "result " << (result ? "success" : "failure") 
            << " data\n" << pvDest << "\n";
        }
        monitor->releaseEvent();
    }
}

void ClientCodec::start(const string &channelName)
{
    if(!channelConnected) {
         cerr << "not connected to codecChannel\n";
         return;
    }
    this->channelName = channelName;
    runIsActive = true;
    thread =  std::auto_ptr<epicsThread>(new epicsThread(
        *this,
        "clientMonitorCodec",
        epicsThreadGetStackSize(epicsThreadStackSmall),
        epicsThreadPriorityLow));
    thread->start();
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
            cout << "enter one of: start stop exit\n";
            string str;
            getline(cin,str);
            if(str.compare("start")==0){
                 cout << "enter channelName\n";
                 getline(cin,str);
                 clientCodec->start(str);
                 continue;
            }
            if(str.compare("stop")==0){
                 clientCodec->stop();
                 continue;
            }
            if(str.compare("exit")==0) break;
            cout << "illegal command\n";
        }
        clientCodec->stop();
    } catch (std::exception& e) {
        cerr << "exception " << e.what() << endl;
        return 1;
    }
    return 0;
}
