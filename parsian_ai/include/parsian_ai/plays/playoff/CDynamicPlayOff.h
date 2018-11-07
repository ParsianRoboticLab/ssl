//
// Created by parsian-ai on 11/7/18.
//

#ifndef PARSIAN_AI_DYNAMICPLAYOFF_H
#define PARSIAN_AI_DYNAMICPLAYOFF_H

#include <parsian_ai/plays/masterplay.h>

class CDynamicPlayOff : public CMastserPlay {
public:
    CDynamicPlayOff();
    ~CDynamicPlayOff() override;
    void reset() override;
    void execute_x() override;
    void init(QList<Agent*> _agents) override;
};


#endif //PARSIAN_AI_DYNAMICPLAYOFF_H
