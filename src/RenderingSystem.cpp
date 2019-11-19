#include "RenderingSystem.h"

void RenderingSystem::initialize(Scene& scene, Settings& settings, PhysicsWorld& world, SubSystems& ssystems) {

    currentScene    = &scene;
    currentSettings = &settings;
    physicsWorld    = &world;
    systems         = &ssystems;

    currentScene->loopEntities([&](const Scene::Entity& entity) {
        if (SkyBox* skyBox = currentScene->getComponent<SkyBox>(entity.id)) {
            systems->skyBoxSystem.init(*skyBox);
        }
        if (LitShader* shader = currentScene->getComponent<LitShader>(entity.id)) {
            initializeLights(*shader);
            initializeModels(*shader, entity.id);

            if (Material* mat = currentScene->getComponent<Material>(entity.id)) {
                shader->setMaterial(*mat);
            }
            if (SimpleMaterial* mat = currentScene->getComponent<SimpleMaterial>(entity.id)) {
                shader->setSimpleMaterial(*mat);
            }
        }

        if (DefaultShader* shader = currentScene->getComponent<DefaultShader>(entity.id)) {
            if (shader->getShaderType() == SHADER_TYPE::Default) {
                initializeModels(*shader, entity.id);
            }
        }
        return false;
    });
}

void RenderingSystem::initializeLights(LitShader& litShader) {

    const std::vector<PointLight*>& pointLights = currentScene->getAllComponentsOfType<PointLight>();

    //Right now I'm only using 1 dir light
    const DirectionalLight* directionalLight = currentScene->getFirstActiveComponentOfType<DirectionalLight>();

    litShader.useProgram();

    if (directionalLight) {
        litShader.setDirectionalLight(*directionalLight);
    }

    //We use point index because the light might not be active.
    int pointIndex = 0;

    for (unsigned int i = 0; i < pointLights.size(); i++) {
        if (pointLights[i]->isActive()) {
            if (pointIndex < currentSettings->getLightsPerEntity()) {
                litShader.setPointLight(*currentSettings, *pointLights[i], pointIndex);
                pointIndex++;
            } else {
                break;
            }
        }
    }
}

void RenderingSystem::initializeModels(ShaderBase& shader, const int32_t& entity) {
    if (_3DM::AnimatedModel* animatedModel = currentScene->getComponent<_3DM::AnimatedModel>(entity)) {
        animatedModel->initialize(shader);
    }
    if (_3DM::Model* model = currentScene->getComponent<_3DM::Model>(entity)) {
        model->initialize(shader);
    }
}

void RenderingSystem::render(PointLightShadowMap& pointLightDepthMap, DirectionalLightShadowMap& directionalLightDepthMap, Time& currentTime) {

    /******************************************|
    |*		The Current Rendering Order		  *|
    |*----------------------------------------*|
    |*First, render All Opaque Objects.		  *|
    |*Last, Render All GUI Objects.			  *|
    |******************************************/

    if (areVitalsNull() == true) {
        DBG_LOG("Vitals are null (RenderingSystem.cpp)\n");
        return;
    }

    Camera* currentCamera = currentScene->getFirstActiveComponentOfType<ThirdPersonCamera>();

    if (!currentCamera) {
        return;
    }

    const std::vector<LitShader*>& litShaders = currentScene->getAllComponentsOfType<LitShader>();

    for (unsigned int i = 0; i < litShaders.size(); i++) {
        initializeLights(*litShaders.at(i));
    }

    renderDebugging(*currentCamera);
    renderModels(*currentCamera, currentTime, pointLightDepthMap, directionalLightDepthMap);
    renderOthers(*currentCamera, currentTime);
}

void RenderingSystem::renderDebugging(Camera& currentCamera) {
    // Execute debug drawing if enabled
    if (ShaderBase::getShaderTask() == SHADER_TASK::Normal_Render_Task) {

        const std::vector<DebuggingController*>& debugControllers = currentScene->getAllComponentsOfType<DebuggingController>();

        for (unsigned int i = 0; i < debugControllers.size(); i++) {
            debugControllers[i]->executeDebugRendering(*physicsWorld, *currentCamera.getViewMatrix(), *currentCamera.getProjectionMatrix());
        }

        systems->dayNightCycleSystem.debugRender(*currentCamera.getViewMatrix(), *currentCamera.getProjectionMatrix());
    }
}

