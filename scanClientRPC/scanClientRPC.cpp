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
#include <pv/rpcClient.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;
static StructureConstPtr makeRequestStructure()
{
    static StructureConstPtr requestStructure;
    if (requestStructure.get() == 0)
    {
        FieldCreatePtr fieldCreate = getFieldCreate();

        requestStructure = fieldCreate->createFieldBuilder()->
            add("method", pvString)->
            createStructure();
    }
    return requestStructure;
}

static StructureConstPtr makePointStructure()
{
    static StructureConstPtr pointStructure;
    if (pointStructure.get() == 0)
    {
        FieldCreatePtr fieldCreate = getFieldCreate();

        pointStructure = fieldCreate->createFieldBuilder()->
            setId("point_t")->
            add("x",pvDouble)->
            add("y",pvDouble)->
            createStructure();
    }
    return pointStructure;
}

static StructureConstPtr makeArgumentStructure()
{
    static StructureConstPtr argStructure;
    if (argStructure.get() == 0)
    {
        FieldCreatePtr fieldCreate = getFieldCreate();

        argStructure = fieldCreate->createFieldBuilder()->
            createStructure();
    }
    return argStructure;
}

static StructureConstPtr makeConfigureArgumentStructure()
{
    static StructureConstPtr argStructure;
    if (argStructure.get() == 0)
    {
        FieldCreatePtr fieldCreate = getFieldCreate();

        argStructure = fieldCreate->createFieldBuilder()->
            addArray("value", makePointStructure())->
            createStructure();
    }
    return argStructure;
}


class ClientRPC;
typedef std::tr1::shared_ptr<ClientRPC> ClientRPCPtr;

class ClientRPC :
    public PvaClientChannelStateChangeRequester,
    public std::tr1::enable_shared_from_this<ClientRPC>
{
private:
    string channelName;
    string providerName;
    string request;
    bool channelConnected;

    PvaClientChannelPtr pvaClientChannel;

    void init(PvaClientPtr const &pvaClient)
    {
        pvaClientChannel = pvaClient->createChannel(channelName,providerName);
        pvaClientChannel->setStateChangeRequester(shared_from_this());
        pvaClientChannel->issueConnect();
    }
public:
    POINTER_DEFINITIONS(ClientRPC);
    ClientRPC(
        const string &channelName,
        const string &providerName,
        const string &request)
    : channelName(channelName),
      providerName(providerName),
      request(request),
      channelConnected(false)
    {
    }
    
    static ClientRPCPtr create(
        PvaClientPtr const &pvaClient,
        const string & channelName,
        const string & providerName,
        const string  & request)
    {
        ClientRPCPtr client(ClientRPCPtr(
             new ClientRPC(channelName,providerName,request)));
        client->init(pvaClient);
        return client;
    }

    virtual void channelStateChange(PvaClientChannelPtr const & channel, bool isConnected)
    {
        channelConnected = isConnected;
    }

    void commandConfigure(const string & input)
    {
        if(!channelConnected) {
            cout << channelName << " channel not connected\n";
            return;
        }
        vector<string> strvalues;
        size_t pos = 0;
        size_t n = 1;
        while(true)
        {
            size_t offset = input.find(" ",pos);
            if(offset==string::npos) {
                 strvalues.push_back(input.substr(pos));
                 break;
            }
            strvalues.push_back(input.substr(pos,offset-pos));
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
             x[i] = strvalues[ind++];
             y[i] = strvalues[ind++];
        }
        PVStructurePtr pvArguments(
             getPVDataCreate()->createPVStructure(
                 makeConfigureArgumentStructure()));
        PVStructureArray::svector values;
        for(size_t i=0; i< npts; ++i) 
        {
             PVStructurePtr point(getPVDataCreate()->createPVStructure(makePointStructure()));
             point->getSubField<PVDouble>("x")->put(stod(x[i]));
             point->getSubField<PVDouble>("y")->put(stod(y[i]));
             values.push_back(point);
        }
        pvArguments->getSubField<PVStructureArray>("value")->replace(freeze(values));
        PVStructurePtr pvRequest = 
             getPVDataCreate()->createPVStructure(makeRequestStructure());
        pvRequest->getSubFieldT<PVString>("method")->put("configure");
        PVStructurePtr response(pvaClientChannel->rpc(pvRequest,pvArguments));
        cout << "response\n" << response << "\n";
    }

    void commandStart()
    {
        if(!channelConnected) {
            cout << channelName << " channel not connected\n";
            return;
        }
        PVStructurePtr pvArguments(
             getPVDataCreate()->createPVStructure(
                 makeArgumentStructure()));
        PVStructurePtr pvRequest = 
             getPVDataCreate()->createPVStructure(makeRequestStructure());
        pvRequest->getSubFieldT<PVString>("method")->put("start");
        PVStructurePtr response(pvaClientChannel->rpc(pvRequest,pvArguments));
        cout << "response\n" << response << "\n";
    }

    void commandStop()
    {
        if(!channelConnected) {
            cout << channelName << " channel not connected\n";
            return;
        }
        PVStructurePtr pvArguments(
             getPVDataCreate()->createPVStructure(
                 makeArgumentStructure()));
        PVStructurePtr pvRequest = 
             getPVDataCreate()->createPVStructure(makeRequestStructure());
        pvRequest->getSubFieldT<PVString>("method")->put("stop");
        PVStructurePtr response(pvaClientChannel->rpc(pvRequest,pvArguments));
        cout << "response\n" << response << "\n";       
    }
};

static void help()
{
    cout << "if -i is specified then interactive mode is specified\n"
         << "     in this case enter -h for details\n";
    cout << "following are choices for non interactive mode:\n";
    cout << "   configure x0 y0 ... xn yn\n";
    cout << "   start\n";
    cout << "   stop\n";
}

int main(int argc,char *argv[])
{
    if(argc<2)
    {
        help();
        return 0;
    }
    if(argc==2)
    {
        string value(argv[1]);
        if(value=="help")
        {
            help();
            return 0;
        }
    }
    string provider("pva");
    string channelName("scanServerRPC");
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
                  << " -i " << (interactive ? "true" : "false") 
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
    int nPvs = argc - optind;       /* Remaining arg list for not interactive */
    if(debug) PvaClient::setDebug(true);
    try {   
        PvaClientPtr pva= PvaClient::get(provider);
        ClientRPCPtr clientRPC(ClientRPC::create(pva,channelName,provider,request));
        if(!interactive) {
            epicsThreadSleep(1.0);
            if(nPvs<1) throw std::runtime_error("no command specified");
            string command = argv[optind];
            if(command=="configure") {
                string input;
                bool first = true;
                for(int i= optind+1; i < argc; ++i)
                {
                    if(!first) { input += " ";} else { first=false;}
                    input += argv[i];
                }
                clientRPC->commandConfigure(input);
            } else if(command=="start") {
                 clientRPC->commandStart();
            } else if(command=="stop") {
                 clientRPC->commandStop();
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
                    clientRPC->commandConfigure(input);
                } else if(command=="start") {
                     clientRPC->commandStart();
                } else if(command=="stop") {
                     clientRPC->commandStop();
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
