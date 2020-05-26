//
// Created by happysun on 27/12/2019.
//

#include <ExtendedSearchNode.h>
#include <MCTSEntry.h>
#include <UCT_PTA.h>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <tuple>
#include <utility>

UCT_PTA::UCT_PTA(UppaalEnvironmentInterface &environment, int unrolledStatesLimit)
    : _environment(environment), generator(std::mt19937(time(nullptr))),
      _defaultPolicy(DelaySamplingDefaultPolicy(_environment, unrolledStatesLimit)),
      root_node(ExtendedSearchNode::create_ExtendedSearchNode(nullptr, false, true)) {
    /** UCT TreePolicy setup
    std::function<std::shared_ptr<ExtendedSearchNode>(std::shared_ptr<ExtendedSearchNode>)> f_expand =
            std::bind(&UCT_PTA::m_expand, this, std::placeholders::_1);

    std::function<std::shared_ptr<ExtendedSearchNode>(std::shared_ptr<ExtendedSearchNode>, double)> f_best_child =
            std::bind(&UCT_PTA::m_best_child, this, std::placeholders::_1, std::placeholders::_2); **/
}

State UCT_PTA::run(int n_searches, int exploreLimitAbs, double exploreLimitPercent, int bootstrapLimit) {
    time_t max_start = time(nullptr);
    long max_time = n_searches;
    long max_timeLeft = max_time;
    long _expanded = 0;

    State initial_state = _environment.GetStartState();
    root_node = ExtendedSearchNode::create_ExtendedSearchNode(nullptr, initial_state, false, true);
    root_node->bounds = _environment.GetDelayBounds(initial_state);

    bootstrap_reward_scaling(bootstrapLimit);

    std::cerr << "Primariy loop" << std::endl;

    while (!best_proved && max_timeLeft > 0) {
        // TreePolicy runs to find an unexpanded node to expand
        std::shared_ptr<ExtendedSearchNode> expandedNode =
            m_tree_policy(root_node, exploreLimitPercent, exploreLimitAbs);

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
        double norm_score = (avg_score - rewardMinMax.first + DBL_MIN) / (rewardMinMax.second - rewardMinMax.first + DBL_MIN);
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
                newBestNode.nodes_expanded = _expanded;
                // insert at beginning
                bestTerminalNodesFound.push_back(newBestNode);
            }
            expandedNode->score = {-100000, 0};
        }

        // update maxTime
        max_timeLeft = max_time - (time(nullptr) - max_start);
        _expanded += 1;
    }

    if (bestTerminalNodesFound.empty()) {
        return nullptr;
    } else {
        return bestTerminalNodesFound.at(0).node->state;
    }
}

void UCT_PTA::bootstrap_reward_scaling(int bootstrapLimit) {
    // rough bootstrap of reward scaling
    std::vector<double> rewards(bootstrapLimit, 0);
    for (int i = 0; i < rewards.size() ; ++i) {
        Reward score = m_default_policy(root_node->state);
        rewards.at(i) = score;
    }

    auto it = std::minmax_element(std::begin(rewards), std::end(rewards));
    rewardMinMax.first = *it.first;
    rewardMinMax.second = *it.second;
}

std::shared_ptr<ExtendedSearchNode> UCT_PTA::m_best_child(std::shared_ptr<ExtendedSearchNode> node, double c) {
    auto baseNode = std::static_pointer_cast<SearchNode>(node);
    auto outChild = m_best_child(baseNode.get(), c);
    return std::static_pointer_cast<ExtendedSearchNode>(outChild);
}

