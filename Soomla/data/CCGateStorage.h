//
// Created by Shubin Fedor on 19/08/14.
// Copyright (c) 2014 SOOMLA. All rights reserved.
//


#ifndef __CCGateStorage_H_
#define __CCGateStorage_H_

#include "cocos2d.h"
#include "CCMission.h"

namespace soomla {
    class CCGateStorage: cocos2d::Ref {
    public:
        static CCGateStorage *getInstance();

        bool isOpen(CCGate *gate);
        void setOpen(CCGate *gate, bool open);
    };
}

#endif //__CCGateStorage_H_