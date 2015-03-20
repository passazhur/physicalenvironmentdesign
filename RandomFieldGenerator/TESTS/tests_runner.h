#ifndef TESTS_RUNNER_H
#define TESTS_RUNNER_H

#include <QStringList>

#include "test_clmanager.h"
#include "test_viennacl.h"
#include "test_simulation.h"

//tip! use "-vs" to see emited signals
QStringList arguments = {
//    " ","-o","UnitTestsResults/UnitTest_XXX.txt"};
    " ","-silent"};

void run_tests_CLManager()
{
    Test_CLManager _myTest_CLManager;
    QTest::qExec(&_myTest_CLManager, arguments);
}
void run_tests_ViennaCL()
{
    Test_ViennaCL _myTest_ViennaCL;
    QTest::qExec(&_myTest_ViennaCL, arguments);
}
void run_tests_Simulation()
{
    Test_Simulation _myTest_Simulation;
    QTest::qExec(&_myTest_Simulation, arguments);
}
void run_tests_all()
{
    run_tests_CLManager();
    run_tests_ViennaCL();
    run_tests_Simulation();
}
#endif // TESTS_RUNNER_H