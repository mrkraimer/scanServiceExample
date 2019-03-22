/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author dgh Dave Hickin and Marty Kraimer
 * @date 2019.04.18
 */

#include <pv/standardField.h>
#include <pv/pvEnumerated.h>

#include <pv/standardField.h>
#include <pv/standardPVField.h>

#include <epicsThread.h>
#include <pv/scanService.h>
#include <epicsExport.h>
#include "pv/scanServerPutGet.h"

using namespace epics::pvData;
using namespace epics::pvDatabase;
using std::tr1::static_pointer_cast;
using std::string;
using std::cout;

namespace epics { namespace exampleScan {


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
            addNestedStructure("argument")->
               add("command",pvString)->
               addNestedStructure("configArg")->
                  addArray("x",pvDouble) ->
                  addArray("y",pvDouble) ->
                  endNested()->
               endNested()->
            addNestedStructure("result")->
               add("value",pvString) ->
               endNested()->
            add("timeStamp", getStandardField()->timeStamp())->
            createStructure();
    }
    return recordStructure;
}

ScanServerPutGet::Callback::shared_pointer ScanServerPutGet::Callback::create(ScanServerPutGetPtr const & record)
{
    return ScanServerPutGet::Callback::shared_pointer(new ScanServerPutGet::Callback(record));
}

void ScanServerPutGet::Callback::update(int flags)
{
    record->update(flags);
}

void ScanServerPutGet::update(int flags)
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
        cout << "update error\n";
    }
    unlock();
}



ScanServerPutGetPtr ScanServerPutGet::create(
    string const & recordName)
{
    StandardFieldPtr standardField = getStandardField();
    FieldCreatePtr fieldCreate = getFieldCreate();
    PVDataCreatePtr pvDataCreate = getPVDataCreate();

    PVStructurePtr pvStructure = pvDataCreate->createPVStructure(makeRecordStructure());
    ScanServerPutGetPtr pvRecord(
        new ScanServerPutGet(recordName,pvStructure));
    pvRecord->initPvt();
    return pvRecord;
}

ScanServerPutGet::ScanServerPutGet(
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

void ScanServerPutGet::initPvt()
{
    initPVRecord();

    PVFieldPtr pvField;
    pvTimeStamp.attach(getPVStructure()->getSubField("timeStamp"));

    scanService->registerCallback(
        Callback::create(std::tr1::dynamic_pointer_cast<ScanServerPutGet>(shared_from_this())));
}


void ScanServerPutGet::process()
{
    PVStructurePtr pvStructure(getPVStructure());
    PVStringPtr pvResult(pvStructure->getSubField<PVString>("result.value"));
    string command(pvStructure->getSubField<PVString>("argument.command")->get());
    if(command=="configure") {
        PVDoubleArrayPtr pvx(pvStructure->getSubField<PVDoubleArray>("argument.configArg.x"));
        PVDoubleArrayPtr pvy(pvStructure->getSubField<PVDoubleArray>("argument.configArg.y"));
        size_t npoints = pvx->getLength();
        if(npoints!=pvy->getLength()) {
           throw std::logic_error(
               "argument.configArg.x and argument.configArg.y not same length");
        }
        std::vector<Point> newPoints(npoints);
        PVDoubleArray::const_svector xvalue(pvx->view());
        PVDoubleArray::const_svector yvalue(pvy->view());
        for(size_t i=0; i<npoints; ++i) {
             double x = xvalue[i];
             double y = xvalue[i];
             newPoints.push_back(Point(x,y));
        }
        try {
            getScanService()->configure(newPoints);
            pvResult->put("configure success");
        } catch (std::exception& e) {
            string result("exception ");
            result += e.what();
            pvResult->put(result);
       }
    } else if(command=="start") {
       try {
            getScanService()->startScan();
            pvResult->put("startScan success");
       } catch (std::exception& e) {
           string result("exception ");
           result += e.what();
           pvResult->put(result);
       }
    } else if(command=="stop") {
       try {
            getScanService()->stopScan();
            pvResult->put("stopScan success");
       } catch (std::exception& e) {
           string result("exception ");
           result += e.what();
           pvResult->put(result);
       }
    } else {
       pvResult->put("illegal command index");
    }
    PVRecord::process();
}


}}
