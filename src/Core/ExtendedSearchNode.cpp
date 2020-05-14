//
// Created by happysun on 15/04/2020.
//

#include "ExtendedSearchNode.h"
#include <optional>

ExtendedSearchNode::ExtendedSearchNode(ExtendedSearchNode *parent_node, State &state, bool isTerminal,
                                       bool children_are_time)
    : SearchNode(parent_node, state, isTerminal), children_are_delay_actions(children_are_time),
      parent_extended(parent_node){};

std::shared_ptr<ExtendedSearchNode>
ExtendedSearchNode::create_ExtendedSearchNode(std::shared_ptr<ExtendedSearchNode> &parent_node, State &state,
                                              bool isTerminal, bool children_are_time_actions) {
    auto newNode = std::make_shared<ExtendedSearchNode>(
        ExtendedSearchNode(parent_node.get(), state, isTerminal, children_are_time_actions));
    if (parent_node) {
        parent_node->add_child((std::shared_ptr<SearchNode>)(newNode));
    }

    return newNode;
}

std::shared_ptr<ExtendedSearchNode> ExtendedSearchNode::create_ExtendedSearchNode(ExtendedSearchNode *parent_node,
                                                                                  State &state, bool isTerminal,
                                                                                  bool children_are_time_actions) {
    auto newNode = std::make_shared<ExtendedSearchNode>(
        ExtendedSearchNode(parent_node, state, isTerminal, children_are_time_actions));
    if (parent_node) {
        parent_node->add_child((std::shared_ptr<SearchNode>)(newNode));
    }

    return newNode;
}

std::shared_ptr<ExtendedSearchNode> ExtendedSearchNode::create_ExtendedSearchNode(ExtendedSearchNode *parent_node,
                                                                                  bool isTerminal,
                                                                                  bool children_are_time_actions) {
    State tempState = State(std::nullopt);
    auto newNode = std::make_shared<ExtendedSearchNode>(
        ExtendedSearchNode(parent_node, tempState, isTerminal, children_are_time_actions));
    if (parent_node) {
        parent_node->add_child((std::shared_ptr<SearchNode>)(newNode));
    }

    return newNode;
}