#pragma once
#include "GarbageCollector.h"


class ISmartObject
{
public:
    bool has_checked;
    AllocatedMemoryChunk* chunk;
public:
    ISmartObject()
    {
        has_checked = false;
        chunk = GarbageCollector::Instance()->AddLinkSource(this);
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
        if (chunk != nullptr)
        {
            cerr << "~ISmartObject() with address " << static_cast<const void* const>(this) << endl;
            chunk->RemoveObject(this);
        }
        else
            GarbageCollector::Instance()->RemoveLinkSource(this);
    }
};
