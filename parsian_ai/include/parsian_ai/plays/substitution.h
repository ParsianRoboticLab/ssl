#ifndef SUBSTITUTION_H
#define SUBSTITUTION_H

#include "masterplay.h"

class CSubstitution : public CMasterPlay{
public:
        CSubstitution();
        ~CSubstitution();
        void execute_x();
        void init(QList<Agent*>& _agents);
private:
        void reset();

};

#endif // SUBSTITUTION_H
