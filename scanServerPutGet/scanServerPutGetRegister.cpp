/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author Dave Hickin and Marty Kraimer
 * @date 2019.04.18
 */

#include <cstddef>
#include <cstdlib>
#include <cstddef>
#include <string>
#include <cstdio>
#include <memory>
#include <iostream>

#include <cantProceed.h>
#include <epicsStdio.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <epicsThread.h>
#include <iocsh.h>

#include <pv/pvIntrospect.h>
#include <pv/pvData.h>
#include <pv/pvAccess.h>
#include <pv/pvDatabase.h>
#include <pv/scanService.h>

#include <epicsExport.h>
#include "pv/scanServerPutGet.h"

using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvDatabase;
using namespace epics::exampleScan;
using std::cout;
using std::endl;

static const iocshArg testArg0 = { "recordName", iocshArgString };
static const iocshArg *testArgs[] = {
    &testArg0};

static const iocshFuncDef scanServerPutGetFuncDef = {
    "scanServerPutGetCreateRecord", 1, testArgs};
static void scanServerPutGetCallFunc(const iocshArgBuf *args)
{
    PVDatabasePtr master = PVDatabase::getMaster();
    char *recordName = args[0].sval;
    ScanServerPutGetPtr record = ScanServerPutGet::create(recordName);
    bool result = master->addRecord(record);
    if(!result) cout << "recordname" << " not added" << endl;
}

static void scanServerPutGetRegister(void)
{
    static int firstTime = 1;
    if (firstTime) {
        firstTime = 0;
        iocshRegister(&scanServerPutGetFuncDef, scanServerPutGetCallFunc);
    }
}

extern "C" {
    epicsExportRegistrar(scanServerPutGetRegister);
}
