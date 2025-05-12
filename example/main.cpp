#include <iostream>

void a() {
    int foo = 1;
    std::cerr << "a" << std::endl;
}

void b() {
    int foo = 2;
    std::cerr << "b" << std::endl;
    a();
}

void c() {
    int foo = 3;
    std::cerr << "c" << std::endl;

    b();
}

void d() {
    int foo = 4;
    std::cerr << "d" << std::endl;
    c();
}

void e() {
    int foo = 5;
    std::cerr << "e" << std::endl;
    d();
}

void f() {
    int foo = 6;
    std::cerr << "f" << std::endl;
    e();
}

int add(int a, int b) 
{
    return a + b;
}

int main() 
{
    int a = 10;
    int b = 20;
    int c = add(a, b);
    f();
    std::cerr << add(a, c) << std::endl;
    return 0;
}