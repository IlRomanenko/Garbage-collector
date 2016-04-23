#pragma once
#include "Base.h"
#include "GarbageCollector.h"


class SmartObject
{
public:
    bool is_stack_object;
public:
    SmartObject()
    {
        is_stack_object = GarbageCollector::Instance()->AddStackObject(this);
    }

    static void* operator new(size_t n, size_t line, const char* file)
    {
        return GarbageCollector::Instance()->Allocate(n, line, file, false);
    }

    static void operator delete(void *data)
    {
        GarbageCollector::Instance()->Deallocate(data);
    }
    
    static void* operator new[](size_t n, size_t line, const char* file)
    {
        return GarbageCollector::Instance()->Allocate(n, line, file, true);
    }

    static void operator delete[](void *data)
    {
        GarbageCollector::Instance()->Deallocate(data);
    }

    static void operator delete (void *data, size_t line, const char* file)
    {
        GarbageCollector::Instance()->Deallocate(data);
    }

    static void operator delete[] (void *data, size_t line, const char* file)
    {
        GarbageCollector::Instance()->Deallocate(data);
    }


    virtual ~SmartObject()
    {
        GarbageCollector::Instance()->RemoveStackObject(this);
    }
};