#pragma once
#include "ISmartObject.h"

class SmartObject : public ISmartObject
{
    vector<ISmartObject*> pntrs;

public:
    SmartObject() { }

    void AddLink(SmartObject* object)
    {
        pntrs.push_back(object);
    }

    void RemoveLink(SmartObject* object)
    {
        auto it = find(pntrs.begin(), pntrs.end(), object);
        if (it != pntrs.end())
            pntrs.erase(it);
    }

    virtual vector<ISmartObject*> pointers() const override
    {
        return pntrs;
    }

    virtual ~SmartObject() { }
};