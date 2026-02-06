#include "BsPrecompileHeader.h"
#include "CameraManager.h"
#include "EngineContext.h"
#include "Scene/GameObject.h"
#include "Editor/EditorCamera.h"
#include "Scene/Components/Component.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/CameraController.h"
#include "Scene/Components/Transform.h"

namespace Pixie
{
    void CameraManager::InitEditor()
    {
        if (m_EditorCamera == entt::null || m_EditorCamera == entt::tombstone)
        {
            Entity entity = m_Scene->CreateEntity("Editor Camera");

            m_EditorCamera = EditorCamera(entity, m_Scene);
        }

        m_ActiveCamera = m_EditorCamera;
    }

    void CameraManager::Init()
    {
        if (m_Scene == nullptr)
        {
            Logger::Core(LOG_ERROR, "CameraManager Init called with an nullptr scene. this should not be possible");
            return;
        }

        entt::registry& registry = m_Scene->GetRegistry();
        auto cameras = registry.view<CameraComponent>();
        GameObject potentialDefault = GameObject();
        for (auto&& [entity, camera, heirarchy] : registry.view<CameraComponent, HeirarchyComponent>().each())
        {
            if (camera.IsDefault)
            {
                GameObject defaultCam = GameObject(entity, m_Scene);
                SetDefaultCamera(defaultCam);
            }
            else
            {
                potentialDefault = GameObject(entity, m_Scene);
            }
        }
        
        if (m_DefaultCamera == entt::null && potentialDefault.GetEnttHandle() != entt::null)
        {
            SetDefaultCamera(potentialDefault);
        }

        // now that default is set up set up starting active cam
        if (EngineContext::GetEngine()->IsEditorEnabled())
        {
            InitEditor();
        }
        else
        {
            m_ActiveCamera = m_DefaultCamera;
        }
    }

    void CameraManager::OnEditorUpdate(float deltaTime)
    {
        GameObject activeCam = GameObject(m_ActiveCamera, m_Scene);
        CameraController* controller = activeCam.TryGetComponent<CameraController>();
        if (!controller) return;
        controller->OnUpdate(deltaTime, activeCam);
    }

    void CameraManager::OnPlayUpdate(float deltaTime)
    {
        GameObject activeCam = GameObject(m_ActiveCamera, m_Scene);
        CameraController* controller = activeCam.TryGetComponent<CameraController>();
        if (!controller || controller->IsEditorOnly()) return;
        controller->OnUpdate(deltaTime, activeCam);
    }

    bool CameraManager::OnEvent(Event& event)
    {
        //EventDispatcher dispatcher{ event };
        //dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FUNCTION(CameraManager::OnKeyPressed));

        if (!event.Handled)
        {
            CameraController* controllerComponent = m_Scene->GetRegistry().try_get<CameraController>(m_ActiveCamera);
            if (!controllerComponent) 
                return false;

            if (m_Scene->GetSceneState() != SceneState::Edit && controllerComponent->IsEditorOnly())
                return false;

            return controllerComponent->OnEvent(event);
        }

        return false;
    }

    bool CameraManager::OnKeyPressed(KeyPressedEvent& event)
    {
        return false;
    }

    void CameraManager::OnBeginPlayMode()
    {
        if (EngineContext::GetEngine()->IsEditorEnabled() && m_ActiveCamera == m_EditorCamera)
        {
            if (m_DefaultCamera == entt::null) return;
            //set active camera to main scene camera
            m_ActiveCamera = m_DefaultCamera;
            GameObject activeCam = GameObject(m_ActiveCamera, m_Scene);
            if (!activeCam) return;

            CameraComponent& cameraComponent = activeCam.GetComponent<CameraComponent>();

            glm::vec2 viewport = EngineContext::GetEngine()->GetViewportSize();

            cameraComponent.Cam.SetAspectRatio(viewport.x / viewport.y);
        }
    }

    //void CameraManager::OnPlayModeUpdate()
    //{}

    void CameraManager::OnEndPlayMode()
    {
        if (EngineContext::GetEngine()->IsEditorEnabled())
        {
            m_ActiveCamera = m_EditorCamera;

            GameObject activeCam = GameObject(m_ActiveCamera, m_Scene);
            if (!activeCam) return;

            CameraComponent& cameraComponent = activeCam.GetComponent<CameraComponent>();

            glm::vec2 viewport = EngineContext::GetEngine()->GetViewportSize();

            cameraComponent.Cam.SetAspectRatio(viewport.x / viewport.y);
        }
    }

