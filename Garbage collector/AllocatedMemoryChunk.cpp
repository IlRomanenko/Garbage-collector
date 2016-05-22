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

void AllocatedMemoryChunk::Print() const
{
    cerr << "Address = " << mem_ptr << " Size = " << length << endl;
    cerr << "Count = " << smartObjects.size() << endl;
    for (auto& pnt : smartObjects)
        cout << static_cast<const void* const>(pnt) << ' ';
    cout << endl;
}

void AllocatedMemoryChunk::Destroy()
{
    while (!smartObjects.empty())
        smartObjects.front()->~ISmartObject();
    free(mem_ptr);
    mem_ptr = nullptr;
}

bool& AllocatedMemoryChunk::IsReachable()
{
    return is_reachable;
}