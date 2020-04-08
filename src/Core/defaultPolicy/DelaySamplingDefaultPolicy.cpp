#include <DelaySamplingDefaultPolicy.h>
#include <random>
#include <set>
#include <vector>

DelaySamplingDefaultPolicy::DelaySamplingDefaultPolicy(UppaalEnvironmentInterface &environment)
    : DefaultPolicyBase(environment){};

/**
 * Performs a PTA simulation by involving a delay of a state before it randomly samples for its valid child states.
 * Delay is chosen randomly between lower and upper bound.
 *
 *
 * @param state a state being explored
 * @return Reward that is a result after 50 steps of a performed simulation
 */
Reward DelaySamplingDefaultPolicy::defaultPolicy(State state) {
    int states_unrolled = 0;
    int i_random;
    std::vector<State> validChildStates = ((UppaalEnvironmentInterface &)_environment).GetValidChildStates(state);
    bool isTerminal = _environment.IsTerminal(state);

    while (states_unrolled < 50 && (!validChildStates.empty()) && (!isTerminal)) {
        validChildStates = findValidDelayedState(state, (UppaalEnvironmentInterface &)_environment);

        std::uniform_int_distribution<int> uniformIntDistribution(0, validChildStates.size() - 1);
        i_random = uniformIntDistribution(generator);
        state = validChildStates[i_random];

        // Fetch info from the new child state
        // v_1 child states without delay included
        validChildStates = ((UppaalEnvironmentInterface &)_environment).GetValidChildStatesNoDelay(state);
        // v_2 child states with delays included
        // validChildStates = ((UppaalEnvironmentInterface&)_environment).GetValidChildStates(state);
        isTerminal = (_environment).IsTerminal(state);
        states_unrolled++;
    }

    return (_environment.EvaluateRewardFunction(state));
}

/**
 * Performs delay sampling between possible delay bounds in a given state.
 * Sampling is being performed until a valid delay is found. A valid delay is one that results with a state with valid
 * Child states
 *
 * @param state a state being delayed
 * @param _environment environment with defined delay contex
 * @return Vector containing valid child states of a delayed state.
 */
std::vector<State> DelaySamplingDefaultPolicy::findValidDelayedState(State &state,
                                                                     UppaalEnvironmentInterface &_environment) {
    std::tuple<int, int> delayBounds;
    int lowerDelayBound, upperDelayBound;
<<<<<<< HEAD
    srand(time(NULL));
    int p, rndDelay;
=======
    int rndDelay;
>>>>>>> 6e72b0e0aad9c80d9b8e276d8593938aa5bfe56a
    // TODO check if the initialization is ok
    State delayedState = nullptr;
    std::set<int> nonValidDelays;
    std::vector<State> validDelayedChildStates;
    bool validDelayFound = false;

    delayBounds = _environment.GetDelayBounds(state);
    lowerDelayBound = std::get<0>(delayBounds);
    upperDelayBound = std::get<1>(delayBounds);

    while (!validDelayFound) {
<<<<<<< HEAD
        std::uniform_int_distribution<int> uniformIntDistribution(1, 10);
        p = uniformIntDistribution(generator);
        if (p <= 3) {
            if (rand() % 2 == 0) {
                rndDelay = lowerDelayBound;
            } else {
                rndDelay = upperDelayBound;
            }
        } else {
            std::uniform_int_distribution<int> uniformIntDistribution(lowerDelayBound + 1, upperDelayBound - 1);
            rndDelay = uniformIntDistribution(generator);
        }

=======
        // TODO take into consideration "Andrej's probabilities" (budget of 30% on min and max delay,sth like that)
        std::uniform_int_distribution<int> uniformIntDistribution(lowerDelayBound, upperDelayBound);
        rndDelay = uniformIntDistribution(generator);
>>>>>>> 6e72b0e0aad9c80d9b8e276d8593938aa5bfe56a
        // skip chosen delay if it has been explored before and resulted in no ValidChildStates
        if (nonValidDelays.count(rndDelay) != 0) {
            continue;
        }
        delayedState = std::get<0>(((UppaalEnvironmentInterface &)_environment).DelayState(state, rndDelay));
        validDelayedChildStates = _environment.GetValidChildStates(delayedState);
        // if the current delayed states has no valid child states, sample another delay
        if (validDelayedChildStates.empty()) {
            nonValidDelays.insert(rndDelay);
            continue;
        }
        validDelayFound = true;
    }
    return validDelayedChildStates;
}