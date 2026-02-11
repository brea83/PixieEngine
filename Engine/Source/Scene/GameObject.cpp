#include "BsPrecompileHeader.h"
#include "GameObject.h"
#define GLFW_INCLUDE_NONE
#include "EngineContext.h"
#include "Scene.h"
#include "Scene/Components/Component.h"
#include "Components/Transform.h"
#include "Components/MeshComponent.h"
#include "Components/CameraController.h"
#include "Components/CollisionComponent.h"

namespace Pixie
{
	static int s_NumGameObjects = 1;
	GameObject::GameObject(entt::entity entity, std::shared_ptr<Scene> scene)
		: Entity(entity, scene)
	{}

	GameObject::~GameObject()
	{}

	void GameObject::OnUpdate(float deltaTime)
	{
		//Logger::Core(LOG_TRACE, "GameObject update, entity id: {}", (int)m_EntityHandle);
		
		Move(deltaTime);
	}

	void GameObject::Move(float deltaTime)
	{
		MovementComponent* moveComponent = TryGetComponent<MovementComponent>();
		if (moveComponent == nullptr) 
			return;

		if (moveComponent->Direction == glm::vec3(0.0f))
			return;

		TransformComponent& transform = GetTransform();

			glm::vec3 direction = glm::normalize(moveComponent->Direction);
			float velocity = moveComponent->Speed * deltaTime; // adjust accordingly

			glm::vec3 forward = transform.Forward() * direction.z;
			glm::vec3 right = transform.Right() * direction.x;
			glm::vec3 up = transform.Up() * direction.y;

			glm::vec3 currentPosition = transform.GetPosition();

			transform.SetPosition(currentPosition + (velocity * (forward + right + up)));
	
	}

	void GameObject::Serialize(StreamWriter* fileWriter, const GameObject& object)
	{
		if (!object.HasCompoenent<IDComponent>())
		{
			NameComponent* nameComponent = object.TryGetComponent<NameComponent>();
			std::string name = nameComponent == nullptr ? "un-named entity" : nameComponent->Name;
			Logger::Core(LOG_WARNING, "Skipping Serializeation of {} because it has no GUID", name);
			return;
		}

		uint64_t guid = (uint64_t)object.GetGUID();
		fileWriter->WriteRaw<uint64_t>(guid);

		TagComponent* tag = object.TryGetComponent<TagComponent>();
		NameComponent* name = object.TryGetComponent<NameComponent>();
		HeirarchyComponent* heirarchy = object.TryGetComponent<HeirarchyComponent>();
		TransformComponent* transform = object.TryGetComponent<TransformComponent>();
		MeshComponent* mesh = object.TryGetComponent<MeshComponent>();
		LightComponent* light = object.TryGetComponent<LightComponent>();
		CameraComponent* camera = object.TryGetComponent<CameraComponent>();
		CameraController* camController = object.TryGetComponent<CameraController>();
		CircleRendererComponent* circleComponent = object.TryGetComponent<CircleRendererComponent>();
		CollisionComponent* collision = object.TryGetComponent<CollisionComponent>();
		PlayerInputComponent* inputComponent = object.TryGetComponent<PlayerInputComponent>();
		MovementComponent* movement = object.TryGetComponent<MovementComponent>();
		SplineComponent* spline = object.TryGetComponent<SplineComponent>();

		std::vector<SerializableComponentID> components;
		if (tag) components.push_back(SerializableComponentID::TagComponent);
		if (name) components.push_back(SerializableComponentID::NameComponent);
		if (heirarchy) components.push_back(SerializableComponentID::HeirarchyComponent);
		if (transform) components.push_back(SerializableComponentID::TransformComponent);
		if (mesh) components.push_back(SerializableComponentID::MeshComponent);
		if (light) components.push_back(SerializableComponentID::LightComponent);
		if (camera) components.push_back(SerializableComponentID::CameraComponent);
		if (camController) components.push_back(SerializableComponentID::CameraController);
		if (circleComponent) components.push_back(SerializableComponentID::CircleRenderer);
		if (collision) components.push_back(SerializableComponentID::CollisionComponent);
		if (inputComponent) components.push_back(SerializableComponentID::PlayerInput);
		if (movement) components.push_back(SerializableComponentID::MovementComponent);
		if (spline) components.push_back(SerializableComponentID::SplineComponent);

		fileWriter->WriteArray<SerializableComponentID>(components);

		for (auto id : components)
		{
			if (id == SerializableComponentID::TagComponent)
				fileWriter->WriteObject(object.GetComponent<TagComponent>());

			if (id == SerializableComponentID::NameComponent)
				fileWriter->WriteObject(object.GetComponent<NameComponent>());

			if (id == SerializableComponentID::HeirarchyComponent)
				fileWriter->WriteObject(object.GetComponent<HeirarchyComponent>());

			if (id == SerializableComponentID::TransformComponent)
				fileWriter->WriteRaw<TransformComponent>(object.GetComponent<TransformComponent>());

			if (id == SerializableComponentID::MeshComponent)
				fileWriter->WriteObject(object.GetComponent<MeshComponent>());

			if (id == SerializableComponentID::LightComponent)
				fileWriter->WriteRaw<LightComponent>(object.GetComponent<LightComponent>());

			if (id == SerializableComponentID::CameraComponent)
				fileWriter->WriteRaw(object.GetComponent<CameraComponent>());

			if (id == SerializableComponentID::CameraController)
				fileWriter->WriteRaw(object.GetComponent<CameraController>());
			
			if (id == SerializableComponentID::CircleRenderer)
				fileWriter->WriteObject(object.GetComponent<CircleRendererComponent>());

			if (id == SerializableComponentID::CollisionComponent)
			{
				CollisionComponent& component = object.GetComponent<CollisionComponent>();
				fileWriter->WriteObject(component);
				switch (component.Type)
				{
					case Pixie::ColliderType::Sphere:
					{
						SphereCollider& collider = object.GetComponent<SphereCollider>();
						fileWriter->WriteObject(collider);
						break;
					}
					case Pixie::ColliderType::Plane:
					{
						PlaneCollider& collider = object.GetComponent<PlaneCollider>();
						fileWriter->WriteObject(collider);
						break;
					}
					case Pixie::ColliderType::Cube:
					{
						CubeCollider& collider = object.GetComponent<CubeCollider>();
						fileWriter->WriteObject(collider);
						break;
					}
					case Pixie::ColliderType::Mesh:
					{
						MeshCollider& collider = object.GetComponent<MeshCollider>();
						fileWriter->WriteObject(collider);
						break;
					}
					default:
					{
						break;
					}
				}

			}

			if (id == SerializableComponentID::MovementComponent)
				fileWriter->WriteObject(object.GetComponent<MovementComponent>());

			if (id == SerializableComponentID::SplineComponent)
				fileWriter->WriteObject(object.GetComponent<SplineComponent>());

		}

	}

