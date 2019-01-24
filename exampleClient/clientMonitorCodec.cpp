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
#include <pv/pvEnumerated.h>


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
    bool firstStart;
    PVEnumerated compressor;
    PVEnumerated shuffle;
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
    string createPrompt(PVEnumerated & pvEnumerated);
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

string ClientCodec::createPrompt(PVEnumerated & pvEnumerated)
{
    stringstream ss;
    int n = pvEnumerated.getNumberChoices();
    for(int i=0; i<n; ++i)
    {
        if(i!=0) ss << ",";
        ss << i;
        ss << "=";
        pvEnumerated.setIndex(i);
        ss << pvEnumerated.getChoice();
    }
    string result = ss.str() + "\n";
    return result;
}

void ClientCodec::startMonitor(const string &channelName)
{
    if(!channelConnected) {
         cerr << "not connected to codecChannel\n";
         return;
    }
    if(firstStart) {
        PvaClientGetPtr pvaClientGet(pvaClientChannel->createGet("bloscArgs"));
        pvaClientGet->get();
        PvaClientGetDataPtr data(pvaClientGet->getData());
        PVStructurePtr pvStructure(data->getPVStructure());
        compressor.attach(pvStructure->getSubField("bloscArgs.compressor"));
        shuffle.attach(pvStructure->getSubField("bloscArgs.shuffle"));
        firstStart = false;
    }
    string putRequest(
      "putField(channelName,command.index,bloscArgs{level,compressor.index,shuffle.index,threads})");
    string getRequest(
      "getField(alarm)");
    PvaClientPutGetPtr pvaClientPutGet(
         pvaClientChannel->createPutGet(putRequest + getRequest));
    pvaClientPutGet->connect();
    PvaClientPutDataPtr putData(pvaClientPutGet->getPutData());
    PVStructurePtr pvPutData(putData->getPVStructure());
    pvPutData->getSubField<PVInt>("command.index")->put(3);
    pvPutData->getSubField<PVString>("channelName")->put(channelName);
    string str;
    cout << "do You want to modify any bloscArgs? answer y or n\n";
    getline(cin,str);
    if(str.compare("y")==0){
        int val = pvPutData->getSubField<PVInt>("bloscArgs.level")->get();
        cout << "level is " << val << " do you want to change it?\n";
        getline(cin,str);
        if(str.compare("y")==0){
             cout << "enter level\n";
             getline(cin,str);
             const char * cstr(str.c_str());
             val = std::stoul (cstr,nullptr,0);
             pvPutData->getSubField<PVInt>("bloscArgs.level")->put(val);
        }
        int index = pvPutData->getSubField<PVInt>("bloscArgs.compressor.index")->get();
        compressor.setIndex(index);
        string strval = compressor.getChoice();
        string prompt("compressor is ");
        prompt += strval + " do you want to change?\n";
        cout << prompt;
        getline(cin,str);
        if(str.compare("y")==0){
            cout << createPrompt(compressor);
            getline(cin,str);
            const char * cstr(str.c_str());
            index = std::stoul (cstr,nullptr,0);
            pvPutData->getSubField<PVInt>("bloscArgs.compressor.index")->put(index);
        }
        index = pvPutData->getSubField<PVInt>("bloscArgs.shuffle.index")->get();
        shuffle.setIndex(index);
        strval = shuffle.getChoice();
        prompt = "shuffle is ";
        prompt += strval + " do you want to change?\n";
        cout << prompt;
        getline(cin,str);
        if(str.compare("y")==0){
            cout << createPrompt(shuffle);
            getline(cin,str);
            const char * cstr(str.c_str());
            index = std::stoul (cstr,nullptr,0);
            pvPutData->getSubField<PVInt>("bloscArgs.shuffle.index")->put(index);
        }
        val = pvPutData->getSubField<PVInt>("bloscArgs.threads")->get();
        cout << "threads is " << val << " do you want to change it?\n";
        getline(cin,str);
        if(str.compare("y")==0){
             cout << "enter threads\n";
             getline(cin,str);
             const char * cstr(str.c_str());
             val = std::stoul (cstr,nullptr,0);
             pvPutData->getSubField<PVInt>("bloscArgs.threads")->put(val);
        }
    }
    putData->getChangedBitSet()->set(0);
    pvaClientPutGet->putGet();
    PvaClientGetDataPtr getData(pvaClientPutGet->getGetData());
    cout << getData->getPVStructure()->getSubField("alarm.message") << "\n";
}


void ClientCodec::stopMonitor()
{
    if(!channelConnected) {
         cerr << "not connected to codecChannel\n";
         return;
    }
    string putRequest(
      "putField(command.index)");
    string getRequest(
      "getField(alarm)");
    PvaClientPutGetPtr pvaClientPutGet(
         pvaClientChannel->createPutGet(putRequest + getRequest));
    pvaClientPutGet->connect();
    PvaClientPutDataPtr putData(pvaClientPutGet->getPutData());
    PVStructurePtr pvPutData(putData->getPVStructure());
    pvPutData->getSubField<PVInt>("command.index")->put(4);
    putData->getChangedBitSet()->set(0);
    pvaClientPutGet->putGet();
    PvaClientGetDataPtr getData(pvaClientPutGet->getGetData());
    cout << getData->getPVStructure()->getSubField("alarm.message") << "\n";
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
