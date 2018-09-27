#include "parsian_agent/skill.h"

CSkill::CSkill(Agent* _agent) : localAgentName() {
    agent = _agent;
}

CSkill::~CSkill() = default;


QList<CSkills::RegisteredSkill>* CSkills::Skills;
bool CSkills::inited = false;

CSkills::CSkills() = default;

CSkills::~CSkills() {
    delete Skills;
}

bool CSkills::registerSkill(const char *name, CSkill* Skill) {
    if (!inited) {
        Skills = new QList<CSkills::RegisteredSkill>;
        inited = true;
    }
    void* info = nullptr;
//    if (Skill->level()==2)
//    {
//        CRole* role = static_cast <CRole*> (Skill);
//        info = (void*) (role->generateInfoClass());
//    }
    Skills->append((RegisteredSkill) {
        name, Skill, info
    });
    return true;
}

int CSkills::skillsCount() {
    return Skills->count();
}

CSkill* CSkills::skill(int i) {
    if (i >= 0 && i < skillsCount()) {
        return (*Skills)[i].Skill;
    }
    return nullptr;
}
