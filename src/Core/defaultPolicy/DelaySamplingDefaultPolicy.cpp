#include <DelaySamplingDefaultPolicy.h>
#include <random>
#include <set>
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
    State delayedState = State(nullptr);
    std::vector<State> validChildStates =
        ((UppaalEnvironmentInterface &)_environment).GetValidChildStatesNoDelay(state);
    bool isTerminal = _environment.IsTerminal(state);
    bool delayFound = false;

    while (states_unrolled < unrolledStatesLimit && (!validChildStates.empty()) && (!isTerminal)) {
        // Part 1: Randomly picking a delayed state
        // std::cout << "Delaying " << states_unrolled << "th delay..." << std::endl;
        std::tie(delayedState, delayFound, isTerminal) =
            findDelayedState(state, (UppaalEnvironmentInterface &)_environment);
        // if no delay was found skip reinitilizing the state into delayed state
        if (delayFound) {
            state = delayedState;
        } else if (isTerminal) {
            state = delayedState;
            break;
        }
        // Part2: Randomly picking next state from children states
        validChildStates = ((UppaalEnvironmentInterface &)_environment).GetValidChildStatesNoDelay(state);
        // uniformly choose one of the found children
        std::uniform_int_distribution<int> uniformIntDistribution2(0, validChildStates.size() - 1);
        i_random = uniformIntDistribution2(generator);
        state = validChildStates[i_random];
        // Fetch info from the new child state
        validChildStates = ((UppaalEnvironmentInterface &)_environment).GetValidChildStatesNoDelay(state);
        isTerminal = (_environment).IsTerminal(state);
        states_unrolled++;
    }

    return (_environment.EvaluateRewardFunction(state));
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
    int p, rndDelay;
    bool validDelayFound = false;
    std::uniform_int_distribution<int> uniformIntDistribution1(1, 10);
    srand(time(NULL));

    std::pair<int, int> delayBounds = _environment.GetDelayBounds(state);
    int lowerDelayBound = (int)(delayBounds).first;
    int upperDelayBound = (int)(delayBounds).second;

    // Search for a delayed state with children states , otherwise return null
    while (!validDelayFound) {
        // Bounds are the same, doesn't matter which we choose
        if (lowerDelayBound == upperDelayBound) {
            if (lowerDelayBound == 0) {
                // std::cout << "No delay for this state (Upper delay bound is 0)" << std::endl;
                return std::make_tuple(State(nullptr), false, false);
            }
            rndDelay = lowerDelayBound;
            // If bounds are different and the lower bound is 0 choose the upper one
        } else {
            p = uniformIntDistribution1(generator);
            // If there is are only bounds to choose between
            if (upperDelayBound - lowerDelayBound - 1 == 0 || p <= 3) {
                if (rand() % 2 == 0) {
                    rndDelay = lowerDelayBound;
                } else {
                    rndDelay = upperDelayBound;
                }
            } else {
                std::uniform_int_distribution<int> uniformIntDistribution2(lowerDelayBound + 1, upperDelayBound - 1);
                rndDelay = uniformIntDistribution2(generator);
            }
        }

        // Fetch the delayed state
        // std::cout << "Delaying state by " << rndDelay << std::endl;
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