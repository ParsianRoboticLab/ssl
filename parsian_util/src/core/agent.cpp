//
// Created by parsian-ai on 9/21/17.
//
#include "parsian_util/core/agent.h"

CAgent::CAgent(const parsian_msgs::parsian_agent &_agent)  : self(_agent.self) {
    action = nullptr;
}

CAgent::CAgent(int id): self(id) {
    action = nullptr;
}

Vector2D CAgent::pos() {
    return self.pos;
}

Vector2D CAgent::acc() {
    return self.acc;
}

Vector2D CAgent::vel() {
    return self.vel;
}

Vector2D CAgent::dir() {
    return self.dir;
}

int CAgent::id() {
    return self.id;
}
