#include "CollisionMesh.h"

void CollisionMesh::initializeMeshShape(
    const btTransform& transformation,
    _3DM::Model& model,
    unsigned int meshIndex,
    float mass,
    CollisionTag usrTag,
    bool trigger,
    btVector3 localInertia,
    float friction,
    float restitution,
    bool kinematic) {

    if (hasInit) {
        DBG_LOG("CollisionMesh already initialized!\n");
        deInit();
    }

    isMesh = true;

    //this allocates memory
    thisMesh = createTriangleMeshCollider(&model, meshIndex);

    if (!thisMesh) {
        DBG_LOG("Error creating triangle mesh. (CollisionMesh.cpp CollisionMesh::initialize())\n");

        isMesh = false;
        return;
    }

    thisShape = new btBvhTriangleMeshShape(thisMesh, true);

    thisShape->calculateLocalInertia(mass, localInertia);

    thisMotionState = new btDefaultMotionState(transformation);

    btCI collisionInfo = btCI(mass, thisMotionState, thisShape, localInertia);

    collisionInfo.m_friction = friction;

    collisionInfo.m_restitution = restitution;

    rigidBody = new btRigidBody(collisionInfo);

    thisTag = usrTag;

    rigidBody->setUserPointer(&thisTag);

    setTrigger(trigger);
    setKinematic(kinematic);
    hasInit = true;
}

void CollisionMesh::initializeModelShape(
    const btTransform& transformation,
    _3DM::Model& model,
    float mass,
    CollisionTag usrTag,
    bool trigger,
    btVector3 localInertia,
    float friction,
    float restitution,
    bool kinematic) {

    if (hasInit) {
        DBG_LOG("CollisionMesh already initialized!\n");
        deInit();
    }

    isMesh = true;

    //this allocates memory
    thisMesh = createTriangleModelCollider(&model);

    if (!thisMesh) {
        DBG_LOG("Error creating triangle mesh. (CollisionMesh.cpp CollisionMesh::initialize())\n");

        isMesh = false;
        return;
    }

    thisShape = new btBvhTriangleMeshShape(thisMesh, true);

    // Concave inertia not supported
    // thisShape->calculateLocalInertia(mass, localInertia);

    thisMotionState = new btDefaultMotionState(transformation);

    btCI collisionInfo = btCI(mass, thisMotionState, thisShape, localInertia);

    collisionInfo.m_friction = friction;

    collisionInfo.m_restitution = restitution;

    rigidBody = new btRigidBody(collisionInfo);

    thisTag = usrTag;

    rigidBody->setUserPointer(&thisTag);

    setTrigger(trigger);
    setKinematic(kinematic);
    hasInit = true;
}

void CollisionMesh::initializeMeshShape(const btTransform& transformation, _3DM::AnimatedModel& model, unsigned int meshIndex, float mass, CollisionTag usrTag, bool trigger, btVector3 localInertia, float friction, float restitution, bool kinematic) {

    if (hasInit) {
        DBG_LOG("CollisionMesh already initialized!\n");
        deInit();
    }

    isMesh = true;

    //this allocates memory
    thisMesh = createTriangleMeshCollider(&model, meshIndex);

    if (!thisMesh) {
        DBG_LOG("Error creating triangle mesh. (CollisionMesh.cpp CollisionMesh::initialize())\n");

        isMesh = false;
        return;
    }

    thisShape = new btBvhTriangleMeshShape(thisMesh, true);

    thisShape->calculateLocalInertia(mass, localInertia);

    thisMotionState = new btDefaultMotionState(transformation);

    btCI collisionInfo = btCI(mass, thisMotionState, thisShape, localInertia);

    collisionInfo.m_friction = friction;

    collisionInfo.m_restitution = restitution;

    rigidBody = new btRigidBody(collisionInfo);

    thisTag = usrTag;

    rigidBody->setUserPointer(&thisTag);

    setTrigger(trigger);
    setKinematic(kinematic);
    hasInit = true;
}

void CollisionMesh::initializeModelShape(const btTransform& transformation, _3DM::AnimatedModel& model, float mass, CollisionTag usrTag, bool trigger, btVector3 localInertia, float friction, float restitution, bool kinematic) {

    if (hasInit) {
        DBG_LOG("CollisionMesh already initialized!\n");
        deInit();
    }

    isMesh = true;

    //this allocates memory
    thisMesh = createTriangleModelCollider(&model);

    if (!thisMesh) {
        DBG_LOG("Error creating triangle mesh. (CollisionMesh.cpp CollisionMesh::initialize())\n");

        isMesh = false;
        return;
    }

    thisShape = new btBvhTriangleMeshShape(thisMesh, true);

    thisShape->calculateLocalInertia(mass, localInertia);

    thisMotionState = new btDefaultMotionState(transformation);

    btCI collisionInfo = btCI(mass, thisMotionState, thisShape, localInertia);

    collisionInfo.m_friction = friction;

    collisionInfo.m_restitution = restitution;

    rigidBody = new btRigidBody(collisionInfo);

    thisTag = usrTag;

    rigidBody->setUserPointer(&thisTag);

    setTrigger(trigger);
    setKinematic(kinematic);
    hasInit = true;
}

