/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */
/**
 * @author dgh Dave Hickin and Marty Kraimer
 * @date 2019.04.18
 */

#ifndef SCANSERVERRPC_H
#define SCANSERVERRPC_H

#include <pv/pvDatabase.h>
#include <pv/timeStamp.h>
#include <pv/pvTimeStamp.h>
#include <pv/scanService.h>

#include <shareLib.h>


namespace epics { namespace exampleScan { 

class AbortService;
typedef std::tr1::shared_ptr<AbortService> AbortServicePtr;

class ConfigureService;
typedef std::tr1::shared_ptr<ConfigureService> ConfigureServicePtr;

class RunService;
typedef std::tr1::shared_ptr<RunService> RunServicePtr;

class PauseService;
typedef std::tr1::shared_ptr<PauseService> PauseServicePtr;

class ResumeService;
typedef std::tr1::shared_ptr<ResumeService> ResumeServicePtr;

class StopService;
typedef std::tr1::shared_ptr<StopService> StopServicePtr;

class RewindService;
typedef std::tr1::shared_ptr<RewindService> RewindServicePtr;

class ScanRPCService;
typedef std::tr1::shared_ptr<ScanRPCService> ScanRPCServicePtr;

class ScanServerRPC;
typedef std::tr1::shared_ptr<ScanServerRPC> ScanServerRPCPtr;

class epicsShareClass AbortService :
    public epics::pvAccess::RPCServiceAsync
{
public:
    POINTER_DEFINITIONS(AbortService);

    static AbortService::shared_pointer create(ScanServerRPCPtr const & pvRecord)
    {
        return AbortServicePtr(new AbortService(pvRecord));
    }
    ~AbortService() {};
 
    void request(
        epics::pvData::PVStructurePtr const & args,
        epics::pvAccess::RPCResponseCallback::shared_pointer const & callback
    );
private:
    AbortService(ScanServerRPCPtr const & pvRecord)
    : pvRecord(pvRecord)
    {
    }

    ScanServerRPCPtr pvRecord;
};


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


class epicsShareClass RunService :
    public virtual epics::pvAccess::RPCServiceAsync
{
public:
    POINTER_DEFINITIONS(RunService);

    static RunService::shared_pointer create(ScanServerRPCPtr const & pvRecord)
    {
        return RunServicePtr(new RunService(pvRecord));
    }
    ~RunService() {};

    void request(
        epics::pvData::PVStructurePtr const & args,
        epics::pvAccess::RPCResponseCallback::shared_pointer const & callback
    ); 
private:
    RunService(ScanServerRPCPtr const & pvRecord)
    : pvRecord(pvRecord)
    {
    }

    ScanServerRPCPtr pvRecord;
};

class epicsShareClass PauseService :
    public virtual epics::pvAccess::RPCServiceAsync
{
public:
    POINTER_DEFINITIONS(PauseService);

    static PauseService::shared_pointer create(ScanServerRPCPtr const & pvRecord)
    {
        return PauseServicePtr(new PauseService(pvRecord));
    }
    ~PauseService() {};

    void request(
        epics::pvData::PVStructurePtr const & args,
        epics::pvAccess::RPCResponseCallback::shared_pointer const & callback
    ); 
private:
    PauseService(ScanServerRPCPtr const & pvRecord)
    : pvRecord(pvRecord)
    {
    }

    ScanServerRPCPtr pvRecord;
};

class epicsShareClass ResumeService :
    public virtual epics::pvAccess::RPCServiceAsync
{
public:
    POINTER_DEFINITIONS(ResumeService);

    static ResumeService::shared_pointer create(ScanServerRPCPtr const & pvRecord)
    {
        return ResumeServicePtr(new ResumeService(pvRecord));
    }
    ~ResumeService() {};

    void request(
        epics::pvData::PVStructurePtr const & args,
        epics::pvAccess::RPCResponseCallback::shared_pointer const & callback
    ); 
private:
    ResumeService(ScanServerRPCPtr const & pvRecord)
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

class epicsShareClass RewindService :
    public virtual epics::pvAccess::RPCServiceAsync
{
public:
    POINTER_DEFINITIONS(RewindService);

    static RewindService::shared_pointer create(ScanServerRPCPtr const & pvRecord)
    {
        return RewindServicePtr(new RewindService(pvRecord));
    }
    ~RewindService() {};

    void request(
        epics::pvData::PVStructurePtr const & args,
        epics::pvAccess::RPCResponseCallback::shared_pointer const & callback
    ); 
private:
    int getRequestedSteps(epics::pvData::PVStructurePtr const & args);

private:
    RewindService(ScanServerRPCPtr const & pvRecord)
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

    virtual void stateChanged(ScanService::State state);

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

    epics::pvData::PVIntPtr         pvStateIndex;
    epics::pvData::PVStringArrayPtr pvStateChoices;

    epics::pvData::PVTimeStamp pvTimeStamp;
    epics::pvData::PVTimeStamp pvTimeStamp_sp;
    epics::pvData::PVTimeStamp pvTimeStamp_rb;
    epics::pvData::PVTimeStamp pvTimeStamp_st;

    bool firstTime;

    ScanServicePtr scanService;
};


}}

#endif  /* SCANSERVERRPC_H */
