/*
 Copyright (C) 2012-2014 Soomla Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */


#include "CCChallenge.h"
#include "CCDomainHelper.h"
#include "CCLevelUpConsts.h"
#include "CCSoomlaUtils.h"
#include "CCLevelUpEventDispatcher.h"
#include "CCMissionStorage.h"

namespace soomla {

    USING_NS_CC;

    #define TAG "SOOMLA Challenge"

    CCChallenge *CCChallenge::create(cocos2d::__String *id, cocos2d::__String *name, cocos2d::__Array *missions, cocos2d::__Array *rewards) {
        CCChallenge *ret = new CCChallenge();
        if (ret->init(id, name, missions, rewards)) {
            ret->autorelease();
        }
        else {
            CC_SAFE_DELETE(ret);
        }

        return ret;
    }

    bool CCChallenge::init(cocos2d::__String *id, cocos2d::__String *name, cocos2d::__Array *missions, cocos2d::__Array *rewards) {
        bool result = CCMission::init(id, name, rewards);
        if (result) {
            setMissions(missions);
            return true;
        }
        return result;
    }

    bool CCChallenge::initWithDictionary(cocos2d::__Dictionary *dict) {
        bool result = CCMission::initWithDictionary(dict);
        if (result) {
            Ref *ref = dict->objectForKey(CCLevelUpConsts::JSON_LU_MISSIONS);
            if (ref) {
                __Array *missionsDict = dynamic_cast<__Array *>(ref);
                setMissions(CCDomainHelper::getInstance()->getDomainsFromDictArray(missionsDict));
            }
            return true;
        }
        return result;
    }

    cocos2d::__Dictionary *CCChallenge::toDictionary() {
        __Dictionary *dict = CCMission::toDictionary();

        if (mMissions) {
            dict->setObject(CCDomainHelper::getInstance()->getDictArrayFromDomains(mMissions), CCLevelUpConsts::JSON_LU_MISSIONS);
        }

        return dict;
    }

    const char *CCChallenge::getType() const {
        return CCLevelUpConsts::JSON_JSON_TYPE_CHALLENGE;
    }

    CCChallenge::~CCChallenge() {
        CC_SAFE_RELEASE(mMissions);
    }

    bool CCChallenge::isCompleted() {
        // could happen in construction
        // need to return false in order to register for child events
        if (mMissions == NULL || mMissions->count() == 0) {
            return false;
        }

        Ref *ref;
        CCMission *mission;
        CCARRAY_FOREACH(mMissions, ref) {
                mission = dynamic_cast<CCMission *>(ref);
                if (!mission->isCompleted()) {
                    return false;
                }
            }

        return true;
    }

    CCChallengeEventHandler *CCChallengeEventHandler::create(CCChallenge *challenge) {
        CCChallengeEventHandler *ret = new CCChallengeEventHandler();
        ret->autorelease();
        ret->mChallenge = challenge;
        return ret;
    }

    void CCChallenge::registerEvents() {
        CCSoomlaUtils::logDebug (TAG, "registerEvents called");
        if (!this->isCompleted()) {
            CCSoomlaUtils::logDebug (TAG, "registering!");
            // register for events
            setEventHandler(CCChallengeEventHandler::create(this));
            CCLevelUpEventDispatcher::getInstance()->addEventHandler(getEventHandler());
        }
    }

    void CCChallenge::unregisterEvents() {
        if (getEventHandler()) {
            CCLevelUpEventDispatcher::getInstance()->removeEventHandler(getEventHandler());
            setEventHandler(NULL);
        }
    }

    void CCChallengeEventHandler::onMissionCompleted(CCMission *completedMission) {
        CCSoomlaUtils::logDebug (TAG, "onMissionCompleted");
        if (mChallenge->mMissions->containsObject(completedMission)) {
            CCSoomlaUtils::logDebug (TAG, __String::createWithFormat("Mission %s is part of challenge %s (%zi) total",
                    completedMission->getId()->getCString(),
                    mChallenge->getId()->getCString(),
                    mChallenge->mMissions->count())->getCString());
            bool completed = true;
            Ref *ref;
            CCMission *mission;
            CCARRAY_FOREACH(mChallenge->mMissions, ref) {
                    mission = dynamic_cast<CCMission *>(ref);
                    CC_ASSERT(mission);
                    if (!mission->isCompleted()) {
                        CCSoomlaUtils::logDebug(TAG, __String::createWithFormat(
                                "challenge mission not completed?=%s",
                                mission->getId()->getCString()
                        )->getCString());
                        completed = false;
                        break;
                    }
                }

            if (completed) {
                CCSoomlaUtils::logDebug(TAG, __String::createWithFormat(
                        "Challenge %s completed!", mChallenge->getId()->getCString())->getCString());
                mChallenge->setCompletedInner(true);
            }
        }
    }

    void CCChallengeEventHandler::onMissionCompletionRevoked(CCMission *mission) {
        if (mChallenge->mMissions->containsObject(mission)) {
            // if the challenge was completed before, but now one of its child missions
            // was uncompleted - the challenge is revoked as well
            if (CCMissionStorage::getInstance()->isCompleted(mChallenge)) {
                mChallenge->setCompletedInner(false);
            }
        }
    }

}
