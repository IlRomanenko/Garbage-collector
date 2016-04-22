#include "Base.h"
#include "SmartObject.h"
#include "GarbageCollector.h"

class A : public SmartObject
{
    int x;
public:
    A() : x(0) 
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
    A *a = new A[1];
    delete []a;
     
    B *b = new B[1];
    b->Print();
    delete []b;

    cout << "B = " << sizeof(B) << endl;
    cout << "A = " << sizeof(A) << endl;
    cout << "SmartObject = " << sizeof(SmartObject) << endl;

    system("pause");
    return 0;
}