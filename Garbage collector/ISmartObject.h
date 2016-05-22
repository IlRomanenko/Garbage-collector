#pragma once
#include "GarbageCollector.h"


class ISmartObject
{
public:
    bool has_checked;
public:
    ISmartObject()
    {
        has_checked = false;
        GarbageCollector::Instance()->AddLinkSource(this);
    }

    virtual vector<ISmartObject*> pointers() const = 0;
    
    static void* operator new(size_t n, const GC_NEW_STRUCT&)
    {
        return GarbageCollector::Instance()->Allocate(n);
    }

    static void operator delete(void *data)
    {
        GarbageCollector::Instance()->Deallocate(data);
    }

    static void operator delete(void *data, const GC_NEW_STRUCT&)
    {
        GarbageCollector::Instance()->Deallocate(data);
    }


    static void* operator new[](size_t n, const GC_NEW_STRUCT&)
    {
        return GarbageCollector::Instance()->Allocate(n);
    }

    static void operator delete[](void *data)
    {
        GarbageCollector::Instance()->Deallocate(data);
    }
    
    static void operator delete[](void *data, const GC_NEW_STRUCT&)
    {
        GarbageCollector::Instance()->Deallocate(data);
    }

    static void* operator new(size_t n, const GC_NEW_STRUCT&, const nothrow_t&) noexcept
    {
        return GarbageCollector::Instance()->NoexceptAllocate(n);
    }
    
    static void operator delete(void *data , const GC_NEW_STRUCT&, const nothrow_t&) noexcept
    {
        if (data == nullptr)
            return;
        GarbageCollector::Instance()->Deallocate(data);
    }


    static void* operator new[](size_t n, const GC_NEW_STRUCT&, const nothrow_t&) noexcept
    {
        return GarbageCollector::Instance()->NoexceptAllocate(n);
    }

    static void operator delete[](void *data, const GC_NEW_STRUCT&, const nothrow_t&) noexcept
    {
        if (data == nullptr)
            return;
        GarbageCollector::Instance()->Deallocate(data);
    }

    virtual ~ISmartObject()
    {
        GarbageCollector::Instance()->RemoveLinkSource(this);
    }
};
