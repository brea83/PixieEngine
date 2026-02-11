#include "BsPrecompileHeader.h"
#include "CircleRenderPass.h"
#include "Graphics/Primitives/CircleMesh.h"
#include "Scene/Components/Collider.h"
#include "Scene/GameObject.h"

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

        GameObject cam = sceneToRender->GetActiveCameraGameObject();
        
        glm::mat4 viewMatrix{ 1.0f };
        if (cam)
        {
            viewMatrix = glm::inverse(cam.GetTransform().GetObjectToWorldMatrix());
        }

        auto group = registry.group<CircleRendererComponent>(entt::get<TransformComponent>);

        //CircleUniforms uniforms; TODO figure out sorting components by matching uniform sets
        m_Shader->SetUniformBool("IsBillboard", false);
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

       
        //collider debug vis
        DrawSphereColliders(registry);

        // spline debug vis
        DrawSplineDebug(registry, viewMatrix);

        m_Shader->EndUse();
    }

    void CircleRenderPass::SetCircleUniforms(CircleRendererComponent& circle)
    {
       


        m_Shader->SetUniformVec4("Color", circle.Color);
        m_Shader->SetUniformFloat("Radius", circle.Radius);
        m_Shader->SetUniformFloat("LineWidth", circle.LineWidth);
        m_Shader->SetUniformFloat("Fade", circle.Fade);
    }

    void CircleRenderPass::DrawSphereColliders(entt::registry& registry)
    {
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
        for (auto&& [entity, collider] : view.each())
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

            m_Shader->SetUniformMat4("Transform", circleXY);
            colliderCircle.MeshResource->Render(*m_Shader);

            m_Shader->SetUniformMat4("Transform", circleXZ);
            colliderCircle.MeshResource->Render(*m_Shader);

            m_Shader->SetUniformMat4("Transform", circleYZ);
            colliderCircle.MeshResource->Render(*m_Shader);
        }
    }

    void CircleRenderPass::DrawSplineDebug(entt::registry& registry, glm::mat4& viewMatrix)
    {
        m_Shader->SetUniformBool("IsBillboard", true);
        glm::mat4 inverseViewBase = glm::inverse(viewMatrix);
        CircleRendererComponent splineCircle = CircleRendererComponent();
        m_Shader->SetUniformFloat("Radius", splineCircle.Radius);
        m_Shader->SetUniformFloat("Fade", splineCircle.Fade);

        CircleRendererComponent pathCircle = CircleRendererComponent();
        pathCircle.Radius = 0.01f;
        glm::mat4 identity = glm::mat4(1.0f);
        glm::mat4 pathScale = glm::scale(identity, glm::vec3(0.1f));

        for (auto&& [entt, spline] : registry.view<SplineComponent>().each())
        {
            // these are the same for all points on the spline
            m_Shader->SetUniformVec4("Color", spline.DebugColor);

            //draw controll points
            for (int i = 0; i < spline.Points.size(); i++)
            {
                TransformComponent* point = spline.Points[i];
                glm::mat4 scale = glm::scale(identity, point->GetScale() * 0.25f);
                glm::mat4 inverseView = inverseViewBase;
                inverseView[3] = point->GetWorld()[3];
                glm::mat4 modelViewMat = viewMatrix * inverseView * scale;

                m_Shader->SetUniformMat4("Transform", modelViewMat);
                m_Shader->SetUniformFloat("LineWidth", 1.0f);

                splineCircle.MeshResource->Render(*m_Shader);
            }

           

            //draw path indicators

            m_Shader->SetUniformFloat("LineWidth", 0.5f);
            float t = 0;
            int numSegments = spline.GetNumSegments();
            if (numSegments <= 0)
                return;

            while (t < spline.GetNumSegments() )
            {
                t += 0.1;
                if (glm::mod<float>(t, 1) == 0)
                {
                    continue;
                }
                glm::vec3 position = spline.GetPostionT(t);
                glm::mat4 inverseView = inverseViewBase;
                inverseView[3] = glm::vec4(position, 1.0f);
                glm::mat4 modelViewMat = viewMatrix * inverseView * pathScale;

                m_Shader->SetUniformMat4("Transform", modelViewMat);
                pathCircle.MeshResource->Render(*m_Shader);
            }

            
            // render preview point
            glm::vec3 position = spline.GetPostionT(spline.PreviewTime);
            glm::mat4 inverseView = inverseViewBase;
            inverseView[3] = glm::vec4(position, 1.0f);
            glm::mat4 modelViewMat = viewMatrix * inverseView * pathScale;
            glm::vec4 previewColor{ 1.0f, 0.75f, 0.0f, 1.0f };

            m_Shader->SetUniformFloat("LineWidth", 1.0f);
            m_Shader->SetUniformVec4("Color", previewColor);
            m_Shader->SetUniformMat4("Transform", modelViewMat);
            pathCircle.MeshResource->Render(*m_Shader);

        }
    }

}