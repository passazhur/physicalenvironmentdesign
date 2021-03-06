#include "test_simulation.h"

void Test_Simulation::constructLocalStiffnessMatrix()
{
    float _K[4][4];
    int _A[] = {0,0,0};
    int _B[] = {1,0,0};
    int _C[] = {0,1,0};
    int _D[] = {0,0,1};
    Simulation::constructLocalStiffnessMatrix(
                1.0f / 10,
                _A, _B, _C, _D,
                3.0f, _K);
    float _KTrue[4][4] = {
        { 0.15f, -0.05f, -0.05f, -0.05f},
        {-0.05f,  0.05f,   0.0f,   0.0f},
        {-0.05f,   0.0f,  0.05f,   0.0f},
        {-0.05f,   0.0f,   0.0f,  0.05f}};
    float _maxError = 0.0f;
    for(int i=0; i<4; ++i)
        for(int j=0; j<4; ++j)
        {
            float err = std::fabs(_K[i][j]-_KTrue[i][j]);
            if(_KTrue[i][j]!=0.0f)
                err /= _KTrue[i][j];
            if(err>_maxError)
                _maxError = err;
        }
    QVERIFY(_maxError < 1e-4f);
}

void Test_Simulation::applyLocalDirichletConditions()
{
    float _K[4][4] = {
        { 0.15f, -0.05f, -0.05f, -0.05f},
        {-0.05f,  0.05f,   0.0f,   0.0f},
        {-0.05f,   0.0f,  0.05f,   0.0f},
        {-0.05f,   0.0f,   0.0f,  0.05f}};
    float _f[4] = {10.0f, 20.0f, 30.0f, 40.0f};
    Simulation::applyLocalDirichletConditions(
                0b00000101,
                10.0f,
                _K, _f);
    float _KTrue[4][4] = {
        { 0.15f,   0.0f,   0.0f,   0.0f},
        {  0.0f,  0.05f,   0.0f,   0.0f},
        {  0.0f,   0.0f,  0.05f,   0.0f},
        {  0.0f,   0.0f,   0.0f,  0.05f}};
    float _fTrue[4] = {1.5f, 20.0f + 0.5f, 0.5f, 40.0f + 0.5f};
    float _maxError = 0.0f;
    for(int i=0; i<4; ++i)
        for(int j=0; j<4; ++j)
        {
            float err = std::fabs(_K[i][j]-_KTrue[i][j]);
            if(_KTrue[i][j]!=0.0f)
                err /= _KTrue[i][j];
            if(err>_maxError)
                _maxError = err;
        }
    QVERIFY(_maxError < 1e-4f);
    _maxError = 0.0f;
    for(int i=0; i<4; ++i)
    {
        float err = std::fabs(_f[i]-_fTrue[i]);
        if(_fTrue[i]!=0.0f)
            err /= _fTrue[i];
        if(err>_maxError)
            _maxError = err;
    }
    QVERIFY(_maxError < 1e-4f);
}

void Test_Simulation::applyLocalNeumannConditions()
{
    float _f[4] = {1e6f, 2e6f, 3e6f, 4e6f};
    Simulation::applyLocalNeumannConditions(
                0b00001101,
                1e6f, _f);
    float _fTrue[4] = {1e6f + 1e6f, 2e6f , 3e6f + 1e6f, 4e6f + 1e6f};
    float _maxError = 0.0f;
    for(int i=0; i<4; ++i)
    {
        float err = std::fabs(_f[i]-_fTrue[i]);
        if(_fTrue[i]!=0.0f)
            err /= _fTrue[i];
        if(err>_maxError)
            _maxError = err;
    }
    QVERIFY(_maxError < 1e-4f);
}
