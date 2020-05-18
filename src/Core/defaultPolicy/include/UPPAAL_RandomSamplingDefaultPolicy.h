#ifndef MCTS_LIBRARY_UPPAAL_RANDOMDEFAULTPOLICY_H
#define MCTS_LIBRARY_UPPAAL_RANDOMDEFAULTPOLICY_H

#include <DefaultPolicyBase.h>
#include <UppaalEnvironmentInterface.h>

class UPPAAL_RandomSamplingDefaultPolicy : DefaultPolicyBase {
  public:
    explicit UPPAAL_RandomSamplingDefaultPolicy(UppaalEnvironmentInterface &environment, int unrolledStatesLimit);

    Reward defaultPolicy(State) override;

  private:
    std::vector<State> validStates;
    int unrolledStatesLimit;
};

#endif // MCTS_LIBRARY_UPPAAL_RANDOMDEFAULTPOLICY_H