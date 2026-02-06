#include "BsPrecompileHeader.h"
#include "Scene.h"
#include "GameObject.h"
#include "Graphics/Primitives/Cube.h"

#include "EngineContext.h"
#include "Entity.h"


namespace Pixie
{
	std::shared_ptr<Scene> Scene::Create()
	{
		return std::make_shared<Scene>(Private());
	}

	void Scene::Initialize()
	{
		m_CameraManager = CameraManager{ shared_from_this(), false };
		m_CameraManager.Init();
		
		if (EngineContext::GetEngine()->IsEditorEnabled())
		{
			m_SceneState = SceneState::Edit;
		}
		else
		{
			m_SceneState = SceneState::Play;
		}

		
	}

	void Scene::PopulateWithTestObjects()
	{
		GameObject mainLight = CreateEmptyGameObject("Main Light");
		LightComponent& light = mainLight.AddComponent<LightComponent>();
		light.Type = LightType::Directional;

		GameObject cube1 = CreateCube();

		GameObject house = CreateEmptyGameObject("Viking House");
		TransformComponent& transform2 = house.GetComponent<TransformComponent>();
		transform2.SetPosition(glm::vec3(2.5f, -0.5f, 0.0f));
		transform2.SetScale(glm::vec3(20.0f));
		house.AddComponent<MeshComponent, const  std::string&, const  std::string&>("../Assets/Meshes/Viking_House.obj", "../Assets/Textures/Viking_House.png");

		GameObject sphere = CreateEmptyGameObject("Sphere");
		TransformComponent& transform3 = sphere.GetComponent<TransformComponent>();
		transform3.SetPosition(glm::vec3(-2.0f, 0.0f, 0.0f));
		sphere.AddComponent<MeshComponent, const std::string&>("../Assets/Meshes/Sphere.obj");

		cube1.AddChild(sphere);

	}
	void Scene::CollisionStressTest(int colliderCount)
	{
		// random generator
		
		for (int i = 0; i < colliderCount; i++)
		{
			glm::vec3 randomPosition;
			randomPosition.x = glm::min((float)(std::rand()) / (float)(rand()), 20.0f);
			randomPosition.y = glm::min((float)(std::rand()) / (float)(rand()), 20.0f);
			randomPosition.z = glm::min((float)(std::rand()) / (float)(rand()), 20.0f);

			GameObject sphere = CreateEmptyGameObject("Sphere");
			TransformComponent& transform3 = sphere.GetComponent<TransformComponent>();
			transform3.SetPosition(randomPosition);

			CollisionComponent& collisionComp = sphere.AddComponent<CollisionComponent>();
			collisionComp.Type = ColliderType::Sphere;
			SphereCollider& collider = sphere.AddComponent<SphereCollider>();

			collider.Radius = glm::min((float)(std::rand()) / (float)(rand()), 3.0f);
		}
	}

	 template<typename Component>
	 // returns pointer to destination's copy of component if successful, nullptr if unsuccessfull
	 static Component* TryCopyEntityComponent(Entity destination, Entity source)
	 {
		 if (source.HasCompoenent<Component>())
		 {
			 Component& comp = source.GetComponent<Component>();
			destination.AddOrReplaceComponent<Component>(comp);
		 }
		 return nullptr;
	 }

	 static void TryCopyAllComponents(Entity destination, Entity source)
	 {
		 
		 TryCopyEntityComponent<TagComponent>(destination, source);
		 TryCopyEntityComponent<TransformComponent>(destination, source);
		 TryCopyEntityComponent<MeshComponent>(destination, source);
		 TryCopyEntityComponent<LightComponent>(destination, source);
		 TryCopyEntityComponent<CameraComponent>(destination, source);
		 TryCopyEntityComponent<CameraController>(destination, source);
		 TryCopyEntityComponent<CollisionComponent>(destination, source);
		 TryCopyEntityComponent<MovementComponent>(destination, source);

		 if (source.HasCompoenent<PlayerInputComponent>())
		 {
			 PlayerInputComponent& sourceComp = source.GetComponent<PlayerInputComponent>();
			 PlayerInputComponent& destinationComp = destination.AddComponent<PlayerInputComponent>();
			 destinationComp.BIsActive = sourceComp.BIsActive;
		 }
	 }
	 static void TryCopyAllComponents(GameObject destination, GameObject source)
	 {
		 Entity duplicate = Entity(destination, destination.GetScene());
		 Entity original = Entity(source, source.GetScene());
		 TryCopyAllComponents(duplicate, original);
	 }

