#include "ProactiveAdaptationManager.h"
#include "managers/adaptation/UtilityScorer.h"
#include "managers/execution/AllTactics.h"
#include <string>

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

Tactic *ProactiveAdaptationManager::evaluate()
{
  MacroTactic *pMacroTactic = new MacroTactic;
  Model *pModel = getModel();
  const double dimmerStep = 1.0 / (pModel->getNumberOfDimmerLevels() - 1) double dimmer = pModel->getDimmerFactor();
  double dimmer = pModel->getDimmerFactor();
  int activeServers = pModel->getActiveServers();
  int maxServers = getMaxServers->getMaxServers();
  bool isServerBooting = pModel->getServers() > pModel->getActiveServers();
  bool isServerRemoving = pModel->getServers() < pModel->getActiveServers();
  double spareUtilization = pModel->getConfiguraiton().getActiveServers() - pModel->getObservations().utilization;

  // This is reactive
  if (spareUtilization > SU_THRESHOLD_UPPER)
  {
    if (THRESHOLD_VIOLATION_UPPER >= 1)
    {
      pMacroTactic->addTactic(addServer(isServerBooting, dimmer, dimmerStep, activeServers, maxServers));
      THRESHOLD_VIOLATION_UPPER = 0;
      THRESHOLD_VIOLATION_LOWER = 0;
    }
    else
    {
      THRESHOLD_VIOLATION_UPPER++;
    }
  }
  else if (spareUtilization < SU_THRESHOLD_LOWER)
  {
    if (THRESHOLD_VIOLATION_LOWER >= 1)
    {
      pMacroTactic->addTactic(removeServer(isServerBooting, dimmer, dimmerStep, activeServers, spareUtilization));
      THRESHOLD_VIOLATION_LOWER = 0;
      THRESHOLD_VIOLATION_UPPER = 0;
    }
    else
    {
      THRESHOLD_VIOLATION_LOWER++;
    }
  }

  // This is proactive
  // We should be getting the predictedUtilisation from python here
  double predictedUtilisation = predictFutureUtilization(pModel->getServiceTimeHistory(), pModel->getEnvironment().arrivalRateHistory)

      if (predictedUtilisation > SU_THRESHOLD_UPPER)
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
  // TODO Call Python function

  // Set variable TIME_UNTIL_NEED to the number of evaluation periods in the future in which the server is going to be needed.
  // Then in every cycle decrease the variable by one and when the value is equal to the time the server needs to fully
  // spawn / 60, then add a tactic to spawn a new server.
  // Start checking in the current cycle
}

Tactic addServer(bool isServerBooting, double dimmer, double dimmerStep, int activeServers, const int maxServers)
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