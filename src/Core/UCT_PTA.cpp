//
// Created by happysun on 27/12/2019.
//

#include <UCT_PTA.h>
#include <cassert>
#include <cfloat>
#include <cmath>

UCT_PTA::UCT_PTA(EnvironmentInterface &environment)
    : _environment(environment), generator(std::mt19937(time(nullptr))),
      _defaultPolicy(UPPAAL_RandomSamplingDefaultPolicy(_environment)),
      root_node(SearchNode::create_SearchNode(nullptr, false)) {
    // UCT TreePolicy setup
    std::function<std::shared_ptr<SearchNode>(std::shared_ptr<SearchNode>)> f_expand =
        std::bind(&UCT_PTA::m_expand, this, std::placeholders::_1);

    std::function<std::shared_ptr<SearchNode>(std::shared_ptr<SearchNode>, double)> f_best_child =
        std::bind(&UCT_PTA::m_best_child, this, std::placeholders::_1, std::placeholders::_2);

    _tpolicy = UCT_TreePolicy(f_expand, f_best_child);
}

State UCT_PTA::run(int n_searches) {

    time_t max_start = time(nullptr);
    long max_time = n_searches;
    long max_timeLeft = max_time;

    State initial_state = _environment.GetStartState();
    std::vector<State> unvisited_child_states = _environment.GetValidChildStates(initial_state);
    m_root = SearchNode::create_SearchNode(nullptr, initial_state, false);
    m_root->set_unvisited_child_states(unvisited_child_states);

    // rough bootstrap of reward scaling
    std::vector<double> rewards(100, 0);
    for (int i = 0; i < rewards.size(); ++i) {
        Reward score = m_default_policy(m_root->state);
        rewards.at(i) = score;
    }

    auto it = std::minmax_element(std::begin(rewards), std::end(rewards));
    rewardMinMax.first = *it.first;
    rewardMinMax.second = *it.second;

    while (!best_proved && max_timeLeft > 0) {
        // TreePolicy runs to find an unexpanded node to expand
        auto expandedNode = m_tree_policy(m_root);
        // From the expanded node, a simulation runs that returns a score
        std::vector<double> sim_scores(1, 0);
        for (int i = 0; i < sim_scores.size(); ++i) {
            Reward simulation_score = m_default_policy(expandedNode->state);
            sim_scores.at(i) = simulation_score;
        }
        
        auto simMinMax = std::minmax_element(std::begin(sim_scores), std::end(sim_scores));
        // eventually update min max reward
        if (*simMinMax.first < rewardMinMax.first) {
            rewardMinMax.first = *simMinMax.first;
        } else if (*simMinMax.second > rewardMinMax.second) {
            rewardMinMax.second = *simMinMax.second;
        }

        auto avg_score = 0;
        avg_score = std::accumulate(std::begin(sim_scores), std::end(sim_scores), 0.0) / sim_scores.size();

        // normalize data
        double norm_score = (avg_score - rewardMinMax.first) / (rewardMinMax.second - rewardMinMax.first);

        // The score is backpropagated up through the search tree
        m_backpropagation(expandedNode, norm_score);

        // if new terminal node is encountered, update the bestTerminalNode
        if (expandedNode->isTerminalState) {
            Reward termReward = _environment.EvaluateRewardFunction(expandedNode->state);
            if (bestTerminalNodesFound.empty() || bestTerminalNodesFound.back().score < termReward) {
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

    root_node = m_root;

    if (bestTerminalNodesFound.empty()) {
        return nullptr;
    } else {
        return bestTerminalNodesFound.at(0).node->state;
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

std::shared_ptr<SearchNode> UCT_PTA::m_expand(std::shared_ptr<SearchNode> node) {

    int i_random = 0;

    if (node->unvisited_child_states.size() > 1) {
        // Get unvisited state randomly
        std::uniform_int_distribution<int> uniformIntDistribution(0, node->unvisited_child_states.size() - 1);
        i_random = uniformIntDistribution(generator);
    }

    State expanded_state = node->unvisited_child_states.at(i_random);

    // Create node from unvisited state
    auto is_terminal = _environment.IsTerminal(expanded_state);
    auto expanded_node = SearchNode::create_SearchNode(node, expanded_state, is_terminal);

    // Set unvisited child States
    auto unvisitedChildStates = _environment.GetValidChildStates(expanded_state);
    expanded_node->set_unvisited_child_states(unvisitedChildStates);

    // Remove the state from unvisited states
    node->unvisited_child_states.erase(node->unvisited_child_states.begin() + i_random);

    return expanded_node;
}

Reward UCT_PTA::m_default_policy(State &state) { return _defaultPolicy.defaultPolicy(state); }

void UCT_PTA::m_backpropagation(std::shared_ptr<SearchNode> node, Reward score) { return _backup.backup(node, score); }

std::shared_ptr<SearchNode> UCT_PTA::m_tree_policy(std::shared_ptr<SearchNode> node) {
    return _tpolicy.treePolicy(node);
}

std::shared_ptr<SearchNode> UCT_PTA::m_search(int n_searches) { return std::shared_ptr<SearchNode>(); }
