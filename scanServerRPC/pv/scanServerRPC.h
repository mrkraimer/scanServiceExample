/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */
/**
 * @author dgh Dave Hickin and Marty Kraimer
 * @date 2019.04
 */

#ifndef SCANSERVERRPC_H
#define SCANSERVERRPC_H

#include <pv/pvDatabase.h>
#include <pv/timeStamp.h>
#include <pv/pvTimeStamp.h>
#include <pv/scanService.h>

#include <shareLib.h>


namespace epics { namespace exampleScan { 


class ConfigureService;
typedef std::tr1::shared_ptr<ConfigureService> ConfigureServicePtr;

class StartService;
typedef std::tr1::shared_ptr<StartService> StartServicePtr;

class StopService;
typedef std::tr1::shared_ptr<StopService> StopServicePtr;

class SetRateService;
typedef std::tr1::shared_ptr<SetRateService> SetRateServicePtr;

class SetDebugService;
typedef std::tr1::shared_ptr<SetDebugService> SetDebugServicePtr;


class ScanRPCService;
typedef std::tr1::shared_ptr<ScanRPCService> ScanRPCServicePtr;

class ScanServerRPC;
typedef std::tr1::shared_ptr<ScanServerRPC> ScanServerRPCPtr;

class epicsShareClass ConfigureService :
    public virtual epics::pvAccess::RPCServiceAsync
{
public:
    POINTER_DEFINITIONS(ConfigureService);

    static ConfigureService::shared_pointer create(ScanServerRPCPtr const & pvRecord)
    {
        return ConfigureServicePtr(new ConfigureService(pvRecord));
    }
    ~ConfigureService() {};

    void request(
        epics::pvData::PVStructurePtr const & args,
        epics::pvAccess::RPCResponseCallback::shared_pointer const & callback
    );
private:
    ConfigureService(ScanServerRPCPtr const & pvRecord)
    : pvRecord(pvRecord)
    {
    }

    ScanServerRPCPtr pvRecord;
};


class epicsShareClass StartService :
    public virtual epics::pvAccess::RPCServiceAsync
{
public:
    POINTER_DEFINITIONS(StartService);

    static StartService::shared_pointer create(ScanServerRPCPtr const & pvRecord)
    {
        return StartServicePtr(new StartService(pvRecord));
    }
    ~StartService() {};

    void request(
        epics::pvData::PVStructurePtr const & args,
        epics::pvAccess::RPCResponseCallback::shared_pointer const & callback
    ); 
private:
    StartService(ScanServerRPCPtr const & pvRecord)
    : pvRecord(pvRecord)
    {
    }

    ScanServerRPCPtr pvRecord;
};

class epicsShareClass StopService :
    public virtual epics::pvAccess::RPCServiceAsync
{
public:
    POINTER_DEFINITIONS(StopService);

    static StopService::shared_pointer create(ScanServerRPCPtr const & pvRecord)
    {
        return StopServicePtr(new StopService(pvRecord));
    }
    ~StopService() {};

    void request(
        epics::pvData::PVStructurePtr const & args,
        epics::pvAccess::RPCResponseCallback::shared_pointer const & callback
    ); 
private:
    StopService(ScanServerRPCPtr const & pvRecord)
    : pvRecord(pvRecord)
    {
    }

    ScanServerRPCPtr pvRecord;
};

class epicsShareClass SetRateService :
    public virtual epics::pvAccess::RPCServiceAsync
{
public:
    POINTER_DEFINITIONS(SetRateService);

    static SetRateService::shared_pointer create(ScanServerRPCPtr const & pvRecord)
    {
        return SetRateServicePtr(new SetRateService(pvRecord));
    }
    ~SetRateService() {};

    void request(
        epics::pvData::PVStructurePtr const & args,
        epics::pvAccess::RPCResponseCallback::shared_pointer const & callback
    ); 
private:
    SetRateService(ScanServerRPCPtr const & pvRecord)
    : pvRecord(pvRecord)
    {
    }

    ScanServerRPCPtr pvRecord;
};

class epicsShareClass SetDebugService :
    public virtual epics::pvAccess::RPCServiceAsync
{
public:
    POINTER_DEFINITIONS(SetDebugService);

    static SetDebugService::shared_pointer create(ScanServerRPCPtr const & pvRecord)
    {
        return SetDebugServicePtr(new SetDebugService(pvRecord));
    }
    ~SetDebugService() {};

    void request(
        epics::pvData::PVStructurePtr const & args,
        epics::pvAccess::RPCResponseCallback::shared_pointer const & callback
    ); 
private:
    SetDebugService(ScanServerRPCPtr const & pvRecord)
    : pvRecord(pvRecord)
    {
    }

    ScanServerRPCPtr pvRecord;
};

class epicsShareClass ScanRPCService :
    public epics::pvAccess::RPCServiceAsync,
    public std::tr1::enable_shared_from_this<ScanRPCService>
{
public:
    POINTER_DEFINITIONS(ScanRPCService);

    virtual void update(int flags);

    class Callback : public ScanService::Callback
    {
    public:
        POINTER_DEFINITIONS(Callback);
        static Callback::shared_pointer create(ScanRPCServicePtr const & record);

        virtual void update(int flags);

    private:
        Callback(ScanRPCServicePtr service)
        : service(service)
        {}

        ScanRPCServicePtr service;
    };

    static ScanRPCService::shared_pointer create(ScanServerRPCPtr const & pvRecord)
    {
        return ScanRPCServicePtr(new ScanRPCService(pvRecord));
    }
    
    void request(
        epics::pvData::PVStructurePtr const & args,
        epics::pvAccess::RPCResponseCallback::shared_pointer const & callback
    );
private:
    ScanRPCService(ScanServerRPCPtr const & pvRecord)
    : pvRecord(pvRecord)
    {
    }

    virtual void scanComplete();

    void handleError(const std::string & message);

    epics::pvAccess::RPCResponseCallback::shared_pointer rpcCallback;

    Callback::shared_pointer scanServiceCallback;

    ScanServerRPCPtr pvRecord;
};



class epicsShareClass ScanServerRPC :
    public epics::pvDatabase::PVRecord
{
public:
    POINTER_DEFINITIONS(ScanServerRPC);
    static ScanServerRPCPtr create(
        std::string const & recordName);
    virtual ~ScanServerRPC() {}
    virtual bool init() {return false;}
    virtual void process();
    virtual epics::pvAccess::RPCServiceAsync::shared_pointer getService(
        epics::pvData::PVStructurePtr const & pvRequest);

    class Callback : public ScanService::Callback
    {
    public:
        POINTER_DEFINITIONS(Callback);
        static Callback::shared_pointer create(ScanServerRPCPtr const & record);

        virtual void update(int flags);

    private:
        Callback(ScanServerRPCPtr record)
        : record(record)
        {}
        ScanServerRPCPtr record;
    };

    virtual void update(int flags);

    ScanServicePtr getScanService() { return scanService; }

private:

    ScanServerRPC(std::string const & recordName,
        epics::pvData::PVStructurePtr const & pvStructure);
    void initPvt();

    epics::pvData::PVDoublePtr      pvx;
    epics::pvData::PVDoublePtr      pvy;
    epics::pvData::PVDoublePtr      pvx_rb;
    epics::pvData::PVDoublePtr      pvy_rb;

    epics::pvData::PVTimeStamp pvTimeStamp;
    epics::pvData::PVTimeStamp pvTimeStamp_sp;
    epics::pvData::PVTimeStamp pvTimeStamp_rb;

    bool firstTime;

    ScanServicePtr scanService;
};


}}

#endif  /* SCANSERVERRPC_H */
