#pragma once
#include "AllocatedMemoryChunk.h"

struct GC_NEW_STRUCT { };
extern  GC_NEW_STRUCT gc_new_struct;

class GarbageCollector
{
    
private:
    static GarbageCollector *self;
    static const bad_alloc alloc_exception;
    static string file_name;

    ofstream* gc_log;
    vector<AllocatedMemoryChunk*> allocatedMemory;
    vector<ISmartObject*> stackObjects;

private:

    GarbageCollector();
    
    ~GarbageCollector();
    
    void InitializeMemory(void* data, size_t n);

    void CollectGarbage();
    
    void Dfs(ISmartObject* node);

    void CheckMemoryLeaks();

public:
    static GarbageCollector* Instance()
    {
        if (self == nullptr)
        {
            self = new GarbageCollector();
            atexit(DestroyInstance);
        }
        return self;
    }

    static void DestroyInstance()
    {
        if (self != nullptr)
        {
            delete self;
            self = nullptr;
        }
    }

    static void SetGCLogFile(string filename)
    {
        file_name = filename;
    }

    static void ForciblyCollectGarbage()
    {
        if (self == nullptr)
            return;
        self->CollectGarbage();
    }

    void* Allocate(size_t n);
    
    void* NoexceptAllocate(size_t n) noexcept;

    void Deallocate(void *data);

    AllocatedMemoryChunk* AddLinkSource(ISmartObject *data);

    void RemoveLinkSource(const ISmartObject *data);
};


#define gc_new new(gc_new_struct)
#define gc_new_noexcept new(gc_new_struct, nothrow)

#define gc_delete delete
