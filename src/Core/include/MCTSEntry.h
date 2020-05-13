/*
 *  Copyright Peter G. Jensen, all rights reserved.
 */

/*
 * File:   MCTSEntry.h
 * Author: Peter G. Jensen <root@petergjoel.dk>
 *
 * Created on December 12, 2019, 10:27 AM
 */

#ifndef MCTSENTRY_H
#define MCTSENTRY_H

#include "UCT_PTA.h"
#include <SearchNode.h>
#include <UppaalEnvironmentInterface.h>

class MCTSEntry {

  public:
    explicit MCTSEntry(UppaalEnvironmentInterface &env);
    std::vector<State> state_trace{};
    bool run();
    bool bfs();
    bool dfs();
    int time_limit_sec = 10;
    long states_explored = 0;
    long count_states(std::shared_ptr<SearchNode> &root);

    inline std::vector<TerminalNodeScore> getTerminalNodeScores() { return terminalNodeScores; };

  protected:
    void dfsLoop(State &currentState, int levels);

    UppaalEnvironmentInterface &_environment;
    std::vector<State> compute_state_trace(const std::shared_ptr<SearchNode> &final_node);
    std::vector<TerminalNodeScore> terminalNodeScores;
};

#endif /* MCTSENTRY_H */