    void CameraManager::OnBeginEditMode()
    {
        if (EngineContext::GetEngine()->IsEditorEnabled())
        {
            m_ActiveCamera = m_EditorCamera;

            GameObject activeCam = GameObject(m_ActiveCamera, m_Scene);
            if (!activeCam) return;

            CameraComponent& cameraComponent = activeCam.GetComponent<CameraComponent>();

            glm::vec2 viewport = EngineContext::GetEngine()->GetViewportSize();

            cameraComponent.Cam.SetAspectRatio(viewport.x / viewport.y);
        }
    }

    void CameraManager::OnCameraAdded(entt::entity entity, CameraComponent& cameraComponent)
    {
        glm::vec2 viewport = EngineContext::GetEngine()->GetViewportSize();

        cameraComponent.Cam.SetAspectRatio(viewport.x / viewport.y);

        if (m_DefaultCamera != entt::null && m_DefaultCamera != entt::tombstone) return ; // early out b/c we have default already

        GameObject cameraObject = GameObject(entity, m_Scene);

        if (cameraObject.TryGetComponent<HeirarchyComponent>() == nullptr) return; // early out b/c this is an editor not scene camera

        m_DefaultCamera = entity;

    }

    bool CameraManager::IsCameraRemovable(entt::entity entityToRemove)
    {
        entt::registry& registry = m_Scene->GetRegistry();
        if (m_EditorCamera == entityToRemove)
        {
            Logger::Core(LOG_WARNING, "You may not delete the editor camera");
            return false;
        }

        auto view = registry.view<CameraComponent>();
        if (view.size() < 2)
        {
            Logger::Core(LOG_WARNING, "You may not delete the only camera");
            return false;
        }

        int sceneCameraCount = 0;
        for (auto entityHandle : view)
        {
            HeirarchyComponent* component = registry.try_get<HeirarchyComponent>(entityHandle);
            if(component) sceneCameraCount++;
        }
        
        if (sceneCameraCount < 2)
        {
            Logger::Core(LOG_WARNING, "You may not delete the only non editor camera");
            return false;
        }
        return true;
    }

    bool CameraManager::OnRemoveCamera(entt::entity entityToRemove)
    {
        if(!IsCameraRemovable(entityToRemove)) return false;
        //GameObject toRemove = GameObject(entityToRemove, m_Scene);

        entt::registry& registry = m_Scene->GetRegistry();

        auto view = registry.view<CameraComponent>();
        if (entityToRemove == m_DefaultCamera)
        {
            for (auto entity : view)
            {
                if (entity != entityToRemove)
                {
                    m_DefaultCamera = GameObject(entity, m_Scene);
                    break;
                }
            }
        }

        if (entityToRemove == m_ActiveCamera)
        {
            m_ActiveCamera = m_DefaultCamera;
            registry.get<CameraComponent>(m_ActiveCamera).IsActive = true;
        }

        return true;
    }

    void CameraManager::OnViewportSizeEvent(float width, float height)
    {
        GameObject activeCam = GameObject(m_ActiveCamera, m_Scene);

        // if active cam is not entt valid we don't want to try and get components for it
        if (!activeCam) return; 

        //cam controller tracks viewport size for move speed stuff
        CameraController& controller = activeCam.GetComponent<CameraController>();
        controller.OnViewportSizeChange(width, height);

        // cam component.cam for rendering viewport size 
        activeCam.GetComponent<CameraComponent>().Cam.SetAspectRatio(width / height);
    }

    void CameraManager::SetActiveCamera(GameObject& gameObject)
    {
        if (m_ActiveCamera == gameObject.GetEnttHandle()) return;
        CameraComponent* nextCamera = gameObject.TryGetComponent<CameraComponent>();
        if (!nextCamera) return;

        if (m_ActiveCamera != entt::null && m_ActiveCamera != entt::tombstone)
        {
            CameraComponent& previousCamera = m_Scene->GetRegistry().get<CameraComponent>(m_ActiveCamera);
            previousCamera.IsActive = false;
        }

        glm::vec2 viewport = EngineContext::GetEngine()->GetViewportSize();
        nextCamera->Cam.SetAspectRatio(viewport.x / viewport.y);
        nextCamera->IsActive = true;
        m_ActiveCamera = gameObject;
    }

