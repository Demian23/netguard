#ifndef ARPER_DEF
#define ARPER_DEF


#include "../include/scheduler.h"
class Arper : public Task{
public:
    Arper(Scheduler& m);
    bool Execute() override;
private:
    Scheduler& master;
};

#endif 
