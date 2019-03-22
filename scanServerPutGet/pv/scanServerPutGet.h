/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */
/**
 * @author dgh Dave Hickin and Marty Kraimer
 * @date 2019.04.18
 */

#ifndef SCANSERVERPUTGET_H
#define SCANSERVERPUTGET_H

#include <pv/pvDatabase.h>
#include <pv/timeStamp.h>
#include <pv/pvTimeStamp.h>
#include <pv/scanService.h>

#include <shareLib.h>


namespace epics { namespace exampleScan { 

class ScanServerPutGet;
typedef std::tr1::shared_ptr<ScanServerPutGet> ScanServerPutGetPtr;

class epicsShareClass ScanServerPutGet :
    public epics::pvDatabase::PVRecord
{
public:
    POINTER_DEFINITIONS(ScanServerPutGet);
    static ScanServerPutGetPtr create(
        std::string const & recordName);
    virtual ~ScanServerPutGet() {}
    virtual bool init() {return false;}
    virtual void process();

    class Callback : public ScanService::Callback
    {
    public:
        POINTER_DEFINITIONS(Callback);
        static Callback::shared_pointer create(ScanServerPutGetPtr const & record);

        virtual void update(int flags);

    private:
        Callback(ScanServerPutGetPtr record)
        : record(record)
        {}
        ScanServerPutGetPtr record;
    };

    virtual void update(int flags);

    ScanServicePtr getScanService() { return scanService; }

private:

    ScanServerPutGet(std::string const & recordName,
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

#endif  /* SCANSERVERPUTGET_H */
