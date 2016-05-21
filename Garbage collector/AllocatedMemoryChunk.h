#pragma once
#include "ISmartObject.h"

class AllocatedMemoryChunk
{
    void* mem_ptr;
    memtype length;
    vector<ISmartObject*> smartObjects;

public:

    AllocatedMemoryChunk() = delete;

    AllocatedMemoryChunk(void* memory_ptr, size_t memory_length) :
        mem_ptr(memory_ptr), length(memory_length) { }

    bool IsInner(const void* ptr) const;

    void AddObject(ISmartObject* ptr);

    void RemoveObject(const ISmartObject* ptr);

    vector<ISmartObject*>& SmartObjects();

    void Print();

    void Destroy();

    ~AllocatedMemoryChunk() { }
};