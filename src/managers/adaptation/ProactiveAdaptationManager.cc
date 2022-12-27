#include "ProactiveAdaptationManager.h"
#include "managers/adaptation/UtilityScorer.h"
#include "managers/execution/AllTactics.h"
#include <string>
#include <python3.10/Python.h>

using namespace std;

Define_Module(ProactiveAdaptationManager);

/**
 * Proactive adaptation
 *
 * RT = response time
 * RTT = response time threshold
 * ST = service time
 * SU = system utilisation
 * SUT = system utilisation threshold
 *
 * - if RT > RTT, add a server if possible, if not decrease dimmer if possible
 * - if RT < RTT and spare utilization > 1
 *      -if dimmer < 1, increase dimmer else if servers > 1 and no server booting remove server
 */

/*
(\_/)
(=.=)
(")(")
*/
Tactic *ProactiveAdaptationManager::evaluate()
{
    MacroTactic *pMacroTactic = new MacroTactic;
    Model *pModel = getModel();
    const double dimmerStep = 1.0 / (pModel->getNumberOfDimmerLevels() - 1);
    double dimmer = pModel->getDimmerFactor();
    double dimmer = pModel->getDimmerFactor();
    int activeServers = pModel->getActiveServers();
    int maxServers = getMaxServers->getMaxServers();
    bool isServerBooting = pModel->getServers() > pModel->getActiveServers();
    bool isServerRemoving = pModel->getServers() < pModel->getActiveServers();
    double spareUtilization = pModel->getConfiguraiton().getActiveServers() - pModel->getObservations().utilization;

    // In order to find Python file
    PyObject *sys = PyImport_ImportModule("sys");
    PyObject *sys_path = PyObject_GetAttrString(sys, "path");
    PyObject *folder_path = PyUnicode_FromString("../../../predictions"); // TODO check
    PyList_Append(sys_path, folder_path);

    // Python file name
    PyObject *pModule = PyImport_ImportModule("data_exploration");
    if (pModule == NULL)
    {
        PyErr_Print();
        std::cerr << "PyImport_ImportModule call failed\n";
        exit(EXIT_FAILURE);
    }
    // Reactive server spawn time set if needed
    pModel->setTimeUntilServerIsNeeded(0);

    // This is reactive
    if (spareUtilization > pModel.SU_THRESHOLD_UPPER)
    {
        if (pModel.THRESHOLD_VIOLATION_UPPER >= 1)
        {
            pMacroTactic->addTactic(addServer(isServerBooting, dimmer, dimmerStep, activeServers, maxServers));
            pModel.THRESHOLD_VIOLATION_UPPER = 0;
            pModel.THRESHOLD_VIOLATION_LOWER = 0;
        }
        else
        {
            pModel.THRESHOLD_VIOLATION_UPPER++;
        }
    }
    else if (spareUtilization < SU_THRESHOLD_LOWER)
    {
        if (pModel.THRESHOLD_VIOLATION_LOWER >= 1)
        {
            pMacroTactic->addTactic(removeServer(isServerBooting, dimmer, dimmerStep, activeServers, spareUtilization));
            pModel.THRESHOLD_VIOLATION_LOWER = 0;
            pModel.THRESHOLD_VIOLATION_UPPER = 0;
        }
        else
        {
            pModel.THRESHOLD_VIOLATION_LOWER++;
        }
    }

    // This is proactive
    // We should be getting the predictedUtilisation from python here
    predictFutureUtilization(pModel->getServiceTimeHistory(), pModel->getEnvironment().arrivalRateHistory, pModel, pMacroTactic);

    return pMacroTactic;
}

