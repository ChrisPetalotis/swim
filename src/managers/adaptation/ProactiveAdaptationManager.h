#ifndef __PLASASIM_PROACTIVEADAPTATIONMANAGER_H_
#define __PLASASIM_PRO ACTIVEADAPTATIONMANAGER_H_

#include "BaseAdaptationManager.h"

class ProactiveAdaptationManager : public BaseAdaptationManager
{
  protected:
    virtual Tactic* evaluate();
    double predictFutureUtilization(vector<double> historyOfServiceTime, vector<double> historyOfRequestRate);
    Tactic* addServer(bool isServerBooting, double dimmer, double dimmerStep, int activeServers, const int maxServers);
    Tactic* removeServer(bool isServerBooting, double dimmer, double dimmerStep, int activeServers, double spareUilization);

    const int THRESHOLD_VIOLATION_UPPER = 0;
    const int THRESHOLD_VIOLATION_LOWER = 0;
    const int TIME_UNTIL_NEED = -1;
};

#endif