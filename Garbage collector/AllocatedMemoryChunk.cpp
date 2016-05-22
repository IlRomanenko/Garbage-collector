#include "ISmartObject.h"

bool AllocatedMemoryChunk::IsInner(const void* ptr) const
{
    ptrdiff_t offset = static_cast<const char*>(ptr) - static_cast<const char*>(mem_ptr);
    return (offset >= 0 && offset < length);
}

void AllocatedMemoryChunk::AddObject(ISmartObject* ptr)
{
    smartObjects.push_back(ptr);
}

void AllocatedMemoryChunk::RemoveObject(const ISmartObject* ptr)
{
    auto it = find(smartObjects.begin(), smartObjects.end(), ptr);
    if (it != smartObjects.end())
        smartObjects.erase(it);
    else
        cerr << "ATTENTION! Deleting unregistered object" << endl;
}

vector<ISmartObject*>& AllocatedMemoryChunk::SmartObjects()
{
    return smartObjects;
}

void AllocatedMemoryChunk::Print()
{
    cout << "Address = " << addressof(mem_ptr) << " Size = " << length << endl;
    cout << "Count = " << smartObjects.size() << endl;
}

void AllocatedMemoryChunk::Destroy()
{
    vector<ISmartObject*> tv = smartObjects;
    for (auto& pnt : tv)
        pnt->~ISmartObject();
    free(mem_ptr);
    mem_ptr = nullptr;
}