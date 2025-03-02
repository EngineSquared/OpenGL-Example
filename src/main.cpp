/**
 * Project attended to report some issues from E²(https://github.com/EngineSquared/EngineSquared)
 * 
 * Issues:
 * - Can't report any errors from systems (like OpenGL errors). For example, initialization systems (like systems that use glfw functions that can return errors like "glfwInit") should raise an error if an error occured.
 * - We don't really know what we can do with Core.
 * - No clear way to run something when closing the program (like cleaning up resources, glfw context).
 * - We can't use Plugin::AssetsManager with string as key.
 * - Mesh of E² is too vague and not really useful.
 */

#include <iostream>
#include "GL/gl3w.h"
#include "GLFW/glfw3.h"
#include "Core.hpp"
#include "Object.hpp"
#include "Entity.hpp"

#include "ShaderManager.hpp"
#include "MaterialCache.hpp"
#include "Camera/Camera.hpp"
#include "Model.hpp"

const int DEFAULT_WIDTH = 800;
const int DEFAULT_HEIGHT = 800;

struct ESGLFWWINDOW
{
    GLFWwindow *window;
};

namespace ESGL {

    struct Button
    {
        bool pressed = false;
        bool updated = false;
    };

    struct Buttons
    {
        std::map<int, Button> mouse = {
            {GLFW_MOUSE_BUTTON_LEFT, Button()},
            {GLFW_MOUSE_BUTTON_RIGHT, Button()},
            {GLFW_MOUSE_BUTTON_MIDDLE, Button()}
        };
        glm::vec<2, double, glm::defaultp> lastMousePos = {0.0f, 0.0f};
        glm::vec<2, double, glm::defaultp> currentMousePos = {0.0f, 0.0f};
    };

