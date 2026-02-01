#include "BsPrecompileHeader.h"
#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#define  GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include "Scene/Scene.h"
#include "Scene/GameObject.h"
#include "EngineContext.h"
#include "GUID.h"
//
namespace Pixie
{
    TransformComponent::TransformComponent(glm::vec3 position, glm::vec3 rotation , glm::vec3 scale)
    : m_Position(position), m_EulerRotation(glm::radians(rotation)), 
    m_Orientation(glm::quat(glm::radians(rotation))), m_Scale(scale), 
    m_LocalMatrix(glm::mat4(1.0f)), m_WorldMatrix(glm::mat4(1.0f))
    { }

    void TransformComponent::UnParent(Scene* scene, GameObject& parent, GameObject& grandParent, bool bKeepWorldPosition)
    {

        if (bKeepWorldPosition && grandParent)
        {
            TransformComponent& newParentTransform = grandParent.GetTransform();//scene->GetRegistry().get<TransformComponent>(grandParent);
            glm::mat4 parentMatrix = newParentTransform.m_LocalMatrix;

            m_LocalMatrix = parentMatrix * m_LocalMatrix;

            Decompose(m_LocalMatrix, m_Scale, m_Orientation, m_Position );

            SetRotationQuaternion(m_Orientation, AngleType::Radians);

            if (scene->GetRegistry().valid(grandParent))
            {
                m_ParentGuid = grandParent.GetGUID();
                return;
            }
        }
        m_ParentGuid = 0;
    }

    void TransformComponent::SetPosition(glm::vec3 value)
    {
        m_Position = value;
        m_PositionDirty = true;
    }

    glm::vec3 TransformComponent::GetPosition()
    {
        if (m_PositionDirty || m_ScaleDirty || m_RotationDirty)
        {
            RecalculateModelMatrix();
        }

       /* m_LocalMatrix = glm::translate(m_LocalMatrix, m_Position);
        m_PositionDirty = false;*/
        return m_Position;
    }

    glm::vec3 TransformComponent::Forward() const
    {
        //glm::vec3 direction;

        //direction.x = cos(m_EulerRotation.x) * sin(m_EulerRotation.y);
        //direction.y = sin(m_EulerRotation.x);
        //direction.z = cos(m_EulerRotation.x) * cos(m_EulerRotation.y);

        return -1.0f * glm::normalize(m_LocalMatrix[2]);//glm::normalize(direction);
    }

    glm::vec3 TransformComponent::Up() const
    {
        return glm::normalize(m_LocalMatrix[1]);//glm::normalize(glm::cross(Right(), Forward()));
    }

    glm::vec3 TransformComponent::Left() const
    {
        return glm::normalize(m_LocalMatrix[0]);
    }

