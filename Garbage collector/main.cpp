#include "Base.h"
#include "SmartObject.h"
#include "GarbageCollector.h"


class A : public SmartObject
{
    int x;
public:
    A() : x(5)
    {
        LOG << "A()" << endl;
    }
    A(int _x) : x(_x)
    {
        LOG << "A(int _x)" << endl;
    }

    virtual ~A()
    {
        LOG << "~A()" << endl;
    }
};

class B : public A
{
    string str;
public:
    B() : str("Hello!")
    {
        LOG << "B()" << endl;
    }
    
    B(string _str) : str(_str)
    {
        LOG << "B(string _str)" << endl;
    }

    void Print()
    {
        cout << str << endl;
    }

    virtual ~B()
    {
        LOG << "~B() : str = " << str << endl;
    }
};

class HugeClass : public A
{
    char buffer[100];
public:
    HugeClass() 
    {
        LOG << "HugeClass()" << endl;
    }
    ~HugeClass()
    {
        LOG << "~HugeClass()" << endl;
    }
};

#define TestGC(FunctionName)                                            \
    {                                                                   \
        GarbageCollector::SetGCLogFile(string(#FunctionName) + ".txt"); \
        FunctionName();                                                 \
        GarbageCollector::DestroyInstance();                            \
    }                                                                   \


void SimpleTestGC()
{
    {
        A a, b(2);
    }
    
    A t[2];
    
    A *a = gc_new A[2];

    A *c = gc_new A();
    gc_delete[] a;
    gc_delete c;


    B *b = gc_new B[3]; //memory leak here
    //gc_delete[] b;

    cout << "sizeof(B) = " << sizeof(B) << endl;
    cout << "sizeof(A) = " << sizeof(A) << endl;
    cout << "sizeof(SmartObject) = " << sizeof(ISmartObject) << endl;
   
}

void CreateMoreObjects()
{
    A *a;

    for (int i = 0; i < 50; i++)
        a = gc_new A(i);
}

void ReachableObjects()
{
    A mainNode(0);                                                          //         *
    for (int i = 0; i < 3; i++)                                             //      /  |  \ 
        mainNode.AddLink(gc_new A(i + 1));                                  //     * - *   *
                                                                            //
    ((A*)mainNode.pointers()[0])->AddLink((A*)mainNode.pointers()[1]);      //
}                                                                           //

void CircleTest()
{
    A mainNode(0);
    for (int i = 0; i < 3; i++)
        mainNode.AddLink(gc_new A(i + 1));
    B *b, *c, *d;
    b = gc_new B("It's b object");                                          //
    c = gc_new B("It's c object");                                          //          b
    d = gc_new B("It's d object");                                          //        /   \ 
    b->AddLink(d);                                                          //       c  -  d
    c->AddLink(b);                                                          //
    d->AddLink(c);

    GarbageCollector::ForciblyCollectGarbage();
    cout << "After collecting garbage" << endl;
}

void ArrayTest()
{
    A *mainNode = gc_new A(1);
    for (int i = 0; i < 3; i++)
    {
        mainNode->AddLink(gc_new A[3]);
    }
    {
        A second_holder(2);
        second_holder.AddLink(&((A*)mainNode->pointers()[1])[0]);           //get the first element 
        A* t = gc_new_noexcept HugeClass();                                 //from the second array of mainNode
        if (t != nullptr)
            gc_delete t;
    }
}

class CInternal : public SmartObject {

public:
    CInternal() { }
    void Print()
    {
        cout << "Hello, world! I'm CInternal" << endl;
    }
};

class CExternal : public SmartObject {

public:
    
    CExternal()
    {
        AddLink(&worker);
    }

    CInternal* GetWorkerInterface() { return &worker; }

    void Print()
    {
        cout << "Hello, world2! I'm CExternal" << endl;
    }

private:

    CInternal worker;

};

void Egor_InternalObjects_Test()
{
    CInternal holder;
    CExternal* ext = gc_new CExternal();
    holder.AddLink(ext->GetWorkerInterface());

    for (int i = 0; i < 50; i++)
        gc_new HugeClass();
    ((CInternal*)holder.pointers()[0])->Print();
    ext->Print();
}

void Egor_InternalObject_Test2()
{
    CInternal A;
    CExternal* B = gc_new CExternal();
    A.AddLink(B);
    CInternal* C = B->GetWorkerInterface();
    CInternal* D = gc_new CInternal();
    C->AddLink(D);
    GarbageCollector::ForciblyCollectGarbage();
    A.Print();
    B->Print();
    C->Print();
    D->Print();
}

class ExtA : virtual public SmartObject
{
public:
    virtual void Print() = 0;
};

class CMyClass
{
    int _x;
public:
    CMyClass() : _x(-1) { } 
};



class ExtB : public CMyClass, public ExtA
{
public:
    ExtB() { }
    void Print() override
    {
        cout << "ExtB" << endl;
    }
    virtual void Print2()
    {
        cout << "ExtB2" << endl;
    }
};

void ExtendedObjectTest()
{
    ExtB* t = gc_new ExtB();
    ExtA *d = (ExtA*)t;
    d->Print();
    t->Print2();
    gc_delete dynamic_cast<ISmartObject*>(t);
}


int main()
{
    TestGC(SimpleTestGC);
    TestGC(CreateMoreObjects);
    TestGC(ReachableObjects);
    TestGC(CircleTest);
    TestGC(ArrayTest);
    TestGC(Egor_InternalObjects_Test);

    TestGC(ExtendedObjectTest);
    TestGC(Egor_InternalObject_Test2);
    system("pause");
    return 0;
}
