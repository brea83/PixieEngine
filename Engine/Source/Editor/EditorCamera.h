#pragma once
#include "Graphics/Camera.h"
#include "Scene/Entity.h"

namespace Pixie
{
	struct CameraComponent;
	class CameraController;
	class Scene;

	class EditorCamera : public Entity
	{
	public:
		EditorCamera();
		EditorCamera(entt::entity entity, std::shared_ptr<Scene> scene, float fov = 45.0f, float aspectRatio = 1280.0f / 720.0f, float nearClip = 0.1f, float farClip = 100.0f);

		Camera& Cam();
		CameraController& Controller();

		//static void Serialize(StreamWriter* stream, const 	class EditorCamera& camera)
		//{
		//	
		//}
		//static bool Deserialize(StreamReader* stream, EditorCamera& camera)
		//{
		//	
		//	return true;
		//}
	protected:
	};
}