    glm::vec3 TransformComponent::Right() const
    {

        //glm::vec3 right;
        //right.x = sin(m_EulerRotation.y - 3.14f / 2.0f);
        //right.y = 0;
        //right.z = cos(m_EulerRotation.y - 3.14f / 2.0f);
        return  -1.0f * glm::normalize(m_LocalMatrix[0]);//glm::normalize(glm::cross(Forward(), glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    glm::vec3 TransformComponent::Down() const
    {
        return Up() * -1.0f;
    }

    void TransformComponent::Rotate(float angle, glm::vec3 axis, AngleType angleType)
    {
        if (angleType == AngleType::Degrees)
        {
            m_LocalMatrix = glm::rotate(m_LocalMatrix, glm::radians(angle), axis);
        }
        else
        {
            m_LocalMatrix = glm::rotate(m_LocalMatrix, angle, axis);
        }

        m_Orientation = glm::quat_cast(m_LocalMatrix);
        float yaw = glm::yaw(m_Orientation);
        float pitch = glm::pitch(m_Orientation);
        float roll = glm::roll(m_Orientation);

        m_EulerRotation = glm::vec3(pitch, yaw, roll);
        m_RotationDirty;
    }

    void TransformComponent::SetRotationEuler(glm::vec3 value, AngleType angleType)
    {
        if (angleType == AngleType::Degrees)
        {
            m_EulerRotation = glm::radians(value);  
            m_Orientation = glm::quat(m_EulerRotation); 
        }
        else
        {
            m_EulerRotation = glm::vec3(value);
            m_Orientation = glm::quat(value);
        }

        m_RotationDirty = true;
    }

    void TransformComponent::SetRotationQuaternion(glm::quat orientation, AngleType angleType)
    {
        if (angleType == AngleType::Degrees)
        {
            m_Orientation.w = glm::radians(orientation.w);
            m_Orientation.x = glm::radians(orientation.x);
            m_Orientation.y = glm::radians(orientation.y);
            m_Orientation.z = glm::radians(orientation.z);
            float pitch = glm::pitch(m_Orientation);
            float yaw = glm::yaw(m_Orientation);
            float roll = glm::roll(m_Orientation);

            m_EulerRotation = glm::vec3( pitch, yaw, roll);
        }
        else
        {
            m_Orientation = orientation;
            float pitch = glm::pitch(m_Orientation);
            float yaw =  glm::yaw(m_Orientation);
            float roll = glm::roll(m_Orientation);

            m_EulerRotation = glm::vec3( pitch, yaw, roll);
        }

        m_RotationDirty = true;
    }

    glm::vec3 TransformComponent::GetRotationEuler(AngleType angleType)
    {
        if (m_PositionDirty || m_ScaleDirty || m_RotationDirty)
        {
            RecalculateModelMatrix();
        }

        switch (angleType)
        {
        case AngleType::Degrees:
            return glm::degrees(m_EulerRotation);
            break;
        case AngleType::Radians:
            return m_EulerRotation;
            break;
        default:
            return m_EulerRotation;
            break;
        }
    }

    glm::quat TransformComponent::GetRotationQuaternion()
    {
        if (m_PositionDirty || m_ScaleDirty || m_RotationDirty)
        {
            RecalculateModelMatrix();
        }
        return m_Orientation;
    }

    void TransformComponent::LookAt(glm::vec3 target)
    {

        glm::mat4 newTransform = glm::lookAt(m_Position, target, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::vec3 scale;
        glm::vec3 rotation;
        glm::vec3 translation;

        Decompose(newTransform, scale, rotation, translation);

        SetPosition(translation);
        SetRotationEuler(rotation, AngleType::Radians);
        SetScale(scale);
    }

    glm::vec3 TransformComponent::GetScale()
    {
        if (m_PositionDirty || m_ScaleDirty || m_RotationDirty)
        {
            RecalculateModelMatrix();
        }

        return m_Scale;
    }

    float TransformComponent::GetLargestScaleComponent()
    {
        float max = glm::max(glm::abs(m_Scale.x), glm::abs(m_Scale.y));
        max = glm::max(max, glm::abs(m_Scale.z));
        return max;
    }

    glm::mat4& TransformComponent::GetObjectToWorldMatrix()
    {
        if (m_PositionDirty || m_ScaleDirty || m_RotationDirty)
        {
            RecalculateModelMatrix();
        } 

        if (m_ParentGuid != 0)
        {
            std::shared_ptr<Scene> scene = EngineContext::GetEngine()->GetScene();
            GameObject parentObject = scene->FindGameObjectByGUID(m_ParentGuid);
            if (parentObject)
            {
                TransformComponent& parentTransform = parentObject.GetTransform();
                m_WorldMatrix = parentTransform.GetObjectToWorldMatrix() * m_LocalMatrix;
            }
            else
            {
                m_WorldMatrix = m_LocalMatrix;
            }

            return m_WorldMatrix;
        }

        return m_LocalMatrix;
    }

    void TransformComponent::Serialize(StreamWriter* stream, const TransformComponent& component)
    {
        // current scene and game object serialization uses WriteRaw for TransformComponent
        // so this function should never be called.
        stream->WriteRaw<GUID>(component.m_ParentGuid);
        stream->WriteRaw<GUID>(component.m_Guid);

    }

    bool TransformComponent::Deserialize(StreamReader * stream, TransformComponent & component)
    {
        return false;
    }

    void TransformComponent::RecalculateModelMatrix()
    {
        glm::mat4 identity = glm::mat4(1.0f);
        glm::mat4 translation = glm::translate(identity, m_Position);

        //m_Orientation = glm::normalize(m_Orientation);
        glm::mat4 rotation = glm::mat4_cast(m_Orientation);

    /* glm::mat4 rotation =
            glm::rotate(identity, glm::radians(m_EulerRotation.z), glm::vec3{ 0.0f, 0.0f, 1.0f })
            * glm::rotate(identity, glm::radians(m_EulerRotation.y), glm::vec3{ 0.0f, 1.0f, 0.0f })
            * glm::rotate(identity, glm::radians(m_EulerRotation.x), glm::vec3{ 1.0f, 0.0f, 0.0f });*/

        glm::mat4 scale = glm::scale(identity, m_Scale);

        m_LocalMatrix = translation * rotation * scale;

        m_PositionDirty = false;
        m_ScaleDirty = false;
        m_RotationDirty = false;
    }

    void TransformComponent::Decompose(glm::mat4 const& modelMatrix, glm::vec3& scale, glm::quat& orientation, glm::vec3& translation)
    {
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(modelMatrix, scale, orientation, translation, skew, perspective);
    }

    bool TransformComponent::Decompose(glm::mat4 const& transform, glm::vec3& scale, glm::vec3& rotation, glm::vec3& translation)
    {
        // From glm::decompose in matrix_decompose.inl and Hazel engine

        using namespace glm;
        using T = float;

        mat4 LocalMatrix(transform);

        // Normalize the matrix.
        if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
            return false;

        // First, isolate perspective.  This is the messiest.
        if (
            epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
        {
            // Clear the perspective partition
            LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
            LocalMatrix[3][3] = static_cast<T>(1);
        }

        // Next take care of translation (easy).
        translation = vec3(LocalMatrix[3]);
        LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

        vec3 Row[3];
        vec3 Pdum3;

        // Now get scale and shear.
        for (length_t i = 0; i < 3; ++i)
            for (length_t j = 0; j < 3; ++j)
                Row[i][j] = LocalMatrix[i][j];

        // Compute X scale factor and normalize first row.
        scale.x = length(Row[0]);
        Row[0] = detail::scale(Row[0], static_cast<T>(1));
        scale.y = length(Row[1]);
        Row[1] = detail::scale(Row[1], static_cast<T>(1));
        scale.z = length(Row[2]);
        Row[2] = detail::scale(Row[2], static_cast<T>(1));

        // At this point, the matrix (in rows[]) is orthonormal.
        // Check for a coordinate system flip.  If the determinant
        // is -1, then negate the matrix and the scaling factors.
    #if 0
        Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
        if (dot(Row[0], Pdum3) < 0)
        {
            for (length_t i = 0; i < 3; i++)
            {
                scale[i] *= static_cast<T>(-1);
                Row[i] *= static_cast<T>(-1);
            }
        }
    #endif

        rotation.y = asin(-Row[0][2]);
        if (cos(rotation.y) != 0)
        {
            rotation.x = atan2(Row[1][2], Row[2][2]);
            rotation.z = atan2(Row[0][1], Row[0][0]);
        }
        else
        {
            rotation.x = atan2(-Row[2][0], Row[1][1]);
            rotation.z = 0;
        }


        return true;
    }
}