void CollisionMesh::setTrigger(bool t) {
    DBG_CHECK(rigidBody);
    if (t) {
        rigidBody->setCollisionFlags(
            rigidBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    } else {
        rigidBody->setCollisionFlags(
            rigidBody->getCollisionFlags()
            & ~btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }

    isTriggerBody = t;
}

void CollisionMesh::deInit() {
    DBG_CHECK(hasInit);
    DBG_CHECK(rigidBody != nullptr && thisMotionState != nullptr && thisShape != nullptr);

    delete rigidBody;
    delete thisMotionState;
    delete thisShape;

    rigidBody       = nullptr;
    thisMotionState = nullptr;
    thisShape       = nullptr;

    if (isMesh) {
        delete thisMesh;
        thisMesh = nullptr;
        isMesh   = false;
    }

    hasInit = false;
}

void CollisionMesh::setKinematic(bool t) {
    DBG_CHECK(rigidBody != nullptr);
    if (t) {
        rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    } else {
        rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
    }

    isKinematicBody = t;
}

Transform CollisionMesh::getTransformation() {
    DBG_CHECK(rigidBody != nullptr && thisShape != nullptr);
    return Transform(
        hh::toGlmVec3(rigidBody->getCenterOfMassPosition()),
        hh::toGlmQuat(rigidBody->getCenterOfMassTransform().getRotation()),
        hh::toGlmVec3(thisShape->getLocalScaling()));
}

CollisionMesh::~CollisionMesh() {

    deInit();
}

btTriangleMesh* CollisionMesh::createTriangleMeshCollider(_3DM::AnimatedModel* model, unsigned int meshIndex) {
    if (meshIndex >= model->amountOfMeshes()) {
        return nullptr;
    }

    //Allocate Memory
    btTriangleMesh* tempMesh = new btTriangleMesh(true, false);

    std::vector<uint32_t>* indices = model->getMeshIndices(meshIndex);
    std::vector<glm::vec3>* verts  = model->getMeshVertices(meshIndex);

    btVector3 vertexPos[3];

    for (unsigned int i = 0; i < indices->size(); i += 3) {
        for (unsigned int j = 0; j < 3; j++) {

            int index      = indices->at(i + j);
            glm::vec3& vec = verts->at(index);
            vertexPos[j].setX(vec.x);
            vertexPos[j].setY(vec.y);
            vertexPos[j].setZ(vec.z);
        }

        tempMesh->addTriangle(vertexPos[0], vertexPos[1], vertexPos[2], true);
    }

    return tempMesh;
}

btTriangleMesh* CollisionMesh::createTriangleModelCollider(_3DM::AnimatedModel* model) {
    if (model->amountOfMeshes() <= 0) {
        DBG_LOG("No Meshes in Supplied Model!! (CollisionMesh.cpp)\n");
        return nullptr;
    }

    //Allocate Memory
    btTriangleMesh* tempMesh = new btTriangleMesh(true, false);

    btVector3 vertexPos[3];

    for (unsigned int i = 0; i < model->amountOfMeshes(); i++) {
        std::vector<uint32_t>* indices = model->getMeshIndices(i);
        std::vector<glm::vec3>* verts  = model->getMeshVertices(i);

        for (unsigned int j = 0; j < indices->size(); j += 3) {
            for (unsigned int k = 0; k < 3; k++) {

                int index      = indices->at(j + k);
                glm::vec3& vec = verts->at(index);
                vertexPos[k].setX(vec.x);
                vertexPos[k].setY(vec.y);
                vertexPos[k].setZ(vec.z);
            }

            tempMesh->addTriangle(vertexPos[0], vertexPos[1], vertexPos[2], true);
        }
    }

    return tempMesh;
}

btTriangleMesh* CollisionMesh::createTriangleMeshCollider(_3DM::Model* model, unsigned int meshIndex) {

    if (meshIndex >= model->amountOfMeshes()) {
        return nullptr;
    }

    //Allocate Memory
    btTriangleMesh* tempMesh = new btTriangleMesh(true, false);

    std::vector<uint32_t>* indices = model->getMeshIndices(meshIndex);
    std::vector<glm::vec3>* verts  = model->getMeshVertices(meshIndex);

    btVector3 vertexPos[3];

    for (unsigned int i = 0; i < indices->size(); i += 3) {
        for (unsigned int j = 0; j < 3; j++) {

            int index      = indices->at(i + j);
            glm::vec3& vec = verts->at(index);
            vertexPos[j].setX(vec.x);
            vertexPos[j].setY(vec.y);
            vertexPos[j].setZ(vec.z);
        }

        tempMesh->addTriangle(vertexPos[0], vertexPos[1], vertexPos[2], true);
    }

    return tempMesh;
}

btTriangleMesh* CollisionMesh::createTriangleModelCollider(_3DM::Model* model) {
    if (model->amountOfMeshes() <= 0) {
        DBG_LOG("No Meshes in Supplied Model!! (CollisionMesh.cpp)\n");
        return nullptr;
    }

    //Allocate Memory
    btTriangleMesh* tempMesh = new btTriangleMesh(true, false);

    btVector3 vertexPos[3];

    for (unsigned int i = 0; i < model->amountOfMeshes(); i++) {
        std::vector<uint32_t>* indices = model->getMeshIndices(i);
        std::vector<glm::vec3>* verts  = model->getMeshVertices(i);

        for (unsigned int j = 0; j < indices->size(); j += 3) {
            for (unsigned int k = 0; k < 3; k++) {

                int index      = indices->at(j + k);
                glm::vec3& vec = verts->at(index);
                vertexPos[k].setX(vec.x);
                vertexPos[k].setY(vec.y);
                vertexPos[k].setZ(vec.z);
            }

            tempMesh->addTriangle(vertexPos[0], vertexPos[1], vertexPos[2], true);
        }
    }

    return tempMesh;
}
