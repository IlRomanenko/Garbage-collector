#include "SmartObject.h"

using gc_info = GarbageCollectorInfo;

/*
    What do you think about adding 4 empty bytes for non-arrays objects?
    Hah, there is more intresting way. We can add 4 empty bytes in gc_info, so
    when offsetted array pointer casts to gc_info there we can see
    how 4 bytes, where array size was written, add to our structure and
    validation_field become non-empty. Of course it depends on supposition,
    that structure has direct order in memory.
    That's intresting, IMHO.
*/

GarbageCollector* GarbageCollector::self = nullptr;
const bad_alloc GarbageCollector::alloc_exception = bad_alloc();
char* GarbageCollector::memory_buffer = nullptr;

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
    gc_log = new ofstream("gc_log.txt");

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
}



void* GarbageCollector::get_begin_data(const void *offset_data)
{
    char *res = (char*)offset_data - GCI_SIZE;
    gc_info* validation = (gc_info*)res;
    if (validation->validation_empty_field != 0)
        res -= sizeof(int);
    return res;
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
        assert("Can't find block");
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
    SmartObject* data;
    size_t offset;
    vector<pair<SmartObject*, bool> > need_erased;

    if (pointers.size() != 0)
    {
        *gc_log << endl << endl << "ATTENTION! Memory leaks was founded" << endl << endl;

        for (size_t i = 0; i < pointers.size(); i++)
        {
            info = get_GC_INFO(pointers[i]);
            offset = (info->array ? 1 : 0) * sizeof(size_t);

            data = (SmartObject*)(info->adress + GCI_SIZE + offset);

            *gc_log << *info << endl << endl;
            need_erased.push_back(make_pair(data, info->array));
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
}



void GarbageCollector::AddPointer(void* object)
{
    pointers.push_back((SmartObject*)object);
}

void GarbageCollector::RemovePointer(const void* object, bool is_array)
{
    bool is_ok = true;

    char* object_begin = (char*)object + sizeof(size_t) * (is_array ? 1 : 0);

    bool is_stack = ((SmartObject*)object_begin)->is_stack_object;

    auto find_iterator = find(pointers.begin(), pointers.end(), (SmartObject*)object_begin);
    if (find_iterator == pointers.end())
        throw exception("Delalocating of unregistered object");
    else
    {
        pointers.erase(find_iterator);
    }
    if (!is_stack)
    {
        gc_info *info = get_GC_INFO(object);
        ReleaseChunk(info->adress - (int)memory_buffer, info->size + GCI_SIZE - 1);
    }
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
    info->validation_empty_field = 0;

    *gc_log << "allocating : " << *info << endl;

    int offset = sizeof(size_t) * (is_array ? 1 : 0);

    AddPointer(data + GCI_SIZE + offset);

    char* cur_pointer = data + GCI_SIZE;

    return cur_pointer;
}

void GarbageCollector::Deallocate(void *data, bool is_array)
{
    gc_info *info = get_GC_INFO(data);

    *gc_log << "deallocating " << *info << endl;

    int offset = sizeof(size_t) * (is_array ? 1 : 0);

    RemovePointer(data, is_array);

    memset(get_begin_data(data), 0, GCI_SIZE + info->size);
}



bool GarbageCollector::AddLinkSource(void *data)
{
    int adress = (int)data, offset = adress - (int)memory_buffer;
    if (offset >= 0 && offset < MEMORY_BUFFER_SIZE)
        return false;//heap object

    *gc_log << "AddStackObject" << endl;
    AddPointer(data);
    return true;
}

void GarbageCollector::RemoveLinkSource(const void *data)
{
    int adress = (int)data, offset = adress - (int)memory_buffer;
    if (offset >= 0 && offset < MEMORY_BUFFER_SIZE)
        return;//heap object

    *gc_log << "RemoveStackObject" << endl;
    RemovePointer(data, false);
}



void GarbageCollector::CollectGarbage()
{
    *gc_log << endl << "Garbage collecting has began" << endl;
    auto pointers = GetPointers();
    gc_info *info;
    for (auto & pnt : pointers)
    {
        if (!pnt->is_stack_object)
        {
            info = get_GC_INFO(pnt);
            if (info->array)
            {
                int array_size = *((int*)(info->adress + GCI_SIZE));
                int elem_size = (info->size - sizeof(size_t)) / array_size;
                char *mem = (char*)pnt;
                for (int i = 0; i < array_size; i++)
                {
                    ((SmartObject*)(mem + elem_size * i))->has_checked = false;
                }
            }
        }
        pnt->has_checked = false;
    }
    for (auto & pnt : pointers)
    {
        if (pnt->is_stack_object && !pnt->has_checked)
            Dfs(pnt);
    }
    vector<pair<SmartObject*, bool> > need_erased;
    for (int i = 0; i < (int)pointers.size(); i++)
    {
        if (!pointers[i]->has_checked)
        {
            info = get_GC_INFO(pointers[i]);
            if (info->array)
            {
                int array_size = *((int*)(info->adress + GCI_SIZE));
                int elem_size = (info->size - sizeof(size_t)) / array_size;
                char *mem = (char*)pointers[i];
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
                    need_erased.push_back(make_pair(pointers[i], info->array));
            }
            else
                need_erased.push_back(make_pair(pointers[i], info->array));
        }
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
