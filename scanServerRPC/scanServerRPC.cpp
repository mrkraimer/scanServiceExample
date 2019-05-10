/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author dgh Dave Hickin and Marty Kraimer
 * @date 2019.04.18
 */

#include <pv/standardField.h>

#include <pv/standardField.h>
#include <pv/standardPVField.h>

#include <epicsThread.h>
#include <pv/scanService.h>
#include <epicsExport.h>
#include "pv/scanServerRPC.h"

using namespace epics::pvData;
using namespace epics::pvDatabase;
using std::tr1::static_pointer_cast;
using std::string;

namespace epics { namespace exampleScan {


static PVStructurePtr makeResultStructure(const std::string & result)
{
    static StructureConstPtr resultStructure;
    if (resultStructure.get() == 0)
    {
        FieldCreatePtr fieldCreate = getFieldCreate();

        resultStructure = fieldCreate->createFieldBuilder()->
            add("value",pvString) ->
            createStructure();
    }
    PVStructurePtr pvResult = getPVDataCreate()->createPVStructure(resultStructure);
    pvResult->getSubField<PVString>("value")->put(result);
    return pvResult;
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

static StructureConstPtr makePointTopStructure()
{
    static StructureConstPtr pointStructure;
    if (pointStructure.get() == 0)
    {
        FieldCreatePtr fieldCreate = getFieldCreate();

        pointStructure = fieldCreate->createFieldBuilder()->
            setId("Point")->
            add("value", makePointStructure())->
            add("timeStamp", getStandardField()->timeStamp())->
            createStructure();
    }
    return pointStructure;
}

static StructureConstPtr makeRecordStructure()
{
    static StructureConstPtr recordStructure;
    if (!recordStructure)
    {
        FieldCreatePtr fieldCreate = getFieldCreate();

        recordStructure = fieldCreate->createFieldBuilder()->
            add("positionSP", makePointTopStructure())->
            add("positionRB", makePointTopStructure())->
            add("timeStamp", getStandardField()->timeStamp())->
            createStructure();
    }
    return recordStructure;
}

void ConfigureService::request(
    PVStructure::shared_pointer const & args,
    epics::pvAccess::RPCResponseCallback::shared_pointer const & callback
)
{
    PVStructureArrayPtr valueField = args->getSubField<PVStructureArray>("value");
    if (valueField.get() == 0)
        throw pvAccess::RPCRequestException(Status::STATUSTYPE_ERROR,
            "No structure array value field");

    StructureConstPtr valueFieldStructure = valueField->
        getStructureArray()->getStructure();

    ScalarConstPtr xField = valueFieldStructure->getField<Scalar>("x");
    if (xField.get() == 0 || xField->getScalarType() != pvDouble)
        throw pvAccess::RPCRequestException(Status::STATUSTYPE_ERROR,
            "value field's structure has no double field x");

    ScalarConstPtr yField = valueFieldStructure->getField<Scalar>("y");
    if (xField.get() == 0 || xField->getScalarType() != pvDouble)
        throw pvAccess::RPCRequestException(Status::STATUSTYPE_ERROR,
            "value field's structure has no double field y");

    PVStructureArray::const_svector vals = valueField->view();
    
    std::vector<Point> newPoints;
    newPoints.reserve(vals.size());
    for (PVStructureArray::const_svector::const_iterator it = vals.begin();
         it != vals.end(); ++it)
    {
        double x = (*it)->getSubFieldT<PVDouble>("x")->get();
        double y = (*it)->getSubFieldT<PVDouble>("y")->get();
        newPoints.push_back(Point(x,y));
    }

    try {
        pvRecord->getScanService()->configure(newPoints);
    }
    catch (std::exception& e) {
        throw epics::pvAccess::RPCRequestException(
            Status::STATUSTYPE_ERROR,e.what());
    }
    callback->requestDone(Status::Ok,makeResultStructure("configure success"));
}

void StartService::request(
    PVStructure::shared_pointer const & args,
    epics::pvAccess::RPCResponseCallback::shared_pointer const & callback
)
{
    try {
        pvRecord->getScanService()->startScan();
    }
    catch (std::exception& e) {
        throw epics::pvAccess::RPCRequestException(
            Status::STATUSTYPE_ERROR,e.what());
    }
    callback->requestDone(Status::Ok,makeResultStructure("start success"));
}

void StopService::request(
    PVStructure::shared_pointer const & args,
    epics::pvAccess::RPCResponseCallback::shared_pointer const & callback
)
{
    try {
        pvRecord->getScanService()->stopScan();
    }
    catch (std::exception& e) {
        throw epics::pvAccess::RPCRequestException(
            Status::STATUSTYPE_ERROR,e.what());
    }
    callback->requestDone(Status::Ok,makeResultStructure("stop success"));
}


ScanRPCService::Callback::shared_pointer ScanRPCService::Callback::create(ScanRPCServicePtr const & service)
{
    return ScanRPCService::Callback::shared_pointer(new ScanRPCService::Callback(service));
}

void ScanRPCService::request(
    PVStructurePtr const & args,
    epics::pvAccess::RPCResponseCallback::shared_pointer const & callback)
{
    pvRecord->getScanService()->startScan();
    ScanRPCService::Callback::shared_pointer cb = ScanRPCService::Callback::create(shared_from_this());
    this->rpcCallback = callback;
    this->scanServiceCallback = cb;
    pvRecord->getScanService()->registerCallback(cb);
}
 

void ScanRPCService::handleError(const std::string & message)
{
    rpcCallback->requestDone(
        Status(Status::STATUSTYPE_ERROR, std::string(message)),
        PVStructure::shared_pointer());

    pvRecord->getScanService()->unregisterCallback(
         scanServiceCallback);
}


void ScanRPCService::scanComplete()
{
    rpcCallback->requestDone(Status::Ok, makeResultStructure("configure success"));
    pvRecord->getScanService()->unregisterCallback(scanServiceCallback);
}


void ScanRPCService::update(int flags)
{
    if ((flags & ScanService::Callback::SCAN_COMPLETE) != 0)
    {
       scanComplete();
    }
}

void ScanRPCService::Callback::update(int flags)
{
    service->update(flags);
}


ScanServerRPC::Callback::shared_pointer ScanServerRPC::Callback::create(ScanServerRPCPtr const & record)
{
    return ScanServerRPC::Callback::shared_pointer(new ScanServerRPC::Callback(record));
}

void ScanServerRPC::Callback::update(int flags)
{
    record->update(flags);
}

void ScanServerRPC::update(int flags)
{
    lock();
    try {
        TimeStamp timeStamp;
        timeStamp.getCurrent();
        beginGroupPut();

        if ((flags & ScanService::Callback::SETPOINT_CHANGED) != 0)
        {
            Point sp = scanService->getPositionSetpoint();
            pvx->put(sp.x);
            pvy->put(sp.y);
            pvTimeStamp_sp.set(timeStamp);
        }

        if ((flags & ScanService::Callback::READBACK_CHANGED) != 0)
        {
            Point rb = scanService->getPositionReadback();
            pvx_rb->put(rb.x);
            pvy_rb->put(rb.y);
            pvTimeStamp_rb.set(timeStamp);
        }

        pvTimeStamp.set(timeStamp);
        endGroupPut();
    }
    catch(...)
    {
        unlock();
        throw epics::pvAccess::RPCRequestException(
            Status::STATUSTYPE_ERROR,"update error");
    }
    unlock();
}



ScanServerRPCPtr ScanServerRPC::create(
    string const & recordName)
{
    StandardFieldPtr standardField = getStandardField();
    FieldCreatePtr fieldCreate = getFieldCreate();
    PVDataCreatePtr pvDataCreate = getPVDataCreate();

    PVStructurePtr pvStructure = pvDataCreate->createPVStructure(makeRecordStructure());

    ScanServerRPCPtr pvRecord(
        new ScanServerRPC(recordName,pvStructure));
    pvRecord->initPvt();
    return pvRecord;
}

ScanServerRPC::ScanServerRPC(
    string const & recordName,
    PVStructurePtr const & pvStructure)
: PVRecord(recordName,pvStructure), firstTime(true)
{
    pvx    = pvStructure->getSubFieldT<PVDouble>("positionSP.value.x");
    pvy    = pvStructure->getSubFieldT<PVDouble>("positionSP.value.y");
    pvx_rb = pvStructure->getSubFieldT<PVDouble>("positionRB.value.x");
    pvy_rb = pvStructure->getSubFieldT<PVDouble>("positionRB.value.y");


    pvTimeStamp.attach(pvStructure->getSubFieldT<PVStructure>("timeStamp"));
    pvTimeStamp_sp.attach(pvStructure->getSubFieldT<PVStructure>("positionSP.timeStamp"));
    pvTimeStamp_rb.attach(pvStructure->getSubFieldT<PVStructure>("positionRB.timeStamp"));
    scanService = ScanService::create();
}

void ScanServerRPC::initPvt()
{
    initPVRecord();

    PVFieldPtr pvField;
    pvTimeStamp.attach(getPVStructure()->getSubField("timeStamp"));

    scanService->registerCallback(Callback::create(std::tr1::dynamic_pointer_cast<ScanServerRPC>(shared_from_this())));

    process();
}

epics::pvAccess::RPCServiceAsync::shared_pointer ScanServerRPC::getService(
        PVStructurePtr const & pvRequest)
{
    PVStringPtr methodField = pvRequest->getSubField<PVString>("method");

    if (methodField.get() != 0)
    {
        std::string method = methodField->get();
        if (method == "configure")
        {
            return ConfigureService::create(
                 std::tr1::dynamic_pointer_cast<ScanServerRPC>(
                 shared_from_this()));
        }
        else if (method == "start")
        {
            return StartService::create(
                 std::tr1::dynamic_pointer_cast<ScanServerRPC>(
                 shared_from_this()));
        }
        else if (method == "stop")
        {
            return StopService::create(
                 std::tr1::dynamic_pointer_cast<ScanServerRPC>(
                 shared_from_this()));
        }
    }
    return epics::pvAccess::RPCService::shared_pointer();
}

void ScanServerRPC::process()
{
    PVRecord::process();
}


}}