    void UpdateKey(ES::Engine::Core &core)
    {
        GLFWwindow *window = core.GetResource<ESGLFWWINDOW>().window;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    void UpdateButton(ES::Engine::Core &core)
    {
        GLFWwindow *window = core.GetResource<ESGLFWWINDOW>().window;
        auto &mouseButtons = core.GetResource<ESGL::Buttons>().mouse;
        for (auto &button : mouseButtons) {
            bool pressed = glfwGetMouseButton(window, button.first) == GLFW_PRESS;
            button.second.updated = button.second.pressed != pressed;
            button.second.pressed = pressed;
        }
    }

    void SaveLastMousePos(ES::Engine::Core &core)
    {
        auto &buttons = core.GetResource<ESGL::Buttons>();
        auto &lastMousePos = buttons.lastMousePos;
        auto &mouseButtons = buttons.mouse;
        auto window = core.GetResource<ESGLFWWINDOW>().window;
        if (mouseButtons[GLFW_MOUSE_BUTTON_LEFT].updated ||
            mouseButtons[GLFW_MOUSE_BUTTON_MIDDLE].updated ||
            mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].updated) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            lastMousePos.x = xpos;
            lastMousePos.y = ypos;
        }
    }

    void InitGLFW(ES::Engine::Core &core)
    {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return;
        }
    }

    void SetupGLFWHints(ES::Engine::Core &core)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    void CreateGLFWWindow(ES::Engine::Core &core)
    {
        if (!core.RegisterResource<ESGLFWWINDOW>({glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "OpenGL Framework", NULL, NULL)}).window) {
            glfwTerminate();
            std::cerr << "Failed to create GLFW window" << std::endl;
            return;
        }
    }

    void LinkGLFWContextToGL(ES::Engine::Core &core)
    {
        glfwMakeContextCurrent(core.GetResource<ESGLFWWINDOW>().window);
    }

    void InitGL3W(ES::Engine::Core &core)
    {
        if (gl3wInit()) {
            std::cerr << "Failed to initialize OpenGL" << std::endl;
            return;
        }
    }

    void CheckGL3WVersion(ES::Engine::Core &core)
    {
        if (!gl3wIsSupported(4, 2)) {
            std::cerr << "OpenGL 4.2 not supported" << std::endl;
            return;
        }
    }

    void GLFWEnableVSync(ES::Engine::Core &core)
    {
        glfwSwapInterval(1);
    }

    void UpdatePosCursor(ES::Engine::Core &core)
    {
        auto &currentMousePos = core.GetResource<ESGL::Buttons>().currentMousePos;
        glfwGetCursorPos(core.GetResource<ESGLFWWINDOW>().window, &currentMousePos.x, &currentMousePos.y);
    }

    // Function to handle mouse dragging interactions
    void MouseDragging(ES::Engine::Core &core)
    {
        auto &buttons = core.GetResource<ESGL::Buttons>();
        auto &lastMousePos = buttons.lastMousePos;
        auto &currentMousePos = buttons.currentMousePos;
        auto &mouseButtons = buttons.mouse;
        auto &camera = core.GetResource<Camera>();
        if (mouseButtons[GLFW_MOUSE_BUTTON_LEFT].pressed) {
            float fractionChangeX = static_cast<float>(currentMousePos.x - lastMousePos.x) / static_cast<float>(camera.size.x);
            float fractionChangeY = static_cast<float>(lastMousePos.y - currentMousePos.y) / static_cast<float>(camera.size.y);
            camera.viewer.rotate(fractionChangeX, fractionChangeY);
        }
        else if (mouseButtons[GLFW_MOUSE_BUTTON_MIDDLE].pressed) {
            float fractionChangeY = static_cast<float>(lastMousePos.y - currentMousePos.y) / static_cast<float>(camera.size.y);
            camera.viewer.zoom(fractionChangeY);
        }
        else if (mouseButtons[GLFW_MOUSE_BUTTON_RIGHT].pressed) {
            float fractionChangeX = static_cast<float>(currentMousePos.x - lastMousePos.x) / static_cast<float>(camera.size.x);
            float fractionChangeY = static_cast<float>(lastMousePos.y - currentMousePos.y) / static_cast<float>(camera.size.y);
            camera.viewer.translate(-fractionChangeX, -fractionChangeY, true);
        }
        lastMousePos.x = currentMousePos.x;
        lastMousePos.y = currentMousePos.y;
    }

    void SwapBuffers(ES::Engine::Core &core)
    {
        glfwSwapBuffers(core.GetResource<ESGLFWWINDOW>().window);
    }
    
    void PollEvents(ES::Engine::Core &core)
    {
        glfwPollEvents();
    }
    
    void LoadShaderManager(ES::Engine::Core &core)
    {
        ShaderManager &shaderManager = core.RegisterResource<ShaderManager>(ShaderManager());
        ShaderProgram &program = shaderManager.add("default");
        program.initFromFiles("shaders/simple.vert", "shaders/simple.frag");
    }

    void SetupShaderUniforms(ES::Engine::Core &core) {
        auto &m_shaderProgram = core.GetResource<ShaderManager>().get("default");
    
        // Add uniforms
        m_shaderProgram.addUniform("MVP");
        m_shaderProgram.addUniform("ModelMatrix"); //View*Model : mat4
        m_shaderProgram.addUniform("NormalMatrix"); //Refer next slide : mat3
    
        for (int i = 0; i < 5; i++) {
            m_shaderProgram.addUniform("Light[" + std::to_string(i) + "].Position");
            m_shaderProgram.addUniform("Light[" + std::to_string(i) + "].Intensity");
        }
        m_shaderProgram.addUniform("Material.Ka");
        m_shaderProgram.addUniform("Material.Kd");
        m_shaderProgram.addUniform("Material.Ks");
        m_shaderProgram.addUniform("Material.Shiness");
    
        m_shaderProgram.addUniform("CamPos");
    }

    void LoadMaterialCache(ES::Engine::Core &core)
    {
        auto &materialCache = core.RegisterResource<MaterialCache>({});
        materialCache.add("default");
    }

    void TESTAddQuad(ES::Engine::Core &core)
    {
        using namespace glm;

        auto quad = ES::Engine::Entity(core.GetRegistry().create());

        Model model;

        model.shaderName = "default";
        model.materialName = "default";

        Mesh mesh;

        std::vector<vec3> vertices;
        std::vector<vec3> normals;
        std::vector<vec<3, unsigned int>> triIndices;

        vertices = {
            vec3(-1,  1, 0),
            vec3( 1,  1, 0),
            vec3(-1, -1, 0),
            vec3( 1, -1, 0)
        };

        normals = {
            vec3(0, 0, -1),
            vec3(0, 0, -1),
            vec3(0, 0, -1),
            vec3(0, 0, -1)
        };

        triIndices = {
            {2, 0, 1},
            {2, 1, 3}
        };

        mesh.vertices = vertices;
        mesh.normals = normals;
        mesh.triIndices = triIndices;
        mesh.generateGlBuffers();

        model.mesh = mesh;


        quad.AddComponent<Model>(core, model);
        auto &transform = quad.AddComponent<ES::Plugin::Object::Component::Transform>(core, {});
        
        transform.position = glm::vec3(0.0f, -1.0f, 0.0f);
        transform.rotation = glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
        transform.scale = glm::vec3(10.0f, 10.0f, 10.0f);
    }

    void TESTGenerateData(Mesh &mesh, float outerRadius, float innerRadius)
    {
        using namespace glm;

        float TWOPI = 2 * glm::pi<float>();

        float ringFactor  = (float)(TWOPI / 100);
        float sideFactor = (float)(TWOPI / 100);
        int idx = 0, tidx = 0;
        for( int ring = 0; ring <= 100; ring++ ) {
            float u = ring * ringFactor;
            float cu = cos(u);
            float su = sin(u);
            for( int side = 0; side < 100; side++ ) {
                float v = side * sideFactor;
                float cv = cos(v);
                float sv = sin(v);
                float r = (outerRadius + innerRadius * cv);
                mesh.vertices.push_back(vec3(r * cu, r * su, innerRadius * sv));
                mesh.normals.push_back(vec3(cv * cu * r, cv * su * r, sv * r));
                // Normalize
                float len = sqrt( mesh.normals[idx].x * mesh.normals[idx].x +
                                mesh.normals[idx].y * mesh.normals[idx].y +
                                mesh.normals[idx].z * mesh.normals[idx].z );
                mesh.normals[idx] /= len;
                idx += 1;
            }
        }

        idx = 0;
        for( int ring = 0; ring < 100; ring++ ) {
            int ringStart = ring * 100;
            int nextRingStart = (ring + 1) * 100;
            for( int side = 0; side < 100; side++ ) {
                int nextSide = (side+1) % 100;
                // The quad
                mesh.triIndices.push_back({ringStart + side, nextRingStart + side, nextRingStart + nextSide});
                mesh.triIndices.push_back({ringStart + side, nextRingStart + nextSide, ringStart + nextSide});
            }
        }
    }
        
    void TESTAddTorus(ES::Engine::Core &core)
    {
        using namespace glm;

        auto torus = ES::Engine::Entity(core.GetRegistry().create());
        auto &mat = core.GetResource<MaterialCache>().add("TESTTorus");
        mat.Shiness = 180.0f;
        mat.Ka = vec3(0.1, 0.1, 0.1);
        mat.Kd = vec3(0.4, 0.4, 0.4);
        mat.Ks = vec3(0.9,0.9, 0.9);
        

        Model model;

        model.shaderName = "default";
        model.materialName = "TESTTorus";

        Mesh mesh;

        TESTGenerateData(mesh, 1.5f, 0.3f);
        mesh.generateGlBuffers();

        model.mesh = mesh;


        torus.AddComponent<Model>(core, model);
        auto &transform = torus.AddComponent<ES::Plugin::Object::Component::Transform>(core, {});
        transform.rotation = glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    }

    void CreateCamera(ES::Engine::Core &core)
    {
        auto &camera = core.RegisterResource<Camera>(Camera(DEFAULT_WIDTH, DEFAULT_HEIGHT));
    }

    struct Light {
        glm::vec4 Position;
        glm::vec3 Intensity;
    };
    
    void UpdateMatrices(ES::Engine::Core &core)
    {
        auto &cam = core.GetResource<Camera>();
        cam.view = glm::lookAt(cam.viewer.getViewPoint(), cam.viewer.getViewCenter(), cam.viewer.getUpVector());
        cam.projection = glm::perspective(glm::radians(45.0f), cam.size.x / cam.size.y, 0.1f, 100.0f);
    }
    
    void GLClearColor(ES::Engine::Core &core)
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    void GLClearDepth(ES::Engine::Core &core)
    {
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    
    void GLEnableDepth(ES::Engine::Core &core)
    {
        glEnable(GL_DEPTH_TEST);
    }
    
    void GLEnableCullFace(ES::Engine::Core &core)
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    
    void SetupLights(ES::Engine::Core &core)
    {
        core.GetResource<ShaderManager>().use("default");
    
        Light light[] = {
            {glm::vec4(0, 0, 0, 1), glm::vec3(0.0f, 0.8f, 0.8f)},
            {glm::vec4(0, 0, 0, 1), glm::vec3(0.0f, 0.0f, 0.8f)},
            {glm::vec4(0, 0, 0, 1), glm::vec3(0.8f, 0.0f, 0.0f)},
            {glm::vec4(0, 0, 0, 1), glm::vec3(0.0f, 0.8f, 0.0f)},
            {glm::vec4(0, 0, 0, 1), glm::vec3(0.8f, 0.8f, 0.8f)}
        };
    
        float nbr_lights = 5.f;
        float scale = 2.f * glm::pi<float>() / nbr_lights;
    
        light[0].Position = glm::vec4( 5.f * cosf(scale * 0.f), 5.f, 5.f * sinf(scale * 0.f), 1.f);
        light[1].Position = glm::vec4( 5.f * cosf(scale * 1.f), 5.f, 5.f * sinf(scale * 1.f), 1.f);
        light[2].Position = glm::vec4( 5.f * cosf(scale * 2.f), 5.f, 5.f * sinf(scale * 2.f), 1.f);
        light[3].Position = glm::vec4( 5.f * cosf(scale * 3.f), 5.f, 5.f * sinf(scale * 3.f), 1.f);
        light[4].Position = glm::vec4( 5.f * cosf(scale * 4.f), 5.f, 5.f * sinf(scale * 4.f), 1.f);
        
        auto &shaderProgram = core.GetResource<ShaderManager>().get("default");
        for (int i = 0; i < 5; i++) {
            glUniform4fv(shaderProgram.uniform("Light[" + std::to_string(i) + "].Position"), 1, glm::value_ptr(light[i].Position));
            glUniform3fv(shaderProgram.uniform("Light[" + std::to_string(i) + "].Intensity"), 1, glm::value_ptr(light[i].Intensity));
        }        
        core.GetResource<ShaderManager>().disable("default");
    }

    void SetupCamera(ES::Engine::Core &core)
    {
        auto &shaderProgram = core.GetResource<ShaderManager>().get("default");
        shaderProgram.use();
        glUniform3fv(shaderProgram.uniform("CamPos"), 1, glm::value_ptr(core.GetResource<Camera>().viewer.getViewPoint()));
        shaderProgram.disable();
    }
    
    void RenderMeshes(ES::Engine::Core &core)
    {
        auto &view = core.GetResource<Camera>().view;
        auto &projection = core.GetResource<Camera>().projection;
        core.GetRegistry().view<Model, ES::Plugin::Object::Component::Transform>().each([&](auto entity, Model &model, ES::Plugin::Object::Component::Transform &transform) {
            auto &shader = core.GetResource<ShaderManager>().get(model.shaderName);
            const auto material = core.GetResource<MaterialCache>().get(model.materialName);
            shader.use();
            glUniform3fv(shader.uniform("Material.Ka"), 1, glm::value_ptr(material.Ka));
            glUniform3fv(shader.uniform("Material.Kd"), 1, glm::value_ptr(material.Kd));
            glUniform3fv(shader.uniform("Material.Ks"), 1, glm::value_ptr(material.Ks));
            glUniform1fv(shader.uniform("Material.Shiness"), 1, &material.Shiness);
            glm::mat4 modelmat = transform.getTransformationMatrix();
            glm::mat4 mview = view * modelmat;
            glm::mat4 mvp = projection * view * modelmat;
            glm::mat4 imvp = glm::inverse(modelmat);
            glm::mat3 nmat = glm::mat3(glm::transpose(imvp)); //normal matrix
            glUniformMatrix3fv(shader.uniform("NormalMatrix"), 1, GL_FALSE, glm::value_ptr(nmat));
            glUniformMatrix4fv(shader.uniform("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(modelmat));
            glUniformMatrix4fv(shader.uniform("MVP"), 1, GL_FALSE, glm::value_ptr(mvp));
            model.mesh.draw();
            shader.disable();
        });
    }
}


int main()
{
    ES::Engine::Core core;

    core.RegisterResource<ESGL::Buttons>({});

    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ESGL::InitGLFW);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ESGL::SetupGLFWHints);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ESGL::CreateGLFWWindow);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ESGL::LinkGLFWContextToGL);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ESGL::InitGL3W);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ESGL::CheckGL3WVersion);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ESGL::GLFWEnableVSync);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ESGL::SetupGLFWHints);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ESGL::LoadMaterialCache);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ESGL::LoadShaderManager);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ESGL::CreateCamera);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ESGL::SetupShaderUniforms);

    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ESGL::TESTAddQuad);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(ESGL::TESTAddTorus);

    core.RunSystems();

    core.RegisterSystem<ES::Engine::Scheduler::Update>(ESGL::UpdateKey);
    core.RegisterSystem<ES::Engine::Scheduler::Update>(ESGL::UpdatePosCursor);
    core.RegisterSystem<ES::Engine::Scheduler::Update>(ESGL::UpdateButton);
    core.RegisterSystem<ES::Engine::Scheduler::Update>(ESGL::SaveLastMousePos);
    core.RegisterSystem<ES::Engine::Scheduler::Update>(ESGL::UpdateMatrices);
    core.RegisterSystem<ES::Engine::Scheduler::Update>(ESGL::GLClearColor);
    core.RegisterSystem<ES::Engine::Scheduler::Update>(ESGL::GLClearDepth);
    core.RegisterSystem<ES::Engine::Scheduler::Update>(ESGL::GLEnableDepth);
    core.RegisterSystem<ES::Engine::Scheduler::Update>(ESGL::GLEnableCullFace);
    core.RegisterSystem<ES::Engine::Scheduler::Update>(ESGL::SetupCamera);
    core.RegisterSystem<ES::Engine::Scheduler::Update>(ESGL::SetupLights);
    core.RegisterSystem<ES::Engine::Scheduler::Update>(ESGL::RenderMeshes);
    core.RegisterSystem<ES::Engine::Scheduler::Update>(ESGL::SwapBuffers);
    core.RegisterSystem<ES::Engine::Scheduler::Update>(ESGL::PollEvents);
    core.RegisterSystem<ES::Engine::Scheduler::Update>(ESGL::MouseDragging);

    while (!glfwWindowShouldClose(core.GetResource<ESGLFWWINDOW>().window)) {
        core.RunSystems();
    }

    glfwDestroyWindow(core.GetResource<ESGLFWWINDOW>().window);
    glfwTerminate();

    return 0;
}
