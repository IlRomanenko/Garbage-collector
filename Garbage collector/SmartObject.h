#pragma once
#include "Base.h"
#include "GarbageCollector.h"


class SmartObject
{
private:
    bool is_stack_object;
public:
    SmartObject() 
    {
        is_stack_object = GarbageCollector::Instance()->AddStackObject(this);
    }
 
    static void* operator new(size_t n, size_t line, const char* file)
    {
        return GarbageCollector::Instance()->Allocate(n, line, file);
    }

    static void* operator new[](size_t n, size_t line, const char* file)
    {
        return GarbageCollector::Instance()->Allocate(n, line, file);
    }

    static void operator delete(void *data)
    {
        GarbageCollector::Instance()->Deallocate(data);
    }

    static void operator delete[](void *data)
    {
        GarbageCollector::Instance()->Deallocate(data);
    }
        /*
    static void Destroy(void *data, int line, bool is_array)
    {
        if (is_array)
            operator delete[](data, line, 1);
        else
            operator delete(data, line, );
    }*/

    virtual ~SmartObject() 
    {
        GarbageCollector::Instance()->RemoveStackObject(this);
    }
};