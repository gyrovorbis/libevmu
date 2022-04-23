#include <elysian_qtest.hpp>

using namespace elysian;

int main(int argc, char* argv[]) {

    UnitTestSuite testSuite;
    return !testSuite.exec(argc, argv);
}
