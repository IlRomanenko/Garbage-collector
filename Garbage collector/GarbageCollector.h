#pragma once
#include "Base.h"

class SmartObject;

struct GarbageCollectorInfo
{
    int adress, size, line;
    const char* file;
};

ostream& operator << (ostream &stream, GarbageCollectorInfo info)
{
    stream << "adress = " << info.adress << " size = " << info.size << endl;
    stream << "line = " << info.line << " file = " << info.file;
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
    ofstream* gc_log;

    static GarbageCollector *self;
    static const bad_alloc alloc_exception;


    int cur_position = 0;
    vector<SmartObject*> pointers;
private:

    GarbageCollector() 
    {
        memory_buffer = (char*)malloc(MEMORY_BUFFER_SIZE);
        gc_log = new ofstream("gc_log.txt");
    }
    
    void CheckMemoryLeaks()
    {
        gc_info *info;

        if (pointers.size() != 0)
        {
            *gc_log << "ATTENTION! Memory leaks was founded" << endl << endl;

            for (auto& data : pointers)
            {
                info = (gc_info*)((char*)data - GCI_SIZE);
                *gc_log << *info << endl << endl;
            }
        }
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

    void* Allocate(size_t n, size_t line, const char* file)
    {
        char *data = memory_buffer + cur_position;

        gc_info *info = (gc_info*)data;
        info->adress = (int)data;
        info->size = n;
        info->line = line;
        info->file = file;

        *gc_log << "allocating : " << *info << endl;

        cur_position += n + GCI_SIZE;
        
        AddPointer(data + GCI_SIZE);

        return data + GCI_SIZE;
    }

    void Deallocate(void *data)
    {
        char* ch_data = (char*)data - GCI_SIZE;
        gc_info *info = (gc_info*)ch_data;
        
        *gc_log << "deallocating " << *info << endl;

        RemovePointer(ch_data + GCI_SIZE);

        memset(ch_data, 0, GCI_SIZE + info->size);
    }
   
    bool AddStackObject(void *data)
    {
        int adress = (int)data, offset = adress - (int)memory_buffer;
        if (offset >= 0 && offset * 4 < MEMORY_BUFFER_SIZE)
            return false;//heap object
        
        *gc_log << "AddStackObject" << endl;
        AddPointer(data);
        return true;
    }

    void RemoveStackObject(const void *data)
    {
        int adress = (int)data, offset = adress - (int)memory_buffer;
        if (offset >= 0 && offset * 4 < MEMORY_BUFFER_SIZE)
            return;//heap object

        *gc_log << "RemoveStackObject" << endl;
        RemovePointer(data);
    }

    vector<SmartObject*> GetPointers() const
    {
        return pointers;
    }

    ~GarbageCollector()
    {
        CheckMemoryLeaks();
        free(memory_buffer);
        gc_log->flush();
        gc_log->close();
        delete gc_log;
    }
};
GarbageCollector* GarbageCollector::self = nullptr;
const bad_alloc GarbageCollector::alloc_exception = bad_alloc();
char* GarbageCollector::memory_buffer = nullptr;