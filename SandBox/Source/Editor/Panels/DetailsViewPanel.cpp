#include "BsPrecompileHeader.h"
#include "DetailsViewPanel.h"
#include "Pixie.h"

#include "CameraManager.h"
#include "ImGui/ImGuiPanel.h"
#include "Resources/AssetLoader.h"
#include "PlatformUtils.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/misc/cpp/imgui_stdlib.h>

namespace Pixie
{
	bool DetailsViewPanel::Draw(std::shared_ptr<Scene> scene, std::shared_ptr<GameObject> selected)
	{

		ImGui::Begin("Details View");
		{
			if (scene == nullptr || selected == nullptr || selected->GetScene() == nullptr)
			{
				ImGui::End();
				return false;
			}
			entt::registry& registry = scene->GetRegistry();
			if (!registry.valid(*selected))
			{
				ImGui::End();
				return false;
			}

			NameComponent& nameComp = selected->GetComponent<NameComponent>();
			static std::string editingName = nameComp.Name;


			DrawStringProperty("Name", nameComp.Name, editingName);

			if (ImGui::BeginPopupContextItem("AddComponentPopUp"))
			{
				if (ImGui::Selectable("Mesh Component"))
				{
					selected->AddComponent<MeshComponent>();
				}

				if (ImGui::Selectable("Camera Component"))
				{
					selected->AddComponent<CameraComponent>();
					CameraController& controller = selected->AddComponent<CameraController>();
					//controller.UpdateFocalPoint(selected);
				}
				if (ImGui::Selectable("Light"))
				{
					selected->AddComponent<LightComponent>();
				}

				if (ImGui::Selectable("Circle Renderer"))
				{
					selected->AddComponent<CircleRendererComponent>();
				}

				if (ImGui::Selectable("Collision component"))
				{
					selected->AddComponent<CollisionComponent>();
				}

				if (ImGui::Selectable("Player Input component"))
				{
					selected->AddComponent<PlayerInputComponent>();
				}

				if (ImGui::Selectable("Movement component"))
				{
					selected->AddComponent<MovementComponent>();
				}

				if (ImGui::Selectable("Spline component"))
				{
					selected->AddComponent<SplineComponent>();
				}
				ImGui::EndPopup();
			}
			if (ImGui::Button("AddComponent"))
			{
				ImGui::OpenPopup("AddComponentPopUp");
			}

			IDComponent* id = selected->TryGetComponent<IDComponent>();
			
			std::string guid = "0";
			if(id)
				guid = std::to_string((uint64_t)id->ID);

			std::string enttID = std::to_string((uint32_t)selected->GetEnttHandle());

			ImGui::BeginDisabled();
			DrawStringProperty("Player GUID", guid, guid);
			DrawStringProperty("Player Entt ID", enttID, enttID);
			ImGui::EndDisabled();

			ImGui::SeparatorText("Componenets");
			DrawComponents(scene, *selected/*selectedObject->GetAllComponents()*/);

		}
		ImGui::End();
		return true;
	}


