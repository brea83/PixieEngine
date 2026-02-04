#include "BsPrecompileHeader.h"
#include "CircleRenderPass.h"
#include "Graphics/Primitives/CircleMesh.h"
#include "Scene/Components/Collider.h"

namespace Pixie
{
    CircleRenderPass::CircleRenderPass()
    {
        m_Shader = AssetLoader::LoadShader("../Assets/Shaders/CircleVertex.glsl", "../Assets/Shaders/CircleFragment.glsl");
    }
    void CircleRenderPass::Execute(std::shared_ptr<Scene> sceneToRender, uint32_t prevPassDepthID, uint32_t prevPassColorID)
    {
        m_Shader->Use();
        // get scene registry for renderables
        entt::registry& registry = sceneToRender->GetRegistry();

        auto group = registry.group<CircleRendererComponent>(entt::get<TransformComponent>);

        //CircleUniforms uniforms; TODO figure out sorting components by matching uniform sets
        for (auto entity : group)
        {
            TransformComponent& transform = group.get<TransformComponent>(entity);
            CircleRendererComponent& circle = group.get<CircleRendererComponent>(entity);
            
            //std::shared_ptr<CircleMesh> mesh = std::dynamic_pointer_cast<CircleMesh>(circle.MeshResource);
            m_Shader->SetUniformMat4("Transform", transform.GetObjectToWorldMatrix());
            SetCircleUniforms(circle );
            
            //mesh->Render(*m_Shader);

            circle.MeshResource->Render(*m_Shader);
        }

        if (!m_BDrawDebug)
        {
            m_Shader->EndUse();
            return;
        }

        //prep for batching 
     /*   std::vector<glm::mat4> intersectingColliders;
        std::vector<glm::mat4> unHitColliders;*/

        auto view = registry.view<SphereCollider>();
        if (view.empty()) return;

        CircleRendererComponent colliderCircle = CircleRendererComponent();
        glm::vec4 baseColor = glm::vec4(0.1f, 0.9f, 0.1f, 1.0f);
        glm::vec4 collidingColor = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f);
        colliderCircle.Color = baseColor;
        glm::mat4 rotationXY = glm::mat4(1.0f);
        glm::mat4 rotationXZ = glm::rotate(rotationXY, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 rotationYZ = glm::rotate(rotationXY, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        
        SetCircleUniforms(colliderCircle);

        bool previousColliderWasColliding = false;
        for (auto&& [entity, collider] : registry.view<SphereCollider>().each())
        {
            glm::mat4 identity = glm::mat4(1.0f);
            glm::mat4 scale = glm::scale(identity, glm::vec3(collider.Radius * collider.Transform->GetLargestScaleComponent()));
            glm::mat4 translation = glm::translate(identity, collider.Transform->GetPosition());
            
            glm::mat4 circleXY = translation * rotationXY * scale;
            glm::mat4 circleXZ = translation * rotationXZ * scale;
            glm::mat4 circleYZ = translation * rotationYZ * scale;

            if (collider.Colliding && !previousColliderWasColliding)
            {
                m_Shader->SetUniformVec4("Color", collidingColor);
                previousColliderWasColliding = true;
            }
            else if (!collider.Colliding && previousColliderWasColliding)
            {
                m_Shader->SetUniformVec4("Color", baseColor);
                previousColliderWasColliding = false;
            }
            /*if (collider.Colliding)
            {
                intersectingColliders.push_back(circleXY);
                intersectingColliders.push_back(circleXZ);
                intersectingColliders.push_back(circleYZ);
            }
            else
            {
                unHitColliders.push_back(circleXY);
                unHitColliders.push_back(circleXZ);
                unHitColliders.push_back(circleYZ);
            }*/

            m_Shader->SetUniformMat4("Transform", circleXY);
            colliderCircle.MeshResource->Render(*m_Shader);

            m_Shader->SetUniformMat4("Transform", circleXZ);
            colliderCircle.MeshResource->Render(*m_Shader);

            m_Shader->SetUniformMat4("Transform", circleYZ);
            colliderCircle.MeshResource->Render(*m_Shader);
        }

        m_Shader->EndUse();
    }

    void CircleRenderPass::SetCircleUniforms(CircleRendererComponent& circle)
    {
       


        m_Shader->SetUniformVec4("Color", circle.Color);
        m_Shader->SetUniformFloat("Radius", circle.Radius);
        m_Shader->SetUniformFloat("LineWidth", circle.LineWidth);
        m_Shader->SetUniformFloat("Fade", circle.Fade);
    }

}