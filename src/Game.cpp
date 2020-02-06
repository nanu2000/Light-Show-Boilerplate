#include "Game.h"

void Game::resetVitals(Time* time, Messenger<BackEndMessages>* backEndMessagingSystem) {
    currentTime     = time;
    backEndMessages = backEndMessagingSystem;
}

bool Game::areVitalsNull() {
    if (!currentTime || !backEndMessages) {
        DBG_LOG("ONE OR MORE VITAL OBJECTS IS NULL\n (Game::isVitalsNull())\n");
        return true;
    }

    return false;
}

void Game::initialize(Time* time, Messenger<BackEndMessages>* backEndMessagingSystem) {

    Entities::registerEntities();

    currentTime     = time;
    backEndMessages = backEndMessagingSystem;

    if (areVitalsNull()) {
        return;
    }

    initializeShaders();

    map.createMap("assets/Fonts/CourierNew.FontDat");

    loadScene(0);
}

void Game::initializeShaders() {

    directionalLightDepthMap.initialize();
    pointLightDepthMap.initialize();
    renderTexture.initialize(GameInfo::getWindowWidth(), GameInfo::getWindowHeight());

    Shader::setShaderTask(SHADER_TASK::Normal_Render_Task);
    Shader::setShaderTaskShader(SHADER_TASK::Directional_Depth_Task, directionalLightDepthMap.getDepthMapShader());
    Shader::setShaderTaskShader(SHADER_TASK::Omnidirectional_Depth_Task, pointLightDepthMap.getDepthMapShader());
}

void Game::loadScene(int index) {

    if (index >= scenes.size()) {
        DBG_LOG("Cannot load scene, index out of bounds\n");
        return;
    }

    currentScene = index;

    //Free old scene and entities.
    delete scene;
    delete physicsWorld;
    freeEntities();

    //Allocate new scene and entities.
    scene               = new Scene();
    physicsWorld        = new PhysicsWorld(hh::toBtVec3(GameInfo::DEFAULT_GRAVITY));
    EntityVitals vitals = EntityVitals(&settings, scene, physicsWorld, &map);

    for (unsigned int i = 0; i < scenes.at(currentScene).size(); i++) {

        EntityWrapper* newEntity = Entities::allocateEntity(scenes.at(currentScene).at(i));

        if (newEntity == nullptr) {
            continue; //Only add to scene if new entity is valid.
        }

        sceneEntities.push_back(newEntity);
    }

    //Initialize new entities
    for (unsigned int i = 0; i < sceneEntities.size(); i++) {
        sceneEntities.at(i)->initialize(vitals);
    }

    //Add rigid bodies to physics world.
    scene->performOperationsOnAllOfType<CollisionMesh>(
        [& world = *physicsWorld](const CollisionMesh& mesh) {
            world.addRigidBody(mesh);
            return false;
        });
    scene->performOperationsOnAllOfType<BoneCollisionMesh>(
        [& world = *physicsWorld](BoneCollisionMesh& bmesh) {
            bmesh.iterateThroughColliders(
                [&](CollisionMesh& mesh, int32_t id) {
                    world.addRigidBody(mesh);
                });

            return false;
        });

    //Re-init systems.
    fixedUpdatingSystem.initialize(*scene, settings, *physicsWorld, subSystems);
    updatingSystem.initialize(*scene, settings, *physicsWorld, subSystems);
    renderingSystem.initialize(*scene, settings, *physicsWorld, subSystems);
}

void Game::readBackendEventQueue() {
    BackEndMessages msg;
    while (backEndMessages->getMessagesThenRemove(msg)) {
        fixedUpdatingSystem.handleBackEndMessage(msg, renderTexture);
    }
}

void Game::fixedUpdate() {
    if (areVitalsNull()) {
        return;
    }

    readBackendEventQueue();

    if (InputLocator::getService().keyPressedOnce(SDLK_7)) {

        loadScene((currentScene + 1) % scenes.size());
    }

    fixedUpdatingSystem.fixedUpdate(*currentTime, pointLightDepthMap, directionalLightDepthMap);
}

void Game::update() {
    updatingSystem.update();
}

void Game::render() {
    if (areVitalsNull()) {
        return;
    }

    renderingSystem.render(pointLightDepthMap, directionalLightDepthMap, *currentTime, renderTexture);
}

void Game::freeEntities() {

    for (unsigned int i = 0; i < sceneEntities.size(); i++) {
        if (sceneEntities.at(i) == nullptr) {
            continue;
        }
        delete sceneEntities.at(i);
    }
    sceneEntities.clear();
}

void Game::uninitialize() {
    delete scene;
    delete physicsWorld;
    freeEntities();
}
