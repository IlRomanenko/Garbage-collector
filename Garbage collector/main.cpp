#include "Base.h"
#include "SmartObject.h"
#include "GarbageCollector.h"


class A : public SmartObject
{
    int x;
protected:
    vector<SmartObject*> pntrs;
public:
    A() : x(5)
    {
        LOG << "A()" << endl;
    }
    A(int _x) : x(_x)
    {
        LOG << "A(int _x)" << endl;
    }

    ~A()
    {
        LOG << "~A()" << endl;
    }

    void AddLink(A* object)
    {
        pntrs.push_back(object);
    }

    void RemoveLink(A* object)
    {
        pntrs.erase(find(pntrs.begin(), pntrs.end(), object));
    }

    virtual vector<SmartObject*> pointers() const override
    {
        return pntrs;
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

    ~B()
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
    
    A *a = gc_new A[1];

    A *c = gc_new A();
    gc_delete[] a;
    gc_delete c;


    A *b = gc_new B[3]; //memory leak here
    //gc_delete[] b;

    cout << "sizeof(B) = " << sizeof(B) << endl;
    cout << "sizeof(A) = " << sizeof(A) << endl;
    cout << "sizeof(SmartObject) = " << sizeof(SmartObject) << endl;
   
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
        A* t = gc_new HugeClass();                                          //from the second array of mainNode
        gc_delete t;
    }
}


int main()
{
    TestGC(SimpleTestGC);
    TestGC(CreateMoreObjects);
    TestGC(ReachableObjects);
    TestGC(CircleTest);
    TestGC(ArrayTest);
    system("pause");
    return 0;
}
