#ifndef __PLASASIM_PROACTIVEADAPTATIONMANAGER_H_
#define __PLASASIM_PRO ACTIVEADAPTATIONMANAGER_H_

#include "BaseAdaptationManager.h"

class ProactiveAdaptationManager : public BaseAdaptationManager
{
  protected:
    virtual Tactic* evaluate();
  
  public:
    static int thresholdViolationUpper = 0
    static int thresholdViolationLower = 0

    void AddServer(const bool isServerBooting, const int numberOfServers, const int maxServers)
    void RemoveServer(const bool isServerBooting, const double dimmer, const double spareUtilisation, const int numberOfServers)
};

#endif