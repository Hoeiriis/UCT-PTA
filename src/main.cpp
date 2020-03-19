#include <iostream>
#include <omp.h>
#include "TicTacToeEnvironment.h"
#include "SearchNode.h"
#include "UCT.h"

int main()
{
    std::cout << "Hello world\n";
    State d = State("ekekke");
    auto env = TicTacToeEnv();
    auto sState = env.GetStartState();
    env.PrintBoard(sState);

    auto m_root = SearchNode::create_SearchNode(nullptr, sState, false);
    auto uct = UCT(env);
}