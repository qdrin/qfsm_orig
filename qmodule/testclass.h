#ifndef TESTCLASS_HPP
#define TESTCLASS_HPP
#include <string>
class TestClass
{
private:
    std::string s;
public:
    TestClass(std::string _s = "");
    std::string getString();
};

#endif // TESTCLASS_H
