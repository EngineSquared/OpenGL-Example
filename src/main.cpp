/**
 * Project attended to report some issues from E²(https://github.com/EngineSquared/EngineSquared)
 *
 * Issues:
 * - Can't report any errors from systems (like OpenGL errors). For example,
 * initialization systems (like systems that use glfw functions that can return
 * errors like "glfwInit") should raise an error if an error occured.
 * - We don't really know what we can do with Core.
 * - No clear way to run something when closing the program (like cleaning up resources, glfw context).
 * - We can't use Plugin::AssetsManager with string as key.
 * - Mesh of E² is too vague and not really useful.
 * - Local plugin and global xmake require to put the hard path of the engine.
 */

#include "Core.hpp"
#include "Entity.hpp"
#include "OpenGL.hpp"
#include "Startup.hpp"
#include "Window.hpp"
#include "Mesh.hpp"
#include "UI.hpp"

void TESTAddQuad(ES::Engine::Core &core)
{
    using namespace glm;

    auto quad = ES::Engine::Entity(core.GetRegistry().create());

    ES::Plugin::Object::Component::Mesh mesh;

    mesh.vertices = {
        glm::vec3(-1, 1, 0),
        glm::vec3(1, 1, 0),
        glm::vec3(-1, -1, 0),
        glm::vec3(1, -1, 0)
    };

    mesh.normals = {
        glm::vec3(0, 0, -1),
        glm::vec3(0, 0, -1),
        glm::vec3(0, 0, -1),
        glm::vec3(0, 0, -1)
    };

    mesh.indices = {2, 0, 1, 2, 1, 3};

    quad.AddComponent<ES::Plugin::Object::Component::Mesh>(core, mesh);
    auto &transform = quad.AddComponent<ES::Plugin::Object::Component::Transform>(core, {});

    transform.position = glm::vec3(0.0f, -1.0f, 0.0f);
    transform.rotation = glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    transform.scale = glm::vec3(10.0f, 10.0f, 10.0f);

    quad.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, ES::Plugin::OpenGL::Component::ShaderHandle("default"));
    quad.AddComponent<ES::Plugin::OpenGL::Component::MaterialHandle>(core, ES::Plugin::OpenGL::Component::MaterialHandle("default"));
    quad.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(core, ES::Plugin::OpenGL::Component::ModelHandle("floor"));
}

void TESTGenerateData(ES::Plugin::Object::Component::Mesh &mesh, float outerRadius, float innerRadius)
{
    using namespace glm;

    mesh.vertices.reserve(100 * 100);
    mesh.normals.reserve(100 * 100);
    mesh.indices.reserve(100 * 100 * 6);

    float TWOPI = 2 * glm::pi<float>();

    float ringFactor  = (float)(TWOPI / 100);
    float sideFactor = (float)(TWOPI / 100);
    for (uint32_t ring = 0; ring <= 100; ring++)
    {
        float u = ring * ringFactor;
        float cu = cos(u);
        float su = sin(u);

        for (uint32_t side = 0; side < 100; side++)
        {
            float v = side * sideFactor;
            float cv = cos(v);
            float sv = sin(v);
            float r = (outerRadius + innerRadius * cv);
            // Normalize
            glm::vec3 normal = glm::vec3(cv * cu * r, cv * su * r, sv * r);
            float len = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            normal /= len;
            mesh.vertices.emplace_back(glm::vec3(r * cu, r * su, innerRadius * sv));
            mesh.normals.emplace_back(normal);
        }
    }

    for (uint32_t ring = 0; ring < 100; ring++)
    {
        uint32_t ringStart = ring * 100;
        uint32_t nextRingStart = (ring + 1) * 100;

        for (uint32_t side = 0; side < 100; side++)
        {
            uint32_t nextSide = (side+1) % 100;
            // The quad
            mesh.indices.insert(mesh.indices.end(), {ringStart + side, nextRingStart + side, nextRingStart + nextSide});
            mesh.indices.insert(mesh.indices.end(), {ringStart + side, nextRingStart + nextSide, ringStart + nextSide});
        }
    }
}

void TESTAddTorus(ES::Engine::Core &core)
{
    using namespace glm;

    auto torus = ES::Engine::Entity(core.GetRegistry().create());
    auto &mat = core.GetResource<ES::Plugin::OpenGL::Resource::MaterialCache>().Add(entt::hashed_string("TESTTorus"), std::move(ES::Plugin::OpenGL::Utils::Material()));
    mat.Shiness = 180.0f;
    mat.Ka = vec3(0.1, 0.1, 0.1);
    mat.Kd = vec3(0.4, 0.4, 0.4);
    mat.Ks = vec3(0.9,0.9, 0.9);

    ES::Plugin::Object::Component::Mesh mesh;

    TESTGenerateData(mesh, 1.5f, 0.3f);

    torus.AddComponent<ES::Plugin::Object::Component::Mesh>(core, mesh);
    auto &transform = torus.AddComponent<ES::Plugin::Object::Component::Transform>(core, {});
    transform.rotation = glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));

    torus.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, ES::Plugin::OpenGL::Component::ShaderHandle("default"));
    torus.AddComponent<ES::Plugin::OpenGL::Component::MaterialHandle>(core, ES::Plugin::OpenGL::Component::MaterialHandle("TESTTorus"));
    torus.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(core, ES::Plugin::OpenGL::Component::ModelHandle("torus"));
}

