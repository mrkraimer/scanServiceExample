/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author Dave Hickin and Marty Kraimer
 * @date 2019.04
 */

#ifndef SCANSERVICE_H
#define SCANSERVICE_H

#include <iostream>
#include <pv/pvDatabase.h>
#include <epicsThread.h>
#include <shareLib.h>

namespace epics { namespace exampleScan {

typedef std::tr1::shared_ptr<epicsThread> EpicsThreadPtr;

class Point
{
public:
    Point()
    : x(0), y(0)
    {}
    Point(double x, double y)
    : x(x), y(y)
    {}
    Point(const Point & point)
    : x(point.x), y(point.y)
    {}
    Point operator=(const Point &rhs)
    {
        x = rhs.x;
        y = rhs.y;
        return *this;
    }
    double x;
    double y;
};

inline bool operator==(const Point & lhs, const Point &rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator!=(const Point & lhs, const Point &rhs)
{
    return !(lhs == rhs);
}

inline std::ostream & operator<< (std::ostream& os, const Point& point)
{
   os << "(" << point.x << "," << point.y << ")";
   return os;
}

class ScanService;
typedef std::tr1::shared_ptr<ScanService> ScanServicePtr;

class epicsShareClass ScanService : public epicsThreadRunable,
    public std::tr1::enable_shared_from_this<ScanService>
{
public:
    class Callback : public std::tr1::enable_shared_from_this<Callback>
    {
    public:
        POINTER_DEFINITIONS(Callback);
        virtual void update(int flags) = 0;
        const static int SETPOINT_CHANGED  = 0x1;
        const static int READBACK_CHANGED  = 0x2;
        const static int SCAN_COMPLETE     = 0x4;
    };
public:
    static ScanServicePtr create();
    POINTER_DEFINITIONS(ScanService);
    bool isScanning() {return scanningActive;}
    virtual bool init() { return false;}
    virtual void run();
    void startThread() { thread->start(); }
    void stop() {}
    void registerCallback(Callback::shared_pointer const & callback);
    bool unregisterCallback(Callback::shared_pointer const & callback);
    void update();
    Point getPositionSetpoint();
    Point getPositionReadback();
    void setSetpoint(Point sp);
    void configure(const std::vector<Point> & newPoints);
    void startScan();
    void stopScan();
private:
    ScanService();
    void setSetpointImpl(Point sp);
    void setReadbackImpl(Point rb);
    bool scanningActive;
    int flags;
    Point positionSP;
    Point positionRB;
    std::vector<Callback::shared_pointer> callbacks;
    size_t index;
    std::vector<Point> points;
    epics::pvData::Mutex mutex;
    EpicsThreadPtr thread;
};


}}


#endif //SCANSERVICE_H

