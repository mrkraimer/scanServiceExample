/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 */

/* Author: Marty Kraimer */
#include <iostream>
#include <epicsGetopt.h>
#include <epicsThread.h>
#include <pv/pvaClient.h>
#include <pv/convert.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;

class ClientPutGet;
typedef std::tr1::shared_ptr<ClientPutGet> ClientPutGetPtr;

class ClientPutGet :
    public PvaClientChannelStateChangeRequester,
    public std::tr1::enable_shared_from_this<ClientPutGet>
{
private:
    string channelName;
    string providerName;
    string request;
    bool channelConnected;

    PvaClientChannelPtr pvaClientChannel;
    PvaClientPutGetPtr pvaClientPutGet;

    void init(PvaClientPtr const &pvaClient)
    {
        pvaClientChannel = pvaClient->createChannel(channelName,providerName);
        pvaClientChannel->setStateChangeRequester(shared_from_this());
        pvaClientChannel->issueConnect();
    }
public:
    POINTER_DEFINITIONS(ClientPutGet);
    ClientPutGet(
        const string &channelName,
        const string &providerName,
        const string &request)
    : channelName(channelName),
      providerName(providerName),
      request(request),
      channelConnected(false)
    {
    }
    
    static ClientPutGetPtr create(
        PvaClientPtr const &pvaClient,
        const string & channelName,
        const string & providerName,
        const string  & request)
    {
        ClientPutGetPtr client(ClientPutGetPtr(
             new ClientPutGet(channelName,providerName,request)));
        client->init(pvaClient);
        return client;
    }

    virtual void channelStateChange(PvaClientChannelPtr const & channel, bool isConnected)
    {
        channelConnected = isConnected;
        if(isConnected) {
            if(!pvaClientPutGet) {
                pvaClientPutGet = pvaClientChannel->createPutGet(request);
                pvaClientPutGet->issueConnect();
            }
        }
    }

    void putGetConfigure(const string & input)
    {
        if(!channelConnected) {
            cout << channelName << " channel not connected\n";
            return;
        }
        vector<string> values;
        size_t pos = 0;
        size_t n = 1;
        while(true)
        {
            size_t offset = input.find(" ",pos);
            if(offset==string::npos) {
                 values.push_back(input.substr(pos));
                 break;
            }
            values.push_back(input.substr(pos,offset-pos));
            pos = offset+1;
            n++;
        }
        size_t npts = n/2;
        if(npts*2 != n) {
            cout << "failure: odd number of points\n";
            return;
        }
        vector<string> x(npts);
        vector<string> y(npts);
        size_t ind = 0;
        for(size_t i= 0; i < npts; ++i)
        {
             x[i] = values[ind++];
             y[i] = values[ind++];
        }                   
        PvaClientPutDataPtr putData = pvaClientPutGet->getPutData();
        PVStructurePtr pvStructure = putData->getPVStructure();
cout << "pvStructure\n" << pvStructure << "\n";
        PVScalarArrayPtr pvx(pvStructure->getSubField<PVScalarArray>("argument.configArg.x"));
        if(!pvx) throw std::runtime_error("argument.configArg.x not found");
        PVScalarArrayPtr pvy(pvStructure->getSubField<PVScalarArray>("argument.configArg.y"));
        if(!pvy) throw std::runtime_error("argument.configArg.y not found");
        ConvertPtr convert = getConvert();
        pvx->setLength(npts);
        convert->fromStringArray(pvx,0,npts,x,0);        
        pvy->setLength(npts);
        convert->fromStringArray(pvy,0,npts,y,0);    
        PVStringPtr pvCommand(pvStructure->getSubField<PVString>("argument.command"));
        if(!pvCommand) throw std::runtime_error("argument.command not found");
        pvCommand->put("configure");
        pvaClientPutGet->putGet();
        PvaClientGetDataPtr getData = pvaClientPutGet->getGetData();
        cout << getData->getPVStructure() << endl;
    }

    void putGetStart()
    {
        if(!channelConnected) {
            cout << channelName << " channel not connected\n";
            return;
        }
        PvaClientPutDataPtr putData = pvaClientPutGet->getPutData();
        PVStructurePtr pvStructure = putData->getPVStructure();
        PVStringPtr pvCommand(pvStructure->getSubField<PVString>("argument.command"));
        if(!pvCommand) throw std::runtime_error("argument.command not found");
        pvCommand->put("start");
        pvaClientPutGet->putGet();
    }

    void putGetStop()
    {
        if(!channelConnected) {
            cout << channelName << " channel not connected\n";
            return;
        }
        PvaClientPutDataPtr putData = pvaClientPutGet->getPutData();
        PVStructurePtr pvStructure = putData->getPVStructure();
        PVStringPtr pvCommand(pvStructure->getSubField<PVString>("argument.command"));
        if(!pvCommand) throw std::runtime_error("argument.command not found");
        pvCommand->put("stop");
        pvaClientPutGet->putGet();
    }
};


