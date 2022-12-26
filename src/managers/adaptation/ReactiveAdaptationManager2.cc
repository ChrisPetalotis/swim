/*******************************************************************************
 * Simulator of Web Infrastructure and Management
 * Copyright (c) 2016 Carnegie Mellon University.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS," WITH NO WARRANTIES WHATSOEVER. CARNEGIE
 * MELLON UNIVERSITY EXPRESSLY DISCLAIMS TO THE FULLEST EXTENT PERMITTED BY LAW
 * ALL EXPRESS, IMPLIED, AND STATUTORY WARRANTIES, INCLUDING, WITHOUT
 * LIMITATION, THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, AND NON-INFRINGEMENT OF PROPRIETARY RIGHTS.
 *
 * Released under a BSD license, please see license.txt for full terms.
 * DM-0003883
 *******************************************************************************/

#include "ReactiveAdaptationManager2.h"
#include "managers/adaptation/UtilityScorer.h"
#include "managers/execution/AllTactics.h"

using namespace std;

Define_Module(ReactiveAdaptationManager2);

/**
 * Reactive adaptation
 *
 * RT = response time
 * RTT = response time threshold
 *
 * - if RT > RTT, add a server if possible, if not decrease dimmer if possible
 * - if RT < RTT
 *      -if dimmer < 1, increase dimmer else if servers > 1 and no server booting remove server
 */

/**
 * Reactive adaptation
 *
 * RT = response time
 * RTT = response time threshold
 *
 * - if RT > RTT, add a server if possible, if not decrease dimmer if possible
 * - if RT < RTT and spare utilization > 1
 *      -if dimmer < 1, increase dimmer else if servers > 1 and no server booting remove server
 */
Tactic *ReactiveAdaptationManager2::evaluate()
{
    MacroTactic *pMacroTactic = new MacroTactic;
    Model *pModel = getModel();
    const double dimmerStep = 1.0 / (pModel->getNumberOfDimmerLevels() - 1);
    double dimmer = pModel->getDimmerFactor();
    int activeServers = pModel->getActiveServers();
    int maxServers = getMaxServers->getMaxServers();
    bool isServerBooting = pModel->getServers() > activeServers;
    int thresholdViolationUpper = 0;
    int thresholdViolationLower = 0;
    double utilisationLowerThreshold = 0.3;
    double utilisationUpperThreshold = 0.6;
    double spareUtilization = pModel->getConfiguration().getActiveServers() - pModel->getObservations().utilization;

    // This is reactive
    if (spareUtilization > utilisationUpperThreshold)
    {
        if (thresholdViolationUpper >= 1)
        {
            pMacroTactic->addTactic(addServer(isServerBooting, dimmer, dimmerStep, activeServers, maxServers));
            thresholdViolationUpper = 0;
            thresholdViolationLower = 0;
        }
        else
        {
            thresholdViolationUpper++;
        }
    }
    else if (spareUtilization < utilisationLowerThreshold)
    {
        if (thresholdViolationLower >= 1)
        {
            pMacroTactic->addTactic(removeServer(isServerBooting, dimmer, dimmerStep, activeServers, spareUtilization));
            thresholdViolationLower = 0;
            thresholdViolationUpper = 0;
        }
        else
        {
            thresholdViolationLower++;
        }
    }

    // This is proactive
    // We should be getting the predictedUtilisation from python here
    double predictedUtilisation = predictFutureUtilization(pModel->getServiceTimeHistory(), pModel->getEnvironment().arrivalRateHistory)

        if (predictedUtilisation > utilisationUpperThreshold)
    { // current simTime - the future simtime for our forecast (multiple of 60sec(sim time cycle))
        if (pModel->getSimTime() - timeUntilNeed <=
            min(pModel->getBootDelay(), pModel->getEvaluationPeriod()))
        {
            pMacroTactic->addTactic(addServer(isServerBooting, dimmer, dimmerStep, activeServers, maxServers));
        }
    }

    return pMacroTactic;
}

double predictFutureUtilization(vector<double> historyOfServiceTime, vector<double> historyOfRequestRate)
{
}

Tactic addServer(bool isServerBooting, double dimmer, double dimmerStep, int activeServers, int maxServers)
{
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