//
// Created by natty on 09.10.2026.
//

#ifndef MULLO_MAPPINGS_MANAGER_H
#define MULLO_MAPPINGS_MANAGER_H
#include "mJNI/jvm/Structs.hpp"


namespace mappings {

    void init();

        jfieldID find_minecraft_field_with_owner_klass(jclass klass, const char* ownerClass, const char* fieldName, const char* fieldDesc);
        jmethodID find_minecraft_method_with_owner_klass(jclass klass, const char* ownerClass, const char* methodName, const char* methodDesc);

        jclass find_minecraft_klass(const char* name);
}


#endif //MULLO_MAPPINGS_MANAGER_H