	bool GameObject::Deserialize(StreamReader* fileReader, GameObject& object)
	{
		uint64_t guid;
		fileReader->ReadRaw<uint64_t>(guid);

		object.GetOrAddComponent<IDComponent>().ID = guid;

		std::vector<SerializableComponentID> components;
		fileReader->ReadArray<SerializableComponentID>(components);

		for (auto id : components)
		{
			if (id == SerializableComponentID::TagComponent)
			{
				fileReader->ReadObject(object.GetOrAddComponent<TagComponent>());
				continue;
			}

			if (id == SerializableComponentID::NameComponent)
			{
				fileReader->ReadObject(object.GetOrAddComponent<NameComponent>());
				continue;
			}

			if (id == SerializableComponentID::HeirarchyComponent)
			{
				fileReader->ReadObject(object.GetOrAddComponent<HeirarchyComponent>());
				continue;
			}

			if (id == SerializableComponentID::TransformComponent)
			{
				fileReader->ReadRaw<TransformComponent>(object.GetOrAddComponent<TransformComponent>());
				continue;
			}

			if (id == SerializableComponentID::MeshComponent)
			{
				fileReader->ReadObject(object.GetOrAddComponent<MeshComponent>());
				continue;
			}

			if (id == SerializableComponentID::LightComponent)
			{
				fileReader->ReadRaw<LightComponent>(object.GetOrAddComponent<LightComponent>());
				continue;
			}

			if (id == SerializableComponentID::CameraComponent)
			{
				fileReader->ReadRaw(object.GetOrAddComponent<CameraComponent>());
				continue;
			}

			if (id == SerializableComponentID::CameraController)
			{
				fileReader->ReadRaw(object.GetOrAddComponent<CameraController>());
				continue;
			}

			if (id == SerializableComponentID::CircleRenderer)
			{
				fileReader->ReadObject(object.GetOrAddComponent<CircleRendererComponent>());
				continue;
			}

			if (id == SerializableComponentID::CollisionComponent)
			{
				CollisionComponent& component = object.GetOrAddComponent<CollisionComponent>();
				fileReader->ReadObject(component);

				switch (component.Type)
				{
				case Pixie::ColliderType::Sphere:
				{
					SphereCollider& collider = object.GetOrAddComponent<SphereCollider>();
					fileReader->ReadObject(collider);
					collider.Transform = &object.GetTransform();
					break;
				}
				case Pixie::ColliderType::Plane:
				{
					PlaneCollider& collider = object.GetOrAddComponent<PlaneCollider>();
					fileReader->ReadObject(collider);
					collider.Transform = &object.GetTransform();
					break;
				}
				case Pixie::ColliderType::Cube:
				{
					CubeCollider& collider = object.GetOrAddComponent<CubeCollider>();
					fileReader->ReadObject(collider);
					collider.Transform = &object.GetTransform();
					break;
				}
				case Pixie::ColliderType::Mesh:
				{
					MeshCollider& collider = object.GetOrAddComponent<MeshCollider>();
					fileReader->ReadObject(collider);
					collider.Transform = &object.GetTransform();
					break;
				}
				default:
				{
					break;
				}
				}

				continue;
			}

			if (id == SerializableComponentID::MovementComponent)
			{
				MovementComponent& component = object.GetOrAddComponent<MovementComponent>();
				fileReader->ReadObject(component);
				continue;
			}

			if (id == SerializableComponentID::PlayerInput)
			{
				object.GetOrAddComponent<PlayerInputComponent>();
				continue;
			}

			if (id == SerializableComponentID::SplineComponent)
			{
				SplineComponent& component = object.GetOrAddComponent< SplineComponent>();

				fileReader->ReadObject(component);
				continue;
			}

			
		}

		return true;
	}