int main(int argc,char *argv[])
{
    string provider("pva");
    string channelName("scanServerPutGet");
    string request("putField(argument)getField(result)");
    bool debug(false);
    bool interactive(false);
    int opt;
    while((opt = getopt(argc, argv, "hp:n:r:d:i:")) != -1) {
        switch(opt) {
            case 'p':
                provider = optarg;
                break;
            case 'n':
                channelName = optarg;
                break;
            case 'r':
                request = optarg;
                break;
            case 'h':
             cout << " -h -n channelName -p provider -r request -i interactive - d debug " << endl;
             cout << "default" << endl;
             cout << "-p " << provider 
                  << " -n " <<  channelName
                  << " -r " << request
                  << " -d " << (debug ? "true" : "false") 
                  << endl;           
                return 0;
            case 'd' :
               if(string(optarg)=="true") debug = true;
               break;
            case 'i' :
               if(string(optarg)=="true") interactive = true;
               break;
            default:
                std::cerr<<"Unknown argument: "<<opt<<"\n";
                return -1;
        }
    }
    bool pvaSrv(((provider.find("pva")==string::npos) ? false : true));
    bool caSrv(((provider.find("ca")==string::npos) ? false : true));
    if(pvaSrv&&caSrv) {
        cerr<< "multiple providers are not allowed\n";
        return 1;
    }
    cout << "provider " << provider
         << " channelName " <<  channelName
         << " request " << request
         << " debug " << (debug ? "true" : "false")
         << " interactive " << (interactive ? "true" : "false")
         << endl;
    int nPvs = argc - optind;       /* Remaining arg list for not interactive */
    cout << "_____scanClientPutGet starting__\n";
    if(debug) PvaClient::setDebug(true);
    try {   
        PvaClientPtr pva= PvaClient::get(provider);
        ClientPutGetPtr clientPutGet(ClientPutGet::create(pva,channelName,provider,request));
        if(!interactive) {
            epicsThreadSleep(1.0);
            if(nPvs<1) throw std::runtime_error("no command specified");
            string command = argv[optind];
            if(command=="configure") {
                string input;
                bool first = true;
                for(int i= optind+1; i < argc; ++i)
                {
                    if(!first) { input += "";} else { first=false;}
                    input += argv[i];
                }
                clientPutGet->putGetConfigure(input);
            } else if(command=="start") {
                 clientPutGet->putGetStart();
            } else if(command=="stop") {
                 clientPutGet->putGetStop();
            } else {
                cout << "unknown command\n";
            }
        } else {
            while(true)
            {
                cout << "enter one of: exit configure start stop\n";
                int c = std::cin.peek();  // peek character
                if ( c == EOF ) continue;
                string command;
                getline(cin,command);
                if(command.compare("exit")==0) break;
                if(command=="configure") 
                {
                    cout << "enter x y values\n";
                    string input;
                    getline(cin,input);
                    clientPutGet->putGetConfigure(input);
                } else if(command=="start") {
                     clientPutGet->putGetStart();
                } else if(command=="stop") {
                     clientPutGet->putGetStop();
                } else {
                        cout << "unknown command\n";
                }
                continue;
            }
        }
    } catch (std::exception& e) {
        cerr << "exception " << e.what() << endl;
        return 1;
    }
    return 0;
}
