#ifndef __PLASASIM_PROACTIVEADAPTATIONMANAGER_H_
#define __PLASASIM_PRO ACTIVEADAPTATIONMANAGER_H_
#include <vector>

#include "BaseAdaptationManager.h"

// TODO load from ini file
#define ARIMA_P 0
#define ARIMA_D 0
#define ARIMA_Q 0

class ProactiveAdaptationManager : public BaseAdaptationManager
{
protected:
  virtual Tactic *evaluate();
  double predictFutureUtilization(std::vector<double> historyOfServiceTime, std::vector<double> historyOfRequestRate);
  Tactic *addServer(bool isServerBooting, double dimmer, double dimmerStep, int activeServers, const int maxServers);
  Tactic *removeServer(bool isServerBooting, double dimmer, double dimmerStep, int activeServers, double spareUilization);
};

#endif