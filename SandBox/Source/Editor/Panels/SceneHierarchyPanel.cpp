#include "BsPrecompileHeader.h"
#include "SceneHierarchyPanel.h"
#include <Pixie.h>
#include "ImGui/ImGuiPanel.h"

//#include "Scene/Scene.h"
//#include "Scene/GameObject.h"
//#include "Scene/Components/Component.h"
//#include "Graphics/Primitives/TransformComponent.h"

namespace Pixie
{
	void SceneHierarchyPanel::Draw()
	{
		ImGui::Begin("HierarchyTree");
		if (m_CurrentScene == nullptr)
		{
			m_Selected = nullptr;
			ImGui::End();
			return;// entt::null;
		}
		if(ImGui::BeginChild("##UnparentZone"))
		{
			entt::registry& registry = m_CurrentScene->GetRegistry();


			auto view = registry.view<HeirarchyComponent>();
			if (!view)
			{
				ImGui::End();
				return;// selected;
			}

			for (entt::entity entity : view)
			{
				GameObject currentObject{ entity, m_CurrentScene };
				auto [family] = view.get(entity);
				if (family.Parent != 0) continue;

				DrawNode(currentObject);
			}

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			{
				m_Selected = {};
			}

			// right click on blank space
			if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("Create Empty"))
				{
					m_CurrentScene->CreateEmptyGameObject("Empty Object");
				}

				ImGui::Separator();
				if (ImGui::MenuItem("Add Camera"))
				{
					GameObject newCamera = m_CurrentScene->CreateEmptyGameObject("Camera");
					TransformComponent& transform = newCamera.GetTransform();
					transform.SetPosition(glm::vec3(0.0f, 0.0f, -15.0f));
					transform.SetRotationEuler(glm::vec3(0.0f, 180.0f, 0.0f));
					newCamera.AddComponent<CameraComponent>();
					newCamera.AddComponent<CameraController>();
				}

				ImGui::Separator();
				if (ImGui::BeginMenu("Add Mesh Object"))
				{
					if (ImGui::MenuItem("Cube"))
					{
						m_CurrentScene->CreateCube();
					}
					ImGui::Separator();

					if (ImGui::MenuItem("Sphere"))
					{
						m_CurrentScene->CreateSphere();
					}
					ImGui::Separator();

					if (ImGui::MenuItem("Plane XZ"))
					{
						m_CurrentScene->CreatePlane(glm::vec3(-90.0f, 0.0f, 0.0f));
					}

					if (ImGui::MenuItem("Plane XY"))
					{
						m_CurrentScene->CreatePlane();
					}

					if (ImGui::MenuItem("Plane YZ"))
					{
						m_CurrentScene->CreatePlane(glm::vec3(0.0f, -90.0f, 0.0f));
					}

					ImGui::EndMenu();
				}

				ImGui::Separator();
				if (ImGui::BeginMenu("Add Light"))
				{

					if (ImGui::MenuItem("Directional Light"))
					{
						m_CurrentScene->TryCreateDirectionalLight();
					}

					if (ImGui::MenuItem("Point Light"))
					{
						GameObject newLight = m_CurrentScene->CreateEmptyGameObject("Point Light");
						LightComponent& light = newLight.AddComponent<LightComponent>();
						light.Type = LightType::Point;
					}

					if (ImGui::MenuItem("Spot Light"))
					{
						GameObject newLight = m_CurrentScene->CreateEmptyGameObject("Spot Light");
						TransformComponent& transform = newLight.GetTransform();
						transform.SetPosition(glm::vec3(0.0f, 10.0f, 0.0f));
						transform.SetRotationEuler(glm::vec3(-90.0f, 180.0f, 0.0f));

						LightComponent& light = newLight.AddComponent<LightComponent>();
						light.Type = LightType::Spot;
					}

					ImGui::EndMenu();

				}

				ImGui::EndPopup();
			}

			ImGui::EndChild();
		}
		DropItemUnparents();
		
		ImGui::End();

		//return selected;
	}

	void SceneHierarchyPanel::DrawNode(GameObject& entity)
	{
		std::string& entityName = entity.GetComponent<NameComponent>().Name;
		std::string name = (entityName.empty()) ? "_NameEmpty_" : entityName;

		//is entity a leaf node ie, no children
		//HeirarchyComponent& family = entity.GetComponent<HeirarchyComponent>();
		std::vector<GameObject> children = entity.GetChildren();
		bool bLeafNode = children.empty();

		ImGuiTreeNodeFlags flags = ((m_Selected != nullptr && *m_Selected == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (bLeafNode)
		{
			flags |= ImGuiTreeNodeFlags_Leaf;
		}
		else
		{
			flags |= ImGuiTreeNodeFlags_OpenOnArrow;
		}

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name.c_str());
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload(m_HeirarchyDragType.c_str(), &entity, sizeof(GameObject));
			ImGui::Text(name.c_str());
			ImGui::EndDragDropSource();
		}
		DropItemAsChild(entity);
		

		if (ImGui::IsItemClicked())
		{
			m_Selected = std::make_shared<GameObject>(entity);
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened)
		{
			if (!bLeafNode)
			{
				for (auto child : children)
				{
					DrawNode(child);
				}
			}

			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			entity.m_Scene->RemoveGameObject(entity);
			if (m_Selected != nullptr && *m_Selected == entity)
				m_Selected = nullptr;
		}
	}
	void SceneHierarchyPanel::OnSceneChange(std::shared_ptr<Scene> newScene)
	{
		if (m_Selected != nullptr)
		{
			m_Selected = nullptr;
		}

		m_CurrentScene = newScene;
	}
	void SceneHierarchyPanel::DropItemUnparents()
	{
		ImGuiDragDropFlags drop_target_flags = ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoPreviewTooltip;
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(m_HeirarchyDragType.c_str(), drop_target_flags))
			{
				ImGui::SetTooltip("Unparent");
				if (payload->IsDelivery())
				{
					IM_ASSERT(payload->DataSize == sizeof(GameObject));
					GameObject droppedObject = *(const GameObject*)payload->Data;
					droppedObject.SetParentNone();
				}
			}
			ImGui::EndDragDropTarget();
		}
	}
	void SceneHierarchyPanel::DropItemAsChild(GameObject& targetObject)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(m_HeirarchyDragType.c_str()))
			{
				IM_ASSERT(payload->DataSize == sizeof(GameObject));
				GameObject droppedObject = *(const GameObject*)payload->Data;

				targetObject.AddChild(droppedObject);
			}
			ImGui::EndDragDropTarget();
		}
	}
}

