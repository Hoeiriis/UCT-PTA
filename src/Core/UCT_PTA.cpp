//
// Created by happysun on 27/12/2019.
//

#include <cfloat>
#include <cassert>
#include <cmath>
#include <tuple>
#include <utility>
#include <UCT_PTA.h>
#include <ExtendedSearchNode.h>

UCT_PTA::UCT_PTA(UppaalEnvironmentInterface &environment)
: _environment(environment), generator(std::mt19937(time(nullptr))), _defaultPolicy(UPPAAL_RandomSamplingDefaultPolicy(_environment)), root_node(ExtendedSearchNode::create_ExtendedSearchNode(nullptr, false, true)) {
    /** UCT TreePolicy setup
    std::function<std::shared_ptr<ExtendedSearchNode>(std::shared_ptr<ExtendedSearchNode>)> f_expand =
            std::bind(&UCT_PTA::m_expand, this, std::placeholders::_1);

    std::function<std::shared_ptr<ExtendedSearchNode>(std::shared_ptr<ExtendedSearchNode>, double)> f_best_child =
            std::bind(&UCT_PTA::m_best_child, this, std::placeholders::_1, std::placeholders::_2); **/
}

State UCT_PTA::run(int n_searches) {

    time_t max_start = time(nullptr);
    long max_time = n_searches;
    long max_timeLeft = max_time;

    State initial_state = _environment.GetStartState();
    root_node = ExtendedSearchNode::create_ExtendedSearchNode(nullptr, initial_state, false, true);
    root_node->bounds = _environment.GetDelayBounds(initial_state);

    bootstrap_reward_scaling();

    while(!best_proved && max_timeLeft > 0) {
        // TreePolicy runs to find an unexpanded node to expand
        std::shared_ptr<ExtendedSearchNode> expandedNode = m_tree_policy(root_node);
        // From the expanded node, a simulation runs that returns a score
        Reward simulation_score = m_default_policy(expandedNode->state);

        // eventually update min max reward
        if(simulation_score < rewardMinMax.first){
            rewardMinMax.first = simulation_score;
        } else if(simulation_score > rewardMinMax.second){
            rewardMinMax.second = simulation_score;
        }

        // normalize data
        double norm_score = (simulation_score-rewardMinMax.first)/(rewardMinMax.second - rewardMinMax.first);
        // The score is backpropagated up through the search tree
        m_backpropagation(expandedNode, norm_score);

        // if new terminal node is encountered, update the bestTerminalNode
        if (expandedNode->isTerminalState){
            Reward termReward = _environment.EvaluateRewardFunction(expandedNode->state);
            if (bestTerminalNodesFound.empty() || bestTerminalNodesFound.back().score < termReward){
                auto newBestNode = TerminalNodeScore();
                newBestNode.score = termReward;
                newBestNode.node = expandedNode;
                newBestNode.time_to_find = (time(nullptr) - max_start);
                // insert at beginning
                bestTerminalNodesFound.push_back(newBestNode);
            }
            expandedNode->score = {-100000, 0};
        }

        // update maxTime
        max_timeLeft = max_time - (time(nullptr) - max_start);
    }

    if (bestTerminalNodesFound.empty()){
        return nullptr;
    } else {
        return bestTerminalNodesFound.at(0).node->state;
    }
}

void UCT_PTA::bootstrap_reward_scaling(){
    // rough bootstrap of reward scaling
    std::vector<double> rewards(100, 0);
    for (int i = 0; i < 100; ++i) {
        Reward score = m_default_policy(root_node->state);
        rewards.at(i) = score;
    }

    auto it = std::minmax_element(std::begin(rewards), std::end(rewards));
    rewardMinMax.first = *it.first;
    rewardMinMax.second = *it.second;
}

std::shared_ptr<ExtendedSearchNode> UCT_PTA::m_best_child(const std::shared_ptr<ExtendedSearchNode>& node, double c) {
    auto baseNode = std::static_pointer_cast<SearchNode>(node);
    auto outChild = m_best_child(baseNode.get(), c);
    return std::static_pointer_cast<ExtendedSearchNode>(outChild);
}

std::shared_ptr<SearchNode> UCT_PTA::m_best_child(const SearchNode* node, double c)
{

    auto best_score_so_far = std::numeric_limits<double>::lowest();
    std::vector<double> score_list = {};

    for (int i = 0; i < node->child_nodes.size(); i++) {
        auto child = node->child_nodes.at(i);
        double score = (child->score.at(0) / (child->visits + DBL_MIN)) +
                       c * std::sqrt((std::log(node->visits) / (child->visits + DBL_MIN)));

        score_list.push_back(score);

        if (score > best_score_so_far) {
            best_score_so_far = score;
        }
    }

    std::vector<double> bestChildren{};
    for (int i = 0; i < node->child_nodes.size(); i++) {
        if (score_list.at(i) == best_score_so_far) {
            bestChildren.push_back(i);
        }
    }

    assert(!bestChildren.empty()); // ensure that there is children
    std::uniform_int_distribution<int> uniformIntDistribution(0, bestChildren.size() - 1);
    int i_random = uniformIntDistribution(generator);

    auto outChild = node->child_nodes.at(bestChildren.at(i_random));

    return outChild;

}

