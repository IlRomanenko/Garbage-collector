#pragma once
#include "Base.h"
#include "GarbageCollector.h"


class SmartObject
{
private:
    
public:
    SmartObject() 
    {
        GarbageCollector::Instance()->AddStackObject(this);
    }
 
    void* operator new(size_t n)
    {
        return GarbageCollector::Instance()->Allocate(n);
    }

    void* operator new[](size_t n)
    {
        return GarbageCollector::Instance()->Allocate(n);
    }

    void operator delete(void *data)
    {
        GarbageCollector::Instance()->Deallocate(data);
    }

    void operator delete[](void *data)
    {
        GarbageCollector::Instance()->Deallocate(data);
    }
    
    SmartObject& operator =(const SmartObject &other)
    {

    }

    virtual ~SmartObject() 
    {
        GarbageCollector::Instance()->RemoveStackObject(this);
    }
};

