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
: _environment(environment), generator(std::mt19937(time(nullptr))), _defaultPolicy(UPPAAL_RandomSamplingDefaultPolicy(_environment)), root_node(SearchNode::create_SearchNode(nullptr, false)) {
    // UCT TreePolicy setup
    std::function<std::shared_ptr<SearchNode>(std::shared_ptr<SearchNode>)> f_expand =
            std::bind(&UCT_PTA::m_expand, this, std::placeholders::_1);

    std::function<std::shared_ptr<SearchNode>(std::shared_ptr<SearchNode>, double)> f_best_child =
            std::bind(&UCT_PTA::m_best_child, this, std::placeholders::_1, std::placeholders::_2);
}

State UCT_PTA::run(int n_searches) {

    time_t max_start = time(nullptr);
    long max_time = n_searches;
    long max_timeLeft = max_time;

    State initial_state = _environment.GetStartState();
    std::vector<State> unvisited_child_states = _environment.GetValidChildStates(initial_state);
    root_node = SearchNode::create_SearchNode(nullptr, initial_state, false);
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

std::shared_ptr<SearchNode> UCT_PTA::get_child_states(std::shared_ptr<SearchNode> node){

    if(current_actions_are_time){



    }
}

std::shared_ptr<SearchNode> UCT_PTA::get_time_child_states(std::shared_ptr<SearchNode> node)
{
    int delay;
    // Calculate wether or not we are using a bound value (30% chance)
    std::uniform_int_distribution<int> uniformIntDistribution(1, 10);
    int i_random = uniformIntDistribution(generator);
    bool use_bound_value = i_random <= 3;

    // get bound values
    std::tuple<int, int> bounds = _environment.GetDelayBounds(node->state);
    int lower = std::get<0>(bounds);
    int upper = std::get<1>(bounds);

    if(use_bound_value)
    {
        std::uniform_int_distribution<int> uniformIntDistribution(0, 1);
        int coinflip = uniformIntDistribution(generator);
        delay = coinflip ? lower : upper;
    }
    else
    {
        std::uniform_int_distribution<int> randUpperLower(lower+1, upper-1);
        delay = randUpperLower(generator);
    }

}




std::shared_ptr<SearchNode> UCT_PTA::m_best_child(std::shared_ptr<SearchNode> node, double c) {

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

    return node->child_nodes.at(bestChildren.at(i_random));

}

std::shared_ptr<SearchNode> UCT_PTA::m_expand(std::shared_ptr<SearchNode> node_in) {

    auto node = std::dynamic_pointer_cast<ExtendedSearchNode>(node_in);

    int i_random = 0;
    State expanded_state = nullptr;

    if(node->children_are_time_actions)
    {
        // Choose a delay action and delay the state

        if (node->unvisited_child_states.size() > 1) {
            // Get unvisited delays randomly
            std::uniform_int_distribution<int> uniformIntDistribution(0, node->unvisited_child_states.size() - 1);
            i_random = uniformIntDistribution(generator);
        }

        // delay the nodes state, to get the new child node
        expanded_state = _environment.DelayState(node->state, node->unvisited_child_states.at(i_random));
    }
    else
    {
        // Choose a transition action and transition

        if (node->unvisited_child_states.size() > 1) {
            // Get unvisited state randomly
            std::uniform_int_distribution<int> uniformIntDistribution(0, node->unvisited_child_states.size() - 1);
            i_random = uniformIntDistribution(generator);
        }

        expanded_state = node->unvisited_child_states.at(i_random);
    }

    // Create node from unvisited state
    auto is_terminal = _environment.IsTerminal(expanded_state);
    auto expanded_node = ExtendedSearchNode::create_ExtendedSearchNode(node_in, expanded_state, is_terminal, !node->children_are_time_actions);

    if(node->children_are_time_actions)
    {
        // If the previous node had delay actions, the new node have transition actions which we set as valid child states
        auto unvisitedChildStates = _environment.GetValidChildStatesNoDelay(expanded_state);
        expanded_node->set_unvisited_child_states(unvisitedChildStates);
    }
    else
    {

    }


    // Remove the state from unvisited states
    node->unvisited_child_states.erase(node->unvisited_child_states.begin() + i_random);

    return expanded_node;
}

Reward UCT_PTA::m_default_policy(State &state) {
    return _defaultPolicy.defaultPolicy(state);
}

void UCT_PTA::m_backpropagation(std::shared_ptr<SearchNode> node, Reward score) {
    return _backup.backup(node, score);
}

std::shared_ptr<SearchNode> UCT_PTA::m_tree_policy(std::shared_ptr<SearchNode> node) {

    std::shared_ptr<SearchNode> current_node = std::move(node);
    while (!current_node->isTerminalState) {

        if(std::dynamic_pointer_cast<ExtendedSearchNode>(current_node)->children_are_time_actions)
        {
            // Calculate wether or not we are using a bound value (30% chance)
            std::uniform_int_distribution<int> uniformIntDistribution(1, 100);
            int i_random = uniformIntDistribution(generator);
            bool use_bound_value = i_random <= 30;

            current_node = use_bound_value ? current_node->child_nodes.at(0) : current_node->child_nodes.at(1);
        }

        if (!current_node->unvisited_child_states.empty()) {
            return m_expand(current_node);
        }
        // 0.7071067811865475 = 1 / sqrt(2) which is the default cp value
        current_node = m_best_child(current_node, 0.7071067811865475);
    }
    return current_node;

}

std::shared_ptr<SearchNode> UCT_PTA::m_search(int n_searches) {
    return std::shared_ptr<SearchNode>();
}
