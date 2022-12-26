#include "ProactiveAdaptationManager.h"
#include "managers/adaptation/UtilityScorer.h"
#include "managers/execution/AllTactics.h"
#include <string>
#include <Python.h>

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

Tactic *ProactiveAdaptationManager::evaluate() {
    MacroTactic *pMacroTactic = new MacroTactic;
    Model *pModel = getModel();
    const double dimmerStep = 1.0 / (pModel->getNumberOfDimmerLevels() - 1)
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
    PyObject *folder_path = PyUnicode_FromString(".");
    PyList_Append(sys_path, folder_path);

    // Python file name
    PyObject *pModule = PyImport_ImportModule("data_exploration");
    if (pModule == NULL) {
        PyErr_Print();
        std::cerr << "PyImport_ImportModule call failed\n";
        exit(EXIT_FAILURE);
    }


    // This is reactive
    if (spareUtilization > SU_THRESHOLD_UPPER) {
        if (THRESHOLD_VIOLATION_UPPER >= 1) {
            pMacroTactic->addTactic(addServer(isServerBooting, dimmer, dimmerStep, activeServers, maxServers));
            THRESHOLD_VIOLATION_UPPER = 0;
            THRESHOLD_VIOLATION_LOWER = 0;
        } else {
            THRESHOLD_VIOLATION_UPPER++;
        }
    } else if (spareUtilization < SU_THRESHOLD_LOWER) {
        if (THRESHOLD_VIOLATION_LOWER >= 1) {
            pMacroTactic->addTactic(removeServer(isServerBooting, dimmer, dimmerStep, activeServers, spareUtilization));
            THRESHOLD_VIOLATION_LOWER = 0;
            THRESHOLD_VIOLATION_UPPER = 0;
        } else {
            THRESHOLD_VIOLATION_LOWER++;
        }
    }

    // This is proactive
    // We should be getting the predictedUtilisation from python here
    double predictedUtilisation = predictFutureUtilization(pModel->getServiceTimeHistory(),
                                                           pModel->getEnvironment().arrivalRateHistory)

        // If timeUntilNeed != -1 and the check below is true then add server
        if (pModel->getSimTime() - timeUntilNeed <=
            min(pModel->getBootDelay(), pModel->getEvaluationPeriod())) {
            pMacroTactic->addTactic(addServer(isServerBooting, dimmer, dimmerStep, activeServers, maxServers));
        }

    return pMacroTactic;
}

// FIXME Work In Progress
void predictFutureUtilization(vector<double> historyOfServiceTime, vector<double> historyOfRequestRate) {
    Py_Initialize();

    // which python function to call
    PyObject *pFunc = PyObject_GetAttrString(pModule, "offline_predictions");
    if (pFunc && PyCallable_Check(pFunc)) {
        PyObject *pArgs = PyTuple_New(5);

        PyObject *arima_p = PyFloat_FromDouble(ARIMA_P);
        PyObject *arima_d = PyFloat_FromDouble(ARIMA_D);
        PyObject *arima_q = PyFloat_FromDouble(ARIMA_Q);
        PyObject *pList1 = PyList_New(historyOfServiceTime.size());
        PyObject *pList2 = PyList_New(historyOfRequestRate.size());
        PyOjbect *pNum_pred = PyLong_FromLong(10);

        for (size_t i = 0; i < historyOfServiceTime.size(); ++i)
            PyList_SetItem(pList1, i, PyFloat_FromDouble(historyOfServiceTime[i]));
        for (size_t i = 0; i < historyOfRequestRate.size(); ++i)
            PyList_SetItem(pList2, i, PyFloat_FromDouble(historyOfRequestRate[i]));

        PyTuple_SetItem(pArgs, 0, arima_p);
        PyTuple_SetItem(pArgs, 1, arima_d);
        PyTuple_SetItem(pArgs, 2, arima_q);
        PyTuple_SetItem(pArgs, 3, pList1);
        PyTuple_SetItem(pArgs, 4, pNum_pred);

        PyObject *ret = PyObject_CallObject(pFunc, pArgs);

        if (ret != NULL) {
            double pred = PyFloat_AsDouble(ret); // list of doubles
            // std::cout << "Got " << pred << " back\n";
            
            // Iterate over each value in the list
            // if (predictedUtilisation > SU_THRESHOLD_UPPER) { // current simTime - the future simtime for our forecast (multiple of 60sec(sim time cycle))
            // set timeUntilNeed + 1 to the position/index of the value in the list
            return pred;
        } else {
            PyErr_Print();
            fprintf(stderr, "Function return failed\n");
        }

        // TODO now call with pList2

        Py_DECREF(arima_p);
        Py_DECREF(arima_d);
        Py_DECREF(arima_q);
        Py_DECREF(pNum_pred);
        Py_DECREF(ret);
        Py_DECREF(pList1);
        Py_DECREF(pList2);
        Py_DECREF(pArgs)
    } else {
        PyErr_Print();
        fprintf(stderr, "Function call failed\n");
    }

    // TODO DECREF?
    Py_Finalize();
    
}

Tactic addServer(bool isServerBooting, double dimmer, double dimmerStep, int activeServers, const int maxServers) {
    // reset timeUntilNeed to -1
    if (!isServerBooting && activeServers < maxServers) { // add server
        return AddServerTactic;
    } else if (dimmer > 0.0) { // decrease dimmer
        dimmer = max(0.0, dimmer - dimmerStep);
        return SetDimmerTactic(dimmer);
    }
}

Tactic removeServer(bool isServerBooting, double dimmer, double dimmerStep, int activeServers, double spareUilization) {
    if (spareUilization > 1) {
        // reset timeUntilNeed to -1
        if (dimmer < 1) {
            // increase dimmer;
            dimmer = min(1.0, dimmer + dimmerStep);
            SetDimmerTactic(dimmer);
        } else if (!isServerBooting && numberOfServers > 1) {
            // remove server
            return RemoveServerTactic;
        }
    }
}