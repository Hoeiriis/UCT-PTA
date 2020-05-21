#include <DelaySamplingDefaultPolicy.h>
#include <random>
#include <set>
#include <vector>

DelaySamplingDefaultPolicy::DelaySamplingDefaultPolicy(UppaalEnvironmentInterface &environment, int unrolledStatesLimit)
    : DefaultPolicyBase(environment), unrolledStatesLimit(unrolledStatesLimit){
    exponentialDistribution = std::exponential_distribution<double>(3.5);
};

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
    std::vector<State> validChildStates =
        ((UppaalEnvironmentInterface &)_environment).GetValidChildStatesNoDelay(currentState);
    bool isTerminal = _environment.IsTerminal(currentState);
    bool delayFound = false;

    while (states_unrolled < unrolledStatesLimit && (!isTerminal)) {
        // Part 1: Randomly picking a delayed state
        // std::cout << "Delaying " << states_unrolled << "th delay..." << std::endl;
        std::tie(delayedState, delayFound, isTerminal) =
            findDelayedState(currentState, (UppaalEnvironmentInterface &)_environment);
        // if no delay was found skip reinitilizing the state into delayed state
        if (delayFound) {
            currentState = std::move(delayedState);
        } else if (isTerminal) {
            currentState = std::move(delayedState);
            break;
        }
        // Part2: Randomly picking next state from children states
        validChildStates = ((UppaalEnvironmentInterface &)_environment).GetValidChildStatesNoDelay(currentState);
        if(validChildStates.empty())
        {
            break;
        }
        // uniformly choose one of the found children
        std::uniform_int_distribution<int> uniformIntDistribution2(0, validChildStates.size() - 1);
        i_random = uniformIntDistribution2(generator);
        currentState = std::move(validChildStates[i_random]);
        isTerminal = (_environment).IsTerminal(currentState);
        states_unrolled++;
    }

    int reward = (_environment.EvaluateRewardFunction(currentState));
    return reward;
}

/**
 *tu
 * @param state a state being delayed
 * @param _environment environment with defined delay contex
 * @return tuple <state delayedState,bool isFound,bool isTerminal> contains delayed state, flag if there is one found
 * and a flag that denotes if it is terminal
 */
std::tuple<State, bool, bool> DelaySamplingDefaultPolicy::findDelayedState(State &state,
                                                                           UppaalEnvironmentInterface &_environment) {
    std::set<int> exploredDelays;
    State delayedState = State(nullptr);
    int rndDelay;
    bool validDelayFound = false;

    std::pair<int, int> delayBounds = _environment.GetDelayBounds(state);
    int lowerDelayBound = (int)(delayBounds).first;
    int upperDelayBound = (int)(delayBounds).second;

    // Search for a delayed state with children states , otherwise return null
    while (!validDelayFound) {
        // Bounds are the same, doesn't matter which we choose
        if (lowerDelayBound == upperDelayBound) {
            rndDelay = lowerDelayBound;
        } else {
            // choose delays with exponentially lower chance of larger delays
            double rnd = GetRandomExponential();
            rndDelay = (int)(rnd*upperDelayBound);
        }

        if (rndDelay == 0) {
            // No need to do a delay
            return std::make_tuple(State(nullptr), false, false);
        }

        // Fetch the delayed state
        delayedState = (_environment.DelayState(state, rndDelay)).first;
        // check if delayed state is terminal
        if (_environment.IsTerminal(delayedState)) {
            return std::make_tuple(delayedState, true, true);
        }
        // Check if delayed state does not have children (This should NOT occur)
        if (_environment.GetValidChildStatesNoDelay(delayedState).empty()) {
            std::cout << "Delayed State doesn't have any children that is not delay" << std::endl;
            return std::make_tuple(State(nullptr), false, false);
        }
        validDelayFound = true;
    }
    return std::make_tuple(delayedState, true, false);
}

double DelaySamplingDefaultPolicy::GetRandomExponential() {
    double rnd = 0;

    do
        { rnd = exponentialDistribution(generator); }
    while (rnd > 1);

    return rnd;
}
