#pragma once
#include "Base.h"

class SmartObject;

struct GarbageCollectorInfo
{
    int adress, size, line;
    bool array;
    const char* file;
};

class GarbageCollector
{

    using gc_info = GarbageCollectorInfo;

private:

    const int
        MEMORY_BUFFER_SIZE = 230,
        GCI_SIZE = sizeof(gc_info);

    static char* memory_buffer;
    static GarbageCollector *self;
    static const bad_alloc alloc_exception;

    ofstream* gc_log;
    set<pair<int, int> > size_adress_free_memory, from_to_free_memory;
    int cur_position = 20;
    vector<SmartObject*> pointers;

    
private:

    GarbageCollector();
    
    ~GarbageCollector();
    

    void* get_begin_data(const void *offset_data);

    gc_info* get_GC_INFO(const void *offset_data);

    void CheckMemoryLeaks();

    void AddPointer(void* object);


    void try_merge_iterator(const set<pair<int, int> > ::iterator &it, const pair<int, int> &find_pair)
    {
        if (it == from_to_free_memory.end())
        {
            auto temp_it = it;
            temp_it--;
            if (temp_it->second + 1 == find_pair.first)
            {
                auto pr = *temp_it;
                from_to_free_memory.erase(temp_it);
                from_to_free_memory.insert(make_pair(find_pair.first, find_pair.second));
            }
            else
                from_to_free_memory.insert(find_pair);
        }
        else if (it == from_to_free_memory.begin())
        {
            if (it->first == find_pair.second + 1)
            {
                auto pr = *it;
                from_to_free_memory.erase(it);
                from_to_free_memory.insert(make_pair(find_pair.first, pr.second));
            }
        }
        else
        {
            auto prev_it = it, next_it = it;
            prev_it--;
            pair<int, int> res = find_pair;
            if (prev_it->second + 1 == res.first)
            {
                res.first = prev_it->first;
                from_to_free_memory.erase(prev_it);
            }
        }
    }

    pair<int, int> get_free_block(int size)
    {
        auto it = size_adress_free_memory.lower_bound(make_pair(size, INT_MIN));
        if (it == size_adress_free_memory.end())
        {
            CollectGarbage();
            it = size_adress_free_memory.lower_bound(make_pair(size, INT_MIN));
            if (it == size_adress_free_memory.end())
                throw alloc_exception;
        }
        return *it;
    }

    void RemovePointer(const void* object);

    void CollectGarbage() { }

    static void DestroyInstance()
    {
        if (self != nullptr)
        {
            delete self;
            self = nullptr;
        }
    }

    char* get_memory(size_t n);

    
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

    void* Allocate(size_t n, size_t line, const char* file, bool is_array);

    void Deallocate(void *data);

    bool AddStackObject(void *data);


    void RemoveStackObject(const void *data);

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