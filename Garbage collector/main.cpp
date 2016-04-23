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

int main()
{
    
    {
        A a, b(2);
    }
    A *a = gc_new A[1];
    A *c = gc_new A();
    gc_delete[] a;
    gc_delete c;

    
    A *b = gc_new A[1];
    //delete[] b;

    cout << "B = " << sizeof(B) << endl;
    cout << "A = " << sizeof(A) << endl;
    cout << "SmartObject = " << sizeof(SmartObject) << endl;

    return 0;
}