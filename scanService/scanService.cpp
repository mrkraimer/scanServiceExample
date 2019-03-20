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

std::string ScanService::toString(ScanService::State state)
{
    switch(state)
    {
    case IDLE:
        return "IDLE";
    case READY:
        return "READY";
    case RUNNING:
        return "RUNNING";
    case PAUSED:
        return "PAUSED";
     default:
        throw std::runtime_error("Unknown state");
    }
}

ScanService::State ScanService::getState()
{
    epics::pvData::Lock lock(mutex);
    return state;
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

void ScanService::scanComplete()
{
    flags |= ScanService::Callback::SCAN_COMPLETE;
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
: state(IDLE), index(0)
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
            if (state == IDLE || state == RUNNING)
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
                        setReadbackImpl(Point(
                            positionRB.x + dx, positionRB.y + dy));
                    }
                    else
                    {
                        setReadbackImpl(positionSP);
                    }
                }
            }

            if (state == RUNNING && positionRB == positionSP)
            {
                if (index < points.size())
                {
                    setSetpointImpl(points[index]);
                    ++index;
                }
                else
                {
                    scanComplete();
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
    if (state != IDLE)
    {
        std::stringstream ss;
        ss << "Cannot set position setpoint unless scanService is IDLE. State is " << toString(state);
        throw std::runtime_error(ss.str());
    }
    setSetpointImpl(sp);
}

void ScanService::setSetpointImpl(Point sp)
{
    positionSP = sp;
    flags |= ScanService::Callback::SETPOINT_CHANGED;
}

void ScanService::setReadbackImpl(Point rb)
{
    positionRB = rb;
    flags |= ScanService::Callback::READBACK_CHANGED;
}

void ScanService::setStateImpl(State state)
{
    this->state = state;
    flags |= ScanService::Callback::STATE_CHANGED;  
}


void ScanService::abort()
{
    epics::pvData::Lock lock(mutex);
    std::cout << "Abort" << std::endl;
    setStateImpl(IDLE);
    points.clear();
    if (positionSP != positionRB)
        setSetpointImpl(positionRB);
}

void ScanService::configure(const std::vector<Point> & newPoints)
{
    epics::pvData::Lock lock(mutex);
    if (state != IDLE)
    {
        std::stringstream ss;
        ss << "Cannot configure scanService unless it is IDLE. State is " << toString(state);
        throw std::runtime_error(ss.str());

    }
    std::cout << "Configure" << std::endl;
    setStateImpl(READY);
    points = newPoints;

    if (positionSP != positionRB)
        setSetpointImpl(positionRB);
}

void ScanService::runScan()
{
    epics::pvData::Lock lock(mutex);
    if (state != READY)
    {
        std::stringstream ss;
        ss << "Cannot run scanService unless it is READY. State is " << toString(state);
        throw std::runtime_error(ss.str());
    }
    std::cout << "Run" << std::endl;
    index = 0;
    setStateImpl(RUNNING);
}

void ScanService::pause()
{
    epics::pvData::Lock lock(mutex);
    if (state != RUNNING) 
    {
        std::stringstream ss;
        ss << "Cannot pause scanService unless it is RUNNING. State is " << toString(state);
        throw std::runtime_error(ss.str());
    }
    std::cout << "Pause" << std::endl;
    setStateImpl(PAUSED);
 }

void ScanService::resume()
{
    epics::pvData::Lock lock(mutex);
    if (state != PAUSED) 
    {
        std::stringstream ss;
        ss << "Cannot resume scanService unless it is PAUSED. State is " << toString(state);
        throw std::runtime_error(ss.str());
    }
    std::cout << "Resume" << std::endl;
    setStateImpl(RUNNING);

}

void ScanService::stopScan()
{
    epics::pvData::Lock lock(mutex);
    switch (state)
    {
    case RUNNING:
    case PAUSED:
    case READY:
        std::cout << "Stop" << std::endl;
        setStateImpl(READY);
        if (positionSP != positionRB)
            setSetpointImpl(positionRB);
        break;
    default:
        {
            std::stringstream ss;
            ss << "Cannot stop scanService unless it is RUNNING, PAUSED or READY. State is " << toString(state);
            throw std::runtime_error(ss.str());
        }
    }
}

void ScanService::rewind(int n)
{
    epics::pvData::Lock lock(mutex);
    switch (state)
    {
    case RUNNING:
    case PAUSED:
        if (n < 0)
        {
            std::stringstream ss;
            ss << "Rewind argument cannot be negative. Argument is " << n;
            throw std::runtime_error(ss.str());
        }
        if (n > 0)
        {
            size_t un = static_cast<size_t>(n);
            std::cout << "Rewind(" << n << ")" << std::endl;
            if (un < index)
                index -= un+1;
            else
                index = 0;
            setSetpointImpl(points[index]);
            ++index;
        }
            break;
    default:
            {
            std::stringstream ss;
            ss << "Cannot rewind scanService unless it is RUNNING or PAUSED. State is " << toString(state);
            throw std::runtime_error(ss.str());
        }
    }
}

}}


