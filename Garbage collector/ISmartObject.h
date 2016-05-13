#pragma once
#include "Base.h"
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
    
    virtual vector<ISmartObject*> rev_pointers_links() const = 0;


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


    static void operator delete (void *data, size_t , const char* )
    {
        GarbageCollector::Instance()->Deallocate(data, false);
    }

    static void operator delete[] (void *data, size_t , const char* )
    {
        GarbageCollector::Instance()->Deallocate(data, true);
    }


    static void* operator new(size_t n, size_t line, const char* file, const nothrow_t&) noexcept
    {
        return GarbageCollector::Instance()->NoexceptAllocate(n, line, file, false);
    }
    
    static void operator delete(void *data ,size_t, const char* , const nothrow_t&) noexcept
    {
        if (data == nullptr)
            return;
        GarbageCollector::Instance()->Deallocate(data, false);
    }


    static void* operator new[](size_t n, size_t line, const char* file, const nothrow_t&) noexcept
    {
        return GarbageCollector::Instance()->NoexceptAllocate(n, line, file, true);
    }

    static void operator delete[](void *data, size_t, const char*, const nothrow_t&) noexcept
    {
        if (data == nullptr)
            return;
        GarbageCollector::Instance()->Deallocate(data, true);
    }

    virtual ~ISmartObject()
    {
        GarbageCollector::Instance()->RemoveLinkSource(this);
    }
};
