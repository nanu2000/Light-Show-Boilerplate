#ifndef BONE_COLLISION_MESH_SYSTEM
#define BONE_COLLISION_MESH_SYSTEM
#include "AnimatedModel.h"
#include "BoneCollisionMesh.h"
#include "SystemBase.h"

class BoneCollisionMeshSystem : public SystemBase {
public:
    void fixedUpdate(BoneCollisionMesh& mesh, const _3DM::AnimatedModel& animatedModel);
};

#endif