std::shared_ptr<SearchNode> UCT_PTA::m_best_child(const SearchNode *node, double c) {

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
    int interestingDelay = node->bounds.second;
    int delay = 0;
    int possibleDelays = interestingDelay == 0 ? 1 : 2;
    State expanded_state = nullptr;

    std::vector<State> unvisitedChildStates{};
    bool is_terminal;

    do {
        if (node->visitedDelays.empty())
        {
            delay = 0;
        } else
        {
            delay = interestingDelay;
        }

        // delay the nodes state, to get the new child node
        if (delay == 0) {
            expanded_state = node->state;
        } else {
            auto out = _environment.DelayState(node->state, delay);

            if (!out.second) {
                assert(false); // Delay failed, which should not be possible if GetDelayBounds works correctly
            }
            expanded_state = out.first;
        }

        // Create a node from the newly expanded state
        is_terminal = _environment.IsTerminal(expanded_state);
        unvisitedChildStates = _environment.GetValidChildStatesNoDelay(expanded_state);

        // Set the delay as visited
        node->visitedDelays.push_back(delay);

        // if the newly expanded state has no child state, then we try another delay
    } while (unvisitedChildStates.empty() && (node->visitedDelays.size() < possibleDelays) && !is_terminal);

    // In case no delays had any child states
    if (node->visitedDelays.size() == possibleDelays && unvisitedChildStates.empty()) {
        assert(false); // This should not be possible (in our job-shop and spreadsheet models at least)
    }

    std::shared_ptr<ExtendedSearchNode> expanded_node =
            ExtendedSearchNode::create_ExtendedSearchNode(node, expanded_state, is_terminal, false);
    expanded_node->set_unvisited_child_states(unvisitedChildStates);
    return expanded_node;
}

int UCT_PTA::get_random_int_except(int lower, int upper, std::vector<int> &exceptions) {
    std::uniform_int_distribution<int> uniformIntDistribution(0, upper - lower - exceptions.size());
    int i_random = uniformIntDistribution(generator);

    std::sort(exceptions.begin(), exceptions.end());

    int n_passed = 0;

    for (int exception : exceptions) {
        if (exception <= i_random + lower + n_passed) {
            n_passed += 1;
        }
    }

    return lower + n_passed + i_random;
}

std::shared_ptr<ExtendedSearchNode> UCT_PTA::m_expand_transitions(std::shared_ptr<ExtendedSearchNode> node) {

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
    std::shared_ptr<ExtendedSearchNode> expanded_node =
        ExtendedSearchNode::create_ExtendedSearchNode(node, expanded_state, is_terminal, !is_terminal);
    expanded_node->bounds = bounds;

    return expanded_node;
}

Reward UCT_PTA::m_default_policy(State &state) { return _defaultPolicy.defaultPolicy(state); }

void UCT_PTA::m_backpropagation(const std::shared_ptr<ExtendedSearchNode> &node, Reward score) {
    return _backup.backup(node, score);
}

std::shared_ptr<ExtendedSearchNode> UCT_PTA::m_tree_policy(std::shared_ptr<ExtendedSearchNode> node,
                                                           double exploreLimitPercent, int exploreLimitAbs) {

    std::shared_ptr<ExtendedSearchNode> current_node = std::move(node);

    while (!current_node->isTerminalState) {

        if (current_node->children_are_delay_actions) {
            int interestingDelay = current_node->bounds.second;
            int possibleDelays = interestingDelay == 0 ? 1 : 2;

            if(current_node->visitedDelays.size() < possibleDelays){
                return m_expand_delays(current_node);
            }
        }

        if (!current_node->unvisited_child_states.empty()) // always evaluates to false for nodes with delay actions
        {
            return m_expand_transitions(current_node);
        }

        if (current_node->child_nodes.empty()) {
            ExtendedSearchNode *removeNode = current_node.get();
            ExtendedSearchNode *parent = nullptr;
            bool delaysExplored;

            do {
                parent = removeNode->parent_extended;
                // Node is non-terminal and has no children, remove from parent and set parent as current_node
                for (unsigned long i = 0; i <= parent->child_nodes.size(); i++) {
                    if (parent->child_nodes.at(i).get() == removeNode) {
                        parent->child_nodes.erase(parent->child_nodes.begin() + i);
                        break;
                    }
                }
                removeNode = parent;

                delaysExplored =
                    parent->children_are_delay_actions
                        ? parent->visitedDelays.size() == (parent->bounds.second - parent->bounds.first) + 1
                        : true;

            } while (parent->child_nodes.empty() && parent->unvisited_child_states.empty() && delaysExplored);

            auto outChild = m_best_child(parent, 0.7071067811865475);
            current_node = std::static_pointer_cast<ExtendedSearchNode>(outChild);
            continue;
        }
        // 0.7071067811865475 = 1 / sqrt(2) which is the default cp value
        current_node = m_best_child(current_node, 0.7071067811865475);
    }
    return current_node;
}

std::shared_ptr<ExtendedSearchNode> UCT_PTA::m_search(int n_searches) { return std::shared_ptr<ExtendedSearchNode>(); }
