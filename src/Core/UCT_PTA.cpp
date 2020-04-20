//
// Created by happysun on 27/12/2019.
//

#include <cfloat>
#include <cassert>
#include <cmath>
#include <tuple>
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
    std::vector<State> unvisited_child_states = _environment.GetValidChildStates(initial_state);
    root_node = ExtendedSearchNode::create_ExtendedSearchNode(nullptr, initial_state, false, true);
    root_node->set_unvisited_child_states(unvisited_child_states);

    bootstrap_reward_scaling();

    while(!best_proved && max_timeLeft > 0) {
        // TreePolicy runs to find an unexpanded node to expand
        auto expandedNode = m_tree_policy(root_node);
        // From the expanded node, a simulation runs that returns a score
        std::vector<double> sim_scores(20, 0);
        for (int i=0; i < 20; ++i){
            Reward simulation_score = m_default_policy(expandedNode->state);
            sim_scores.at(i) = simulation_score;
        }
        auto simMinMax = std::minmax_element(std::begin(sim_scores), std::end(sim_scores));
        // eventually update min max reward
        if(*simMinMax.first < rewardMinMax.first){
            rewardMinMax.first = *simMinMax.first;
        } else if(*simMinMax.second > rewardMinMax.second){
            rewardMinMax.second = *simMinMax.second;
        }

        auto avg_score = std::accumulate(std::begin(sim_scores), std::begin(sim_scores), 0) / sim_scores.size();

        // normalize data
        double norm_score = (avg_score-rewardMinMax.first)/(rewardMinMax.second - rewardMinMax.first);
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

std::shared_ptr<ExtendedSearchNode> UCT_PTA::m_best_child(std::shared_ptr<ExtendedSearchNode> node, double c) {

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

    return std::static_pointer_cast<ExtendedSearchNode>(outChild);

}

std::shared_ptr<ExtendedSearchNode> UCT_PTA::m_expand_delays(std::shared_ptr<ExtendedSearchNode> node) {

    State expanded_state = nullptr;

    int lower = node->bounds.first;
    int upper = node->bounds.second;
    int delay;

    if(node->visitedDelays.size() < 2)
    {
        delay = node->visitedDelays.size() == 1 ? upper : lower;
    }
    else 
    {
        // Get unvisited delays randomly
        delay = get_random_int_except((lower+1), (upper-1), node->visitedDelays);
    }

    // delay the nodes state, to get the new child node
    expanded_state = _environment.DelayState(node->state, delay);

    // Create node from unvisited state
    auto is_terminal = _environment.IsTerminal(expanded_state);
    std::shared_ptr<ExtendedSearchNode> expanded_node = ExtendedSearchNode::create_ExtendedSearchNode(node, expanded_state, is_terminal, !node->children_are_delay_actions);

    // Set unvisited child
    auto unvisitedChildStates = _environment.GetValidChildStatesNoDelay(expanded_state);
    expanded_node->set_unvisited_child_states(unvisitedChildStates);

    return expanded_node;
}

int UCT_PTA::get_random_int_except(int lower, int upper, const std::vector<int>& exceptions)
{
    std::uniform_int_distribution<int> uniformIntDistribution(0, upper-lower-exceptions.size());
    int i_random = uniformIntDistribution(generator);

    int n_passed = 0;

    for (int exception : exceptions)
    {
        if(exception <= i_random+lower+n_passed)
        {
            n_passed += 1;
        }
        else
        {
            break;
        }
    }

    return lower+n_passed+i_random;
}


std::shared_ptr<ExtendedSearchNode> UCT_PTA::m_expand_transitions(std::shared_ptr<ExtendedSearchNode> node) {
        
        // Choose a transition action to get to the new state
        int i_random = 0;
        std::pair<int, int> bounds{-1, -1};

        if (node->unvisited_child_states.size() > 1) {
            // Get unvisited state randomly
            std::uniform_int_distribution<int> uniformIntDistribution(0, node->unvisited_child_states.size() - 1);
            i_random = uniformIntDistribution(generator);
        }

        State expanded_state = node->unvisited_child_states.at(i_random);
        auto is_terminal = _environment.IsTerminal(expanded_state);
        bool childIsDelayAction = false;
        
        if(!is_terminal)
        {
            bounds = _environment.GetDelayBounds(expanded_state);
        
            childIsDelayAction = bounds.first != bounds.second;
            // If there is only one valid delay, then we just delay immediately unless the delay is 0 (in which case it is not necessary) or the state is terminal
            // Adriana: unless the delay is or is not 0? Is the condition even necessary ? :D 
            if(childIsDelayAction && bounds.first == 0){
                expanded_state = _environment.DelayState(expanded_state, bounds.first).first;
            }
        }

        // Create node from expanded_state
        std::shared_ptr<ExtendedSearchNode> expanded_node = ExtendedSearchNode::create_ExtendedSearchNode(node, expanded_state, is_terminal, childIsDelayAction);
        
        if(childIsDelayAction)
        {
            expanded_node->bounds = bounds;
        }
        
        // Remove the state from unvisited states
        node->unvisited_child_states.erase(node->unvisited_child_states.begin() + i_random);
}


Reward UCT_PTA::m_default_policy(State &state) {
    return _defaultPolicy.defaultPolicy(state);
}

void UCT_PTA::m_backpropagation(std::shared_ptr<ExtendedSearchNode> node, Reward score) {
    return _backup.backup(node, score);
}

std::shared_ptr<ExtendedSearchNode> UCT_PTA::m_tree_policy(std::shared_ptr<ExtendedSearchNode> node) {

    std::shared_ptr<ExtendedSearchNode> current_node = std::move(node);
    while (!current_node->isTerminalState) {

        if(current_node->children_are_delay_actions)
        {
            int visitedSize = current_node->visitedDelays.size();
            // Check if any bound values are unexpanded
            if(visitedSize < 2)
            {
                return m_expand_delays(current_node);
            }

            int lower = current_node->bounds.first;
            int upper = current_node->bounds.second;
            int bound_range = (upper - lower)+1;
            
            bool allChildrenExplored = visitedSize == bound_range;

            double percentageVisited = (double) visitedSize / (upper - lower);
            bool explore = percentageVisited < 0.25 || visitedSize <= 25;

            if(!allChildrenExplored && explore)
            {
                return m_expand_delays(current_node);
            }
        }

        if (!current_node->unvisited_child_states.empty()) // always evaluates to false for nodes with delay actions
        {
            return m_expand_transitions(current_node);
        }

        // 0.7071067811865475 = 1 / sqrt(2) which is the default cp value
        current_node = m_best_child(current_node, 0.7071067811865475);
    }
    return current_node;

}

std::shared_ptr<ExtendedSearchNode> UCT_PTA::m_search(int n_searches) {
    return std::shared_ptr<ExtendedSearchNode>();
}
