#pragma once
#include "RenderPass.h"

namespace Pixie
{
    class CircleRenderPass : public RenderPass
    {
    public:
        CircleRenderPass();
        // Inherited via RenderPass
        void Execute(std::shared_ptr<Scene> sceneToRender, uint32_t prevPassDepthID, uint32_t prevPassColorID) override;
        std::shared_ptr<Shader> GetShader() override { return m_Shader; }
        std::shared_ptr<FrameBuffer> GetFrameBuffer() const override { return nullptr; };
        uint32_t GetFrameBufferID() const override { return 0; };
        uint32_t GetColorAttatchmentID() const override { return 0; };
        uint32_t GetDepthAttatchmentID() const override { return 0; };
        bool IsLit() const override { return false; };
        void ForceLightsOff(bool value) override { };
    protected:
        std::shared_ptr<Shader> m_Shader;
        bool m_BDrawDebug{ true };

        void SetCircleUniforms(CircleRendererComponent& circle);

        void DrawSphereColliders(entt::registry& registry);
        void DrawSplineDebug(entt::registry& registry, glm::mat4& viewMatrix);
    };
}