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

Tactic* ProactiveAdaptationManager::evaluate() {
  MacroTactic* pMacroTactic = new MacroTactic;
  Model* pModel = getModel();
  const double dimmerStep = 1.0 / (pModel->getNumberOfDimmerLevels() - 1)
  double dimmer = pModel->getDimmerFactor();
  double spareUtilization = pModel->getConfiguraiton().getActiveServers() - pModel->getObservations().utilization;
  bool isServerBooting = pModel->getServers() > pModel->getActiveServers();
  bool isServerRemoving = pModel->getServers() < pModel->getActiveServers();

  
}