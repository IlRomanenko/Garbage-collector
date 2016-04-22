#pragma once
#include "Base.h"

class SmartObject;

struct GarbageCollectorInfo
{
    int adress, size;
};

ostream& operator << (ostream &stream, GarbageCollectorInfo info)
{
    stream << "adress = " << info.adress << " size = " << info.size;
    return stream;
}

class GarbageCollector
{
private:

    using gc_info = GarbageCollectorInfo;

    const int 
        MEMORY_BUFFER_SIZE = 100500, 
        GCI_SIZE = sizeof(gc_info);

    static char* memory_buffer;

    static GarbageCollector *self;
    static const bad_alloc alloc_exception;


    int cur_position = 0;
    vector<SmartObject*> pointers;
private:

    GarbageCollector() 
    {
        memory_buffer = (char*)malloc(MEMORY_BUFFER_SIZE);
    }
    
    ~GarbageCollector()
    {
        free(memory_buffer);
    }
    
    void AddPointer(void* object)
    {
        pointers.push_back((SmartObject*)object);
    }

    void RemovePointer(const void* object)
    {
        bool is_ok = true;

        auto find_iterator = find(pointers.begin(), pointers.end(), (SmartObject*)object);
        if (find_iterator == pointers.end())
            throw exception("Delalocating of unregistered object");
        else
            pointers.erase(find_iterator);
    }

public:
    static GarbageCollector* Instance()
    {
        if (self == nullptr)
        {
            self = new GarbageCollector();
        }
        return self;
    }

    void* Allocate(size_t n)
    {
        char *data = memory_buffer + cur_position;

        gc_info *info = (gc_info*)data;
        info->adress = (int)data;
        info->size = n;
        
        LOG << "allocating : " << *info << endl;

        cur_position += n + GCI_SIZE;
        
        AddPointer(data + GCI_SIZE);

        return data + GCI_SIZE;
    }

    void Deallocate(void *data)
    {
        char* ch_data = (char*)data - GCI_SIZE;
        gc_info *info = (gc_info*)ch_data;
        
        LOG << "deallocating : " << *info << endl;

        RemovePointer(ch_data + GCI_SIZE);

        memset(ch_data, 0, GCI_SIZE + info->size);
    }
   
    void AddStackObject(void *data)
    {
        int adress = (int)data, offset = adress - (int)memory_buffer;
        if (offset >= 0 && offset * 4 < MEMORY_BUFFER_SIZE)
            return;//heap object
        
        LOG << "AddStackObject" << endl;
        AddPointer(data);
    }

    void RemoveStackObject(const void *data)
    {
        int adress = (int)data, offset = adress - (int)memory_buffer;
        if (offset >= 0 && offset * 4 < MEMORY_BUFFER_SIZE)
            return;//heap object

        LOG << "RemoveStackObject" << endl;
        RemovePointer(data);
    }

    vector<SmartObject*> GetPointers() const
    {
        return pointers;
    }
};

GarbageCollector* GarbageCollector::self = nullptr;
const bad_alloc GarbageCollector::alloc_exception = bad_alloc();
char* GarbageCollector::memory_buffer = nullptr;