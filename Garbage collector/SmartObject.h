#pragma once
#include "Base.h"
#include "GarbageCollector.h"


class SmartObject
{
public:
    bool is_stack_object, has_checked;
public:
    SmartObject()
    {
        has_checked = false;
        is_stack_object = GarbageCollector::Instance()->AddLinkSource(this);
    }

    virtual vector<SmartObject*> pointers() const = 0;

    static void* operator new(size_t n, size_t line, const char* file) 
    {
        return GarbageCollector::Instance()->Allocate(n, line, file, false);
    }

    static void operator delete(void *data)
    {
        GarbageCollector::Instance()->Deallocate(data, false);
    }
    
    static void* operator new[](size_t n, size_t line, const char* file)
    {
        return GarbageCollector::Instance()->Allocate(n, line, file, true);
    }

    static void operator delete[](void *data)
    {
        GarbageCollector::Instance()->Deallocate(data, true);
    }

    static void operator delete (void *data, size_t line, const char* file)
    {
        GarbageCollector::Instance()->Deallocate(data, false);
    }

    static void operator delete[] (void *data, size_t line, const char* file)
    {
        GarbageCollector::Instance()->Deallocate(data, true);
    }

    virtual ~SmartObject()
    {
        GarbageCollector::Instance()->RemoveLinkSource(this);
    }
};
