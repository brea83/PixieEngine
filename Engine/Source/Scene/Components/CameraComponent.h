#pragma once
#include "Core.h"
#include "Graphics/Camera.h"
#include "Resources/FileStream.h"
#include "Graphics/Frustum.h"
#include <glm/glm.hpp>
#include <EnTT/entt.hpp>
//#include "Scene/Components/Component.h"


namespace Pixie
{
	struct CameraComponent
	{
		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

		//glm::mat4 ViewMatrix{glm::mat4(1.0f)};
		Camera Cam;
		//Frustum FrustumData{};
		//glm::vec4 FrustumCornersWS[8];
		bool IsActive{ false };
		bool IsDefault{ false };
		bool IsOrthographic() { return Cam.IsOrthographic(); }
		void SetOrthographic(bool value);

		void SetAspectRatio(float width, float height);
		void LockAspectRatio(bool value = true) { Cam.LockAspectRatio(value); }
		bool IsAspectRatioLocked() const { return Cam.IsAspectRatioLocked(); }

		void OnCameraMoved(glm::mat4 transform);

		static void Serialize(StreamWriter* stream, const CameraComponent& component)
		{
			//stream->WriteString(component.Name);
			stream->WriteRaw(component.IsActive);
			stream->WriteRaw(component.IsDefault);
			stream->WriteRaw(component.Cam);
		}
		static bool Deserialize(StreamReader* stream, CameraComponent& component);
		
		//static void on_construct(entt::registry& registry, const entt::entity entt);
	};
}