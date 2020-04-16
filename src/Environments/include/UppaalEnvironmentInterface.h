#ifndef UPPAALENVIRONMENTINTERFACE_H
#define UPPAALENVIRONMENTINTERFACE_H

#include <EnvironmentInterface.h>

class UppaalEnvironmentInterface : public EnvironmentInterface {

  public:
    virtual std::tuple<int, int> GetDelayBounds(State &state) = 0;

    virtual std::tuple<State, bool> DelayState(State &state, int delay) = 0;

    virtual std::vector<State> GetValidChildStatesNoDelay(State &state) = 0;
};

#endif // UPPAALENVIRONMENTINTERFACE_H