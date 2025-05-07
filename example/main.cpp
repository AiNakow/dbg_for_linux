#include <iostream>

int add(int a, int b) 
{
    return a + b;
}

int main() 
{
    int a = 10;
    int b = 20;
    int c = add(a, b);
    std::cerr << add(a, c) << std::endl;
    return 0;
}