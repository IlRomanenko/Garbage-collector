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

    ~A()
    {
        LOG << "~A()" << endl;
    }

    virtual vector<SmartObject*> pointers() const override
    {
        return vector<SmartObject*>();
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

    void Print()
    {
        cout << str << endl;
    }

    ~B()
    {
        LOG << "~B()" << endl;
    }
};

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
    //delete[] b;

    cout << "sizeof(B) = " << sizeof(B) << endl;
    cout << "sizeof(A) = " << sizeof(A) << endl;
    cout << "sizeof(SmartObject) = " << sizeof(SmartObject) << endl;
}


void CreateMoreObjects()
{
    A *a;

    for (int i = 0; i < 10; i++)
        a = gc_new A(i);
}

int main()
{
    SimpleTestGC();
    
    CreateMoreObjects();

    return 0;
}