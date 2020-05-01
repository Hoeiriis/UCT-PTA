//
// Created by happysun on 15/04/2020.
//

#ifndef EXAMPLE_EXEC_EXTENDEDSEARCHNODE_H
#define EXAMPLE_EXEC_EXTENDEDSEARCHNODE_H

#include <SearchNode.h>

class ExtendedSearchNode : public SearchNode {
  private:
    ExtendedSearchNode(ExtendedSearchNode *parent_node, State &state, bool isTerminal, bool children_are_time_actions);

  public:
    static std::shared_ptr<ExtendedSearchNode>
    create_ExtendedSearchNode(std::shared_ptr<ExtendedSearchNode> &parent_node, State &state, bool isTerminal,
                              bool children_are_time_actions);
    static std::shared_ptr<ExtendedSearchNode> create_ExtendedSearchNode(ExtendedSearchNode *parent_node, State &state, bool isTerminal, bool children_are_time_actions);

    static std::shared_ptr<ExtendedSearchNode> create_ExtendedSearchNode(ExtendedSearchNode *parent_node, bool isTerminal, bool children_are_time_actions);

    bool children_are_delay_actions;
    ExtendedSearchNode* parent_extended;
    std::vector<int> visitedDelays{};
    std::pair<int, int> bounds{ -1, -1};
};

#endif // EXAMPLE_EXEC_EXTENDEDSEARCHNODE_H
