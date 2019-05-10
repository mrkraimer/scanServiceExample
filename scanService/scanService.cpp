/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author Dave Hickin and Marty Kraimer
 *
 */



#include <epicsThread.h>

#include <cmath>
#include <sstream>

#include <epicsExport.h>
#include "pv/scanService.h"

namespace epics { namespace exampleScan {


ScanServicePtr ScanService::create()
{
    return ScanServicePtr(new ScanService());
}

void ScanService::registerCallback(Callback::shared_pointer const & callback)
{
    epics::pvData::Lock lock(mutex);
    if (find(callbacks.begin(),callbacks.end(), callback) != callbacks.end()) return;

    callbacks.push_back(callback);
}

bool ScanService::unregisterCallback(ScanService::Callback::shared_pointer const & callback)
{
    epics::pvData::Lock lock(mutex);
    std::vector<Callback::shared_pointer>::iterator foundCB
        = find(callbacks.begin(),callbacks.end(), callback);
    bool found = foundCB == callbacks.end();
    callbacks.erase(foundCB);
    return found;
}

void ScanService::update()
{
    epics::pvData::Lock lock(mutex);
    std::vector<Callback::shared_pointer> callbacks = this->callbacks;

    if (flags != 0)
    {
        for (std::vector<Callback::shared_pointer>::iterator
                 it = callbacks.begin();
             it != callbacks.end(); ++it)
        {
            (*it)->update(flags);
        }

        flags = 0;
    }
}

ScanService::ScanService()
: scanningActive(false), index(0)
{
   thread = EpicsThreadPtr(new epicsThread(
        *this,
        "scanService",
        epicsThreadGetStackSize(epicsThreadStackSmall),
        epicsThreadPriorityLow));
        startThread();
}

void ScanService::run()
{
    while (true)
    {
        try {
            epicsThreadSleep(0.1);
            epics::pvData::Lock lock(mutex);
            if (scanningActive)
            {
                if (positionRB != positionSP)
                {
                    double dx = positionSP.x - positionRB.x;
                    double dy = positionSP.y - positionRB.y;

                    const double ds = sqrt(dx*dx+dy*dy);
                    const double maxds = 0.01;
                    // avoid very small final steps
                    const double maxds_x = maxds + 1.0e-5;

                    if (ds > maxds_x)
                    {
                        double scale = maxds/ds;
                        dx *= scale;
                        dy *= scale;
                        setReadback(Point(
                            positionRB.x + dx, positionRB.y + dy));
                    }
                    else
                    {
                        setReadback(positionSP);
                    }
                }
            }

            if (scanningActive && positionRB == positionSP)
            {
                if (index < points.size())
                {
                    setSetpoint(points[index]);
                    ++index;
                }
                else
                {
                    flags |= ScanService::Callback::SCAN_COMPLETE;
                    stopScan();
                }
            }
        }
        catch (...) { abort(); }
        update();
    }
}

Point ScanService::getPositionSetpoint()
{
    return positionSP;
}

Point ScanService::getPositionReadback()
{
    return positionRB;
}

void ScanService::setSetpoint(Point sp)
{
    positionSP = sp;
    flags |= ScanService::Callback::SETPOINT_CHANGED;
}

void ScanService::setReadback(Point rb)
{
    positionRB = rb;
    flags |= ScanService::Callback::READBACK_CHANGED;
}

void ScanService::configure(const std::vector<Point> & newPoints)
{
    epics::pvData::Lock lock(mutex);
    if(scanningActive) 
    {
        std::stringstream ss;
        ss << "Cannot configure while scanning active ";
        throw std::runtime_error(ss.str());
    }
    std::cout << "Configure" << std::endl;
    points = newPoints;
}

void ScanService::startScan()
{
    epics::pvData::Lock lock(mutex);
    if(scanningActive) 
    {
        std::stringstream ss;
        ss << "Cannot startScan while scanning active ";
        throw std::runtime_error(ss.str());
    }
    if(points.size()<=0) {
        std::stringstream ss;
        ss << "Cannot startScan because no points.";
        throw std::runtime_error(ss.str());
    }
    setSetpoint(points[0]);
    std::cout << "Run" << std::endl;
    index = 0;
    scanningActive = true;
}

void ScanService::stopScan()
{
    epics::pvData::Lock lock(mutex);
    if(!scanningActive) 
    {
        std::stringstream ss;
        ss << "Cannot stopScan unless scanning active ";
        throw std::runtime_error(ss.str());
    }
    std::cout << "Stop" << std::endl;
    scanningActive = false;
}

}}