// Second torus to test that we can add an object with the same mesh but a different material
void TESTAddTorus2(ES::Engine::Core &core)
{
    using namespace glm;

    auto torus = ES::Engine::Entity(core.GetRegistry().create());
    auto &mat = core.GetResource<ES::Plugin::OpenGL::Resource::MaterialCache>().Add(entt::hashed_string("TESTTorus2"), std::move(ES::Plugin::OpenGL::Utils::Material()));
    mat.Shiness = 180.0f;
    mat.Ka = vec3(0.1, 0.0, 0.0);
    mat.Kd = vec3(0.4, 0.0, 0.0);
    mat.Ks = vec3(0.9, 0.0, 0.0);

    ES::Plugin::Object::Component::Mesh mesh;

    TESTGenerateData(mesh, 1.5f, 0.3f);

    torus.AddComponent<ES::Plugin::Object::Component::Mesh>(core, mesh);
    auto &transform = torus.AddComponent<ES::Plugin::Object::Component::Transform>(core, {});
    transform.rotation = glm::angleAxis(glm::radians(0.f), glm::vec3(1.f, 0.f, 0.f));

    torus.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, ES::Plugin::OpenGL::Component::ShaderHandle("default"));
    torus.AddComponent<ES::Plugin::OpenGL::Component::MaterialHandle>(core, ES::Plugin::OpenGL::Component::MaterialHandle("TESTTorus2"));
    torus.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(core, ES::Plugin::OpenGL::Component::ModelHandle("torus2"));
}

void TESTAddText(ES::Engine::Core &core)
{
    auto &font = core.GetResource<ES::Plugin::OpenGL::Resource::FontManager>().Add(
        entt::hashed_string("tomorrow"),
        ES::Plugin::OpenGL::Utils::Font("./assets/Tomorrow-Medium.ttf", 32)
    );

    auto text1 = ES::Engine::Entity(core.GetRegistry().create());

    text1.AddComponent<ES::Plugin::UI::Component::Text>(core, ES::Plugin::UI::Component::Text("The quick, brown fox jumped over the lazy dog", glm::vec2(50.0f, 100.0f), 1.0f, glm::vec3(1.0f, 1.0f, 1.0f)));

    text1.AddComponent<ES::Plugin::OpenGL::Component::FontHandle>(core, ES::Plugin::OpenGL::Component::FontHandle("tomorrow"));
    text1.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, ES::Plugin::OpenGL::Component::ShaderHandle("textDefault"));
    text1.AddComponent<ES::Plugin::OpenGL::Component::TextHandle>(core, ES::Plugin::OpenGL::Component::TextHandle("text1"));

    auto text2 = ES::Engine::Entity(core.GetRegistry().create());

    // Test some symbols and scaling
    // Warning: text looks blocky when scaling up, which is why here we scale down
    text2.AddComponent<ES::Plugin::UI::Component::Text>(core, ES::Plugin::UI::Component::Text("Some symbols &~!%*^,;\\_", glm::vec2(50.0f, 69.0f), 0.667f, glm::vec3(1.0f, 1.0f, 1.0f)));

    text2.AddComponent<ES::Plugin::OpenGL::Component::FontHandle>(core, ES::Plugin::OpenGL::Component::FontHandle("tomorrow"));
    text2.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, ES::Plugin::OpenGL::Component::ShaderHandle("textDefault"));
    text2.AddComponent<ES::Plugin::OpenGL::Component::TextHandle>(core, ES::Plugin::OpenGL::Component::TextHandle("text2"));

    auto timeElapsedText = ES::Engine::Entity(core.GetRegistry().create());

    timeElapsedText.AddComponent<ES::Plugin::UI::Component::Text>(core, ES::Plugin::UI::Component::Text("Time elapsed: 0.0s", glm::vec2(50.0f, 680.0f), 1.0f, glm::vec3(1.0f, 1.0f, 1.0f)));
    timeElapsedText.AddComponent<ES::Plugin::OpenGL::Component::FontHandle>(core, ES::Plugin::OpenGL::Component::FontHandle("tomorrow"));
    timeElapsedText.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, ES::Plugin::OpenGL::Component::ShaderHandle("textDefault"));
    timeElapsedText.AddComponent<ES::Plugin::OpenGL::Component::TextHandle>(core, ES::Plugin::OpenGL::Component::TextHandle("timeElapsedText"));
}

void UpdateTextTime(ES::Engine::Core &core)
{
    // Yes, this should be a resource and not a static variable
    static float ts = 0.0f;
    ts += core.GetScheduler<ES::Engine::Scheduler::Update>().GetDeltaTime();

    core.GetRegistry()
        .view<ES::Plugin::OpenGL::Component::TextHandle, ES::Plugin::UI::Component::Text>()
        .each([&core](auto entity, auto &textHandle, auto &text) {
            if (textHandle.name == "timeElapsedText")
            {
                text.text = "Time elapsed: " + std::to_string(ts) + "s";
            }
        });
}

int main()
{
    ES::Engine::Core core;

    core.AddPlugins<ES::Plugin::OpenGL::Plugin>();

    core.RegisterSystem<ES::Engine::Scheduler::Startup>(TESTAddQuad);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(TESTAddTorus);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(TESTAddTorus2);
    core.RegisterSystem<ES::Engine::Scheduler::Startup>(TESTAddText);

    core.RegisterSystem<ES::Engine::Scheduler::Update>(UpdateTextTime);

    core.RunCore();

    glfwDestroyWindow(core.GetResource<ES::Plugin::Window::Resource::Window>().GetGLFWWindow());
    glfwTerminate();

    return 0;
}
