#include "ISmartObject.h"

GC_NEW_STRUCT gc_new_struct;

GarbageCollector* GarbageCollector::self = nullptr;
const bad_alloc GarbageCollector::alloc_exception = bad_alloc();
string GarbageCollector::file_name = "gc_info.txt";


GarbageCollector::GarbageCollector()
{
    gc_log = new ofstream(file_name);
}

GarbageCollector::~GarbageCollector()
{
    CheckMemoryLeaks();

    gc_log->flush();
    gc_log->close();
    delete gc_log;
}

void GarbageCollector::CheckMemoryLeaks()
{
    if (allocatedMemory.size() != 0)
        cout << "Memory leaks was founded" << endl;

    for (auto& chunk : allocatedMemory)
    {
        chunk.Print();
        chunk.Destroy();
    }
    allocatedMemory.clear();
}

void GarbageCollector::InitializeMemory(void* data, size_t n)
{
    allocatedMemory.push_back(AllocatedMemoryChunk(data, n));
}

void* GarbageCollector::NoexceptAllocate(size_t n) noexcept
{
    void* data = calloc(n, n);

    if (data == nullptr)
    {
        *gc_log << endl << "Can't allocate. Not enough memory!" << endl << endl;
        free(data);
        return data;
    }
    InitializeMemory(data, n);

    return data;
}

void* GarbageCollector::Allocate(size_t n)
{
    void *data = NoexceptAllocate(n);

    if (data == nullptr)
        throw alloc_exception;

    return data;
}

void GarbageCollector::Deallocate(void *data)
{
    *gc_log << "Deallocating" << endl;
    for (size_t i = 0; i < allocatedMemory.size(); i++)
    {
        auto& chunk = allocatedMemory[i];
        if (chunk.IsInner(data) && chunk.SmartObjects().size() == 0)
        {
            chunk.Print();
            chunk.Destroy();
            swap(allocatedMemory[i], allocatedMemory.back());
            allocatedMemory.pop_back();
            return;
        }
    }
    *gc_log << "Error! Element was'n find in Deallocate" << endl;
}



void GarbageCollector::AddLinkSource(ISmartObject *data)
{
    *gc_log << "AddLinkSource" << endl;
    for (auto& chunk : allocatedMemory)
        if (chunk.IsInner(data))
        {
            *gc_log << "Added heap object" << endl;
            chunk.AddObject(data);
            return;
        }
    *gc_log << "Added stack object" << endl;
    stackObjects.push_back(data);
}

void GarbageCollector::RemoveLinkSource(const ISmartObject *data)
{
    *gc_log << "RemoveLinkSource" << endl;
    for (auto& chunk : allocatedMemory)
        if (chunk.IsInner(data))
        {
            *gc_log << "Removed heap Link" << endl;
            chunk.RemoveObject(data);
            return;
        }
    *gc_log << "Removed stack Link" << endl;
    auto it = find(stackObjects.begin(), stackObjects.end(), data);
    if (it != stackObjects.end())
        stackObjects.erase(it);
    else
        *gc_log << "Something strange. Can't remove element by stackObject" << endl;
}



void GarbageCollector::CollectGarbage()
{
    *gc_log << endl << "Garbage collecting has began" << endl;

    for (auto& pnt : stackObjects)
        pnt->has_checked = false;
    for (auto& chunk : allocatedMemory)
        for (auto& pnt : chunk.SmartObjects())
            pnt->has_checked = false;

    for (auto& pnt : stackObjects)
    {
        if (!pnt->has_checked)
            Dfs(pnt);
    }

    bool need_delete = true;

    for (size_t i = 0; i < allocatedMemory.size(); i++)
    {
        auto& chunk = allocatedMemory[i];
        need_delete = true;
        for (auto& pnt : chunk.SmartObjects())
            if (pnt->has_checked)
            {
                need_delete = false;
                break;
            }
        if (need_delete)
        {
            chunk.Print();
            chunk.Destroy();
            swap(allocatedMemory[i], allocatedMemory.back());
            allocatedMemory.pop_back();
            i--;
        }
    }

    *gc_log << endl << "All garbage has destroyed" << endl;
}

void GarbageCollector::Dfs(ISmartObject* node)
{
    node->has_checked = true;
    for (auto& pnt : node->pointers())
    {
        if (!pnt->has_checked)
            Dfs(pnt);
    }
}

void GarbageCollector::UncheckDfs(ISmartObject * node)
{
    if (!node->has_checked)
        return;
    node->has_checked = false;
    for (auto& pnt : node->pointers())
        if (pnt->has_checked)
            UncheckDfs(pnt);
}
