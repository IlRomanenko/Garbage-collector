#pragma once
#include "Base.h"

class AllocatedMemoryChunk
{
    void* mem_ptr;
    ptrdiff_t length;
    vector<ISmartObject*> smartObjects;
    bool is_reachable;

public:

    AllocatedMemoryChunk() = delete;

    AllocatedMemoryChunk(void* memory_ptr, size_t memory_length) :
        mem_ptr(memory_ptr), length(memory_length) { }

    bool IsInner(const void* ptr) const;

    void AddObject(ISmartObject* ptr);

    void RemoveObject(const ISmartObject* ptr);

    vector<ISmartObject*>& SmartObjects();

    void Print() const;

    void Destroy();

    bool& IsReachable();

    ~AllocatedMemoryChunk() { }
};