	TransformComponent& GameObject::GetTransform()
	{
		return m_Scene->GetRegistry().get_or_emplace<TransformComponent>(*this);
	}

	void GameObject::SetParentNone()
	{
		GameObject parent = GetParent();
		if (parent)
		{
			parent.RemoveChild(*this);
		}

		HeirarchyComponent& family = GetComponent<HeirarchyComponent>();
		family.Parent = 0;

		TransformComponent* transform = TryGetComponent<TransformComponent>();
		if (transform)
		{
			transform->m_ParentGuid = 0;
		}
	}

	void GameObject::SetParent(GameObject& newParent, bool bSentFromAddChild)
	{
		HeirarchyComponent& family = GetComponent<HeirarchyComponent>();
		if (family.Parent != 0)
		{
			
			GameObject parentObject = m_Scene->FindGameObjectByGUID(family.Parent);
			parentObject.RemoveChild(*this);
		}

		family.Parent = newParent.GetGUID();
		TransformComponent& transform = GetComponent<TransformComponent>();
		transform.m_ParentGuid = newParent.GetGUID();

		if (!bSentFromAddChild)
		{
			GameObject parentObject = m_Scene->FindGameObjectByGUID(family.Parent);
			if(parentObject)
				parentObject.AddChild(*this, true);
		}
	}

	GameObject GameObject::GetParent()
	{
		HeirarchyComponent& family = GetComponent<HeirarchyComponent>();
		if (family.Parent != 0)
		{
			return m_Scene->FindGameObjectByGUID(family.Parent);
		}
		return GameObject();
	}

	std::vector<GameObject> GameObject::GetChildren()
	{
		std::vector<GameObject> children;
		HeirarchyComponent& family = GetComponent<HeirarchyComponent>();
		if (family.Children.size() > 0)
		{
			for (auto guid : family.Children)
			{
				GameObject child = m_Scene->FindGameObjectByGUID(guid);
				if(child)
					children.push_back(child);
			}
		}
		return children;
	}

	//void GameObject::UnParent(GameObject grandParent, bool bKeepWorldPosition)
	//{
	//	
	//	HeirarchyComponent& family = GetComponent<HeirarchyComponent>();
	//	TransformComponent& transform = GetComponent<TransformComponent>();
	//	transform.UnParent(m_Scene, *this, grandParent);
	//	family.Parent = grandParent.GetGUID();
	//}

	void GameObject::AddChild(GameObject& child, bool bSentFromSetParent)
	{
		if (!m_Scene->GetRegistry().valid(child)) return;

		//m_Scene->FindGameObjectByGUID(guid);
		GUID childGuid = child.GetGUID();
		HeirarchyComponent& family = GetComponent<HeirarchyComponent>();
		auto itterator = std::find(family.Children.begin(), family.Children.end(), childGuid);

		if (itterator != family.Children.end())
		{
			// already is a child. do nothing
			return;
		}

		family.Children.push_back(childGuid);

		if (!bSentFromSetParent)
		{
			child.SetParent(*this, true);
		}
	}

	void GameObject::RemoveChild(GameObject& child)
	{
		if (!m_Scene->GetRegistry().valid(child)) return;
		HeirarchyComponent& family = GetComponent<HeirarchyComponent>();

		if (family.Children.empty()) return;

		GUID childGuid = child.GetGUID();
		auto itterator = std::find(family.Children.begin(), family.Children.end(), childGuid);
		if (itterator != family.Children.end())
		{
			family.Children.erase(itterator);
		}
	}
}