void RenderingSystem::renderModels(Camera& currentCamera, Time& currentTime, PointLightShadowMap& pointLightDepthMap, DirectionalLightShadowMap& directionalLightDepthMap) {

    currentScene->loopEntities([&](const Scene::Entity& entity) {
        if (!entity.isActive) {
            return false;
        }

        ModelBase* modelToRender = nullptr;
        bool isAnimated          = false;

        if (_3DM::Model* model = currentScene->getComponent<_3DM::Model>(entity.id)) {
            modelToRender = model;

        } else if (_3DM::AnimatedModel* animatedModel = currentScene->getComponent<_3DM::AnimatedModel>(entity.id)) {
            modelToRender = animatedModel;
            isAnimated    = true;
        }

        if (!modelToRender) {
            return false;
        }

        if (LitShader* shdr = currentScene->getComponent<LitShader>(entity.id)) {

            shdr->useProgram();
            supplyLitShaderUniforms(*shdr, pointLightDepthMap, directionalLightDepthMap, currentCamera, currentTime);
            glUniform1i(Shaders::getUniformLocation(shdr->getProgramID(), Shaders::UniformName::IsModelAnimated), isAnimated);

            modelToRender->renderAll(*shdr);
        } else if (DefaultShader* dshdr = currentScene->getComponent<DefaultShader>(entity.id)) {

            dshdr->useProgram();
            supplyDefaultShaderUniforms(*dshdr, currentCamera, currentTime);

            glUniform1i(Shaders::getUniformLocation(dshdr->getProgramID(), Shaders::UniformName::IsModelAnimated), isAnimated);

            modelToRender->renderAll(*dshdr);
        }

        return false;
    });
}

void RenderingSystem::renderOthers(Camera& currentCamera, Time& currentTime) {

    // Render Particles and GUI
    if (ShaderBase::getShaderTask() == SHADER_TASK::Normal_Render_Task) {

        const std::vector<Particles*>& particles = currentScene->getAllComponentsOfType<Particles>();

        currentScene->loopEntities([&](const Scene::Entity& entity) {
            SkyBox* skyBox      = currentScene->getComponent<SkyBox>(entity.id);
            DefaultShader* shdr = currentScene->getComponent<DefaultShader>(entity.id);

            if (!skyBox || !shdr) {
                return false;
            }

            shdr->useProgram();
            supplyDefaultShaderUniforms(*shdr, currentCamera, currentTime);
            systems->skyBoxSystem.render(*skyBox, currentCamera, *shdr);

            return false;
        });

        for (unsigned int i = 0; i < particles.size(); i++) {
            renderParticles(*particles[i], currentCamera, currentTime);
        }

        currentScene->loopEntities([&](const Scene::Entity& entity) {
            DefaultShader* shader = currentScene->getComponent<DefaultShader>(entity.id);

            if (!shader || shader->getShaderType() != SHADER_TYPE::GUI) {
                return false;
            }

            if (EntityStats* stats = currentScene->getComponent<EntityStats>(entity.id)) {
                if (EntityStatsDisplayer* disp = currentScene->getComponent<EntityStatsDisplayer>(entity.id)) {
                    shader->useProgram();
                    systems->EntityStatsDisplayerSystem.render(*disp, *shader);
                }
            }

            if (DisplayStatistics* stats = currentScene->getComponent<DisplayStatistics>(entity.id)) {
                shader->useProgram();
                systems->displayStatisticsSystem.render(*shader, *stats);
            }

            if (PauseMenu* menu = currentScene->getComponent<PauseMenu>(entity.id)) {
                shader->useProgram();
                systems->pauseMenuSystem.render(*shader, *menu);
            }

            return false;
        });
    }
}

void RenderingSystem::renderParticles(Particles& particles,
                                      Camera& currentCamera,
                                      Time& currentTime) {
    if (DefaultShader* thisShader = currentScene->getComponent<DefaultShader>(particles.getEntityID())) {
        // Check to see if the shader is a particle shader
        if (thisShader->getShaderType() == SHADER_TYPE::Particle) {
            supplyParticleShaderUniforms(*thisShader, currentCamera, currentTime);

            if (ParticleEmitter* particleEmitter = getParticleEmitter(particles.getEntityID())) {
                particleEmitter->renderParticles(*thisShader, particles);
            }
        }
    }
}

ParticleEmitter*
RenderingSystem::getParticleEmitter(int32_t entity) {
    // Search for fountain particle system
    if (ParticleEmitter* particleEmitter = currentScene->getComponent<FountainParticleEmitter>(entity)) {
        return particleEmitter;
    }

    return nullptr;
}

