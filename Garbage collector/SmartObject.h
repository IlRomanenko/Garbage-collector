#pragma once
#include "ISmartObject.h"

class SmartObject : public ISmartObject
{
    vector<ISmartObject*> pntrs, rev_pntrs;

    void Add_Rev_Link(ISmartObject* object)
    {
        rev_pntrs.push_back(object);
    }

    void Remove_Rev_Link(ISmartObject* object)
    {
        auto it = find(rev_pntrs.begin(), rev_pntrs.end(), object);
        if (it != rev_pntrs.end())
            rev_pntrs.erase(it);
    }

public:
    SmartObject() { }

    void AddLink(SmartObject* object)
    {
        pntrs.push_back(object);
        object->Add_Rev_Link(this);
    }

    void RemoveLink(SmartObject* object)
    {
        auto it = find(pntrs.begin(), pntrs.end(), object);
        if (it != pntrs.end())
            pntrs.erase(it);
        object->Remove_Rev_Link(this);
    }

    virtual vector<ISmartObject*> pointers() const override
    {
        return pntrs;
    }

    virtual vector<ISmartObject*> rev_pointers_links() const override
    {
        return rev_pntrs;
    }


    virtual ~SmartObject()
    {
        cout << "~SmartObject" << endl;
        SmartObject *obj;
        for (auto& pnt : pntrs)
        {
            obj = (SmartObject*)pnt;
            obj->Remove_Rev_Link(this);
        }
        vector<ISmartObject*> t_pntrs;
        swap(t_pntrs, rev_pntrs);
        for (auto& pnt : t_pntrs)
        {
            obj = (SmartObject*)pnt;
            obj->RemoveLink(this);
        }
    }
};