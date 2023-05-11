#ifndef ROUTER_DEF
#define ROUTER_DEF

#include "scheduler.h"
class FindGate: public Task{
public:
    FindGate(Scheduler& m):master(m){}
    bool Execute()override;
private:
    Scheduler& master;
};

#endif // !ROUTER_DEF
