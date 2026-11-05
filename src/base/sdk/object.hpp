#pragma once
#include "../mapps/mappings_manager.h"
#include "mJNI/mJNI.h"
#include "mJNI/jvm/Structs.hpp"


template<auto klassName>
class object {
private:
    jobject mObject = nullptr;
    static inline jclass mKlass = nullptr;
public:
    object(jobject instance) {
        this->mObject = instance;
    }

    bool isValid() {
        return this->mObject != nullptr;
    }

    static jclass getKlass() {
        if (!mKlass) {
            mKlass = mappings::find_minecraft_klass(klassName);
            if (!mKlass) {
                return mJNI::findClass(klassName);
            }
        }
        return mKlass;
    }

    jobject getObject() const {
        return this->mObject;
    }

    jobject getFieldObject(const jfieldID fieldId) const {
        return  mJNI::getObjectField(this->mObject, fieldId) ;
    }

    operator jobject() const { return mObject; }

    explicit operator bool() const { return mObject != nullptr; }
    bool operator!() const { return mObject == nullptr; }

    bool operator==(std::nullptr_t) const { return mObject == nullptr; }
    bool operator!=(std::nullptr_t) const { return mObject != nullptr; }
};
