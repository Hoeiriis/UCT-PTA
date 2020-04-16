//
// Created by happysun on 15/04/2020.
//

#include <optional>
#include "ExtendedSearchNode.h"

ExtendedSearchNode::ExtendedSearchNode(SearchNode *parent_node, State &state, bool isTerminal,
                                       bool children_are_time)
                                       :SearchNode(parent_node, state, isTerminal) {
    children_are_time_actions = children_are_time;
}

std::shared_ptr<ExtendedSearchNode> ExtendedSearchNode::create_ExtendedSearchNode(std::shared_ptr<SearchNode> &parent_node,
                                                                  State &state, bool isTerminal,
                                                                  bool children_are_time_actions){
    State tempState = State(std::nullopt);
    auto newNode = std::make_shared<ExtendedSearchNode>(ExtendedSearchNode(parent_node.get(), tempState, isTerminal, children_are_time_actions));
    if (parent_node) {
        parent_node->add_child((std::shared_ptr<SearchNode&>)(newNode));
    }

    return newNode;
}