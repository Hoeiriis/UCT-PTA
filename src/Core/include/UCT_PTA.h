//
// Created by happysun on 27/12/2019.
//

#ifndef MCTS_UCT_H
#define MCTS_UCT_H

#include <BasicBackup.h>
#include <UCT_TreePolicy.h>
#include <UPPAAL_RandomSamplingDefaultPolicy.h>
#include <UppaalEnvironmentInterface.h>
#include <functional>
#include <random>
#include "ExtendedSearchNode.h"

class TerminalNodeScore {
  public:
    std::shared_ptr<SearchNode> node;
    double score;
    long time_to_find;
};

class UCT_PTA {
  public:
    explicit UCT_PTA(UppaalEnvironmentInterface &environment);
    State run(int n_searches);
    inline UppaalEnvironmentInterface &getEnvironment() { return _environment; }
    inline std::vector<TerminalNodeScore> &getBestTerminalNodeScore() { return bestTerminalNodesFound; }

    std::shared_ptr<SearchNode> root_node;

    std::mt19937 generator;

  protected:
    Reward m_default_policy(State &state);
    std::shared_ptr<SearchNode> m_search(int n_searches);
    std::shared_ptr<SearchNode> m_tree_policy(std::shared_ptr<SearchNode> node);
    std::shared_ptr<SearchNode> m_best_child(std::shared_ptr<SearchNode> node, double c);
    std::shared_ptr<SearchNode> m_expand(std::shared_ptr<ExtendedSearchNode> node);
    void m_backpropagation(std::shared_ptr<SearchNode> node, Reward score);
    void bootstrap_reward_scaling();
    std::shared_ptr<SearchNode> get_child_states(std::shared_ptr<SearchNode> node);

    bool best_proved = false;
    bool current_actions_are_time = true;
    UppaalEnvironmentInterface &_environment;
    std::vector<TerminalNodeScore> bestTerminalNodesFound;
    

  private:
    // UCT Backup setup
    BasicBackup _backup = BasicBackup();

    // UCT TreePolicy setup
    std::function<std::shared_ptr<SearchNode>(std::shared_ptr<SearchNode> node)> placeholderFunc =
        [](std::shared_ptr<SearchNode> node) { return node; };

    std::function<std::shared_ptr<SearchNode>(std::shared_ptr<SearchNode> node, double c)> placeholderFunc2 =
        [](std::shared_ptr<SearchNode> node, double c) { return node; };

    // UCT Default Policy setup
    UPPAAL_RandomSamplingDefaultPolicy _defaultPolicy;

    // Reward min/max
    std::pair<double, double> rewardMinMax = {0, 0};
};

#endif // MCTS_UCT_H
