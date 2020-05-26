#include <DelaySamplingDefaultPolicy.h>
#include <random>
#include <vector>

DelaySamplingDefaultPolicy::DelaySamplingDefaultPolicy(UppaalEnvironmentInterface &environment, int unrolledStatesLimit)
    : DefaultPolicyBase(environment), unrolledStatesLimit(unrolledStatesLimit){};

/**
 * Performs a PTA simulation by involving a (randomly chosen) delay of a state before it samples for its valid child
 * states. Delay is chosen randomly between lower and upper bound.
 *
 *
 * @param state a state being explored
 * @return Reward that is a result after 50 steps of a performed simulation
 */
Reward DelaySamplingDefaultPolicy::defaultPolicy(State state) {
    int states_unrolled = 0;
    int i_random;
    State currentState = std::move(state);
    State delayedState = State(nullptr);
    std::vector<State> validChildStates = {};
    bool isTerminal = _environment.IsTerminal(currentState);

    while (states_unrolled < unrolledStatesLimit && (!isTerminal)) {
        // Part 1:  Check if the current state have children
        validChildStates = ((UppaalEnvironmentInterface &)_environment).GetValidChildStatesNoDelay(currentState);

        // If not, delay
        if (validChildStates.empty()) {
            std::pair<int, int> bounds = ((UppaalEnvironmentInterface &)_environment).GetDelayBounds(currentState);

            if(bounds.first == 0) break;

            currentState = std::move(((UppaalEnvironmentInterface &)_environment).DelayState(currentState, bounds.first).first);
            continue;
        }

        // Randomly picking next state from children states
        std::uniform_int_distribution<int> uniformIntDistribution2(0, validChildStates.size() - 1);
        i_random = uniformIntDistribution2(generator);
        currentState = std::move(validChildStates[i_random]);
        isTerminal = (_environment).IsTerminal(currentState);

        states_unrolled++;
    }

    int reward = (_environment.EvaluateRewardFunction(currentState));
    return reward;
}