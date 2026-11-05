#pragma once
//
// Created by natty on 22.07.2026.
//

#include "object.hpp"
#include "../mapps/client_parser.h"
#include "../mapps/mappings_manager.h"
#include "../mapps/mapping_parser.h"


#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cmath>

class LocalPlayer;

static constexpr char JAVA_STRING[] = "java/lang/String";
class String : public object<JAVA_STRING>{

private:
    static inline jfieldID valuesID = 0;

public:
    String(jobject instance) : object<JAVA_STRING>(instance) {}

    static void updateCache() {
        valuesID = mJNI::findFieldID(getKlass(), "value", "[B");
    }

    std::string toString() {
        if (*this == nullptr) {
            return " ";
        }

        jobject values = getFieldObject(valuesID);
        if (!values) return " ";

        int size = mJNI::getArrayLength(values);
        if (size <= 0) return "";

        std::vector<jbyte> bytes(size);
        mJNI::getArrayElement<jbyte>(values, bytes.data(), 0, size);

        return std::string((char*)bytes.data(), size);
    }
};

class ItemStack;

static constexpr char JAVA_LIST[] = "java/util/List";
class List : public object<JAVA_LIST>
{
public:
    List(jobject instance) : object<JAVA_LIST>(instance) {};
    static void updateCache() {}

