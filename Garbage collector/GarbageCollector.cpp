#include "SmartObject.h"

using gc_info = GarbageCollectorInfo;

GarbageCollector* GarbageCollector::self = nullptr;
const bad_alloc GarbageCollector::alloc_exception = bad_alloc();
string GarbageCollector::file_name = "gc_info.txt";

ostream& operator << (ostream &stream, GarbageCollectorInfo info)
{
    stream << "adress = " << info.adress << " size = " << info.size << endl;
    stream << "line = " << info.line << " file = " << info.file << endl;
    stream << "total size = " << info.size + sizeof(GarbageCollectorInfo) << endl;
    return stream;
}



GarbageCollector::GarbageCollector()
{
    memory_buffer = new char[MEMORY_BUFFER_SIZE];
    memset(memory_buffer, 0, MEMORY_BUFFER_SIZE);
    gc_log = new ofstream(file_name);

    boundaries_occupied_memory.insert(make_pair(-1, -1));
    boundaries_occupied_memory.insert(make_pair(MEMORY_BUFFER_SIZE, MEMORY_BUFFER_SIZE));

    size_adress_free_memory.insert(make_pair(MEMORY_BUFFER_SIZE, 0));
}

GarbageCollector::~GarbageCollector()
{
    CheckMemoryLeaks();

#ifdef PARANOID
    for (int i = 0; i < MEMORY_BUFFER_SIZE; i++)
        if (memory_buffer[i] != 0)
        {
            *gc_log << "ALARM! Something is still alive" << endl;
        }
#endif

    delete memory_buffer;
    gc_log->flush();
    gc_log->close();
    delete gc_log;
    stack_pointers.clear();
    heap_pointers.clear();
    array_pointers.clear();
    size_adress_free_memory.clear();
    boundaries_occupied_memory.clear();
    memory_buffer = nullptr;
}



void* GarbageCollector::get_begin_data(const void *offset_data)
{
    return (char*)offset_data - GCI_SIZE;
}

gc_info* GarbageCollector::get_GC_INFO(const void *offset_data)
{
    return (gc_info*)get_begin_data(offset_data);
}



int GarbageCollector::get_free_block_position(int size)
{
    auto it = size_adress_free_memory.lower_bound(make_pair(size, INT_MIN));
    if (it == size_adress_free_memory.end())
    {
        CollectGarbage();
        it = size_adress_free_memory.lower_bound(make_pair(size, INT_MIN));
        if (it == size_adress_free_memory.end())
        {
            *gc_log << "Not enough memory" << endl;
            throw alloc_exception;
        }
    }
    auto res = *it;
    size_adress_free_memory.erase(*it);
    if (res.first - size > 0)
        size_adress_free_memory.insert(make_pair(res.first - size, res.second + size));

    boundaries_occupied_memory.insert(make_pair(res.second, res.second + size - 1));
    return res.second;
}

char* GarbageCollector::get_memory(size_t size)
{
    return memory_buffer + get_free_block_position(size);
}

void GarbageCollector::remove_free_chunk(int begin_pos, int end_pos)
{
    auto it = size_adress_free_memory.lower_bound(make_pair(end_pos - begin_pos + 1, begin_pos));
    if (it == size_adress_free_memory.end())
    {
        cout << "aaaa" << endl;
        assert("Can't find block");
    }
    size_adress_free_memory.erase(it);
}

void GarbageCollector::ReleaseChunk(int beg_position, int size)
{
    int end_position = beg_position + size;
    pair<int, int> free_memory = make_pair(beg_position, end_position);
    auto it = boundaries_occupied_memory.lower_bound(make_pair(beg_position, end_position));

    if (it == boundaries_occupied_memory.end() || *it != make_pair(beg_position, end_position))
        throw exception("Deallocating of unregisterd object");

    auto next_it = it;
    next_it++;
    if (next_it != boundaries_occupied_memory.end() && next_it->first - 1 != end_position)
    {
        remove_free_chunk(end_position + 1, next_it->first - 1);
        end_position = next_it->first - 1;
    }

    if (it != boundaries_occupied_memory.begin())
    {
        auto prev_it = it;
        prev_it--;
        if (prev_it->second + 1 != beg_position)
        {
            remove_free_chunk(prev_it->second + 1, beg_position - 1);
            beg_position = prev_it->second + 1;
        }
    }
    boundaries_occupied_memory.erase(it);
    size_adress_free_memory.insert(make_pair(end_position - beg_position + 1, beg_position));
}



void GarbageCollector::CheckMemoryLeaks()
{
    gc_info *info;
    size_t offset;
    vector<pair<SmartObject*, bool> > need_erased;

    bool is_all_stack_objects = (heap_pointers.empty() && array_pointers.empty());

    if (is_all_stack_objects)
        return;

    if (!is_all_stack_objects)
    {
        *gc_log << endl << endl << "ATTENTION! Memory leaks was founded" << endl << endl;

        for (auto pointer : heap_pointers)
        {
            info = get_GC_INFO(pointer);
            *gc_log << *info << endl << endl;
            need_erased.push_back(make_pair(pointer, false));
        }
        for (auto array_pointer : array_pointers)
        {
            info = get_GC_INFO((char*)array_pointer - sizeof(size_t));
            *gc_log << *info << endl << endl;
            need_erased.push_back(make_pair(array_pointer, true));
        }

    }
    else
        *gc_log << endl << endl << "OK, no memory leaks was founded" << endl;

    for (auto & pair : need_erased)
    {
        if (pair.second)
            delete[] pair.first;
        else
            delete pair.first;
    }
    stack_pointers.clear(); //remove stack objects
}



