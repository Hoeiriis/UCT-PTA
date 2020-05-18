//
// Created by happysun on 27/12/2019.
//

#ifndef UCTPTA_UCT_H
#define UCTPTA_UCT_H

#include "ExtendedSearchNode.h"
#include <BasicBackup.h>
#include <DelaySamplingDefaultPolicy.h>
#include <UCT_TreePolicy.h>
#include <UppaalEnvironmentInterface.h>
#include <functional>
#include <random>

class TerminalNodeScore {
  public:
    std::shared_ptr<ExtendedSearchNode> node;
    double score;
    long time_to_find;
    long nodes_expanded;
};

class UCT_PTA {
  public:
    explicit UCT_PTA(UppaalEnvironmentInterface &environment);
    explicit UCT_PTA(UppaalEnvironmentInterface &environment, int unrolledStatesLimit);
    State run(int n_searches);
    State run(int n_searches, int exploreLimitAbs, double exploreLimitPercent, int boostrapLimit);
    inline UppaalEnvironmentInterface &getEnvironment() { return _environment; }
    inline std::vector<TerminalNodeScore> &getBestTerminalNodeScore() { return bestTerminalNodesFound; }

    std::shared_ptr<ExtendedSearchNode> root_node;

    std::mt19937 generator;

  protected:
    Reward m_default_policy(State &state);
    std::shared_ptr<ExtendedSearchNode> m_search(int n_searches);
    std::shared_ptr<ExtendedSearchNode> m_tree_policy(std::shared_ptr<ExtendedSearchNode> node);
    std::shared_ptr<ExtendedSearchNode> m_tree_policy(std::shared_ptr<ExtendedSearchNode> node, double explorePercent, int exploreAbs);
    std::shared_ptr<ExtendedSearchNode> m_best_child(std::shared_ptr<ExtendedSearchNode> node, double c);
    std::shared_ptr<SearchNode> m_best_child(const SearchNode *node, double c);
    std::shared_ptr<ExtendedSearchNode> m_expand_delays(std::shared_ptr<ExtendedSearchNode> node_in);
    std::shared_ptr<ExtendedSearchNode> m_expand_transitions(std::shared_ptr<ExtendedSearchNode> node_in);
    void m_backpropagation(const std::shared_ptr<ExtendedSearchNode> &node, Reward score);
    void bootstrap_reward_scaling(int bootstrapLimit);
    int get_random_int_except(int lower, int upper, std::vector<int> &exceptions);

    bool best_proved = false;
    UppaalEnvironmentInterface &_environment;
    std::vector<TerminalNodeScore> bestTerminalNodesFound;

  private:
    // UCT Backup setup
    BasicBackup _backup = BasicBackup();

    // UCT Default Policy setup
    DelaySamplingDefaultPolicy _defaultPolicy;

    // Reward min/max
    std::pair<double, double> rewardMinMax = {0, 0};
};

#endif // UCTPTA_UCT_H
