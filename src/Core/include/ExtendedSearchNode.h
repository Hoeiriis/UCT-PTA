//
// Created by happysun on 15/04/2020.
//

#ifndef EXAMPLE_EXEC_EXTENDEDSEARCHNODE_H
#define EXAMPLE_EXEC_EXTENDEDSEARCHNODE_H

#include <SearchNode.h>

class ExtendedSearchNode : public SearchNode
{
    private:
        ExtendedSearchNode(SearchNode *parent_node, State &state, bool isTerminal, bool children_are_time_actions);

    public:
    //    std::vector<int>
        static std::shared_ptr<ExtendedSearchNode> create_ExtendedSearchNode(std::shared_ptr<SearchNode> &parent_node, State &state,
                                                         bool isTerminal, bool children_are_time_actions);
        bool children_are_time_actions;
};


#endif //EXAMPLE_EXEC_EXTENDEDSEARCHNODE_H
