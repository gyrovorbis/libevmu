#include <elysian_qtest.hpp>
#include <evmu/util/evmu_context.h>

using namespace elysian;

int main(int argc, char* argv[]) {

    UnitTestSuite testSuite;
    return !testSuite.exec(argc, argv);
}
