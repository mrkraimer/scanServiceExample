/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 */

/* Author: Marty Kraimer */
#include <iostream>
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
        putData->getChangedBitSet()->clear();
        PVStructurePtr pvStructure = putData->getPVStructure();
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
        putData->getChangedBitSet()->clear();
        PVStructurePtr pvStructure = putData->getPVStructure();
        PVStringPtr pvCommand(pvStructure->getSubField<PVString>("argument.command"));
        if(!pvCommand) throw std::runtime_error("argument.command not found");
        pvCommand->put("start");
        pvaClientPutGet->putGet();
        PvaClientGetDataPtr getData = pvaClientPutGet->getGetData();
        cout << getData->getPVStructure() << endl;
    }

    void putGetStop()
    {
        if(!channelConnected) {
            cout << channelName << " channel not connected\n";
            return;
        }
        PvaClientPutDataPtr putData = pvaClientPutGet->getPutData();
        putData->getChangedBitSet()->clear();
        PVStructurePtr pvStructure = putData->getPVStructure();
        PVStringPtr pvCommand(pvStructure->getSubField<PVString>("argument.command"));
        if(!pvCommand) throw std::runtime_error("argument.command not found");
        pvCommand->put("stop");
        pvaClientPutGet->putGet();
        PvaClientGetDataPtr getData = pvaClientPutGet->getGetData();
        cout << getData->getPVStructure() << endl;
    }

    void putGetSetRate(double stepDelay,double stepDistance)
    {
        if(!channelConnected) {
            cout << channelName << " channel not connected\n";
            return;
        }
        PvaClientPutDataPtr putData = pvaClientPutGet->getPutData();
        putData->getChangedBitSet()->clear();
        PVStructurePtr pvStructure = putData->getPVStructure();
        PVDoublePtr pvStepDelay(pvStructure->getSubField<PVDouble>("argument.rateArg.stepDelay"));
        if(!pvStepDelay) throw std::runtime_error("argument.rateArg.stepDelay not found");
        PVDoublePtr pvStepDistance(pvStructure->getSubField<PVDouble>("argument.rateArg.stepDistance"));
        if(!pvStepDistance) throw std::runtime_error("argument.rateArg.stepDistance not found");
        pvStepDelay->put(stepDelay);
        pvStepDistance->put(stepDistance);
        PVStringPtr pvCommand(pvStructure->getSubField<PVString>("argument.command"));
        if(!pvCommand) throw std::runtime_error("argument.command not found");
        pvCommand->put("setRate");
        pvaClientPutGet->putGet();
        PvaClientGetDataPtr getData = pvaClientPutGet->getGetData();
        cout << getData->getPVStructure() << endl;
    }

    void putGetSetDebug(bool value)
    {
        if(!channelConnected) {
            cout << channelName << " channel not connected\n";
            return;
        }
        PvaClientPutDataPtr putData = pvaClientPutGet->getPutData();
        putData->getChangedBitSet()->clear();
        PVStructurePtr pvStructure = putData->getPVStructure();
        PVBooleanPtr pvDebug(pvStructure->getSubField<PVBoolean>("argument.debugArg.value"));
        if(!pvDebug) throw std::runtime_error("argument.debugArg.value not found");
        pvDebug->put(value);
        PVStringPtr pvCommand(pvStructure->getSubField<PVString>("argument.command"));
        if(!pvCommand) throw std::runtime_error("argument.command not found");
        pvCommand->put("setDebug");
        pvaClientPutGet->putGet();
        PvaClientGetDataPtr getData = pvaClientPutGet->getGetData();
        cout << getData->getPVStructure() << endl;
    }


};

static void help()
{
    cout << "if interactive is specified then interactive mode is specified\n";
    cout << "following are choices for non interactive mode:\n";
    cout << "   configure x0 y0 ... xn yn\n";
    cout << "   start\n";
    cout << "   stop\n";
    cout << "   setRate stepDelay stepDistance\n";
    cout << "   setDebug true|false\n";
}

int main(int argc,char *argv[])
{
    if(argc<2)
    {
        help();
        return 0;
    }
    bool interactive(false);
    if(argc==2)
    {
        string value(argv[1]);
        if(value.size()>0 && value[0]=='i') interactive = true;
    }
    string provider("pva");
    string channelName("scanServerPutGet");
    string request("putField(argument)getField(result)");
    try {   
        PvaClientPtr pva= PvaClient::get(provider);
        ClientPutGetPtr clientPutGet(ClientPutGet::create(pva,channelName,provider,request));
        if(!interactive) {
            epicsThreadSleep(1.0);
            if(argc<2) throw std::runtime_error("no command specified");
            string command = argv[1];
            if(command=="configure") {
                string input;
                bool first = true;
                if(argc<4) throw std::runtime_error("illegal number of points");
                for(int i= 2; i < argc; ++i)
                {
                    if(!first) { input += " ";} else { first=false;}
                    input += argv[i];
                }
                clientPutGet->putGetConfigure(input);
            } else if(command=="start") {
                 clientPutGet->putGetStart();
            } else if(command=="stop") {
                 clientPutGet->putGetStop();
            } else if(command=="setRate") {
                 if(argc!=4) throw std::runtime_error("illegal number of arguments");
                 double stepDelay = stod(argv[2]);
                 double stepDistance = stod(argv[3]);
                 clientPutGet->putGetSetRate(stepDelay,stepDistance);
            } else if(command=="setDebug") {
                 if(argc!=3) throw std::runtime_error("illegal number of arguments");
                 string sval = argv[2];
                 bool value = (sval=="true" ? "true" : "false");
                 clientPutGet->putGetSetDebug(value);
            } else {
                cout << "unknown command\n";
            }
        } else {
            while(true)
            {
                cout << "enter one of: exit configure start stop setRate setDebug\n";
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
                } else if(command=="setRate") {
                     cout << "enter stepDelay\n";
                     string input;
                     getline(cin,input);
                     double stepDelay = stod(input);
                     cout << "enter stepDistance\n";
                     getline(cin,input);
                     double stepDistance = stod(input);
                     clientPutGet->putGetSetRate(stepDelay,stepDistance);
                } else if(command=="setDebug") {
                     cout << "enter true or false\n";
                     string input;
                     getline(cin,input);
                     bool value = (input=="true" ? "true" : "false");
                     clientPutGet->putGetSetDebug(value);
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