	template<typename Component>
	static void CopyRegistryComponents(entt::registry& destination, entt::registry& source, const std::unordered_map<GUID, entt::entity>& guidToDestinationEntt)
	{
		auto view = source.view<Component>();
		for (auto entity : view)
		{
			IDComponent* guidComponent = source.try_get<IDComponent>(entity);
			if (!guidComponent) 
				continue; // TODO: change how editor only things are handled, they should probably have guids too.
			
			GUID guid = guidComponent->ID;

			if (guidToDestinationEntt.find(guid) != guidToDestinationEntt.end())
			{
				entt::entity newEnttID = guidToDestinationEntt.at(guid);

				auto& sourceComponent = source.get<Component>(entity);

				destination.emplace_or_replace<Component>(newEnttID, sourceComponent);
			}

		}
	}

	std::shared_ptr<Scene> Scene::Copy(std::shared_ptr<Scene> sourceScene)
	{
		std::shared_ptr<Scene> newScene = Scene::Create();

		newScene->m_Name = sourceScene->m_Name + " Copy";
		
		entt::registry& sourceRegistry = sourceScene->GetRegistry();
		entt::registry& destinationRegistry = newScene->GetRegistry();


		std::unordered_map<GUID, entt::entity> guidToDestinationEntt;
		//auto idView = sourceRegistry.view<IDComponent>();
		for (auto&& [entity, id, nameComp] : sourceRegistry.view<IDComponent, NameComponent>().each())
		{
			//GUID guid = sourceRegistry.get<IDComponent>(entity).ID;
			GUID guid = sourceRegistry.get<IDComponent>(entity).ID;
			/*NameComponent* nameComp = sourceRegistry.try_get<NameComponent>(entity);
			if (!nameComp) continue;*/
			std::string name = nameComp.Name;

			Entity newEntity = newScene->CreateEntityWithGUID(guid, name);
			guidToDestinationEntt[guid] = newEntity;
		}

		CopyRegistryComponents<TagComponent>(destinationRegistry, sourceRegistry, guidToDestinationEntt);
		CopyRegistryComponents<HeirarchyComponent>(destinationRegistry, sourceRegistry, guidToDestinationEntt);
		CopyRegistryComponents<TransformComponent>(destinationRegistry, sourceRegistry, guidToDestinationEntt);
		CopyRegistryComponents<MeshComponent>(destinationRegistry, sourceRegistry, guidToDestinationEntt);
		CopyRegistryComponents<LightComponent>(destinationRegistry, sourceRegistry, guidToDestinationEntt);
		CopyRegistryComponents<CameraComponent>(destinationRegistry, sourceRegistry, guidToDestinationEntt);
		CopyRegistryComponents<CameraController>(destinationRegistry, sourceRegistry, guidToDestinationEntt);
		CopyRegistryComponents<CircleRendererComponent>(destinationRegistry, sourceRegistry, guidToDestinationEntt);
		CopyRegistryComponents<CollisionComponent>(destinationRegistry, sourceRegistry, guidToDestinationEntt);
		CopyRegistryComponents<SphereCollider>(destinationRegistry, sourceRegistry, guidToDestinationEntt);
		CopyRegistryComponents<CubeCollider>(destinationRegistry, sourceRegistry, guidToDestinationEntt);
		CopyRegistryComponents<PlayerInputComponent>(destinationRegistry, sourceRegistry, guidToDestinationEntt);
		CopyRegistryComponents<MovementComponent>(destinationRegistry, sourceRegistry, guidToDestinationEntt);
		CopyRegistryComponents<PlayerFollowCompononent>(destinationRegistry, sourceRegistry, guidToDestinationEntt);

		return newScene;
	}


