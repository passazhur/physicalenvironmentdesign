#ifndef TESTS_RUNNER_H
#define TESTS_RUNNER_H

#include <QStringList>

#include "test_datamanager.h"
#include "test_gridelement.h"
#include "test_nodewrapper.h"
#include "test_piecewiselinearcomplex.h"

//tip! use "-vs" to see emited signals
QStringList arguments = {
//    " ","-o","UnitTestsResults/UnitTest_XXX.txt"};
    " ","-silent"};

void run_tests_NodeWrapper()
{
    Test_NodeWrapper _myTest_NodeWrapper;
    QTest::qExec(&_myTest_NodeWrapper, arguments);
}

void run_tests_GridElement()
{
    Test_GridElement _myTest_GridElement;
    QTest::qExec(&_myTest_GridElement, arguments);
}

void run_tests_DataManager()
{
    Test_DataManager _myTest_DataManager;
    QTest::qExec(&_myTest_DataManager, arguments);
}

void run_tests_DelaunayGridGenerator()
{
    /// \todo
    //Test_DelaunayGridGenerator _myTest_DelaunayGridGenerator;
    //QTest::qExec(&_myTest_DelaunayGridGenerator, arguments);
}

void run_tests_PiecewiseLinearComplex()
{
    Test_PiecewiseLinearComplex _myTest_PiecewiseLinearComplex;
    QTest::qExec(&_myTest_PiecewiseLinearComplex, arguments);
}

void run_tests_all()
{
    run_tests_NodeWrapper();
    run_tests_GridElement();
    run_tests_DataManager();
    run_tests_PiecewiseLinearComplex();
    run_tests_DelaunayGridGenerator();
}
#endif // TESTS_RUNNER_H