	bool DetailsViewPanel::DrawVec3Control(const std::string& label, glm::vec3& values, SliderParams params, float columnWidth)
	{
		bool bValueChanged = false;

		if (ImGui::BeginTable(label.c_str(), 2, ImGuiTableFlags_Resizable))
		{
			float fontSize = ImGui::GetFontSize();
			ImGui::TableSetupColumn("Labels", ImGuiTableColumnFlags_WidthFixed, fontSize * columnWidth);
			ImGui::TableSetupColumn("Values"/*, ImGuiTableColumnFlags_WidthFixed,*/);
			ImGui::TableNextRow();
			// the label
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(label.c_str());

			// the values
			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);// fontSize * 4.0f);

			ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 0.0f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.25f, 0.25f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 1.0f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.7f, 0.0f, 0.05f, 1.0f });
			if (ImGui::Button("X"))
			{
				values.x = params.ResetValue;
				bValueChanged = true;
			}
			ImGui::SameLine();
			if (ImGui::DragFloat("##floatX", &values.x, params.Speed, params.Min, params.Max, params.Format.c_str(), params.Flags))
			{
				bValueChanged = true;
			}
			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();

			ImGui::SameLine();
			ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 0.0f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.3f, 0.55f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.9f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.0f, 0.6f, 0.05f, 1.0f });
			if (ImGui::Button("Y"))
			{
				values.y = params.ResetValue;
				bValueChanged = true;
			}
			ImGui::SameLine();
			if (ImGui::DragFloat("##floatY", &values.y, params.Speed, params.Min, params.Max, params.Format.c_str(), params.Flags))
			{
				bValueChanged = true;
			}
			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();

			ImGui::SameLine();
			ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 0.0f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.25f, 0.25f, 0.7f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.2f, 1.0f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.0f, 0.0f, 0.6f, 1.0f });
			if (ImGui::Button("Z"))
			{
				values.z = params.ResetValue;
				bValueChanged = true;
			}
			ImGui::SameLine();
			if (ImGui::DragFloat("##floatZ", &values.z, params.Speed, params.Min, params.Max, params.Format.c_str(), params.Flags))
			{
				bValueChanged = true;
			}
			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();

			ImGui::PopItemWidth();

			ImGui::EndTable();
		}

		return bValueChanged;
	}

	bool DetailsViewPanel::DrawVec2Control(const std::string& label, glm::vec2& values, SliderParams params, float columnWidth)
	{
		bool bValueChanged = false;

		if (ImGui::BeginTable(label.c_str(), 2, ImGuiTableFlags_Resizable))
		{
			float fontSize = ImGui::GetFontSize();
			ImGui::TableSetupColumn("Labels", ImGuiTableColumnFlags_WidthFixed, columnWidth);
			ImGui::TableSetupColumn("Values"/*, ImGuiTableColumnFlags_WidthFixed,*/);
			ImGui::TableNextRow();
			// the label
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(label.c_str());

			// the values
			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);// fontSize * 4.0f);

			ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 0.0f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.25f, 0.25f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 1.0f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.7f, 0.0f, 0.05f, 1.0f });
			if (ImGui::Button("X"))
			{
				values.x = params.ResetValue;
				bValueChanged = true;
			}
			ImGui::SameLine();
			if (ImGui::DragFloat("##floatX", &values.x, params.Speed, params.Min, params.Max, params.Format.c_str(), params.Flags))
			{
				bValueChanged = true;
			}
			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();

			ImGui::SameLine();
			ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 0.0f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.3f, 0.55f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.9f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.0f, 0.6f, 0.05f, 1.0f });
			if (ImGui::Button("Y"))
			{
				values.y = params.ResetValue;
				bValueChanged = true;
			}
			ImGui::SameLine();
			if (ImGui::DragFloat("##floatY", &values.y, params.Speed, params.Min, params.Max, params.Format.c_str(), params.Flags))
			{
				bValueChanged = true;
			}
			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();

			ImGui::PopItemWidth();

			ImGui::EndTable();
		}

		return bValueChanged;
	}

	bool DetailsViewPanel::DrawFloatControl(const std::string& label, float& value, SliderParams params, float columnWidth)
	{
		bool bValueChanged = false;

		if (ImGui::BeginTable(label.c_str(), 2, ImGuiTableFlags_Resizable))
		{
			float fontSize = ImGui::GetFontSize();
			ImGui::TableSetupColumn("Labels", ImGuiTableColumnFlags_WidthFixed, fontSize * columnWidth);
			ImGui::TableSetupColumn("Values"/*, ImGuiTableColumnFlags_WidthFixed,*/);
			ImGui::TableNextRow();
			// the label
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(label.c_str());

			// the values
			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);

			ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 0.0f);
			if (ImGui::DragFloat("##float", &value, params.Speed, params.Min, params.Max, params.Format.c_str(), params.Flags))
			{
				bValueChanged = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset"))
			{
				value = params.ResetValue;
				bValueChanged = true;
			}

			ImGui::PopStyleVar();

			
			ImGui::PopItemWidth();

			ImGui::EndTable();
		}

		return bValueChanged;
	}


	bool DetailsViewPanel::DrawStringProperty(const std::string& label, std::string& value, std::string& editingValue, float columnWidth)
	{
		bool bValueSubmitted = false;

		if (ImGui::BeginTable(label.c_str(), 2, ImGuiTableFlags_Resizable/* | ImGuiTableFlags_RowBg*/))
		{
			float fontSize = ImGui::GetFontSize();
			ImGui::TableSetupColumn("Labels", ImGuiTableColumnFlags_WidthFixed, fontSize * columnWidth);
			ImGui::TableSetupColumn("Values", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			// the label
			ImGui::TableSetColumnIndex(0);
			ImGui::PushItemWidth(fontSize * columnWidth);
			ImGui::Text(label.c_str());
			ImGui::PopItemWidth();

			// the values
			ImGui::TableSetColumnIndex(1);
			//ImGui::PushItemWidth(fontSize * value.size());
			// do stuff

			ImGui::InputTextWithHint("##EditableString", value.c_str(), &editingValue);
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				Logger::Core(LOG_DEBUG, "DetailsViewPanel::DrawSTringProperty() pressed enter on text input");
				if (value != editingValue)
				{
					if (editingValue != "" && editingValue != " ")
					{
						value = editingValue;
						bValueSubmitted = true;
					}
				}

			}

			if (!ImGui::IsItemActivated())
			{
				editingValue = value;
			}

			//ImGui::PopItemWidth();

			ImGui::EndTable();
		}

		return bValueSubmitted;
	}

	void DetailsViewPanel::DrawComponents(std::shared_ptr<Scene> scene, GameObject& selected)
	{
		entt::registry& registry = scene->GetRegistry();
		if (selected.HasCompoenent<TransformComponent>())
		{
			ImGui::SeparatorText("Transform");

			TransformComponent& transform = selected.GetTransform();
			
			SliderParams params;
			params.Speed = 0.01f;
			if (DrawVec3Control("Position", transform.m_Position, params))
			{
				transform.m_PositionDirty = true;
			}

			//translate rotation from radians to degrees
			glm::vec3 eulerDegrees = transform.GetRotationEuler();
			if (DrawVec3Control("Rotation", eulerDegrees, params))
			{
				transform.SetRotationEuler(eulerDegrees);
			}

			params.ResetValue = 1.0f;
			if (DrawVec3Control("Scale", transform.m_Scale, params))
			{
				transform.m_ScaleDirty = true;
			}

			ImGui::TextWrapped("There is a known issue with ImGuizmo's Rotation gizmo:");
			ImGui::TextWrapped("If the camera forward vector is paralel to one of the gizmo circle planes those handles will not behave.");
		}

		if (selected.HasCompoenent<SplineComponent>())
		{
			ImGui::PushID("Spline");
			ImGui::Separator();
			SplineComponent& component = selected.GetComponent<SplineComponent>();
			ImGui::Text("Slpine Component");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 25.0f);

			ImVec2 buttonSize{ ImGui::CalcTextSize("X").x + (ImGui::GetStyle().FramePadding.x * 2.0f),
			ImGui::CalcTextSize("X").y + (ImGui::GetStyle().FramePadding.y * 2.0f) };

			bool removeComponent{ false };
			if (ImGui::Button("X", buttonSize))
			{
				removeComponent = true;
			}

			ImGui::Separator();

			ImGui::Text("Type");
			ImGui::SameLine();
			ImGui::BeginDisabled();
			int currentType = static_cast<int>(component.Type);
			if (ImGui::Combo("##ColliderType", &currentType, SplineComponent::TypeNames, IM_ARRAYSIZE(SplineComponent::TypeNames)))
			{
				Logger::Core(LOG_DEBUG, "test");
			}

			ImGui::Text("Is Loop ");
			ImGui::SameLine();
			ImGui::Checkbox("##isLoop", &component.IsLoop);
			ImGui::EndDisabled();


			float previewTime = component.PreviewTime;
			float maxTime = component.GetNumSegments() + 1.0f;
			SliderParams params;
			params.Min = 0.0f;
			params.Max = maxTime;
			params.Speed = 0.01f;

			DrawFloatControl("Preview T", previewTime, params);

			ImGui::Text("Debug Color");
			ImGui::SameLine();
			ImGui::ColorEdit3("##Color", glm::value_ptr(component.DebugColor));

			ImGui::Text("Segments ");
			ImGui::SameLine();
			int segmentCount = component.GetNumSegments();
			if (ImGui::InputInt("##segmentCount", &segmentCount))
			{
				Logger::Core(LOG_DEBUG, "segment count changed to {}", segmentCount);
			}

			ImGui::PopID();

			if (removeComponent)
			{
				selected.RemoveComponent<SplineComponent>();
			}
		}

		if (selected.HasCompoenent<PlayerInputComponent>())
		{
			ImGui::PushID("PlayerInput");
			ImGui::Separator();
			PlayerInputComponent& component = selected.GetComponent<PlayerInputComponent>();
			ImGui::Text("Input Component");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 25.0f);

			ImVec2 buttonSize{ ImGui::CalcTextSize("X").x + (ImGui::GetStyle().FramePadding.x * 2.0f),
			ImGui::CalcTextSize("X").y + (ImGui::GetStyle().FramePadding.y * 2.0f) };

			bool removeComponent{ false };
			if (ImGui::Button("X", buttonSize))
			{
				removeComponent = true;
			}

			ImGui::Separator();

			std::string guid = std::to_string(component.PlayerGUID);
			std::string enttID = std::to_string((uint32_t)component.PlayerEnttID);

			ImGui::BeginDisabled();
			DrawStringProperty("Player GUID", guid, guid);
			DrawStringProperty("Player Entt ID", enttID, enttID);
			ImGui::EndDisabled();

			ImGui::PopID();

			if (removeComponent)
			{
				selected.RemoveComponent<PlayerInputComponent>();
			}
		}

		if (selected.HasCompoenent<MovementComponent>())
		{
			ImGui::PushID("MoveComponent");
			ImGui::Separator();
			MovementComponent& component = selected.GetComponent<MovementComponent>();
			ImGui::Text("Movement Component");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 25.0f);

			ImVec2 buttonSize{ ImGui::CalcTextSize("X").x + (ImGui::GetStyle().FramePadding.x * 2.0f),
			ImGui::CalcTextSize("X").y + (ImGui::GetStyle().FramePadding.y * 2.0f) };

			bool removeComponent{ false };
			if (ImGui::Button("X", buttonSize))
			{
				removeComponent = true;
			}

			ImGui::Separator();

			SliderParams params;
			params.ResetValue = 1.0f;
			params.Speed = 0.01f;

			DrawFloatControl("Speed", component.Speed, params);

			ImGui::BeginDisabled();
			glm::vec3 direction = component.Direction;
			DrawVec3Control("Direction", direction, params);
			ImGui::EndDisabled();

			ImGui::PopID();

			if (removeComponent)
			{
				selected.RemoveComponent<MovementComponent>();
			}
		}

		if (selected.HasCompoenent<MeshComponent>())
		{
			ImGui::PushID("MeshComponent");
			ImGui::Separator();
			MeshComponent& component = selected.GetComponent<MeshComponent>();
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), component.Name().c_str());
			ImGui::Text(buffer);
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 25.0f);

			ImVec2 buttonSize{ ImGui::CalcTextSize("X").x + (ImGui::GetStyle().FramePadding.x * 2.0f),
			ImGui::CalcTextSize("X").y + (ImGui::GetStyle().FramePadding.y * 2.0f) };

			bool removeComponent{ false };
			if (ImGui::Button("X", buttonSize))
			{
				removeComponent = true;
			}

			ImGui::Separator();

			std::string previousMeshPath = component.m_FilePath;


			if (FileProperty("Mesh File", component.m_FilePath, "All Formats (*.fbx, *.obj)\0*.fbx;*.obj\0FBX Model (*.fbx)\0*.fbx\0OBJ Mesh(*.obj)\0*.obj\0"))
			{
				if (!AssetLoader::LoadMesh(selected, component, component.m_FilePath))
				{
					Logger::Core(LOG_WARNING, "DetailsViewPanel::DrawComponents() MESH FILE There was an error loading mesh file, reverting to old mesh path");
					component.m_FilePath = previousMeshPath;
					component.Reload();
				}
			}

			ImGui::SeparatorText("Material Instance");
			MaterialInstance& material = component.m_MaterialInstance;

			std::string previousBasePath = material.BaseMapPath;

			if (FileProperty("Base Texture", material.BaseMapPath,
				"All Formats (*.png, *.jpeg, *.jpg)\0*.png;*.jpeg;*.jpg\0png (*.png)\0*.png\0Jpeg (*.jpeg)\0*.jpeg\0Jpg (*.jpg)\0*.jpg\0"))
			{
				std::shared_ptr<Texture> newTexture = AssetLoader::LoadTexture(material.BaseMapPath);
				if (newTexture == nullptr)
				{
					Logger::Core(LOG_WARNING, "DetailsViewPanel::DrawComponents() BASE TEXTURE There was an error loading Texture file, reverting to old Texture path");
					material.BaseMapPath = previousBasePath;
				}
				else
				{
					component.m_MaterialInstance.BaseMap = newTexture;
				}
			}

			ImGui::SameLine();
			ImGui::PushID("RemoveColorTexture");
			if (ImGui::Button("X"))
			{
				material.BaseMap = nullptr;
				material.BaseMapPath = "";
			}
			ImGui::PopID();

			std::string previousNormalPath = material.NormalMapPath;

			if (FileProperty("Normal Map", material.NormalMapPath,
				"All Formats (*.png, *.jpeg, *.jpg)\0*.png;*.jpeg;*.jpg\0png (*.png)\0*.png\0Jpeg (*.jpeg)\0*.jpeg\0Jpg (*.jpg)\0*.jpg\0"))
			{
				std::shared_ptr<Texture> newTexture = AssetLoader::LoadTexture(material.NormalMapPath, TextureType::Normal);
				if (newTexture == nullptr)
				{
					Logger::Core(LOG_WARNING, "DetailsViewPanel::DrawComponents() NORMAL TEXTURE There was an error loading Texture file, reverting to old Texture path");
					material.NormalMapPath = previousBasePath;
				}
				else
				{
					component.m_MaterialInstance.NormalMap = newTexture;
				}
			}

			ImGui::SameLine();
			ImGui::PushID("RemoveNormalMap");
			if (ImGui::Button("X"))
			{
				material.NormalMap = nullptr;
				material.NormalMapPath = "";
			}
			ImGui::PopID();

			std::string previousMetalPath = material.MetallicMapPath;


			if (FileProperty("Metallic Map", material.MetallicMapPath,
				"All Formats (*.png, *.jpeg, *.jpg)\0*.png;*.jpeg;*.jpg\0png (*.png)\0*.png\0Jpeg (*.jpeg)\0*.jpeg\0Jpg (*.jpg)\0*.jpg\0"))
			{
				std::shared_ptr<Texture> newTexture = AssetLoader::LoadTexture(material.MetallicMapPath, TextureType::GltfMetalicRoughness);
				if (newTexture == nullptr)
				{
					Logger::Core(LOG_WARNING, "DetailsViewPanel::DrawComponents() METALLIC TEXTURE There was an error loading Texture file, reverting to old Texture path");
					material.MetallicMapPath = previousMetalPath;
				}
				else
				{
					component.m_MaterialInstance.MetallicMap = newTexture;
				}
			}

			ImGui::SameLine();
			ImGui::PushID("RemoveMetalMap");
			if (ImGui::Button("X"))
			{
				material.MetallicMap = nullptr;
				material.MetallicMapPath = "";
			}
			ImGui::PopID();

			SliderParams smoothnessParams;
			smoothnessParams.Min = 0.0f;
			smoothnessParams.Max = 1.0f;
			smoothnessParams.Speed = 0.001f;
			smoothnessParams.ResetValue = 0.3f;
			DrawFloatControl("Smoothness", material.Smoothness, smoothnessParams);

			SliderParams specParams;
			specParams.Min = 1.0f;
			specParams.Max = 200.0f;
			specParams.Speed = 0.1f;
			specParams.ResetValue = 32.0f;
			DrawFloatControl("Specular Power", material.SpecularPower, specParams);

			if (removeComponent)
			{
				selected.RemoveComponent<MeshComponent>();
			}
			ImGui::PopID();
		}

		if (selected.HasCompoenent<CircleRendererComponent>())
		{
			ImGui::PushID("CircleRenderer");
			CircleRendererComponent& component = selected.GetComponent<CircleRendererComponent>();
			ImGui::Text("Circle Renderer Component");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 25.0f);

			ImVec2 buttonSize{ ImGui::CalcTextSize("X").x + (ImGui::GetStyle().FramePadding.x * 2.0f),
			ImGui::CalcTextSize("X").y + (ImGui::GetStyle().FramePadding.y * 2.0f) };

			bool removeComponent{ false };
			if (ImGui::Button("X", buttonSize))
			{
				removeComponent = true;
			}

			ImGui::Separator();

			ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
			ImGui::DragFloat("Line Width", &component.LineWidth, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);

			ImGui::PopID();
		}

		if (selected.HasCompoenent<CollisionComponent>())
		{
			ImGui::PushID("CollisionComponent");
			ImGui::Separator();
			CollisionComponent& component = selected.GetComponent<CollisionComponent>();
			ImGui::Text("Collision Component");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 25.0f);

			ImVec2 buttonSize{ ImGui::CalcTextSize("X").x + (ImGui::GetStyle().FramePadding.x * 2.0f),
			ImGui::CalcTextSize("X").y + (ImGui::GetStyle().FramePadding.y * 2.0f) };

			bool removeComponent{ false };
			if (ImGui::Button("X", buttonSize))
			{
				removeComponent = true;
			}

			ImGui::Separator();

			ImGui::Text("Collider Type");
			ImGui::SameLine();
			int currentType = static_cast<int>(component.Type);
			if (ImGui::Combo("##ColliderType", &currentType, CollisionComponent::TypeNames, IM_ARRAYSIZE(CollisionComponent::TypeNames)))
			{
				ColliderType newType = static_cast<ColliderType>(currentType);

				if (newType != component.Type)
				{
					component.Type = newType;
					scene->GetRegistry().patch<CollisionComponent>(selected.GetEnttHandle());
				}
			}
			ImGui::Checkbox("Active", &component.BIsActive);


			// specific colliders

			if (selected.HasCompoenent<SphereCollider>())
			{
				SphereCollider& collider = selected.GetComponent<SphereCollider>();
				SliderParams params;
				params.ResetValue = 0.5f;
				params.Speed = 0.001;
				params.Min = 0.0f;

				DrawFloatControl("Radius", collider.Radius, params);
			}

			if (selected.HasCompoenent<CubeCollider>())
			{
				CubeCollider& collider = selected.GetComponent<CubeCollider>();
				SliderParams params;
				params.ResetValue = 0.5f;
				params.Speed = 0.001;
				params.Min = 0.0f;

				DrawVec3Control("Extents", collider.Extents, params);
				ImGui::SetItemTooltip("Extents are the HALF width/height/depth values");
			}

			if (removeComponent)
			{
				selected.RemoveComponent<CollisionComponent>();
			}

			ImGui::PopID();
		}

		if (selected.HasCompoenent<CameraComponent>())
		{
			CameraManager& camManager = scene->GetCameraManager();
			ImGui::PushID("CameraComponent");
			ImGui::Separator();
			CameraComponent& component = selected.GetComponent<CameraComponent>();
			
			ImGui::Text("Camera Component");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 25.0f);

			ImVec2 buttonSize{ ImGui::CalcTextSize("X").x + (ImGui::GetStyle().FramePadding.x * 2.0f),
			ImGui::CalcTextSize("X").y + (ImGui::GetStyle().FramePadding.y * 2.0f) };

			bool removeComponent{ false };
			if (ImGui::Button("X", buttonSize))
			{
				removeComponent = true;
			}

			ImGui::Separator();

			if (camManager.GetActiveCameraObject() != selected)
			{
				if (ImGui::Button("Possess"))
				{
					camManager.SetActiveCamera(selected);
				}
			}
			else
			{
				if (ImGui::Button("Un Possess"))
				{
					camManager.SetEditorCamActive();
				}
			}
		
			std::string buttonText = camManager.GetDefaultCamera() != selected ? "Make Default" : "Is Default";
			float textWidth = ImGui::CalcTextSize(buttonText.c_str()).x + (ImGui::GetStyle().FramePadding.x * 2.0f);
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - textWidth);
			if (camManager.GetDefaultCamera() != selected)
			{
				if (ImGui::Button(buttonText.c_str()))
				{
					camManager.SetDefaultCamera(selected);
				}
			}
			else
			{
				ImGui::BeginDisabled();
				ImGui::Button(buttonText.c_str());
				ImGui::EndDisabled();
			}

			ImGui::SeparatorText("ShadowMap Debug");

			if (ImGui::Button("Try Match Light Position"))
			{
				TransformComponent& transform = selected.GetTransform();
				GameObject mainLight = selected.GetScene()->GetMainLight();
				LightComponent* light = mainLight.TryGetComponent<LightComponent>();

				if (light != nullptr)
				{
					TransformComponent& lightTransform = mainLight.GetTransform();
					glm::vec3 forward = lightTransform.Forward();
					glm::vec3 forwardDegrees = glm::degrees(forward);

					transform.SetPosition(forward * -1.0f);
					transform.SetRotationEuler(lightTransform.GetRotationEuler());

					//glm::vec3 rotationDegrees = transform.GetRotationEuler();
				}

			}

			ImGui::Separator();

			ImGui::Text("Projection Type");
			ImGui::SameLine();
			const char* projectionTypeNames[] = { "Perspective", "Orthographic" };
			int bOrthographic = (int)component.IsOrthographic();

			if (ImGui::Combo("##Projection Type", &bOrthographic, projectionTypeNames, IM_ARRAYSIZE(projectionTypeNames)))
			{
				component.SetOrthographic(bOrthographic == 1);
			}

			if (component.IsOrthographic())
			{
				DrawOrthographicCamProps(component.Cam);
			}
			else
			{
				DrawPerspectiveCamProps(component.Cam);
			}

			

			if (removeComponent)
			{
				selected.RemoveComponent<CameraComponent>();
			}
			ImGui::PopID();
		}

		if (selected.HasCompoenent<CameraController>())
		{
			ImGui::PushID("CameraController");
			ImGui::Separator();
			CameraController& component = selected.GetComponent<CameraController>();

			ImGui::Text("Camera Controller");
			/*ImGui::SameLine(ImGui::GetContentRegionAvail().x - 25.0f);

			ImVec2 buttonSize{ ImGui::CalcTextSize("X").x + (ImGui::GetStyle().FramePadding.x * 2.0f),
			ImGui::CalcTextSize("X").y + (ImGui::GetStyle().FramePadding.y * 2.0f) };

			bool removeComponent{ false };
			if (ImGui::Button("X", buttonSize))
			{
				removeComponent = true;
			}
			ImGui::SetItemTooltip("Cannot Remove Controller via ui at this time");*/
			ImGui::Separator();

			ImGui::Text("Fly In Editor Only ");
			ImGui::SameLine();
			bool bEditorOnly = component.IsEditorOnly();
			if(ImGui::Checkbox("##IsEditorOnly", &bEditorOnly))
			{
				component.SetEditorOnly(bEditorOnly);
			}

			ImGui::Text("Focal Point");
			ImGui::SameLine();
			glm::vec3 focalPoint = component.GetFocalPoint();
			float focalDistance = component.GetFocalPointDistance();

			ImGui::BeginDisabled();
			ImGui::InputFloat3("##readOnly", glm::value_ptr(focalPoint));

			ImGui::Text("Distance to Focal Point");
			ImGui::InputFloat("##readOnlyDistance", &focalDistance);

			ImGui::EndDisabled();

			ImGui::PopID();
		}

		if (selected.HasCompoenent<LightComponent>())
		{
			LightComponent& light = selected.GetComponent<LightComponent>();
			ImGui::PushID("LightComponent");
			ImGui::Separator();

			ImGui::Text("Light Component");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 25.0f);

			ImVec2 buttonSize{ ImGui::CalcTextSize("X").x + (ImGui::GetStyle().FramePadding.x * 2.0f),
			ImGui::CalcTextSize("X").y + (ImGui::GetStyle().FramePadding.y * 2.0f) };

			bool removeComponent{ false };
			if (ImGui::Button("X", buttonSize))
			{
				removeComponent = true;
			}

			ImGui::Separator();
			ImGui::Text("Light Type");
			ImGui::SameLine();
			int currentType = static_cast<int>(light.Type);
			if (ImGui::Combo("##Light Type", &currentType, LightComponent::LightTypeNames, IM_ARRAYSIZE(LightComponent::LightTypeNames)))
			{
				light.Type = static_cast<LightType>(currentType);
			}

			ImGui::Text("Light Color");
			ImGui::SameLine();
			ImGui::ColorEdit3("##Color", glm::value_ptr(light.Color));

			/*if (light.Type == LightType::Directional)
			{
				DrawVec3Control("Direction", light.Direction, 0.5f);

			}*/
			
			SliderParams attenuationParams;
			attenuationParams.Speed = 0.001f;
			attenuationParams.Format = "%.4f";
			if (light.Type == LightType::Point)
			{
				DrawVec3Control("Attenuations", light.Attenuation, attenuationParams);

			}

			if (light.Type == LightType::Spot)
			{
				//DrawVec3Control("Direction", light.Direction, 0.5f);
				DrawVec3Control("Attenuations", light.Attenuation, attenuationParams);

				SliderParams params;
				params.Min = 1.0f;
				params.Max = 180.0f;
				params.ResetValue = 12.5f;
				params.Speed = 0.01f;
				DrawFloatControl("Inner Radius", light.InnerRadius, params);

				SliderParams paramsOuter;
				paramsOuter.Min = 1.0f;
				paramsOuter.Max = 180.0f;
				paramsOuter.ResetValue = 15.0f;
				paramsOuter.Speed = 0.01f;
				DrawFloatControl("Outer Radius", light.OuterRadius, paramsOuter);
			}

			if (removeComponent)
			{
				selected.RemoveComponent<LightComponent>();
			}
			ImGui::PopID();

		}
	}

	void DetailsViewPanel::DrawOrthographicCamProps(Camera& camera)
	{
		float labelWidth = (ImGui::GetFontSize() * 10.0f);
		ImGui::SeparatorText("Zoom");

		ImGui::DragFloat("##Value", &camera.m_ZoomLevel, 0.01f, 0.001f);
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			camera.m_ZoomLevel = 1;
		}

		ImGui::SeparatorText("Aspect Ratio");
		ImGui::Text("Manually Set Aspect Ratio");
		ImGui::SameLine();
		glm::vec2 viewportSize = EngineContext::GetEngine()->GetViewportSize();

		static glm::vec2 size = viewportSize;
		ImGui::Checkbox("##lockAspectRatio", &camera.m_LockAspectRatio);

		if (!camera.m_LockAspectRatio)
			ImGui::BeginDisabled();

		SliderParams viewportParams;
		viewportParams.Speed = 0.01f;
		viewportParams.Min = 0.01f;
		viewportParams.ResetValue = 1.0f;
		if (DrawVec2Control("Aspect Ratio", size, viewportParams, labelWidth))
		{
			camera.m_ManualRatio = size.x / size.y;
		}

		if (!camera.m_LockAspectRatio)
			ImGui::EndDisabled();
	}

	void DetailsViewPanel::DrawPerspectiveCamProps(Camera& camera)
	{
		float labelWidth = (ImGui::GetFontSize() * 10.0f);
		std::vector<std::string> labels{ "FoV", "Near Plane", "Far Plane" };
		std::vector<float*> values{ &camera.m_Fov, &camera.m_Near, &camera.m_Far };
		std::vector<float> resetValues{ 1.0f, 0.1f, 100.0f };
		if (ImGui::BeginTable("##CameraProperties", 2))
		{
			ImGui::TableSetupColumn("Labels", ImGuiTableColumnFlags_WidthFixed, labelWidth);
			ImGui::TableSetupColumn("Values");

			for (int i = 0; i < labels.size(); i++)
			{
				ImGui::PushID(i);
				ImGui::TableNextRow();
				// the label
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(labels[i].c_str());

				// the values
				ImGui::TableSetColumnIndex(1);
				ImGui::DragFloat("##Value", values[i], 0.01f);
				ImGui::SameLine();
				if (ImGui::Button("Reset"))
				{
					*values[i] = resetValues[i];
				}
				ImGui::PopID();

			}

			ImGui::EndTable();
		}
	}

	bool DetailsViewPanel::FileProperty(const std::string& label, std::string& value, const char* filter, float columnWidth)
	{
		std::filesystem::path fileName = value;
		if (!value.empty() && fileName.has_filename())
		{
			fileName = fileName.filename();
		}


		if (ImGui::BeginTable(label.c_str(), 2, ImGuiTableFlags_Resizable/* | ImGuiTableFlags_RowBg*/))
		{
			float fontSize = ImGui::GetFontSize();
			ImGui::TableSetupColumn("Labels", ImGuiTableColumnFlags_WidthFixed, fontSize * columnWidth);
			ImGui::TableSetupColumn("Values", ImGuiTableColumnFlags_WidthStretch);
			//ImGui::TableSetupColumn("EditButton", ImGuiTableColumnFlags_WidthStretch/*, -FLT_MIN*/);

			ImGui::TableNextRow();
			// the label
			ImGui::TableSetColumnIndex(0);

			ImGui::PushItemWidth(fontSize * columnWidth);
			ImGui::Text(label.c_str());
			ImGui::PopItemWidth();

			// the values
			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(fontSize * value.size());
			// do stuff

			ImGui::Text((fileName.string().c_str()));
			
			ImGui::PopItemWidth();

			// the button to turn  the value field into an edit field
			
			ImGui::EndTable();

			ImGui::SameLine();
			std::string buttonText = "...";
			std::string deleteText = "X";
			float deleteWidth = ImGui::CalcTextSize(deleteText.c_str()).x + (ImGui::GetStyle().FramePadding.x * 2.f);
			float buttonWidth = ImGui::CalcTextSize(buttonText.c_str()).x + (ImGui::GetStyle().FramePadding.x * 2.f);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - ( buttonWidth + deleteWidth));

			ImGui::PushID(label.c_str());
			if (ImGui::Button(buttonText.c_str()))
			{
				//bValueChanged = (value != editingValue);
				std::string filePath = FileDialogs::OpenFile(filter);

				if (!filePath.empty())
				{
					value = filePath;
					ImGui::PopID();
					return true;
				}
					
			}
			ImGui::PopID();
			//ImGui::PopItemWidth();
		}

		return false;
	}
}