ShaderBase*
RenderingSystem::prepareShader(
    const int32_t& entity,
    PointLightShadowMap& pointLightDepthMap,
    DirectionalLightShadowMap& directionalLightDepthMap,
    Camera& currentCamera,
    Time& currentTime) {
    ShaderBase* thisShader;
    if (thisShader = currentScene->getComponent<DefaultShader>(entity)) {
        if (thisShader->getShaderType() == SHADER_TYPE::Default) {
            if (currentScene->isEntityActive(entity)) {
                thisShader->useProgram();
                supplyDefaultShaderUniforms(*thisShader, currentCamera, currentTime);
                return thisShader;
            }
        }
    }

    if (thisShader = currentScene->getComponent<LitShader>(entity)) {
        if (thisShader->getShaderType() == SHADER_TYPE::Default) {
            if (currentScene->isEntityActive(entity)) {
                thisShader->useProgram();
                supplyLitShaderUniforms(*thisShader,
                                        pointLightDepthMap,
                                        directionalLightDepthMap,
                                        currentCamera,
                                        currentTime);
                return thisShader;
            }
        }
    }

    return nullptr;
}

void RenderingSystem::supplyLitShaderUniforms(
    ShaderBase& shader,
    PointLightShadowMap& pointLightDepthMap,
    DirectionalLightShadowMap& directionalLightDepthMap,
    Camera& currentCamera,
    Time& currentTime) {
    supplyDefaultShaderUniforms(shader, currentCamera, currentTime);

    glActiveTexture(GL_TEXTURE0 + Shaders::DEPTH_MAP_LOCATION_OMNIDIRECTIONAL);
    glUniform1i(Shaders::getUniformLocation(shader.getProgramID(), Shaders::UniformName::PointShadowMap), Shaders::DEPTH_MAP_LOCATION_OMNIDIRECTIONAL);

    //glUniform1i(Shaders::getUniformLocation(shader.getProgramID(), Shaders::UniformName::TimeMS), currentTime.sinceStartMS32());

    if (pointLightDepthMap.isActive()) {
        glUniform1f(Shaders::getUniformLocation(shader.getProgramID(), Shaders::UniformName::FarPlane), pointLightDepthMap.getFarPlane());

        glUniform3fv(Shaders::getUniformLocation(shader.getProgramID(), Shaders::UniformName::LightPosition), 1, &pointLightDepthMap.getCurrentLightPosition()[0]);

        glBindTexture(GL_TEXTURE_CUBE_MAP, pointLightDepthMap.getCubeMap());

    } else {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    glUniformMatrix4fv(Shaders::getUniformLocation(shader.getProgramID(), Shaders::UniformName::LightSpaceMatrix),
                       1,
                       GL_FALSE,
                       glm::value_ptr(*directionalLightDepthMap.getLightSpaceMatrix()));

    glUniform1i(Shaders::getUniformLocation(shader.getProgramID(), Shaders::UniformName::DirectionalShadowMap),
                Shaders::DEPTH_MAP_LOCATION_DIRECTIONAL);

    glActiveTexture(GL_TEXTURE0 + Shaders::DEPTH_MAP_LOCATION_DIRECTIONAL);
    glBindTexture(GL_TEXTURE_2D, directionalLightDepthMap.getDepthMap());
}

void RenderingSystem::supplyDefaultShaderUniforms(ShaderBase& shader,
                                                  Camera& currentCamera,
                                                  Time& currentTime) {
    const GLint& programID = shader.getProgramID();

    glUniformMatrix4fv(
        Shaders::getUniformLocation(programID, Shaders::UniformName::ViewMatrix),
        1,
        GL_FALSE,
        glm::value_ptr(*currentCamera.getViewMatrix()));

    glUniformMatrix4fv(Shaders::getUniformLocation(
                           programID, Shaders::UniformName::ProjectionMatrix),
                       1,
                       GL_FALSE,
                       glm::value_ptr(*currentCamera.getProjectionMatrix()));

    glUniform3f(
        Shaders::getUniformLocation(programID, Shaders::UniformName::ViewPosition),
        currentCamera.position.x,
        currentCamera.position.y,
        currentCamera.position.z);
}

void RenderingSystem::supplyParticleShaderUniforms(ShaderBase& particleShader,
                                                   Camera& currentCamera,
                                                   Time& currentTime) {
    particleShader.useProgram();

    const GLint& programID = particleShader.getProgramID();

    glUniformMatrix4fv(
        Shaders::getUniformLocation(programID, Shaders::UniformName::ViewMatrix),
        1,
        GL_FALSE,
        glm::value_ptr(*currentCamera.getViewMatrix()));

    glUniformMatrix4fv(Shaders::getUniformLocation(
                           programID, Shaders::UniformName::ProjectionMatrix),
                       1,
                       GL_FALSE,
                       glm::value_ptr(*currentCamera.getProjectionMatrix()));
}
