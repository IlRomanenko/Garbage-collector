#pragma once
#include "Base.h"
#include <cassert>

class SmartObject;

struct GarbageCollectorInfo
{
    int adress, validation_empty_field; //a small trick
    int size, line;
    bool array;
    const char* file;
};

class GarbageCollector
{

    using gc_info = GarbageCollectorInfo;

private:

    const int
        MEMORY_BUFFER_SIZE = 200,
        GCI_SIZE = sizeof(gc_info);

    static char* memory_buffer;
    static GarbageCollector *self;
    static const bad_alloc alloc_exception;

    ofstream* gc_log;
    vector<SmartObject*> pointers;
    set<pair<int, int> > size_adress_free_memory, boundaries_occupied_memory;

    
private:

    GarbageCollector();
    
    ~GarbageCollector();
    
    
    //memory allocation functions

    void remove_free_chunk(int begin_pos, int end_pos);

    int get_free_block_position(int size);

    char* get_memory(size_t n);

    void ReleaseChunk(int beg_position, int size);

    //gc_info functions

    void* get_begin_data(const void *offset_data);

    gc_info* get_GC_INFO(const void *offset_data);

    //collecting garbage

    void CollectGarbage();
    
    void Dfs(SmartObject* node);

    //others

    void CheckMemoryLeaks();

    void AddPointer(void* object);

    void RemovePointer(const void* object, bool is_array);

    
    
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

    void* Allocate(size_t n, size_t line, const char* file, bool is_array);

    void Deallocate(void *data, bool is_array);

    bool AddLinkSource(void *data);

    void RemoveLinkSource(const void *data);

    vector<SmartObject*> GetPointers() const
    {
        return pointers;
    }

};


#ifdef FullNew
#define gc_new new(__LINE__, __FILE__)
#else
#define gc_new new(__LINE__, "")
#endif

#define gc_delete delete