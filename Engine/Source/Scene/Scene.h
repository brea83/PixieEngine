#pragma once
#include "Core.h"
#include <glm/glm.hpp>
#include <EnTT/entt.hpp>
#include "GUID.h"
#include "CameraManager.h"
#include "Events/Event.h"
#include "Scene/Components/Component.h"
#include "Components/CameraController.h"

namespace Pixie
{
	class GameObject;
	class MeshComponent;
	class Camera;
	struct CameraComponent;
	class Entity;

	enum class SceneState
	{
		UnInitialized = -1,
		Edit = 0,
		Play = 1,
		Pause = 2,
		SimulatePhysics = 3
	};

	class Scene
	{
	public:
		Scene();
		~Scene();

		std::string GetName() const { return m_Name; }
		void SetName(const std::string& newName) { m_Name = newName; }
		
		GameObject CreateEmptyGameObject(const std::string& name);
		GameObject CreateGameObjectWithGUID(GUID guid, const std::string& name = "");
		void RemoveGameObject(GameObject objectToRemove);
		//void RemoveEntity(entt::entity entityHandle);
		bool IsEntityValid(entt::entity entity) const { return m_Registry.valid(entity); }
		
		GameObject GetGameObjectByEntityHandle(entt::entity entityHandle);
		GameObject FindGameObjectByName(const std::string& name);
		GameObject FindGameObjectByGUID(GUID guid);

		GameObject DuplicateGameObject(GameObject sourceObject);
		GameObject DuplicateChild(GameObject destinationParent, GameObject sourceChild);

		void Initialize();
		void PopulateWithTestObjects();
		void CollisionStressTest(int colliderCount = 20);

		Scene* Copy(Scene* sourceScene);

		void BeginPlayMode();
		void EndPlayMode();
		void OnUpdate(float deltaTime);
		void OnEditorUpdate(float deltaTime);

		bool OnEvent(Event& event);

		entt::registry& GetRegistry() { return m_Registry; }
		const entt::registry& GetRegistry() const{ return m_Registry; }
		Entity CreateEntity(const std::string& name = "");
		Entity CreateEntityWithGUID(GUID guid, const std::string& name = "");
		Entity DuplicateEntity(Entity source);

		GameObject CreateCube();
		// creates a plane rotated to match the provided rotation in degrees default is paralell to the XZ plane
		GameObject CreatePlane(glm::vec3 eulerRotation = glm::vec3(0.0f));
		GameObject CreateSphere();
		GameObject TryCreateDirectionalLight();
		

		CameraManager& GetCameraManager() { return m_CameraManager; }
		void ForwardAspectRatio(float width, float height);
		Camera* GetActiveCamera();
		Camera* GetActiveCamera(glm::mat4& viewMatrix);
		GameObject GetActiveCameraGameObject();
		void SetActiveCamera(GameObject& gameObject);
		void SetDefaultCamera(GameObject& gameObject);

		bool TryRemoveCamera(entt::entity entityHandle);

		GameObject GetMainLight();

		// ToStudy: why do I need to declare all the sub templates, why isn't the first one enough?
		template<typename T>
		void OnComponentAdded(Entity& entity, T& component) { }
		template<>
		void OnComponentAdded<IDComponent>(Entity& entity, IDComponent& component) { }
		template<>
		void OnComponentAdded<TagComponent>(Entity& entity, TagComponent& component) { }
		template<>
		void OnComponentAdded<NameComponent>(Entity& entity, NameComponent& component) { }
		template<>
		void OnComponentAdded<HeirarchyComponent>(Entity& entity, HeirarchyComponent& component) { }
		template<>
		void OnComponentAdded<TransformComponent>(Entity& entity, TransformComponent& component) { }
		template<>
		void OnComponentAdded<MeshComponent>(Entity& entity, MeshComponent& component) { }
		template<>
		void OnComponentAdded<CircleRendererComponent>(Entity& entity, CircleRendererComponent& component) { }
		template<>
		void OnComponentAdded<LightComponent>(Entity& entity, LightComponent& component) { }
		template<>
		void OnComponentAdded<CollisionComponent>(Entity& entity, CollisionComponent& component) { }
		template<>
		void OnComponentAdded<SphereCollider>(Entity& entity, SphereCollider& component) { }
		template<>
		void OnComponentAdded<CubeCollider>(Entity& entity, CubeCollider& component) { }
		template<>
		void OnComponentAdded<PlaneCollider>(Entity& entity, PlaneCollider& component) { }
		template<>
		void OnComponentAdded<MeshCollider>(Entity& entity, MeshCollider& component) { }
		
		template<>
		void OnComponentAdded<CameraComponent>(Entity& entity, CameraComponent& component);
		template<>
		void OnComponentAdded<CameraController>(Entity& entity, CameraController& component);
		
	private:
		std::string m_Name{ "New Scene" };
		entt::registry m_Registry;


		CameraManager m_CameraManager;
		SceneState m_SceneState{ SceneState::UnInitialized };

		friend class SceneSerializer;
		friend class Entity;
	};


}