	Scene::~Scene()
	{
		// unload scene objects that do not persist across scenes

	}

	
	void  Scene::BeginPlayMode()
	{
		if (m_SceneState == SceneState::UnInitialized)
			Initialize();
		m_SceneState = SceneState::Play;
		m_CameraManager.OnBeginPlayMode();

		
	}

	void  Scene::EndPlayMode()
	{
		m_SceneState = SceneState::Pause;
		m_CameraManager.OnEndPlayMode();
	}

	void Scene::EditMode()
	{
		m_SceneState = SceneState::Edit;
		m_CameraManager.OnBeginEditMode();
	}

	void Scene::OnUpdate(float deltaTime)
	{
		if (m_SceneState != SceneState::Play) return;

		m_CameraManager.OnPlayUpdate(deltaTime);

		for (auto&& [entity, component] : m_Registry.view<HasUpdateableComponents>().each())
		{
			GameObject gameObject(entity, shared_from_this());
			gameObject.OnUpdate(deltaTime);
		}

	}

	void Scene::OnEditorUpdate(float deltaTime)
	{
		if (m_SceneState != SceneState::Edit) return;
		m_CameraManager.OnEditorUpdate(deltaTime);

	}

	bool Scene::OnEvent(Event& event)
	{
		EventDispatcher dispatcher{ event };
		dispatcher.Dispatch<SceneChangedEvent>(BIND_EVENT_FUNCTION(Scene::OnSceneChanged));
		m_CameraManager.OnEvent(event);
		return event.Handled;
	}

	bool Scene::OnSceneChanged(SceneChangedEvent& event)
	{
		std::shared_ptr<Scene> newScene = event.GetScene();

		if (*newScene != *this)
		{
			m_SceneState = SceneState::Pause;
		}
		return false; // this never consumes the event
	}

	GameObject Scene::CreateEmptyGameObject(const std::string& name)
	{
		GameObject gameObject = { m_Registry.create(), shared_from_this()};
		gameObject.AddComponent<IDComponent>();
		gameObject.AddComponent<TransformComponent>();
		gameObject.AddComponent<HeirarchyComponent>();
		NameComponent& nameComponent = gameObject.AddComponent<NameComponent>();
		nameComponent.Name = name.empty() ? "Empty Object" : name;
		return gameObject;
	}
	
	GameObject Scene::CreateGameObjectWithGUID(GUID guid, const std::string& name)
	{
		GameObject gameObject = { m_Registry.create(), shared_from_this()};
		gameObject.AddComponent<IDComponent>(guid);
		gameObject.AddComponent<TransformComponent>();
		gameObject.AddComponent<HeirarchyComponent>();
		NameComponent& nameComponent = gameObject.AddComponent<NameComponent>();
		nameComponent.Name = name.empty() ? "Empty Object" : name;
		return gameObject;
	}

	void Scene::RemoveGameObject(GameObject objectToRemove)
	{
		if (!m_Registry.valid(objectToRemove))
		{
			Logger::Core(LOG_WARNING, "Tried to remove game object with entt handle: {}, from Scene, but could not find it", int(objectToRemove));
			return;
		}

		if (objectToRemove.TryGetComponent<CameraComponent>())
		{
			if (!TryRemoveCamera(objectToRemove))
			{
				return;
			}
		}

		//check if need to unparent
		GameObject parent = objectToRemove.GetParent();
		//HeirarchyComponent* family = m_Registry.try_get<HeirarchyComponent>(objectToRemove);
		if (parent)
		{
			parent.RemoveChild(objectToRemove);
		}

		std::vector<GameObject> children = objectToRemove.GetChildren();
		for (GameObject child : children)
		{
			if (parent)
				child.SetParent(parent);
			else
				child.SetParentNone();
		}

		m_Registry.destroy(objectToRemove);
	}


	GameObject Scene::GetGameObjectByEntityHandle(entt::entity entityHandle)
	{
		if (m_Registry.valid(entityHandle))
		{
			return GameObject(entityHandle, shared_from_this());
		}
		return GameObject();
	}