void GarbageCollector::AddPointer(void* object, vector<SmartObject*> &pointers)
{
    pointers.push_back((SmartObject*)object);
}

//Using info = nullptr for stack objects
void GarbageCollector::RemovePointer(const void* object, gc_info* info, vector<SmartObject*> &pointers)
{
    bool is_ok = true;
    
    auto find_iterator = find(pointers.begin(), pointers.end(), (SmartObject*)object);
    if (find_iterator == pointers.end())
        throw exception("Delalocating of unregistered object");
    else
    {
        pointers.erase(find_iterator);
    }
    if (info != nullptr)
        ReleaseChunk(info->adress - (int)memory_buffer, info->size + GCI_SIZE - 1);
}



void* GarbageCollector::Allocate(size_t n, size_t line, const char* file, bool is_array)
{
    char *data = get_memory(n + GCI_SIZE);

    gc_info *info = (gc_info*)data;
    info->adress = (int)data;
    info->size = n;
    info->line = line;
    info->file = file;

    *gc_log << "allocating : " << *info << endl;

    if (is_array)
        AddPointer(data + GCI_SIZE + sizeof(size_t), array_pointers);
    else
        AddPointer(data + GCI_SIZE, heap_pointers);

    char* cur_pointer = data + GCI_SIZE;

    return cur_pointer;
}

void GarbageCollector::Deallocate(void *data, bool is_array)
{
    char *data_begin = (char*)data;
    gc_info *info = get_GC_INFO(data_begin);

    *gc_log << "deallocating " << *info << endl;

    if (is_array)
        RemovePointer(data_begin + sizeof(size_t), info, array_pointers);
    else
        RemovePointer(data, info, heap_pointers);

    memset(get_begin_data(data_begin), 0, GCI_SIZE + info->size);
}



void GarbageCollector::AddLinkSource(void *data)
{
    int adress = (int)data, offset = adress - (int)memory_buffer;
    if (offset >= 0 && offset < MEMORY_BUFFER_SIZE)
        return;//heap object

    *gc_log << "AddStackObject" << endl;
    AddPointer(data, stack_pointers);
}

void GarbageCollector::RemoveLinkSource(const void *data)
{
    int adress = (int)data, offset = adress - (int)memory_buffer;
    if (offset >= 0 && offset < MEMORY_BUFFER_SIZE)
        return;//heap object

    *gc_log << "RemoveStackObject" << endl;
    RemovePointer(data, nullptr, stack_pointers);
}



void GarbageCollector::CollectGarbage()
{
    *gc_log << endl << "Garbage collecting has began" << endl;
    
    gc_info *info;
    for (auto& pnt : stack_pointers)
        pnt->has_checked = false;
    for (auto& pnt : heap_pointers)
        pnt->has_checked = false;
    for (auto& pnt : array_pointers)
    {
        info = get_GC_INFO((char*)pnt - sizeof(size_t));
        int array_size = *((int*)(info->adress + GCI_SIZE));
        int elem_size = (info->size - sizeof(size_t)) / array_size;
        char *mem = (char*)pnt;
        for (int i = 0; i < array_size; i++)
        {
            ((SmartObject*)(mem + elem_size * i))->has_checked = false;
        }
    }

    for (auto& pnt : stack_pointers)
    {
        if (!pnt->has_checked)
            Dfs(pnt);
    }
    vector<pair<SmartObject*, bool> > need_erased;

    for (auto& pnt : heap_pointers)
        if (!pnt->has_checked)
            need_erased.push_back(make_pair(pnt, false));
    for (auto& pnt : array_pointers)
    {
        info = get_GC_INFO((char*)pnt - sizeof(size_t));
        int array_size = *((int*)(info->adress + GCI_SIZE));
        int elem_size = (info->size - sizeof(size_t)) / array_size;
        char *mem = (char*)pnt;
        bool is_all_garbage = true;
        for (int arr_it = 0; arr_it < array_size; arr_it++)
        {
            if (((SmartObject*)(mem + elem_size * arr_it))->has_checked)
            {
                is_all_garbage = false;
                break;
            }
        }
        if (is_all_garbage)
            need_erased.push_back(make_pair(pnt, true));
    }

    for (int i = 0; i < (int)need_erased.size(); i++)
    {
        if (need_erased[i].second)
            delete[] need_erased[i].first;
        else
            delete need_erased[i].first;
    }
    *gc_log << endl << "All garbage has destroyed" << endl;
}

void GarbageCollector::Dfs(SmartObject* node)
{
    node->has_checked = true;
    for (auto& pnt : node->pointers())
    {
        if (!pnt->has_checked)
            Dfs(pnt);
    }
}
