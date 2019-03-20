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
            add("state", getStandardField()->enumerated("timeStamp"))->
            addNestedStructure("argument")->
               addNestedStructure("command")->
                  add("index",pvInt) ->
                  addArray("choices",pvString)->
                  endNested()->
               addNestedStructureArray("configArg")->
                  add("x",pvDouble) ->
                  add("y",pvDouble) ->
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

        if ((flags & ScanService::Callback::STATE_CHANGED) != 0)
        {
            int index = static_cast<int>(scanService->getState());
            if (index != pvStateIndex->get())
            {
                pvStateIndex->put(index);
                pvTimeStamp_st.set(timeStamp);
            }
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
    PVEnumerated pvEnumerated;
    bool result = pvEnumerated.attach(pvStructure->getSubField("argument.command"));
    if(!result) throw std::runtime_error("attach failure");
    StringArray choices(3);
    choices[0] = "configure";
    choices[1] = "start";
    choices[2] = "stop";
    result = pvEnumerated.setChoices(choices);
    if(!result) throw std::runtime_error("setChoices failure");
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

    pvStateIndex = pvStructure->getSubFieldT<PVInt>("state.value.index");
    PVStringArrayPtr pvStateChoices
        = pvStructure->getSubFieldT<PVStringArray>("state.value.choices");
    pvTimeStamp.attach(pvStructure->getSubFieldT<PVStructure>("timeStamp"));
    pvTimeStamp_sp.attach(pvStructure->getSubFieldT<PVStructure>("positionSP.timeStamp"));
    pvTimeStamp_rb.attach(pvStructure->getSubFieldT<PVStructure>("positionRB.timeStamp"));
        pvTimeStamp_st.attach(pvStructure->getSubFieldT<PVStructure>("state.timeStamp"));

    PVStringArray::svector choices;
    choices.reserve(4);
    choices.push_back(ScanService::toString(ScanService::IDLE));
    choices.push_back(ScanService::toString(ScanService::READY));
    choices.push_back(ScanService::toString(ScanService::RUNNING));
    choices.push_back(ScanService::toString(ScanService::PAUSED));
    pvStateChoices->replace(freeze(choices));

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
    PVStringPtr pvResult = pvStructure->getSubField<PVString>("result.value");
    PVIntPtr pvInt = pvStructure->getSubField<PVInt>("argument.command.index");
    switch(pvInt->get()) 
    {
        case 0 :  // configure
        {
             PVStructureArrayPtr configArg
                  = pvStructure->getSubField<PVStructureArray>("argument.configArg");
             PVStructureArray::const_svector vals = configArg->view();
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
                 getScanService()->configure(newPoints);
                 pvResult->put("configure success");
             } catch (std::exception& e) {
                 string result("exception ");
                 result += e.what();
                 pvResult->put(result);
            }
        }
        break;
        case 1 :  // start
        {
            try {
                 getScanService()->runScan();
                 pvResult->put("run success");
            } catch (std::exception& e) {
                string result("exception ");
                result += e.what();
                pvResult->put(result);
            }
        }
        break;
        case 2 :  // stop
        {
            try {
                 getScanService()->stopScan();
                 pvResult->put("stopScan success");
            } catch (std::exception& e) {
                string result("exception ");
                result += e.what();
                pvResult->put(result);
            }
        }
        break;
        default:
            pvResult->put("illegal command index");
        break;
    }
    PVRecord::process();
}


}}