void predictFutureUtilization(vector<double> historyOfServiceTime, vector<double> historyOfRequestRate, Model *pModel, MacrpTactic *pMacroTactic)
    Tactic addServer(bool isServerBooting, double dimmer, double dimmerStep, int activeServers, const int maxServers)
        Tactic removeServer(bool isServerBooting, double dimmer, double dimmerStep, int activeServers, double spareUilization)

    // FIXME Work In Progress
    void predictFutureUtilization(vector<double> historyOfServiceTime, vector<double> historyOfRequestRate, Model *pModel, MacrpTactic *pMacroTactic)
{
    std::vector<double> avgServiceTimeVals;
    double serviceTimeSum = 0;
    std::vector<double> avgRequestRateVals;
    double requestRateSum = 0;

    for (size_t i = 0; i < historyOfServiceTime.size(); ++i)
    {
        if (i % 60 == 0)
        {
            avgServiceTimeVals.push_back(serviceTimeSum / 60);
            serviceTimeSum = 0;
        }
        else
        {
            serviceTimeSum = serviceTimeSum + historyOfServiceTime[i];
        }
    }
    for (size_t i = 0; i < historyOfRequestRate.size(); ++i)
    {
        if (i % 60 == 0)
        {
            avgRequestRateVals.push_back(requestRateSum / 60);
            requestRateSum = 0;
        }
        else
        {
            requestRateSum = requestRateSum + historyOfRequestRate[i];
        }
    }

    Py_Initialize();

    PyObject *pFunc = PyObject_GetAttrString(pModule, "offline_predictions");
    if (pFunc && PyCallable_Check(pFunc))
    {
        PyObject *pArgs = PyTuple_New(5);

        PyObject *arima_p = PyLong_FromLong(ARIMA_P);
        PyObject *arima_d = PyLong_FromLong(ARIMA_D);
        PyObject *arima_q = PyLong_FromLong(ARIMA_Q);
        PyObject *pList1 = PyList_New(avgServiceTimeVals.size());
        PyObject *pList2 = PyList_New(avgRequestRateVals.size());
        PyOjbect *pNum_pred = PyLong_FromLong(10);

        for (size_t i = 0; i < avgServiceTimeVals.size(); ++i)
            PyList_SetItem(pList1, i, PyFloat_FromDouble(avgServiceTimeVals[i]));
        for (size_t i = 0; i < avgRequestRateVals.size(); ++i)
            PyList_SetItem(pList2, i, PyFloat_FromDouble(avgRequestRateVals[i]));

        PyTuple_SetItem(pArgs, 0, arima_p);
        PyTuple_SetItem(pArgs, 1, arima_d);
        PyTuple_SetItem(pArgs, 2, arima_q);
        PyTuple_SetItem(pArgs, 3, pList1);
        PyTuple_SetItem(pArgs, 4, pNum_pred);

        PyObject *predServiceTime = PyObject_CallObject(pFunc, pArgs);

        PyTuple_SetItem(pArgs, 3, pList2);
        PyObject *predRequestRate = PyObject_CallObject(pFunc, pArgs);

        if (predServiceTime != NULL && predRequestRate != NULL)
        {
            std::vector<double> predictedServiceTimeVals;
            std::vector<double> predictedRequestRateVals;

            if (PyList_Check(predServiceTime))
            {
                for (Py_ssize_t i = 0; i < PyList_Size(predServiceTime); ++i)
                {
                    PyObject *next = PyList_GetItem(predServiceTime, i);
                    predictedServiceTimeVals.push_back(PyFloat_AsDouble(next));
                }
            }
            else
            {
                std::cerr << "return value is not a list\n";
                return;
            }

            if (PyList_Check(predRequestRate))
            {
                for (Py_ssize_t i = 0; i < PyList_Size(predRequestRate); ++i)
                {
                    PyObject *next = PyList_GetItem(predRequestRate, i);
                    predictedRequestRateVals.push_back(PyFloat_AsDouble(next));
                }
            }
            else
            {
                std::cerr << "return value is not a list\n";
                return;
            }

            for (size_t i = 0; i < predictedServiceTimeVals.size(); ++i)
            {
                double predictedUtilisation = predictedServiceTimeVals[i] * predictedRequestRateVals[i];
                if (predictedUtilisation > SU_THRESHOLD_UPPER)
                {
                    pModel->setTimeUntilServerIsNeeded(i * 60 != 0 ? i * 60 : 60 - pModel->getBootDelay());
                    if (pModel->getTimeUntilServerIsNeeded() <= min(pModel->getBootDelay(), pModel->getEvaluationPeriod()))
                    {
                        pMacroTactic->addTactic(addServer(isServerBooting, dimmer, dimmerStep, activeServers, maxServers));
                    }
                    break;
                }
            }
        }
        else
        {
            PyErr_Print();
            fprintf(stderr, "Function return failed\n");
        }

        // Py_DECREF(arima_p);
        // Py_DECREF(arima_d);
        // Py_DECREF(arima_q);
        // Py_DECREF(pNum_pred);
        // Py_DECREF(pList1);
        // Py_DECREF(pList2);
        // Py_DECREF(pArgs)
    }
    else
    {
        PyErr_Print();
        fprintf(stderr, "Function call failed\n");
    }
    Py_Finalize();
}

Tactic addServer(bool isServerBooting, double dimmer, double dimmerStep, int activeServers, const int maxServers)
{
    // reset timeUntilNeed to -1
    if (!isServerBooting && activeServers < maxServers)
    { // add server

        return AddServerTactic;
    }
    else if (dimmer > 0.0)
    { // decrease dimmer
        dimmer = max(0.0, dimmer - dimmerStep);
        return SetDimmerTactic(dimmer);
    }
}

Tactic removeServer(bool isServerBooting, double dimmer, double dimmerStep, int activeServers, double spareUilization)
{
    if (spareUilization > 1)
    {
        // reset timeUntilNeed to -1
        if (dimmer < 1)
        {
            // increase dimmer;
            dimmer = min(1.0, dimmer + dimmerStep);
            SetDimmerTactic(dimmer);
        }
        else if (!isServerBooting && numberOfServers > 1)
        {
            // remove server
            return RemoveServerTactic;
        }
    }
}