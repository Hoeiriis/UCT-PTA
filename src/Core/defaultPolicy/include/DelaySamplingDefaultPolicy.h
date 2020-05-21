#ifndef UCTPTA_LIBRARY_DELAYSAMPLINGDEFAULTPOLICY_H
#define UCTPTA_LIBRARY_DELAYSAMPLINGDEFAULTPOLICY_H

#include <DefaultPolicyBase.h>
#include <State.h>
#include <UppaalEnvironmentInterface.h>

class DelaySamplingDefaultPolicy : DefaultPolicyBase {
  public:
    explicit DelaySamplingDefaultPolicy(UppaalEnvironmentInterface &environment, int unrolledStatesLimit);

    Reward defaultPolicy(State) override;
    std::tuple<State, bool, bool> findDelayedState(State &state, UppaalEnvironmentInterface &_environment);

  private:
    std::vector<State> validStates;
    int unrolledStatesLimit;
    std::exponential_distribution<double> exponentialDistribution;
    double GetRandomExponential();
};

#endif // UCTPTA_LIBRARY_DELAYSAMPLINGDEFAULTPOLICY_H