std::shared_ptr<ExtendedSearchNode> UCT_PTA::m_expand_delays(std::shared_ptr<ExtendedSearchNode> node) {

    // choose a delay
    int lower = node->bounds.first;
    int upper = node->bounds.second;
    int delay;
    State expanded_state = nullptr;

    if(lower == upper) //If they are equal, just choose one of them
    {
        delay = lower;
    }
    else if(node->visitedDelays.size() < 2) // If size is less than two, the bounds have not been expanded
    {
        delay = node->visitedDelays.size() == 1 ? upper : lower;
    }
    else // lastly expand in the range between the bounds
    {
        delay = get_random_int_except(lower, upper, node->visitedDelays);
    }

    // delay the nodes state, to get the new child node
    if(delay == 0)
    {
        expanded_state = node->state;
    }
    else
    {
        auto out = _environment.DelayState(node->state, delay);

        if(!out.second)
        {
            assert(false); // Delay failed, which should not be possible if GetDelayBounds works correctly
        }
        expanded_state = out.first;
    }

    // Create a node from the newly expanded state
    bool is_terminal = _environment.IsTerminal(expanded_state);
    auto unvisitedChildStates = _environment.GetValidChildStatesNoDelay(expanded_state);

    std::shared_ptr<ExtendedSearchNode> expanded_node = ExtendedSearchNode::create_ExtendedSearchNode(
            node, expanded_state, is_terminal, false);
    expanded_node->set_unvisited_child_states(unvisitedChildStates);

    return expanded_node;
}

int UCT_PTA::get_random_int_except(int lower, int upper, std::vector<int>& exceptions)
{
    std::uniform_int_distribution<int> uniformIntDistribution(0, upper-lower-exceptions.size());
    int i_random = uniformIntDistribution(generator);

    std::sort(exceptions.begin(), exceptions.end());

    int n_passed = 0;

    for (int exception : exceptions)
    {
        if(exception <= i_random+lower+n_passed)
        {
            n_passed += 1;
        }
    }

    return lower+n_passed+i_random;
}

std::shared_ptr<ExtendedSearchNode> UCT_PTA::m_expand_transitions(std::shared_ptr<ExtendedSearchNode> node)
{

    // Choose a transition action to get to the new state
    int i_random = 0;

    if (node->unvisited_child_states.size() > 1) {
        // Get unvisited state randomly
        std::uniform_int_distribution<int> uniformIntDistribution(0, node->unvisited_child_states.size() - 1);
        i_random = uniformIntDistribution(generator);
    }

    State expanded_state = node->unvisited_child_states.at(i_random);
    // Remove the state from unvisited states
    node->unvisited_child_states.erase(node->unvisited_child_states.begin() + i_random);


    // Create a node from the newly expanded state
    auto is_terminal = _environment.IsTerminal(expanded_state);

    std::pair<int, int> bounds = _environment.GetDelayBounds(expanded_state);

    // Create node from expanded_state
    std::shared_ptr<ExtendedSearchNode> expanded_node = ExtendedSearchNode::create_ExtendedSearchNode(node, expanded_state, is_terminal, !is_terminal);
    expanded_node->bounds = bounds;

    return  expanded_node;
}

Reward UCT_PTA::m_default_policy(State &state) {
    return _defaultPolicy.defaultPolicy(state);
}

void UCT_PTA::m_backpropagation(const std::shared_ptr<ExtendedSearchNode>& node, Reward score) {
    return _backup.backup(node, score);
}

std::shared_ptr<ExtendedSearchNode> UCT_PTA::m_tree_policy(std::shared_ptr<ExtendedSearchNode> node) {

    std::shared_ptr<ExtendedSearchNode> current_node = std::move(node);

    while (!current_node->isTerminalState) {

        if(current_node->children_are_delay_actions)
        {
            int visitedSize = current_node->visitedDelays.size();
            int lower = current_node->bounds.first;
            int upper = current_node->bounds.second;
            int n_bounds = lower == upper ? 1 : 2;

            // Check if any bound values are unexpanded
            if(visitedSize < n_bounds)
            {
                return m_expand_delays(current_node);
            }

            int visitedBoundRangeSize = visitedSize-n_bounds;
            int bound_range = std::max((upper-1) - (lower+1), 0);
            
            bool allChildrenExplored = visitedBoundRangeSize == bound_range;

            double percentageVisited = (double) visitedBoundRangeSize / (bound_range+DBL_MIN);
            bool explore = visitedBoundRangeSize <= 15; //&& percentageVisited < 0.2

            if(!allChildrenExplored && explore)
            {
                return m_expand_delays(current_node);
            }
        }

        if (!current_node->unvisited_child_states.empty()) // always evaluates to false for nodes with delay actions
        {
            return m_expand_transitions(current_node);
        }

        if(current_node->child_nodes.empty())
        {
            SearchNode* removeNode = current_node.get();
            SearchNode* parent = nullptr;

            do
            {
                parent = removeNode->parent;
                // Node is non-terminal and has no children, remove from parent and set parent as current_node
                for(unsigned long i = 0; i <= parent->child_nodes.size(); i++)
                {
                    if(parent->child_nodes.at(i).get() == removeNode)
                    {
                        parent->child_nodes.erase(parent->child_nodes.begin()+i);
                        break;
                    }
                }
                removeNode = parent;
            } while(parent->child_nodes.empty());

            auto outChild = m_best_child(parent, 0.7071067811865475);
            current_node = std::static_pointer_cast<ExtendedSearchNode>(outChild);
            continue;
        }
        // 0.7071067811865475 = 1 / sqrt(2) which is the default cp value
        current_node = m_best_child(current_node, 0.7071067811865475);
    }
    return current_node;

}

std::shared_ptr<ExtendedSearchNode> UCT_PTA::m_search(int n_searches) {
    return std::shared_ptr<ExtendedSearchNode>();
}