	GameObject Scene::FindGameObjectByName(const std::string& name)
	{
		auto view = m_Registry.view<NameComponent>();
		for (auto entity : view)
		{
			const NameComponent& component = view.get<NameComponent>(entity);
			if (component.Name == name)
			{
				return GameObject(entity, shared_from_this());
			}
		}

		return GameObject();
	}

	GameObject Scene::FindGameObjectByGUID(GUID guid)
	{
		auto view = m_Registry.view<IDComponent>();
		for (auto entity : view)
		{
			const IDComponent& component = view.get<IDComponent>(entity);
			if (component.ID == guid)
			{
				return GameObject(entity, shared_from_this());
			}
		}

		return GameObject();
	}

	GameObject Scene::DuplicateGameObject(GameObject sourceObject)
	{
		
		std::string name = sourceObject.GetName() + " Copy";
		GameObject duplicate = CreateEmptyGameObject(name);
		// create new guid for game object 
		TryCopyAllComponents(duplicate, sourceObject);

		bool bNeedsToDupllicateFamily = sourceObject.HasCompoenent<HeirarchyComponent>();
		if (bNeedsToDupllicateFamily)
		{
			HeirarchyComponent& destinationFamily = duplicate.GetComponent<HeirarchyComponent>();

			// send duplicate's info to their new parent
			GameObject parent = sourceObject.GetParent();
			if (parent)
				duplicate.SetParent(parent);

			// recursively create duplicate children
			std::vector<GameObject> children = sourceObject.GetChildren();

			for (GameObject child : children)
			{
				GameObject duplicateChild = DuplicateChild(duplicate, child);
			}
		}
		
		if(sourceObject.HasCompoenent<CollisionComponent>() && sourceObject.GetComponent<CollisionComponent>().Type != ColliderType::END)
		{
			SphereCollider* sphere = TryCopyEntityComponent<SphereCollider>(duplicate, sourceObject);
			if (sphere)
				sphere->Transform = &duplicate.GetTransform();

			CubeCollider* cube = TryCopyEntityComponent<CubeCollider>(duplicate, sourceObject);
			if (cube)
				cube->Transform = &duplicate.GetTransform();

			PlaneCollider* plane = TryCopyEntityComponent<PlaneCollider>(duplicate, sourceObject);
			if (plane)
				plane->Transform = &duplicate.GetTransform();

			MeshCollider* mesh = TryCopyEntityComponent<MeshCollider>(duplicate, sourceObject);
			if (mesh)
				mesh->Transform = &duplicate.GetTransform();
		}
		return duplicate;
	}

	GameObject Scene::DuplicateChild(GameObject destinationParent, GameObject sourceChild)
	{
		std::string name = sourceChild.GetName() + " Copy";
		GameObject duplicateChild = CreateEmptyGameObject(name);
		// create new guid for game object 
		TryCopyAllComponents(duplicateChild, sourceChild);

		HeirarchyComponent& destinationFamily = duplicateChild.GetComponent<HeirarchyComponent>();

		// send duplicate's info to their new parent
		duplicateChild.SetParent(destinationParent);

		// recursively create duplicate children
		std::vector<GameObject> children = sourceChild.GetChildren();

		for (GameObject child : children)
		{
			GameObject duplicateChildofChild = DuplicateChild(duplicateChild, child);
		}

		return duplicateChild;
	}

	void Scene::ForwardAspectRatio(float width, float height)
	{
		m_CameraManager.OnViewportSizeEvent(width, height);
	}

	Camera* Scene::GetActiveCamera()
	{
		return m_CameraManager.GetActiveCamera();
	}

	Camera* Scene::GetActiveCamera(glm::mat4& viewMatrix)
	{
		return m_CameraManager.GetActiveCamera(viewMatrix);
	}

	GameObject Scene::GetActiveCameraGameObject()
	{
		return m_CameraManager.GetActiveCameraObject();
	}

	void Scene::SetActiveCamera(GameObject& gameObject)
	{
		m_CameraManager.SetActiveCamera(gameObject);
		
	}

	void Scene::SetDefaultCamera(GameObject& gameObject)
	{
		m_CameraManager.SetDefaultCamera(gameObject);
	}

