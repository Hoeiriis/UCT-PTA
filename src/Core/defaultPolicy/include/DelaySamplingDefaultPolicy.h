#ifndef UCTPTA_LIBRARY_DELAYSAMPLINGDEFAULTPOLICY_H
#define UCTPTA_LIBRARY_DELAYSAMPLINGDEFAULTPOLICY_H

#include <DefaultPolicyBase.h>
#include <State.h>
#include <UppaalEnvironmentInterface.h>

class DelaySamplingDefaultPolicy : DefaultPolicyBase {
  public:
    explicit DelaySamplingDefaultPolicy(UppaalEnvironmentInterface &environment);

    Reward defaultPolicy(State) override;
    std::vector<State> findValidDelayedState(State &state, UppaalEnvironmentInterface &_environment);

  private:
    std::vector<State> validStates;
};

#endif // UCTPTA_LIBRARY_DELAYSAMPLINGDEFAULTPOLICY_H