    jobject get(int index) {
        if (!*this || (uintptr_t)getObject() < 0x10000) return nullptr;

        jclass klass = mJNI::getObjectClass(*this);
        if (!klass || (uintptr_t)klass < 0x10000) return nullptr;

        struct CachedListField {
            jfieldID fieldID = (jfieldID)0;
            uintptr_t directOffset = 0;
            bool found = false;
        };
        static std::unordered_map<uintptr_t, CachedListField> sListCache;
        static std::mutex sListMutex;

        CachedListField cEntry;
        {
            std::lock_guard<std::mutex> lock(sListMutex);
            auto it = sListCache.find((uintptr_t)klass);
            if (it == sListCache.end()) {
                CachedListField entry;
                jfieldID elID = mJNI::findFieldID(klass, "elementData", "[Ljava/lang/Object;");
                if (!elID) elID = mJNI::findFieldID(klass, "a", "[Ljava/lang/Object;");
                if (elID) {
                    entry.fieldID = elID;
                    entry.found = true;
                } else {
                    for (uintptr_t off = 0x8; off <= 0x80; off += 4) {
                        jobject fieldObj = mJNI::getObjectField(*this, (jfieldID)off);
                        if (fieldObj && (uintptr_t)fieldObj >= 0x10000) {
                            jclass fKlass = mJNI::getObjectClass(fieldObj);
                            if (fKlass && (uintptr_t)fKlass >= 0x10000) {
                                std::string cName = mJNI::getClassName(fKlass);
                                if (!cName.empty() && cName[0] == '[') {
                                    entry.directOffset = off;
                                    entry.found = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                sListCache[(uintptr_t)klass] = entry;
                cEntry = entry;
            } else {
                cEntry = it->second;
            }
        }

        if (!cEntry.found) return nullptr;

        jobject arrObj = nullptr;
        if (cEntry.fieldID) {
            arrObj = mJNI::getObjectField(*this, cEntry.fieldID);
        } else if (cEntry.directOffset) {
            arrObj = mJNI::getObjectField(*this, (jfieldID)cEntry.directOffset);
        }

        if (arrObj && (uintptr_t)arrObj >= 0x10000) {
            int len = mJNI::getArrayLength(arrObj);
            if (index >= 0 && index < len) {
                jobject element = nullptr;
                mJNI::getObjectArrayElement(arrObj, &element, index, 1);
                if (element && (uintptr_t)element >= 0x10000) return element;
            }
        }

        return nullptr;
    }
};

static constexpr char YARN_NONNULLLIST[] = "net/minecraft/util/collection/DefaultedList";
class NonNullList : public object<YARN_NONNULLLIST> {

private:
    static inline jfieldID listID = 0;

public:
    NonNullList(jobject instance) : object<YARN_NONNULLLIST>(instance) {}

    static void updateCache() {
        jclass klass = mappings::find_minecraft_klass("net/minecraft/util/collection/DefaultedList");
        listID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/util/collection/DefaultedList",
            "delegate",
            "Ljava/util/List;");
    }

    List list() {
        if (!listID) updateCache();
        jobject listObj = listID ? getFieldObject(listID) : nullptr;
        return {listObj};
    }

    jobject get(int index) {
        if (!*this) return nullptr;
        if (!listID) updateCache();

        jobject innerList = listID ? getFieldObject(listID) : nullptr;
        if (innerList) {
            List l(innerList);
            return l.get(index);
        }
        return nullptr;
    }
};

static constexpr char YARN_VEC3[] = "net/minecraft/util/math/Vec3d";
class Vec3 : public object<YARN_VEC3> {

private:
    static inline jfieldID xID = 0;
    static inline jfieldID yID = 0;
    static inline jfieldID zID = 0;

public:
    Vec3() : object<YARN_VEC3>(nullptr) {}
    Vec3(jobject instance) : object<YARN_VEC3>(instance) {}

    static void updateCache() {
        jclass klass = mappings::find_minecraft_klass("net/minecraft/util/math/Vec3d");
        xID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/util/math/Vec3d", "x", "D");
        yID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/util/math/Vec3d", "y", "D");
        zID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/util/math/Vec3d", "z", "D");
    }

    [[nodiscard]] double getX() {
        if (!xID) updateCache();
        return mJNI::getField<double>(*this, xID);
    }

    [[nodiscard]] double getY() {
        if (!yID) updateCache();
        return mJNI::getField<double>(*this, yID);
    }

    [[nodiscard]] double getZ() {
        if (!zID) updateCache();
        return mJNI::getField<double>(*this, zID);
    }

    void setX(double x) {
        if (!xID) updateCache();
        mJNI::setField<double>(*this, xID, x);
    }

    void setY(double y) {
        if (!yID) updateCache();
        mJNI::setField<double>(*this, yID, y);
    }

    void setZ(double z) {
        if (!zID) updateCache();
        mJNI::setField<double>(*this, zID, z);
    }
};

static constexpr char YARN_AABB[] = "net/minecraft/util/math/Box";
class AABB : public object<YARN_AABB> {

private:
    static inline jfieldID minXID = 0;
    static inline jfieldID minYID = 0;
    static inline jfieldID minZID = 0;

    static inline jfieldID maxXID = 0;
    static inline jfieldID maxYID = 0;
    static inline jfieldID maxZID = 0;

public:
    AABB(jobject instance) : object<YARN_AABB>(instance) {}

    static void updateCache() {
        jclass klass = mappings::find_minecraft_klass("net/minecraft/util/math/Box");

        minXID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/util/math/Box", "minX", "D");
        minYID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/util/math/Box", "minY", "D");
        minZID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/util/math/Box", "minZ", "D");

        maxXID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/util/math/Box", "maxX", "D");
        maxYID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/util/math/Box", "maxY", "D");
        maxZID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/util/math/Box", "maxZ", "D");
    }

    jdouble minX() { if (!minXID) updateCache(); return mJNI::getField<double>(*this, minXID); }
    jdouble minY() { if (!minYID) updateCache(); return mJNI::getField<double>(*this, minYID); }
    jdouble minZ() { if (!minZID) updateCache(); return mJNI::getField<double>(*this, minZID); }

    jdouble maxX() { if (!maxXID) updateCache(); return mJNI::getField<double>(*this, maxXID); }
    jdouble maxY() { if (!maxYID) updateCache(); return mJNI::getField<double>(*this, maxYID); }
    jdouble maxZ() { if (!maxZID) updateCache(); return mJNI::getField<double>(*this, maxZID); }

    jdouble getMinX() { return minX(); }
    jdouble getMinY() { return minY(); }
    jdouble getMinZ() { return minZ(); }

    jdouble getMaxX() { return maxX(); }
    jdouble getMaxY() { return maxY(); }
    jdouble getMaxZ() { return maxZ(); }

    void setMinX(jdouble val) { if (!minXID) updateCache(); mJNI::setField<double>(*this, minXID, val); }
    void setMinY(jdouble val) { if (!minYID) updateCache(); mJNI::setField<double>(*this, minYID, val); }
    void setMinZ(jdouble val) { if (!minZID) updateCache(); mJNI::setField<double>(*this, minZID, val); }

    void setMaxX(jdouble val) { if (!maxXID) updateCache(); mJNI::setField<double>(*this, maxXID, val); }
    void setMaxY(jdouble val) { if (!maxYID) updateCache(); mJNI::setField<double>(*this, maxYID, val); }
    void setMaxZ(jdouble val) { if (!maxZID) updateCache(); mJNI::setField<double>(*this, maxZID, val); }
};

static constexpr char YARN_ENTITY_KLASS[] = "net/minecraft/entity/Entity";
class Entity : public object<YARN_ENTITY_KLASS> {

private:
    static inline jclass livingEntityKlass = nullptr;
    static inline jclass dataTrackerEntryKlass = nullptr;
    static inline jclass floatKlass = nullptr;
    static inline jclass strKlass = nullptr;

    static inline jfieldID vec3ID = 0;
    static inline jfieldID aabbID = 0;

    static inline jfieldID yRotID = 0;
    static inline jfieldID xRotID = 0;

    static inline jfieldID yRotOID = 0;
    static inline jfieldID xRotOID = 0;

    static inline jfieldID xOID = 0;
    static inline jfieldID yOID = 0;
    static inline jfieldID zOID = 0;

    static inline jfieldID fallDistanceID = 0;
    static inline jfieldID eyeHeightID = 0;
    static inline jfieldID onGroundID = 0;

    static inline jfieldID deltaMovementID = 0;
    static inline jfieldID wasTouchingWaterID = 0;

    static inline jfieldID hurtTimeID = 0;
    static inline jfieldID hurtDurationID = 0;
    static inline jfieldID healthID = 0;

    static inline jfieldID dataTrackerID = 0;
    static inline jfieldID entriesID = 0;
    static inline jfieldID entryValueID = 0;

    static inline jfieldID healthAccessorID = 0;
    static inline jfieldID entityDataID = 0;
    static inline jfieldID itemsByIdID = 0;
    static inline jfieldID entryDataID = 0;
    static inline jfieldID dataItemValueID = 0;
    static inline jfieldID floatValueID = 0;

public:
    Entity(jobject instance) : object<YARN_ENTITY_KLASS>(instance) {}

    static void updateCache() {
        jclass klass = mappings::find_minecraft_klass("net/minecraft/entity/Entity");
        livingEntityKlass = mappings::find_minecraft_klass("net/minecraft/entity/LivingEntity");

        vec3ID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "pos", "Lnet/minecraft/util/math/Vec3d;");
        aabbID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "boundingBox", "Lnet/minecraft/util/math/Box;");

        yRotID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "yaw", "F");
        xRotID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "pitch", "F");

        yRotOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "lastYaw", "F");
        xRotOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "lastPitch", "F");

        xOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "prevX", "D");
        if (!xOID) xOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "lastRenderX", "D");
        if (!xOID) xOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "field_6038", "D");
        if (!xOID) xOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "field_28637", "D");
        if (!xOID) xOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "field_1390", "D");

        yOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "prevY", "D");
        if (!yOID) yOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "lastRenderY", "D");
        if (!yOID) yOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "field_6035", "D");
        if (!yOID) yOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "field_28638", "D");
        if (!yOID) yOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "field_1391", "D");

        zOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "prevZ", "D");
        if (!zOID) zOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "lastRenderZ", "D");
        if (!zOID) zOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "field_6037", "D");
        if (!zOID) zOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "field_28639", "D");
        if (!zOID) zOID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "field_1392", "D");

        fallDistanceID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "fallDistance", "F");
        eyeHeightID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "standingEyeHeight", "F");
        onGroundID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "onGround", "Z");

        deltaMovementID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "velocity", "Lnet/minecraft/util/math/Vec3d;");
        wasTouchingWaterID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "touchingWater", "Z");

        if (livingEntityKlass) {
            hurtTimeID = mappings::find_minecraft_field_with_owner_klass(livingEntityKlass, "net/minecraft/entity/LivingEntity", "hurtTime", "I");
            hurtDurationID = mappings::find_minecraft_field_with_owner_klass(livingEntityKlass, "net/minecraft/entity/LivingEntity", "maxHurtTime", "I");
            healthID = mappings::find_minecraft_field_with_owner_klass(livingEntityKlass, "net/minecraft/entity/LivingEntity", "health", "F");

            healthAccessorID = mappings::find_minecraft_field_with_owner_klass(livingEntityKlass, "net/minecraft/entity/LivingEntity", "HEALTH", "Lnet/minecraft/entity/data/TrackedData;");
            if (!healthAccessorID) healthAccessorID = mappings::find_minecraft_field_with_owner_klass(livingEntityKlass, "net/minecraft/entity/LivingEntity", "DATA_HEALTH_ID", nullptr);
        }

        strKlass = mJNI::findClass("java/lang/String");
        jclass trackerKlass = mappings::find_minecraft_klass("net/minecraft/entity/data/DataTracker");
        dataTrackerEntryKlass = mappings::find_minecraft_klass("net/minecraft/entity/data/DataTracker$Entry");
        floatKlass = mJNI::findClass("java/lang/Float");

        if (klass) dataTrackerID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "dataTracker", nullptr);
        if (!dataTrackerID && klass) dataTrackerID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/Entity", "entityData", nullptr);
        entityDataID = dataTrackerID;

        if (trackerKlass) entriesID = mappings::find_minecraft_field_with_owner_klass(trackerKlass, "net/minecraft/entity/data/DataTracker", "entries", nullptr);
        if (!entriesID && trackerKlass) entriesID = mappings::find_minecraft_field_with_owner_klass(trackerKlass, "net/minecraft/entity/data/DataTracker", "itemsById", nullptr);
        itemsByIdID = entriesID;

        if (dataTrackerEntryKlass) {
            entryValueID = mappings::find_minecraft_field_with_owner_klass(dataTrackerEntryKlass, "net/minecraft/entity/data/DataTracker$Entry", "value", nullptr);
            dataItemValueID = entryValueID;
            entryDataID = mappings::find_minecraft_field_with_owner_klass(dataTrackerEntryKlass, "net/minecraft/entity/data/DataTracker$Entry", "data", "Lnet/minecraft/entity/data/TrackedData;");
        }

        if (floatKlass) floatValueID = mJNI::findFieldID(floatKlass, "value", "F");
    }

    [[nodiscard]] Vec3 position()  {
        if (!vec3ID) updateCache();
        return {mJNI::getObjectField(*this, vec3ID)};
    }

    [[nodiscard]] AABB bb()  {
        if (!aabbID) updateCache();
        return {getFieldObject(aabbID)};
    }

    jfloat getYaw() {
        if (!yRotID) updateCache();
        return mJNI::getField<float>(*this, yRotID);
    }
    jfloat getPitch() {
        if (!xRotID) updateCache();
        return mJNI::getField<float>(*this, xRotID);
    }

    void setYaw(float yaw) {
        if (!yRotID) updateCache();
        mJNI::setField<float>(*this, yRotID, yaw);
    }

    void setPitch(float pitch) {
        if (!xRotID) updateCache();
        mJNI::setField<float>(*this, xRotID, pitch);
    }

    jfloat getPrevPitch() {
        if (!xRotOID) updateCache();
        return mJNI::getField<float>(*this, xRotOID);
    }

    jfloat getPrevYaw() {
        if (!yRotOID) updateCache();
        return mJNI::getField<float>(*this, yRotOID);
    }

    void setPrevYaw(float yaw) {
        if (!yRotOID) updateCache();
        mJNI::setField<float>(*this, yRotOID, yaw);
    }

    void setPrevPitch(float pitch) {
        if (!xRotOID) updateCache();
        mJNI::setField<float>(*this, xRotOID, pitch);
    }

    jboolean isOnGround() {
        if (!onGroundID) updateCache();
        return mJNI::getField<bool>(*this, onGroundID);
    }
    void setOnGround(bool var) {
        if (!onGroundID) updateCache();
        mJNI::setField<bool>(*this, onGroundID, var);
    }

    jdouble fallDistance() {
        if (!fallDistanceID) updateCache();
        return mJNI::getField<float>(*this, fallDistanceID);
    }

    jdouble getLastX() {
        if (!xOID) updateCache();
        return mJNI::getField<double>(*this, xOID);
    }

    jdouble getLastY() {
        if (!yOID) updateCache();
        return mJNI::getField<double>(*this, yOID);
    }

    jdouble getLastZ() {
        if (!zOID) updateCache();
        return mJNI::getField<double>(*this, zOID);
    }

    jdouble getPrevX() { return getLastX(); }
    jdouble getPrevY() { return getLastY(); }
    jdouble getPrevZ() { return getLastZ(); }

    struct EntityRenderPos { double x, y, z; };

    static inline uint64_t sLastTickTimeMs = 0;

    static void markTickTime() {
        sLastTickTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    static float getPartialTicks() {
        if (sLastTickTimeMs == 0) return 1.0f;
        uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        float pt = static_cast<float>(now - sLastTickTimeMs) / 50.0f;
        return std::clamp(pt, 0.0f, 1.0f);
    }

    EntityRenderPos getRenderPos(float pt = -1.0f) {
        if (pt < 0.0f) pt = getPartialTicks();
        double lx = getLastX();
        double ly = getLastY();
        double lz = getLastZ();
        Vec3 cur = position();
        double cx = cur.getX();
        double cy = cur.getY();
        double cz = cur.getZ();
        if (lx == 0.0 || std::abs(cx - lx) > 10.0) lx = cx;
        if (ly == 0.0 || std::abs(cy - ly) > 10.0) ly = cy;
        if (lz == 0.0 || std::abs(cz - lz) > 10.0) lz = cz;
        return {
            lx + (cx - lx) * pt,
            ly + (cy - ly) * pt,
            lz + (cz - lz) * pt
        };
    }

    jfloat getEyeHeight() {
        if (!eyeHeightID) updateCache();
        return mJNI::getField<float>(*this, eyeHeightID);
    }

    Vec3 deltaMovement() {
        if (!deltaMovementID) updateCache();
        return { mJNI::getObjectField(*this, deltaMovementID) };
    }
    void setDeltaMovement(Vec3 var) {
        if (!deltaMovementID) updateCache();
        mJNI::setObjectField(*this, deltaMovementID, var);
    }

    jboolean isInWater() {
        if (!wasTouchingWaterID) updateCache();
        return mJNI::getField<bool>(*this, wasTouchingWaterID);
    }

    jint hurtTime() {
        if (!hurtTimeID) updateCache();
        return mJNI::getField<int>(*this, hurtTimeID);
    }

    std::string getCustomNameText() {
        if (!*this) return "";
        if (!dataTrackerID) updateCache();

        jobject entityData = dataTrackerID ? mJNI::getObjectField(*this, dataTrackerID) : nullptr;
        if (!entityData) return "";

        jobject itemsArray = entriesID ? mJNI::getObjectField(entityData, entriesID) : nullptr;
        if (!itemsArray) return "";

        int arrayLen = mJNI::getArrayLength(itemsArray);
        if (arrayLen > 2) {
            jobject dataItem = nullptr;
            mJNI::getObjectArrayElement(itemsArray, &dataItem, 2, 1);
            if (dataItem) {
                jobject optText = entryValueID ? mJNI::getObjectField(dataItem, entryValueID) : nullptr;
                if (optText && strKlass) {
                    for (uintptr_t off = 0x8; off <= 0x40; off += 4) {
                        jobject val = mJNI::getObjectField(optText, (jfieldID)off);
                        if (val) {
                            if (mJNI::instanceOf(val, strKlass)) {
                                return String(val).toString();
                            }
                            for (uintptr_t innerOff = 0x8; innerOff <= 0x30; innerOff += 4) {
                                jobject innerVal = mJNI::getObjectField(val, (jfieldID)innerOff);
                                if (innerVal && mJNI::instanceOf(innerVal, strKlass)) {
                                    return String(innerVal).toString();
                                }
                            }
                        }
                    }
                }
            }
        }
        return "";
    }

    jfloat getHealth() {
        if (!*this) return 20.0f;
        if (!healthAccessorID) updateCache();

        if (livingEntityKlass && healthAccessorID && entityDataID && itemsByIdID && floatKlass && floatValueID) {
            jobject healthAccessor = mJNI::getStaticObjectField(livingEntityKlass, healthAccessorID);
            if (healthAccessor) {
                jobject dataTracker = mJNI::getObjectField(*this, entityDataID);
                if (dataTracker) {
                    jobjectArray entriesArr = (jobjectArray)mJNI::getObjectField(dataTracker, itemsByIdID);
                    if (entriesArr) {
                        int arrLen = mJNI::getArrayLength(entriesArr);
                        for (int i = 0; i < arrLen; ++i) {
                            jobject dataItem = nullptr;
                            mJNI::getObjectArrayElement(entriesArr, &dataItem, i, 1);
                            if (!dataItem) continue;

                            jobject entryDataObj = nullptr;
                            if (entryDataID) {
                                entryDataObj = mJNI::getObjectField(dataItem, entryDataID);
                            }

                            if (!entryDataObj) {
                                for (uintptr_t off = 0x8; off <= 0x30; off += 4) {
                                    jobject child = mJNI::getObjectField(dataItem, (jfieldID)off);
                                    if (child && mJNI::isSameObject(child, healthAccessor)) {
                                        entryDataObj = child;
                                        break;
                                    }
                                }
                            }

                            if (entryDataObj && mJNI::isSameObject(entryDataObj, healthAccessor)) {
                                jobject boxedFloat = nullptr;
                                if (dataItemValueID) {
                                    boxedFloat = mJNI::getObjectField(dataItem, dataItemValueID);
                                }
                                if (!boxedFloat) {
                                    for (uintptr_t off = 0x8; off <= 0x30; off += 4) {
                                        jobject child = mJNI::getObjectField(dataItem, (jfieldID)off);
                                        if (child && child != entryDataObj && mJNI::instanceOf(child, floatKlass)) {
                                            boxedFloat = child;
                                            break;
                                        }
                                    }
                                }
                                if (boxedFloat) {
                                    float hp = mJNI::getField<float>(boxedFloat, floatValueID);
                                    if (hp >= 0.0f && hp <= 1024.0f) {
                                        return hp;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return 20.0f;
    }

    jfloat getMaxHealth() {
        if (!*this) return 20.0f;
        return 20.0f;
    }

    jboolean isAlive() {
        if (!*this) return false;
        return getHealth() > 0.0f;
    }

    jdouble getHeight() {
        if (!*this) return 1.8;
        AABB box = bb();
        if (!box.getObject()) return 1.8;
        double h = box.getMaxY() - box.getMinY();
        if (h <= 0.1 || h > 10.0) return 1.8;
        return h;
    }
};

static constexpr char YARN_GAMEPROFILE[] = "com/mojan/authlib/GameProfile";
class GameProfile : public object<YARN_GAMEPROFILE> {
private:
    static inline jfieldID nameID = 0;

public:
    GameProfile(jobject instance) : object<YARN_GAMEPROFILE>(instance) {}

    static void updateCache() {
        jclass klass = mJNI::findClass("com/mojang/authlib/GameProfile");
        if (klass) {
            nameID = mJNI::findFieldID(klass, "name", "Ljava/lang/String;");
            if (!nameID) {
                nameID = mappings::find_minecraft_field_with_owner_klass(klass, "com/mojang/authlib/GameProfile", "name", "Ljava/lang/String;");
            }
        }
    }

    String name() {
        if (!*this) return {nullptr};
        if (!nameID) updateCache();

        jobject nameObj = getFieldObject(nameID);
        if (nameObj) return {nameObj};
        return {nullptr};
    }
};

static constexpr char YARN_INVENTORY_ITEM[] = "net/minecraft/item/Item";
class Item : public object<YARN_INVENTORY_ITEM> {
private:
    static inline jfieldID descriptionID = 0;

public:
    Item(jobject instance) : object<YARN_INVENTORY_ITEM>(instance) {}

    static void updateCache() {
        jclass klass = mappings::find_minecraft_klass("net/minecraft/item/Item");
        descriptionID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/item/Item",
            "translationKey",
            "Ljava/lang/String;");
    }

    String description() {
        if (!descriptionID) updateCache();
        return {getFieldObject(descriptionID)};
    }

    int getItemCooldown() {
        String desc = description();
        if (!desc.getObject()) return 250;
        std::string name = desc.toString();

        static const std::unordered_map<std::string, int> cooldowns = {
            {"item.minecraft.wooden_sword", 625},
            {"item.minecraft.stone_sword", 625},
            {"item.minecraft.iron_sword", 625},
            {"item.minecraft.golden_sword", 625},
            {"item.minecraft.diamond_sword", 625},
            {"item.minecraft.netherite_sword", 625},

            {"item.minecraft.wooden_axe", 1250},
            {"item.minecraft.stone_axe", 1250},
            {"item.minecraft.iron_axe", 1111},
            {"item.minecraft.golden_axe", 1000},
            {"item.minecraft.diamond_axe", 1000},
            {"item.minecraft.netherite_axe", 1000},

            {"item.minecraft.mace", 1667},
            {"item.minecraft.trident", 909},
        };

        auto it = cooldowns.find(name);
        if (it != cooldowns.end()) {
            return it->second;
        }

        return 250;
    }
};

static constexpr char YARN_MOB_KLASS[] = "net/minecraft/entity/mob/MobEntity";
class Mob : public object<YARN_MOB_KLASS> {
public:
    Mob(jobject instance) : object<YARN_MOB_KLASS>(instance) {}
    static void updateCache() {}
};

static constexpr char YARN_INVENTORY_ITEMSTACK[] = "net/minecraft/item/ItemStack";
class ItemStack : public object<YARN_INVENTORY_ITEMSTACK> {
private:
    static inline jfieldID itemID = 0;
    static inline jfieldID damageID = 0;

    static inline jfieldID compFieldID = 0;
    static inline jfieldID changedFieldID = 0;
    static inline jfieldID keyFieldID = 0;
    static inline jfieldID valFieldID = 0;
    static inline jfieldID optValFieldID = 0;
    static inline jfieldID intValFieldID = 0;
    static inline jobject damageCompTypeObj = nullptr;

public:
    ItemStack() : object<YARN_INVENTORY_ITEMSTACK>(nullptr) {}
    ItemStack(jobject instance) : object<YARN_INVENTORY_ITEMSTACK>(instance) {}

    static void updateCache() {
        jclass klass = mappings::find_minecraft_klass("net/minecraft/item/ItemStack");
        itemID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/item/ItemStack",
            "item",
            "Lnet/minecraft/item/Item;");

        damageID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/item/ItemStack",
            "damage",
            "I");

        if (klass) {
            compFieldID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/item/ItemStack", "components", "Lnet/minecraft/component/MergedComponentMap;");
        }
        jclass compClass = mappings::find_minecraft_klass("net/minecraft/component/MergedComponentMap");
        if (compClass) {
            changedFieldID = mappings::find_minecraft_field_with_owner_klass(compClass, "net/minecraft/component/MergedComponentMap", "changedComponents", "Lit/unimi/dsi/fastutil/objects/Reference2ObjectMap;");
        }
        jclass typeClass = mappings::find_minecraft_klass("net/minecraft/component/DataComponentTypes");
        if (typeClass) {
            jfieldID damageTypeFieldID = mappings::find_minecraft_field_with_owner_klass(typeClass, "net/minecraft/component/DataComponentTypes", "DAMAGE", "Lnet/minecraft/component/ComponentType;");
            if (damageTypeFieldID) {
                damageCompTypeObj = mJNI::getStaticObjectField(typeClass, damageTypeFieldID);
            }
        }
        jclass optClass = mJNI::findClass("java/util/Optional");
        if (optClass) {
            optValFieldID = mJNI::findFieldID(optClass, "value", "Ljava/lang/Object;");
        }
        jclass intClass = mJNI::findClass("java/lang/Integer");
        if (intClass) {
            intValFieldID = mJNI::findFieldID(intClass, "value", "I");
        }
    }
    Item item() {
        if (!itemID) updateCache();
        jobject itemObj = getFieldObject(itemID);
        return itemObj;
    }

    int getDamageValue() {
        if (!*this) return 0;
        if (!compFieldID && !damageID) updateCache();

        if (compFieldID && changedFieldID && damageCompTypeObj && optValFieldID && intValFieldID) {
            jobject compObj = getFieldObject(compFieldID);
            if (compObj) {
                jobject mapObj = mJNI::getObjectField(compObj, changedFieldID);
                if (mapObj) {
                    jclass mapKlass = mJNI::getObjectClass(mapObj);
                    if (mapKlass) {
                        if (!keyFieldID) keyFieldID = mJNI::findFieldID(mapKlass, "key", "[Ljava/lang/Object;");
                        if (!valFieldID) valFieldID = mJNI::findFieldID(mapKlass, "value", "[Ljava/lang/Object;");

                        if (keyFieldID && valFieldID) {
                            jobject keysArr = mJNI::getObjectField(mapObj, keyFieldID);
                            jobject valsArr = mJNI::getObjectField(mapObj, valFieldID);
                            if (keysArr && valsArr) {
                                int len = mJNI::getArrayLength(keysArr);
                                for (int i = 0; i < len; ++i) {
                                    jobject kObj = nullptr;
                                    mJNI::getObjectArrayElement(keysArr, &kObj, i, 1);
                                    if (kObj && mJNI::isSameObject(kObj, damageCompTypeObj)) {
                                        jobject optObj = nullptr;
                                        mJNI::getObjectArrayElement(valsArr, &optObj, i, 1);
                                        if (optObj) {
                                            jobject intObj = mJNI::getObjectField(optObj, optValFieldID);
                                            if (intObj) {
                                                return mJNI::getField<int>(intObj, intValFieldID);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!damageID) updateCache();
        if (damageID) return mJNI::getField<int>(*this, damageID);
        return 0;
    }
};

static constexpr char YARN_ITEM_ENTITY_KLASS[] = "net/minecraft/entity/ItemEntity";
class ItemEntity : public Entity {
private:
    static inline jfieldID stackID = 0;

public:
    ItemEntity(jobject instance) : Entity(instance) {}

    static void updateCache() {
        jclass klass = mappings::find_minecraft_klass("net/minecraft/entity/ItemEntity");
        if (klass) {
            stackID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/ItemEntity", "stack", "Lnet/minecraft/item/ItemStack;");
            if (!stackID) stackID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/entity/ItemEntity", "itemStack", "Lnet/minecraft/item/ItemStack;");
        }
    }

    ItemStack getItem() {
        if (!*this) return {nullptr};
        if (!stackID) updateCache();

        jobject stackObj = getFieldObject(stackID);
        if (stackObj) return {stackObj};
        return {nullptr};
    }
};

static constexpr char YARN_EQUIPMENT_SLOT_KLASS[] = "net/minecraft/entity/EquipmentSlot";
class EquipmentSlot : public object<YARN_EQUIPMENT_SLOT_KLASS> {
private:
    static inline jclass equipmentSlotKlass = nullptr;
    static inline jfieldID headID = 0;
    static inline jfieldID chestID = 0;
    static inline jfieldID legsID = 0;
    static inline jfieldID feetID = 0;
    static inline jfieldID offhandID = 0;
    static inline jfieldID mainhandID = 0;

public:
    EquipmentSlot(jobject instance) : object<YARN_EQUIPMENT_SLOT_KLASS>(instance) {}

    static void updateCache() {
        equipmentSlotKlass = mappings::find_minecraft_klass("net/minecraft/entity/EquipmentSlot");
        if (equipmentSlotKlass) {
            headID = mappings::find_minecraft_field_with_owner_klass(equipmentSlotKlass, "net/minecraft/entity/EquipmentSlot", "HEAD", "Lnet/minecraft/entity/EquipmentSlot;");
            chestID = mappings::find_minecraft_field_with_owner_klass(equipmentSlotKlass, "net/minecraft/entity/EquipmentSlot", "CHEST", "Lnet/minecraft/entity/EquipmentSlot;");
            legsID = mappings::find_minecraft_field_with_owner_klass(equipmentSlotKlass, "net/minecraft/entity/EquipmentSlot", "LEGS", "Lnet/minecraft/entity/EquipmentSlot;");
            feetID = mappings::find_minecraft_field_with_owner_klass(equipmentSlotKlass, "net/minecraft/entity/EquipmentSlot", "FEET", "Lnet/minecraft/entity/EquipmentSlot;");
            offhandID = mappings::find_minecraft_field_with_owner_klass(equipmentSlotKlass, "net/minecraft/entity/EquipmentSlot", "OFFHAND", "Lnet/minecraft/entity/EquipmentSlot;");
            mainhandID = mappings::find_minecraft_field_with_owner_klass(equipmentSlotKlass, "net/minecraft/entity/EquipmentSlot", "MAINHAND", "Lnet/minecraft/entity/EquipmentSlot;");
        }
    }

    static jobject getHead() { if (!headID) updateCache(); return (equipmentSlotKlass && headID) ? mJNI::getStaticObjectField(equipmentSlotKlass, headID) : nullptr; }
    static jobject getChest() { if (!chestID) updateCache(); return (equipmentSlotKlass && chestID) ? mJNI::getStaticObjectField(equipmentSlotKlass, chestID) : nullptr; }
    static jobject getLegs() { if (!legsID) updateCache(); return (equipmentSlotKlass && legsID) ? mJNI::getStaticObjectField(equipmentSlotKlass, legsID) : nullptr; }
    static jobject getFeet() { if (!feetID) updateCache(); return (equipmentSlotKlass && feetID) ? mJNI::getStaticObjectField(equipmentSlotKlass, feetID) : nullptr; }
    static jobject getOffhand() { if (!offhandID) updateCache(); return (equipmentSlotKlass && offhandID) ? mJNI::getStaticObjectField(equipmentSlotKlass, offhandID) : nullptr; }
    static jobject getMainhand() { if (!mainhandID) updateCache(); return (equipmentSlotKlass && mainhandID) ? mJNI::getStaticObjectField(equipmentSlotKlass, mainhandID) : nullptr; }
};

static constexpr char YARN_INVENTORY_KLASS[] = "net/minecraft/entity/player/PlayerInventory";
class Inventory : public object<YARN_INVENTORY_KLASS> {
public:
    static inline jfieldID selectedID = 0;
    static inline jfieldID itemsID = 0;
    static inline jfieldID armorID = 0;
    static inline jfieldID offhandID = 0;
    static inline jfieldID equipmentID = 0;

    static inline jfieldID livingEqID = 0;
    static inline jfieldID mapID = 0;
    static inline jfieldID valsID = 0;
    static inline jclass itemStackKlass = nullptr;

public:
    Inventory(jobject instance) : object<YARN_INVENTORY_KLASS>(instance) {}

    static void updateCache() {
        jclass klass = mappings::find_minecraft_klass("net/minecraft/entity/player/PlayerInventory");

        selectedID = mappings::find_minecraft_field_with_owner_klass(
            klass, "net/minecraft/entity/player/PlayerInventory", "selectedSlot", "I");

        itemsID = mappings::find_minecraft_field_with_owner_klass(
            klass, "net/minecraft/entity/player/PlayerInventory", "main", nullptr);

        armorID = mappings::find_minecraft_field_with_owner_klass(
            klass, "net/minecraft/entity/player/PlayerInventory", "armor", nullptr);

        offhandID = mappings::find_minecraft_field_with_owner_klass(
            klass, "net/minecraft/entity/player/PlayerInventory", "offHand", nullptr);

        equipmentID = mappings::find_minecraft_field_with_owner_klass(
            klass, "net/minecraft/entity/player/PlayerInventory", "equipment", nullptr);

        jclass livingKlass = mappings::find_minecraft_klass("net/minecraft/entity/LivingEntity");
        if (livingKlass) {
            livingEqID = mappings::find_minecraft_field_with_owner_klass(livingKlass, "net/minecraft/entity/LivingEntity", "equipment", nullptr);
        }
        jclass eqKlass = mappings::find_minecraft_klass("net/minecraft/entity/EntityEquipment");
        if (eqKlass) mapID = mappings::find_minecraft_field_with_owner_klass(eqKlass, "net/minecraft/entity/EntityEquipment", "map", nullptr);

        jclass enumMapKlass = mJNI::findClass("java/util/EnumMap");
        if (enumMapKlass) valsID = mJNI::findFieldID(enumMapKlass, "vals", "[Ljava/lang/Object;");

        itemStackKlass = mappings::find_minecraft_klass("net/minecraft/item/ItemStack");
    }

    int selected() {
        if (!selectedID) updateCache();
        return mJNI::getField<int>(*this, selectedID);
    }

    void setSelected(int index) {
        if (!selectedID) updateCache();
        mJNI::setField(*this, selectedID, index);
    }

    NonNullList items() {
        if (!itemsID) updateCache();
        return {getFieldObject(itemsID)};
    }

    NonNullList armor() {
        if (!armorID && !equipmentID) updateCache();
        if (armorID) return {getFieldObject(armorID)};
        return {nullptr};
    }

    NonNullList offhand() {
        if (!offhandID && !equipmentID) updateCache();
        if (offhandID) return {getFieldObject(offhandID)};
        return {nullptr};
    }

    jobject getEquipmentStackFromEnumMap(int slotOrdinal) {
        if (!equipmentID) updateCache();
        if (!equipmentID) return nullptr;

        jobject eqObj = getFieldObject(equipmentID);
        if (!eqObj || (uintptr_t)eqObj < 0x10000) return nullptr;

        if (mapID && valsID && itemStackKlass) {
            jobject enumMapObj = mJNI::getObjectField(eqObj, mapID);
            if (enumMapObj && (uintptr_t)enumMapObj >= 0x10000) {
                jobject valsArr = mJNI::getObjectField(enumMapObj, valsID);
                if (valsArr && (uintptr_t)valsArr >= 0x10000) {
                    int len = mJNI::getArrayLength(valsArr);
                    if (slotOrdinal >= 0 && slotOrdinal < len) {
                        jobject itemObj = nullptr;
                        mJNI::getObjectArrayElement(valsArr, &itemObj, slotOrdinal, 1);
                        if (itemObj && (uintptr_t)itemObj >= 0x10000 && mJNI::instanceOf(itemObj, itemStackKlass)) {
                            return itemObj;
                        }
                    }
                }
            }
        }
        return nullptr;
    }

    ItemStack getOffhandStack() {
        NonNullList offList = offhand();
        if (offList.getObject()) {
            jobject itemObj = offList.get(0);
            if (itemObj) return {itemObj};
        }
        jobject mapStackObj = getEquipmentStackFromEnumMap(1);
        if (mapStackObj) return {mapStackObj};
        return {nullptr};
    }

    ItemStack getArmorStack(int armorSlotIndex) { // 0=Feet, 1=Legs, 2=Chest, 3=Head
        NonNullList armList = armor();
        if (armList.getObject()) {
            jobject itemObj = armList.get(armorSlotIndex);
            if (itemObj) return {itemObj};
        }
        jobject mapStackObj = getEquipmentStackFromEnumMap(2 + armorSlotIndex);
        if (mapStackObj) return {mapStackObj};
        return {nullptr};
    }
};

inline jobject getLivingEquipmentStack(jobject entityObj, int slotOrdinal) {
    if (!entityObj || (uintptr_t)entityObj < 0x10000) return nullptr;

    if (!Inventory::livingEqID) Inventory::updateCache();

    if (Inventory::livingEqID && Inventory::mapID && Inventory::valsID && Inventory::itemStackKlass) {
        jobject eqObj = mJNI::getObjectField(entityObj, Inventory::livingEqID);
        if (eqObj && (uintptr_t)eqObj >= 0x10000) {
            jobject enumMapObj = mJNI::getObjectField(eqObj, Inventory::mapID);
            if (enumMapObj && (uintptr_t)enumMapObj >= 0x10000) {
                jobject valsArr = mJNI::getObjectField(enumMapObj, Inventory::valsID);
                if (valsArr && (uintptr_t)valsArr >= 0x10000) {
                    int len = mJNI::getArrayLength(valsArr);
                    if (slotOrdinal >= 0 && slotOrdinal < len) {
                        jobject itemObj = nullptr;
                        mJNI::getObjectArrayElement(valsArr, &itemObj, slotOrdinal, 1);
                        if (itemObj && (uintptr_t)itemObj >= 0x10000 && mJNI::instanceOf(itemObj, Inventory::itemStackKlass)) {
                            return itemObj;
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}

class Player : public Entity {
private:
    static inline jclass playerKlass = nullptr;
    static inline jfieldID gameProfileID = 0;
    static inline jfieldID inventoryID = 0;

    // for health
    static inline jclass entityLivingKlass = nullptr;
    static inline jclass SynchedEntityDataKlass = nullptr;
    static inline jclass entitydataaccessor = nullptr;
    static inline jclass entityKlass = nullptr;
    static inline jclass dataItemKlass = nullptr;
    static inline jclass floatKlass = nullptr;

    static inline jfieldID accessorHealthID = 0;
    static inline jfieldID itemsById = 0;
    static inline jfieldID entityDataID = 0;
    static inline jfieldID dataItemValueFieldID = 0;
    static inline jfieldID floatValueField = 0;
    static inline jfieldID accessorIdFieldID = 0;

public:
    Player(jobject instance) : Entity(instance) {}

    static void updateCache() {
        playerKlass = mappings::find_minecraft_klass("net/minecraft/entity/player/PlayerEntity");
        entityKlass = mappings::find_minecraft_klass("net/minecraft/entity/Entity");

        gameProfileID = mappings::find_minecraft_field_with_owner_klass(
            playerKlass,
            "net/minecraft/entity/player/PlayerEntity",
            "gameProfile",
            "Lcom/mojang/authlib/GameProfile;");

        inventoryID = mappings::find_minecraft_field_with_owner_klass(
            playerKlass,
            "net/minecraft/entity/player/PlayerEntity",
            "inventory",
            "Lnet/minecraft/entity/player/PlayerInventory;");

        entityLivingKlass = mappings::find_minecraft_klass("net/minecraft/entity/LivingEntity");
        SynchedEntityDataKlass = mappings::find_minecraft_klass("net/minecraft/entity/data/DataTracker");
        entitydataaccessor = mappings::find_minecraft_klass("net/minecraft/entity/data/TrackedData");
        dataItemKlass = mappings::find_minecraft_klass("net/minecraft/entity/data/DataTracker$Entry");

        accessorHealthID = mappings::find_minecraft_field_with_owner_klass(
            entityLivingKlass,
            "net/minecraft/entity/LivingEntity",
            "HEALTH",
            "Lnet/minecraft/entity/data/TrackedData;");
        if (!accessorHealthID) {
            accessorHealthID = mappings::find_minecraft_field_with_owner_klass(
                entityLivingKlass,
                "net/minecraft/entity/LivingEntity",
                "DATA_HEALTH_ID",
                "Lnet/minecraft/entity/data/TrackedData;");
        }

        itemsById = mappings::find_minecraft_field_with_owner_klass(
            SynchedEntityDataKlass,
            "net/minecraft/entity/data/DataTracker",
            "entries",
            "[Lnet/minecraft/entity/data/DataTracker$Entry;");
        if (!itemsById) {
            itemsById = mappings::find_minecraft_field_with_owner_klass(
                SynchedEntityDataKlass,
                "net/minecraft/entity/data/DataTracker",
                "itemsById",
                "[Lnet/minecraft/entity/data/DataTracker$Entry;");
        }


        entityDataID = mappings::find_minecraft_field_with_owner_klass(
            entityKlass,
            "net/minecraft/entity/Entity",
            "dataTracker",
            "Lnet/minecraft/entity/data/DataTracker;");
        if (!entityDataID) {
            entityDataID = mappings::find_minecraft_field_with_owner_klass(
                entityKlass,
                "net/minecraft/entity/Entity",
                "entityData",
                "Lnet/minecraft/entity/data/DataTracker;");
        }


        dataItemValueFieldID = mappings::find_minecraft_field_with_owner_klass(
            dataItemKlass,
            "net/minecraft/entity/data/DataTracker$Entry",
            "value",
            "Ljava/lang/Object;");

        floatKlass = mJNI::findClass("java/lang/Float");
        if (floatKlass) {
            floatValueField = mJNI::findFieldID(floatKlass, "value", "F");
        }

        if (entitydataaccessor) {
            accessorIdFieldID = mappings::find_minecraft_field_with_owner_klass(
                entitydataaccessor,
                "net/minecraft/entity/data/TrackedData",
                "id",
                "I");
        }
    }

    GameProfile gameProfile() {
        if (!*this) return {nullptr};
        if (!gameProfileID) updateCache();

        jobject gpObj = getFieldObject(gameProfileID);
        if (gpObj) return {gpObj};
        return {nullptr};
    }

    Inventory inventory() {
        if (!inventoryID) updateCache();
        return { getFieldObject(inventoryID) };
    }

    ItemStack getArmorStack(int armorSlotIndex) {
        Inventory pInv = inventory();
        if (pInv.getObject()) {
            ItemStack stack = pInv.getArmorStack(armorSlotIndex);
            if (stack.getObject() && (uintptr_t)stack.getObject() >= 0x10000) return stack;
        }
        jobject itemObj = getLivingEquipmentStack(getObject(), 2 + armorSlotIndex);
        if (itemObj && (uintptr_t)itemObj >= 0x10000) return {itemObj};
        return {nullptr};
    }

    ItemStack getOffhandStack() {
        Inventory pInv = inventory();
        if (pInv.getObject()) {
            ItemStack stack = pInv.getOffhandStack();
            if (stack.getObject() && (uintptr_t)stack.getObject() >= 0x10000) return stack;
        }
        jobject itemObj = getLivingEquipmentStack(getObject(), 1);
        if (itemObj && (uintptr_t)itemObj >= 0x10000) return {itemObj};
        return {nullptr};
    }

    float health() {
        return 20.0f;
    }
};

static constexpr char YARN_HITRESULT[] = "net/minecraft/util/hit/EntityHitResult";
class EntityHitResult : public object<YARN_HITRESULT> {
private:
    static inline jfieldID entityID = 0;

public:
    EntityHitResult(jobject instance) : object<YARN_HITRESULT>(instance) {}

    static void updateCache() {
        jclass klass = mappings::find_minecraft_klass("net/minecraft/util/hit/EntityHitResult");
        entityID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/util/hit/EntityHitResult",
            "entity",
            "Lnet/minecraft/entity/Entity;");
    }

    Entity getEntity() {
        if (!entityID) updateCache();
        return {getFieldObject(entityID)};
    }
};

static constexpr char YARN_REMOTEPLAYER[] = "net/minecraft/client/network/OtherClientPlayerEntity";
class RemotePlayer : public object<YARN_REMOTEPLAYER> {
public:
    RemotePlayer(jobject instance) : object(instance) {}
};

static constexpr char YARN_SCREEN[] = "net/minecraft/client/gui/screen/Screen";
class Screen : public object<YARN_SCREEN> {
public:
    Screen(jobject instance) : object(instance) {}
};

static constexpr char YARN_CHATSCREEN[] = "net/minecraft/client/gui/screen/ChatScreen";
class ChatScreen : public object<YARN_CHATSCREEN> {
public:
    ChatScreen(jobject instance) : object(instance) {}
};

static constexpr char YARN_INVENTORYSCREEN[] = "net/minecraft/client/gui/screen/ingame/InventoryScreen";
class InventoryScreen : public object<YARN_INVENTORYSCREEN> {
public:
    InventoryScreen(jobject instance) : object(instance) {}
};

static constexpr char YARN_ABSTRACTCONTAINERSCREEN[] = "net/minecraft/client/gui/screen/ingame/HandledScreen";
class AbstractContainerScreen : public object<YARN_ABSTRACTCONTAINERSCREEN> {
public:
    AbstractContainerScreen(jobject instance) : object(instance) {}
};

static constexpr char YARN_KEYMAPPING[] = "net/minecraft/client/option/KeyBinding";
class KeyMapping : public object<YARN_KEYMAPPING> {
private:
    static inline jfieldID optionsID = 0;
    static inline jclass optionsKlass = nullptr;
    static inline jclass mcKlass = nullptr;
    static inline jfieldID mcInstanceID = 0;
    static inline jfieldID mcOptionsID = 0;
    static inline jfieldID clickCountID = 0;
    static inline jfieldID isDownID = 0;

public:
    KeyMapping(jobject instance) : object(instance) {}

    static void updateCache() {
        optionsKlass = mappings::find_minecraft_klass("net/minecraft/client/option/GameOptions");
        mcKlass = mappings::find_minecraft_klass("net/minecraft/client/MinecraftClient");

        if (mcKlass) {
            mcInstanceID = mappings::find_minecraft_field_with_owner_klass(mcKlass, "net/minecraft/client/MinecraftClient", "instance", "Lnet/minecraft/client/MinecraftClient;");
            mcOptionsID = mappings::find_minecraft_field_with_owner_klass(mcKlass, "net/minecraft/client/MinecraftClient", "options", "Lnet/minecraft/client/option/GameOptions;");
        }

        clickCountID = mappings::find_minecraft_field_with_owner_klass(
            getKlass(),
            "net/minecraft/client/option/KeyBinding",
            "timesPressed",
            "I");

        isDownID = mappings::find_minecraft_field_with_owner_klass(
            getKlass(),
            "net/minecraft/client/option/KeyBinding",
            "pressed",
            "Z");
    }

    static KeyMapping getKeyMapping(const char* name) {
        if (!optionsKlass || !mcKlass) updateCache();
        if (!mcKlass || !mcInstanceID || !mcOptionsID || !optionsKlass) return {nullptr};

        jobject mcObj = mJNI::getStaticObjectField(mcKlass, mcInstanceID);
        if (!mcObj) return {nullptr};

        jobject optionsOBJ = mJNI::getObjectField(mcObj, mcOptionsID);
        if (!optionsOBJ) return {nullptr};

        jfieldID keyID = mappings::find_minecraft_field_with_owner_klass(
            optionsKlass,
            "net/minecraft/client/option/GameOptions",
            name,
            "Lnet/minecraft/client/option/KeyBinding;");
        return { mJNI::getObjectField(optionsOBJ, keyID) };
    }

    jint clickCount() {
        if (!clickCountID) updateCache();
        return mJNI::getField<int>(*this, clickCountID);
    }

    void setClickCount(int count) {
        if (!clickCountID) updateCache();
        mJNI::setField(*this, clickCountID, count);
    }

    jboolean isDown() {
        if (!isDownID) updateCache();
        return mJNI::getField<jboolean>(*this, isDownID);
    }

    void setDown(jboolean var) {
        if (!isDownID) updateCache();
        mJNI::setField<jboolean>(*this, isDownID, var);
    }

    void setIsDown(bool isDown) {
        setDown(isDown);
    }

    void incrementClick() {
        if (!clickCountID) updateCache();
        mJNI::setField<jint>(*this, clickCountID, mJNI::getField<jint>(*this, clickCountID) + 1);
    }
};

class LocalPlayer : public Player {
private:
    static inline jclass localPlayerKlass = nullptr;
    static inline jclass livingEntityKlass = nullptr;

    static inline jfieldID swingingID = 0;
    static inline jfieldID deltaMovementID = 0;
    static inline jfieldID noJumpDelayID = 0;
    static inline jfieldID sprintingID = 0;

public:
    LocalPlayer(jobject instance) : Player(instance) {}

    static void updateCache() {
        localPlayerKlass = mappings::find_minecraft_klass("net/minecraft/client/network/ClientPlayerEntity");
        livingEntityKlass = mappings::find_minecraft_klass("net/minecraft/entity/LivingEntity");

        swingingID = mappings::find_minecraft_field_with_owner_klass(
           livingEntityKlass,
           "net/minecraft/entity/LivingEntity",
           "handSwinging",
           "Z");

        deltaMovementID = mappings::find_minecraft_field_with_owner_klass(
            localPlayerKlass,
            "net/minecraft/client/network/ClientPlayerEntity",
            "velocity",
            "Lnet/minecraft/util/math/Vec3d;");

        noJumpDelayID = mappings::find_minecraft_field_with_owner_klass(
            livingEntityKlass,
            "net/minecraft/entity/LivingEntity",
            "jumpingCooldown",
            "I");

        jclass entityKlass = mappings::find_minecraft_klass("net/minecraft/entity/Entity");
        if (entityKlass) {
            sprintingID = mappings::find_minecraft_field_with_owner_klass(entityKlass, "net/minecraft/entity/Entity", "sprinting", "Z");
        }
    }

    bool canAttack() const {
        if (!swingingID) const_cast<LocalPlayer*>(this)->updateCache();
        auto swinging = mJNI::getField<jboolean>(*this, swingingID);
        return !swinging;
    }

    jboolean swinging() {
        if (!swingingID) updateCache();
        return mJNI::getField<jboolean>(*this, swingingID);
    }

    Vec3 deltaMovement() {
        return Entity::deltaMovement();
    }

    bool isFallen() {
        auto jumpKey = KeyMapping::getKeyMapping("jumpKey");
        auto y = deltaMovement().getY();

        if (y > -1 && y < -1.14) {
            return false;
        }
        return y <= -0.1 || jumpKey.isDown();
    }

    bool isSprint() {
        if (KeyMapping::getKeyMapping("sprintKey").isDown()) return true;
        if (KeyMapping::getKeyMapping("forwardKey").isDown()) return true;
        return false;
    }

    void setSprint(bool var) {
        if (!sprintingID) updateCache();
        if (sprintingID) {
            mJNI::setField<jboolean>(*this, sprintingID, var);
        }
        KeyMapping::getKeyMapping("sprintKey").setDown(var);
    }

    bool isSneak() {
        return KeyMapping::getKeyMapping("sneakKey").isDown();
    }

    void setSneak(bool var) {
        KeyMapping::getKeyMapping("sneakKey").setDown(var);
    }

    void setNoJumpDelay(bool var) {
        if (!noJumpDelayID) updateCache();
        mJNI::setField<jint>(*this, noJumpDelayID, var);
    }
};

static constexpr char YARN_CLIENTLEVEL[] = "net/minecraft/client/world/ClientWorld";
class ClientLevel : public object<YARN_CLIENTLEVEL> {
private:
    static inline jfieldID playersID = 0;
    static inline jfieldID blockEntityTickersID = 0;

public:
    ClientLevel(jobject instance) : object<YARN_CLIENTLEVEL>(instance) {}

    static void updateCache() {
        jclass klass = mappings::find_minecraft_klass("net/minecraft/client/world/ClientWorld");
        playersID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/world/ClientWorld",
            "players",
            "Ljava/util/List;");

        blockEntityTickersID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/world/ClientWorld",
            "blockEntityTickers",
            "Ljava/util/List;");
    }

    std::vector<Player> players() {
        if (!playersID) updateCache();
        jobject listObj = getFieldObject(playersID);
        if (!listObj) return {};

        List playersList(listObj);
        std::vector<Player> result;

        for (int i = 0; i < 300; ++i) {
            jobject playerObj = playersList.get(i);
            if (!playerObj) break;
            result.push_back(Player(playerObj));
        }

        return result;
    }

    std::vector<Entity> entities() {
        if (!playersID) updateCache();
        jobject listObj = getFieldObject(playersID);
        if (!listObj) return {};

        List playersList(listObj);
        std::vector<Entity> result;

        for (int i = 0; i < 300; ++i) {
            jobject eObj = playersList.get(i);
            if (!eObj) break;
            result.push_back(Entity(eObj));
        }

        return result;
    }

    std::vector<jobject> blockEntities() {
        if (!*this || (uintptr_t)getObject() < 0x10000) return {};
        if (!blockEntityTickersID) updateCache();

        std::vector<jobject> res;

        if (blockEntityTickersID) {
            jobject listObj = getFieldObject(blockEntityTickersID);
            if (listObj && (uintptr_t)listObj >= 0x10000) {
                List tickersList(listObj);
                for (int i = 0; i < 2000; ++i) {
                    jobject elem = tickersList.get(i);
                    if (!elem) break;
                    res.push_back(elem);
                }
            }
        }

        if (!res.empty()) return res;

        auto extractFromContainer = [&](jobject containerObj) {
            if (!containerObj || (uintptr_t)containerObj < 0x10000) return;
            jclass cKlass = mJNI::getObjectClass(containerObj);
            if (!cKlass || (uintptr_t)cKlass < 0x10000) return;

            for (uintptr_t off = 0x8; off <= 0xA0; off += 4) {
                jobject arrObj = mJNI::getObjectField(containerObj, (jfieldID)off);
                if (arrObj && (uintptr_t)arrObj >= 0x10000) {
                    int len = mJNI::getArrayLength(arrObj);
                    if (len > 0 && len < 10000) {
                        for (int i = 0; i < len; ++i) {
                            jobject elem = nullptr;
                            mJNI::getObjectArrayElement(arrObj, &elem, i, 1);
                            if (elem && (uintptr_t)elem >= 0x10000) {
                                res.push_back(elem);
                            }
                        }
                    }
                }
            }
        };

        for (uintptr_t off = 0x8; off <= 0x250; off += 4) {
            jobject fieldObj = mJNI::getObjectField(*this, (jfieldID)off);
            if (fieldObj && (uintptr_t)fieldObj >= 0x10000) {
                extractFromContainer(fieldObj);
                if (res.size() > 500) break;
            }
        }

        return res;
    }
};

static constexpr char YARN_OPTIONINSTANCE[] = "net/minecraft/client/option/SimpleOption";
class OptionInstance : public object<YARN_OPTIONINSTANCE> {
private:
    static inline jfieldID valueID = 0;
    static inline jfieldID integerValueID = 0;
    static inline jclass integerClass = nullptr;
    static inline jfieldID doubleValueID = 0;
    static inline jclass doubleClass = nullptr;

public:
    OptionInstance(jobject instance) : object<YARN_OPTIONINSTANCE>(instance) {}

    static void updateCache() {
        integerClass = mJNI::findClass("java/lang/Integer");
        if (integerClass) {
            integerValueID = mJNI::findFieldID(integerClass, "value", "I");
        }

        doubleClass = mJNI::findClass("java/lang/Double");
        if (doubleClass) {
            doubleValueID = mJNI::findFieldID(doubleClass, "value", "D");
        }

        jclass klass = mappings::find_minecraft_klass("net/minecraft/client/option/SimpleOption");
        valueID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/option/SimpleOption",
            "value",
            "Ljava/lang/Object;");
    }

    object getValue() {
        if (!valueID) updateCache();
        return {getFieldObject(valueID)};
    }

    jint getInt() {
        if (!valueID || !integerValueID) updateCache();
        jobject obj = getValue();
        if (!obj) return 70;
        if (integerValueID) {
            jint val = mJNI::getField<jint>(obj, integerValueID);
            if (val >= 30 && val <= 160) return val;
        }
        return 70;
    }

    jdouble getDouble() {
        if (!valueID || !doubleValueID) updateCache();
        jobject obj = getValue();
        if (!obj) return 0.5;
        if (doubleValueID) {
            return mJNI::getField<jdouble>(obj, doubleValueID);
        }
        return 0.5;
    }

    void setInt(int val) {
    }
};

static constexpr char YARN_OPTIONS[] = "net/minecraft/client/option/GameOptions";
class Options : public object<YARN_OPTIONS> {
private:
    static inline jfieldID keyMappingsID = 0;
    static inline jfieldID fovID = 0;
    static inline jfieldID mouseSensitivityID = 0;

public:
    Options(jobject instance) : object<YARN_OPTIONS>(instance) {}

    static void updateCache() {
        jclass klass = mappings::find_minecraft_klass("net/minecraft/client/option/GameOptions");

        keyMappingsID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/option/GameOptions",
            "allKeys",
            "[Lnet/minecraft/client/option/KeyBinding;");

        fovID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/option/GameOptions",
            "fov",
            "Lnet/minecraft/client/option/SimpleOption;");

        mouseSensitivityID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/option/GameOptions",
            "mouseSensitivity",
            "Lnet/minecraft/client/option/SimpleOption;");
    }

    OptionInstance fov() {
        if (!fovID) updateCache();
        return {getFieldObject(fovID)};
    }

    OptionInstance sensitivity() {
        if (!mouseSensitivityID) updateCache();
        return {getFieldObject(mouseSensitivityID)};
    }
};

static constexpr char YARN_CAMERA[] = "net/minecraft/client/render/Camera";
class Camera : public object<YARN_CAMERA> {
private:
    static inline jfieldID positionID = 0;
    static inline jfieldID xRotID = 0;
    static inline jfieldID yRotID = 0;

public:
    Camera(jobject instance) : object<YARN_CAMERA>(instance) {}

    static void updateCache() {
        jclass klass = mappings::find_minecraft_klass("net/minecraft/client/render/Camera");

        positionID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/render/Camera",
            "pos",
            "Lnet/minecraft/util/math/Vec3d;");

        xRotID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/render/Camera",
            "pitch",
            "F");

        yRotID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/render/Camera",
            "yaw",
            "F");
    }

    Vec3 getPosition() {
        if (!positionID) updateCache();
        return {mJNI::getObjectField(*this, positionID)};
    }

    jfloat xRotation() {
        if (!xRotID) updateCache();
        return mJNI::getField<float>(*this, xRotID);
    }

    jfloat yRotation() {
        if (!yRotID) updateCache();
        return mJNI::getField<float>(*this, yRotID);
    }
};

static constexpr char YARN_GAMERENDERER[] = "net/minecraft/client/render/GameRenderer";
class GameRenderer : public object<YARN_GAMERENDERER> {
private:
    static inline jfieldID mainCameraID = 0;

public:
    GameRenderer(jobject instance) : object<YARN_GAMERENDERER>(instance) {}

    static void updateCache() {
        jclass klass = mappings::find_minecraft_klass("net/minecraft/client/render/GameRenderer");

        mainCameraID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/render/GameRenderer",
            "camera",
            "Lnet/minecraft/client/render/Camera;");
    }

    Camera mainCamera() {
        if (!mainCameraID) updateCache();
        return {getFieldObject(mainCameraID)};
    }
};

static constexpr char YARN_WINDOW[] = "net/minecraft/client/util/Window";
class MinecraftWindow : public object<YARN_WINDOW> {
private:
    static inline jfieldID framebufferWidthID = 0;
    static inline jfieldID framebufferHeightID = 0;
    static inline jfieldID fullscreenID = 0;
    static inline jfieldID windowedXID = 0;
    static inline jfieldID windowedYID = 0;
    static inline jfieldID windowedWidthID = 0;
    static inline jfieldID windowedHeightID = 0;
    static inline jfieldID xID = 0;
    static inline jfieldID yID = 0;

public:
    MinecraftWindow(jobject instance) : object<YARN_WINDOW>(instance) {}

    static void updateCache() {
        jclass klass = mappings::find_minecraft_klass("net/minecraft/client/util/Window");

        framebufferWidthID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/client/util/Window", "framebufferWidth", "I");
        framebufferHeightID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/client/util/Window", "framebufferHeight", "I");
        fullscreenID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/client/util/Window", "fullscreen", "Z");
        windowedXID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/client/util/Window", "windowedX", "I");
        windowedYID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/client/util/Window", "windowedY", "I");
        windowedWidthID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/client/util/Window", "windowedWidth", "I");
        windowedHeightID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/client/util/Window", "windowedHeight", "I");
        xID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/client/util/Window", "x", "I");
        yID = mappings::find_minecraft_field_with_owner_klass(klass, "net/minecraft/client/util/Window", "y", "I");
    }

    int framebufferWidth() { if (!framebufferWidthID) updateCache(); return mJNI::getField<int>(*this, framebufferWidthID); }
    int framebufferHeight() { if (!framebufferHeightID) updateCache(); return mJNI::getField<int>(*this, framebufferHeightID); }
    bool fullscreen() { if (!fullscreenID) updateCache(); return mJNI::getField<jboolean>(*this, fullscreenID); }
    bool isFullscreen() { return fullscreen(); }
    void setFullscreen(bool val) { if (!fullscreenID) updateCache(); mJNI::setField<jboolean>(*this, fullscreenID, val); }
    void syncWindowedRect(int x = 0, int y = 0, int w = 0, int h = 0) {}
    int windowedX() { if (!windowedXID) updateCache(); return mJNI::getField<int>(*this, windowedXID); }
    int windowedY() { if (!windowedYID) updateCache(); return mJNI::getField<int>(*this, windowedYID); }
    int windowedWidth() { if (!windowedWidthID) updateCache(); return mJNI::getField<int>(*this, windowedWidthID); }
    int windowedHeight() { if (!windowedHeightID) updateCache(); return mJNI::getField<int>(*this, windowedHeightID); }
    int x() { if (!xID) updateCache(); return mJNI::getField<int>(*this, xID); }
    int y() { if (!yID) updateCache(); return mJNI::getField<int>(*this, yID); }
};

static constexpr char YARN_MINECRAFT_KLASS[] = "net/minecraft/client/MinecraftClient";
class Minecraft : public object<YARN_MINECRAFT_KLASS> {

private:
    static inline jfieldID instanceID = 0;

    static inline jfieldID levelID = 0;
    static inline jfieldID playerID = 0;
    static inline jfieldID hitResultID = 0;

    static inline jfieldID optionsID = 0;
    static inline jfieldID gameRendererID = 0;

    static inline jfieldID windowID = 0;
    static inline jfieldID screenID = 0;

    static inline jfieldID rightClickDelayID = 0;
    static inline jfieldID ticksID = 0;

    static inline jfieldID targetEntID = 0;

    static inline jfieldID fpsID = 0;
    static inline jfieldID fpsCounterID = 0;

    static inline jfieldID serverEntryID = 0;
    static inline jfieldID pingFieldID = 0;

    static inline jfieldID renderTickCounterID = 0;
    static inline jfieldID tickProgressID = 0;

public:
    Minecraft(jobject instance) : object<YARN_MINECRAFT_KLASS>(instance) {}

    static void updateCache() {
        jclass klass = getKlass();

        instanceID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/MinecraftClient",
            "instance",
            "Lnet/minecraft/client/MinecraftClient;");

        levelID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/MinecraftClient",
            "world",
            "Lnet/minecraft/client/world/ClientWorld;");

        playerID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/MinecraftClient",
            "player",
            "Lnet/minecraft/client/network/ClientPlayerEntity;");

        hitResultID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/MinecraftClient",
            "crosshairTarget",
            "Lnet/minecraft/util/hit/HitResult;");

        optionsID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/MinecraftClient",
            "options",
            "Lnet/minecraft/client/option/GameOptions;");

        gameRendererID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/MinecraftClient",
            "gameRenderer",
            "Lnet/minecraft/client/render/GameRenderer;");

        windowID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/MinecraftClient",
            "window",
            "Lnet/minecraft/client/util/Window;");

        screenID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/MinecraftClient",
            "currentScreen",
            "Lnet/minecraft/client/gui/screen/Screen;");

        rightClickDelayID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/MinecraftClient",
            "itemUseCooldown",
            "I");

        ticksID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/MinecraftClient",
            "ticks",
            "I");

        targetEntID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/MinecraftClient",
            "targetedEntity",
            "Lnet/minecraft/entity/Entity;");

        fpsID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/MinecraftClient",
            "currentFps",
            "I");

        fpsCounterID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/MinecraftClient",
            "fpsCounter",
            "I");

        serverEntryID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/MinecraftClient",
            "currentServerEntry",
            "Lnet/minecraft/client/network/ServerInfo;");
        if (!serverEntryID) {
            serverEntryID = mappings::find_minecraft_field_with_owner_klass(
                klass,
                "net/minecraft/client/MinecraftClient",
                "serverInfo",
                "Lnet/minecraft/client/network/ServerInfo;");
        }

        jclass serverInfoKlass = mappings::find_minecraft_klass("net/minecraft/client/network/ServerInfo");
        if (serverInfoKlass) {
            pingFieldID = mappings::find_minecraft_field_with_owner_klass(
                serverInfoKlass,
                "net/minecraft/client/network/ServerInfo",
                "ping",
                "J");
        }

        renderTickCounterID = mappings::find_minecraft_field_with_owner_klass(
            klass,
            "net/minecraft/client/MinecraftClient",
            "renderTickCounter",
            nullptr);
        if (!renderTickCounterID) {
            renderTickCounterID = mappings::find_minecraft_field_with_owner_klass(
                klass,
                "net/minecraft/client/MinecraftClient",
                "tickCounter",
                nullptr);
        }

        jclass rtcKlass = mappings::find_minecraft_klass("net/minecraft/client/render/RenderTickCounter");
        if (rtcKlass) {
            tickProgressID = mappings::find_minecraft_field_with_owner_klass(rtcKlass, "net/minecraft/client/render/RenderTickCounter", "tickProgress", "F");
            if (!tickProgressID) tickProgressID = mappings::find_minecraft_field_with_owner_klass(rtcKlass, "net/minecraft/client/render/RenderTickCounter", "tickDelta", "F");
            if (!tickProgressID) tickProgressID = mappings::find_minecraft_field_with_owner_klass(rtcKlass, "net/minecraft/client/render/RenderTickCounter", "lastFrameDuration", "F");
        }
    }

    static Minecraft getInstance() {
        if (!instanceID) updateCache();
        jobject instanceObj = mJNI::getStaticObjectField(getKlass(), instanceID);
        return {instanceObj};
    }

    ClientLevel level() {
        if (!levelID) updateCache();
        jobject levelObj = getFieldObject(levelID);
        return {levelObj};
    }

    LocalPlayer player() {
        if (!playerID) updateCache();
        jobject playerObj = getFieldObject(playerID);
        return {playerObj};
    }

    EntityHitResult hitResult() {
        if (!hitResultID) updateCache();
        jobject hitResultObj = getFieldObject(hitResultID);
        return {hitResultObj};
    }

    Entity targetedEntity() {
        if (!targetEntID) updateCache();
        if (targetEntID) {
            return {getFieldObject(targetEntID)};
        }
        return {nullptr};
    }

    static inline jobject sLastHitTargetObj = nullptr;
    static inline uint64_t sLastHitTime = 0;

    static void setLastHitTarget(jobject tObj) {
        if (tObj) {
            sLastHitTargetObj = tObj;
            sLastHitTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
        }
    }

    static Entity lastHitTarget() {
        if (sLastHitTargetObj) {
            uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            if (now - sLastHitTime < 10000) { // Keep last hit for 10 seconds
                return {sLastHitTargetObj};
            }
        }
        return {nullptr};
    }

    void setTargetedEntity(jobject targetObj) {
        if (!targetEntID) updateCache();
        if (targetEntID) {
            mJNI::setField<jobject>(*this, targetEntID, targetObj);
        }
    }

    Options options() {
        if (!optionsID) updateCache();
        jobject optionsObj = getFieldObject(optionsID);
        return {optionsObj};
    }

    GameRenderer gameRenderer() {
        if (!gameRendererID) updateCache();
        jobject gameRendererObj = getFieldObject(gameRendererID);
        return {gameRendererObj};
    }

    MinecraftWindow window() {
        if (!windowID) updateCache();
        jobject windowObj = getFieldObject(windowID);
        return {windowObj};
    }

    Screen screen() {
        if (!screenID) updateCache();
        return {getFieldObject(screenID)};
    }

    int rightClickDelay() {
        if (!rightClickDelayID) updateCache();
        return mJNI::getField<int>(*this, rightClickDelayID);
    }

    void setRightClickDelay(int value) {
        if (!rightClickDelayID) updateCache();
        mJNI::setField(*this, rightClickDelayID, value);
    }

    int getTicks() {
        if (!ticksID) updateCache();
        if (!ticksID) return 0;
        return mJNI::getField<int>(*this, ticksID);
    }

    int getFps(int fallback = 60) {
        if (!fpsID && !fpsCounterID) updateCache();
        jclass mcKlass = getKlass();
        if (mcKlass) {
            if (fpsID) {
                int f = mJNI::getStaticField<int>(mcKlass, fpsID);
                if (f > 0 && f <= 3000) return f;
                if (*this) {
                    f = mJNI::getField<int>(*this, fpsID);
                    if (f > 0 && f <= 3000) return f;
                }
            }

            if (fpsCounterID) {
                int f = mJNI::getStaticField<int>(mcKlass, fpsCounterID);
                if (f > 0 && f <= 3000) return f;
                if (*this) {
                    f = mJNI::getField<int>(*this, fpsCounterID);
                    if (f > 0 && f <= 3000) return f;
                }
            }
        }

        return fallback;
    }

    int getPing(int fallback = 0) {
        if (!serverEntryID || !pingFieldID) updateCache();
        if (!*this || (uintptr_t)getObject() < 0x10000) return fallback;

        if (serverEntryID && pingFieldID) {
            jobject serverObj = getFieldObject(serverEntryID);
            if (serverObj && (uintptr_t)serverObj >= 0x10000) {
                jlong p = mJNI::getField<jlong>(serverObj, pingFieldID);
                if (p >= 0 && p <= 2000) return static_cast<int>(p);
            }
        }

        return fallback;
    }

    float getRenderTickDelta() {
        if (!renderTickCounterID || !tickProgressID) updateCache();
        if (!*this || (uintptr_t)getObject() < 0x10000) return 1.0f;

        if (renderTickCounterID && tickProgressID) {
            jobject rtcObj = getFieldObject(renderTickCounterID);
            if (rtcObj && (uintptr_t)rtcObj >= 0x10000) {
                float td = mJNI::getField<float>(rtcObj, tickProgressID);
                if (td >= 0.0f && td <= 1.0f) return td;
            }
        }

        return Entity::getPartialTicks();
    }
};

namespace Cacher {

    inline void updateCache()
    {
        Minecraft::updateCache();
        ClientLevel::updateCache();
        Vec3::updateCache();
        Entity::updateCache();
        LocalPlayer::updateCache();
        Player::updateCache();
        AABB::updateCache();
        EntityHitResult::updateCache();

        Options::updateCache();
        KeyMapping::updateCache();
        OptionInstance::updateCache();
        GameRenderer::updateCache();
        Camera::updateCache();
        MinecraftWindow::updateCache();
        GameProfile::updateCache();
        String::updateCache();
        Inventory::updateCache();
        NonNullList::updateCache();
        List::updateCache();
        ItemStack::updateCache();
        Item::updateCache();
        ItemEntity::updateCache();
        Mob::updateCache();
    }
}