	bool Scene::TryRemoveCamera(entt::entity entityHandle)
	{
		if (!m_CameraManager.IsCameraRemovable(entityHandle)) return false;

		m_Registry.remove<CameraComponent>(entityHandle);
		m_CameraManager.OnRemoveCamera(entityHandle);

		return true;
	}

	GameObject Scene::GetMainLight()
	{
		auto view = m_Registry.view<LightComponent>();
		if (!view) return GameObject();

		std::vector<entt::entity> directionalLights;
		for (auto entity : view)
		{
			LightComponent& light = view.get<LightComponent>(entity);
			if (light.Type == Directional) directionalLights.push_back(entity);
		}

		if (directionalLights.size() < 1) return GameObject();
		
		if(directionalLights.size() > 1) Logger::Core(LOG_WARNING, "Warning: more than one directional light in scene, but Scene::GetMainLight() only returns the first one found");

		return GameObject(directionalLights.front(), shared_from_this());
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), shared_from_this()};
		//entity.AddComponent<TransformComponent>();
		NameComponent& nameComponent = entity.AddComponent<NameComponent>();
		nameComponent.Name = name.empty() ? "Empty Entity" : name;
		return entity;
	}

	Entity Scene::CreateEntityWithGUID(GUID guid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), shared_from_this()};
		entity.AddComponent<IDComponent>(guid);
		NameComponent& nameComponent = entity.AddComponent<NameComponent>();
		nameComponent.Name = name.empty() ? "Empty Entity" : name;
		return entity;
	}


	Entity Scene::DuplicateEntity(Entity source)
	{
		std::string name = source.GetName();
		Entity duplicate = CreateEntity(name);

		TryCopyEntityComponent<TagComponent>(duplicate, source);
		TryCopyEntityComponent<HeirarchyComponent>(duplicate, source);
		TryCopyEntityComponent<TransformComponent>(duplicate, source);
		TryCopyEntityComponent<MeshComponent>(duplicate, source);
		TryCopyEntityComponent<LightComponent>(duplicate, source);
		TryCopyEntityComponent<CameraComponent>(duplicate, source);
		TryCopyEntityComponent<CameraController>(duplicate, source);
		return duplicate;
	}

	GameObject Scene::CreateCube()
	{
		GameObject object = CreateEmptyGameObject("Cube");
		object.AddComponent<MeshComponent, const std::string&>("../Assets/Meshes/Cube.obj");
		return object;
	}

	GameObject Scene::CreatePlane(glm::vec3 eulerRotation)
	{
		GameObject object = CreateEmptyGameObject("Plane");
		TransformComponent& transform = object.GetTransform();
		transform.SetRotationEuler(eulerRotation);
		object.AddComponent<MeshComponent, const std::string&>("../Assets/Meshes/quadPlane.obj");
		return object;
	}

	GameObject Scene::CreateSphere()
	{
		GameObject object = CreateEmptyGameObject("Sphere");
		object.AddComponent<MeshComponent, const std::string&>("../Assets/Meshes/Sphere.obj");
		return object;
	}

	GameObject Scene::TryCreateDirectionalLight()
	{
		if (GetMainLight())
		{
			Logger::Core(LOG_WARNING, "Tried to add a second Directional light. Pixie Engine does not yet support multiple directional lights in a scene");
			return GameObject();
		}

		GameObject newLight = CreateEmptyGameObject("Directional Light");
		TransformComponent& transform = newLight.GetTransform();
		transform.SetRotationEuler(glm::vec3(-90.0f, 180.0f, 0.0f));

		LightComponent& light = newLight.AddComponent<LightComponent>();
		light.Type = LightType::Directional;
		return newLight;
	}

	template<>
	 void Scene::OnComponentAdded<CameraComponent>(Entity& entity, CameraComponent& component)
	{
		m_CameraManager.OnCameraAdded(entity.GetEnttHandle(), component);
	}

	template<>
	 void Scene::OnComponentAdded<CameraController>(Entity& entity, CameraController& component)
	{
		 component.UpdateFocalPoint(entity);
	}

}