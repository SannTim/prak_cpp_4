// tests/test_smartpointer.cpp
#include <iostream>
#include "../base.h" 

class TestClass {
public:
    TestClass(int value) : value(value) {}
    int getValue() const { return value; }
private:
    int value;
};

void test_delete(SmartPointer<TestClass> sp){
	std::cout << "Count in function: " << *sp.get_count() << "\n"; // Should print 3
}

void testSmartPointer() {
    // Test construction
    SmartPointer<TestClass> sp1(new TestClass(10));
    std::cout << "sp1 value: " << sp1->getValue() << "\n"; // Should print 10

    // Test copy construction
    SmartPointer<TestClass> sp2 = sp1;
    std::cout << "sp2 value: " << sp2->getValue() << "\n"; // Should print 10

    // Test reference count
    std::cout << "Reference count after copy: " << *sp1.get_count() << "\n"; // Should print 2

    // Test assignment operator
    SmartPointer<TestClass> sp3;
    sp3 = sp1;
    std::cout << "sp3 value: " << sp3->getValue() << "\n"; // Should print 10
    std::cout << "Reference count after assignment: " << *sp1.get_count() << "\n"; // Should print 3

    // Test reset
    sp1.reset(new TestClass(20));
    std::cout << "sp1 new value: " << sp1->getValue() << "\n"; // Should print 20
    std::cout << "sp2 value: " << sp2->getValue() << "\n"; // Should still print 10
    std::cout << "Reference count of sp2: " << *sp2.get_count() << "\n"; // Should print 2

    // Test swap
    SmartPointer<TestClass> sp4(new TestClass(30));
    sp1.swap(sp4);
    std::cout << "sp1 value after swap: " << sp1->getValue() << "\n"; // Should print 30
    std::cout << "sp4 value after swap: " << sp4->getValue() << "\n"; // Should print 20
	
	test_delete(sp2);
	std::cout << "Reference count of sp2: " << *sp2.get_count() << "\n"; // Should print 2
}
int main() {
    testSmartPointer();
    return 0;
}