    void CameraManager::SetEditorCamActive()
    {
        GameObject editorCam = GameObject(m_EditorCamera, m_Scene);
        SetActiveCamera(editorCam);
        //if (m_ActiveCamera == m_EditorCamera) return; // already active
        //CameraComponent* editorCamComponent = m_Scene->GetRegistry().try_get<CameraComponent>(m_EditorCamera);
        //if (!editorCamComponent) return; // consider throwing an error if this happens in editor mode?

        //if (m_ActiveCamera != entt::null && m_ActiveCamera != entt::tombstone)
        //{
        //    CameraComponent& previousCamera = m_Scene->GetRegistry().get<CameraComponent>(m_ActiveCamera);
        //    previousCamera.IsActive = false;
        //}

        //glm::vec2 viewport = EngineContext::GetEngine()->GetViewportSize();
        //editorCamComponent->Cam.SetAspectRatio(viewport.x / viewport.y);
        //m_ActiveCamera = m_EditorCamera; 
    }

    Camera* CameraManager::GetActiveCamera()
    {
        entt::registry& registry = m_Scene->GetRegistry();
        CameraComponent* component = registry.try_get<CameraComponent>(m_ActiveCamera);
        if (!component) return nullptr;

        return &(component->Cam);
    }

    Camera* CameraManager::GetActiveCamera(glm::mat4& viewMatrix)
    {
        GameObject activeCam = GameObject(m_ActiveCamera, m_Scene);

        if (!activeCam) return nullptr;
        TransformComponent& transform = activeCam.GetComponent<TransformComponent>();

        //glm::vec3 position = transform.GetPosition();
        //glm::vec3 rotation = transform.GetRotationEuler(AngleType::Radians);
        //glm::vec3 direction;
        //direction.x = cos(rotation.y * cos(rotation.x));
        //direction.y = sin(rotation.x);
        //direction.z = sin(rotation.y) * cos(rotation.x);
        //glm::vec3 forward = glm::normalize(direction);


        //glm::vec3 right; /*= glm::normalize(glm::cross(forward, glm::vec3(0.0f, rotation.z, 0.0f)));*/
        //right.x = cos(rotation.y);
        //right.y = 0.0f;
        //right.z = -sin(rotation.y);

        //glm::vec3 up = glm::normalize(glm::cross(right, forward));

        viewMatrix = glm::inverse(transform.GetObjectToWorldMatrix());//glm::lookAt(position, position + forward, up);


        CameraComponent& cameraComponent = activeCam.GetComponent<CameraComponent>();
        return &cameraComponent.Cam;
    }

    void CameraManager::SetDefaultCamera(GameObject& gameObject)
    {
        if (m_DefaultCamera == gameObject) return;
        CameraComponent* nextCamera = gameObject.TryGetComponent<CameraComponent>();
        if (!nextCamera) return;

        if (m_DefaultCamera != entt::null && m_DefaultCamera != entt::tombstone)
        {
            CameraComponent& previousCamera = m_Scene->GetRegistry().get<CameraComponent>(m_DefaultCamera);
            previousCamera.IsDefault = false;
        }

        nextCamera->IsDefault = true;
        m_DefaultCamera = gameObject;
    }

    glm::mat4 CameraManager::GetProjectionOutView(Camera& inCamera, TransformComponent& inTransform, glm::mat4& outViewMatrix)
    {
        outViewMatrix = glm::inverse(inTransform.GetObjectToWorldMatrix());//glm::lookAt(position, position + forward, up);

        return inCamera.ProjectionMatrix();
    }

    void CameraManager::FocusOnGameObject(std::shared_ptr<GameObject> targetObject, bool bRotateOnly)
    {
        TransformComponent& targetTransform = targetObject->GetTransform();

        GameObject activeCamObject = GetActiveCameraObject();
        TransformComponent& cameraTransform = activeCamObject.GetTransform();
        
        if (!bRotateOnly)
        {
            glm::vec3 newPosition = targetTransform.GetPosition();
            newPosition += m_TargetPosOffset;
            cameraTransform.SetPosition(newPosition);
            cameraTransform.SetRotationEuler(m_TargetRotation);
        }
        else
        {
            cameraTransform.LookAt(targetTransform.GetPosition());
        }

    }

    GameObject CameraManager::GetActiveCameraObject()
    {
        return GameObject(m_ActiveCamera, m_Scene);
    }

    GameObject CameraManager::GetDefaultCamera()
    {
        return GameObject{ m_DefaultCamera, m_Scene };
    }

}