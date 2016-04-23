#include "SmartObject.h"

using gc_info = GarbageCollectorInfo;


GarbageCollector* GarbageCollector::self = nullptr;
const bad_alloc GarbageCollector::alloc_exception = bad_alloc();
char* GarbageCollector::memory_buffer = nullptr;

ostream& operator << (ostream &stream, GarbageCollectorInfo info)
{
    stream << "adress = " << info.adress << " size = " << info.size << endl;
    stream << "line = " << info.line << " file = " << info.file;
    return stream;
}

GarbageCollector::GarbageCollector()
{
    memory_buffer = new char[MEMORY_BUFFER_SIZE];
    memset(memory_buffer, 0, MEMORY_BUFFER_SIZE);
    gc_log = new ofstream("gc_log.txt");
    size_adress_free_memory.insert(make_pair(0, MEMORY_BUFFER_SIZE));
    from_to_free_memory.insert(make_pair(0, MEMORY_BUFFER_SIZE));
}

GarbageCollector::~GarbageCollector()
{
    CheckMemoryLeaks();
    delete memory_buffer;
    gc_log->flush();
    gc_log->close();
    delete gc_log;
}

void* GarbageCollector::get_begin_data(const void *offset_data)
{
    return (char*)offset_data - GCI_SIZE;
}

gc_info* GarbageCollector::get_GC_INFO(const void *offset_data)
{
    return (gc_info*)get_begin_data(offset_data);
}


void GarbageCollector::CheckMemoryLeaks()
{
    gc_info *info;
    SmartObject* data;
    size_t offset;

    char* buf;

    if (pointers.size() != 0)
    {
        *gc_log << endl << endl << "ATTENTION! Memory leaks was founded" << endl << endl;

        for (size_t i = 0; i < pointers.size(); i++)
        {
            data = pointers[i];
            info = get_GC_INFO(data);
            offset = (info->array ? 1 : 0) * sizeof(size_t);

            
            buf = (char*)get_begin_data(data);
            data = (SmartObject*)(buf + GCI_SIZE + offset);

            *gc_log << *info << endl << endl;
            *gc_log << "is stack object " << data->is_stack_object << endl;
            
            if (info->array)
                delete[] data;
            else
                delete data;
        }
    }
    else
        *gc_log << endl << endl << "OK, no memory leaks was founded" << endl;
}

void GarbageCollector::AddPointer(void* object)
{
    pointers.push_back((SmartObject*)object);
}

void GarbageCollector::RemovePointer(const void* object)
{
    bool is_ok = true;

    auto find_iterator = find(pointers.begin(), pointers.end(), (SmartObject*)object);
    if (find_iterator == pointers.end())
        throw exception("Delalocating of unregistered object");
    else
    {
        pointers.erase(find_iterator);
        gc_info *info = get_GC_INFO(object);
        int from = info->adress;
        int len = info->size + GCI_SIZE;

        auto it = from_to_free_memory.upper_bound(make_pair(from, from + len));
        try_merge_iterator(it, make_pair(from, from + len));
    }
}

char* GarbageCollector::get_memory(size_t n)
{
    if (cur_position + n > MEMORY_BUFFER_SIZE)
        CollectGarbage();
    if (cur_position + n > MEMORY_BUFFER_SIZE)
        throw alloc_exception;
    cur_position += n;
    return memory_buffer + cur_position - n;
}


void* GarbageCollector::Allocate(size_t n, size_t line, const char* file, bool is_array)
{
    char *data = get_memory(n + GCI_SIZE);

    gc_info *info = (gc_info*)data;
    info->adress = (int)data;
    info->size = n;
    info->line = line;
    info->file = file;
    info->array = is_array;

    *gc_log << "allocating : " << *info << endl;

    AddPointer(data + GCI_SIZE);
    
    char* cur_pointer = data + GCI_SIZE;

    return cur_pointer;
}

void GarbageCollector::Deallocate(void *data)
{
    gc_info *info = get_GC_INFO(data);

    *gc_log << "deallocating " << *info << endl;

    RemovePointer(data);

    memset(get_begin_data(data), 0, GCI_SIZE + info->size);
}


bool GarbageCollector::AddStackObject(void *data)
{
    int adress = (int)data, offset = adress - (int)memory_buffer;
    if (offset >= 0 && offset < MEMORY_BUFFER_SIZE)
        return false;//heap object

    *gc_log << "AddStackObject" << endl;
    AddPointer(data);
    return true;
}

void GarbageCollector::RemoveStackObject(const void *data)
{
    int adress = (int)data, offset = adress - (int)memory_buffer;
    if (offset >= 0 && offset < MEMORY_BUFFER_SIZE)
        return;//heap object

    *gc_log << "RemoveStackObject" << endl;
    RemovePointer(data);
}