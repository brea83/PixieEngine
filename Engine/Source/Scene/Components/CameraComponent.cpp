#include "BsPrecompileHeader.h"
#include "CameraComponent.h"
#include "Component.h"
#include <glm/glm.hpp>

namespace Pixie
{
	void CameraComponent::SetOrthographic(bool value)
	{
		if (value == IsOrthographic())
			return;

		Cam.SetOrthographic(value);

	}
	void CameraComponent::SetAspectRatio(float width, float height)
	{
		Cam.SetAspectRatio(width / height);

	}

	void CameraComponent::OnCameraMoved(glm::mat4 transform)
	{
		//ViewMatrix = glm::inverse(transform);
		//FrustumCornersWS = Frustum::CalcFrustumCornersWS(ViewMatrix, Cam.ProjectionMatrix());

	}

	bool CameraComponent::Deserialize(StreamReader* stream, CameraComponent& component)
	{
		//SerializableComponentID readID;
		//stream->ReadRaw<SerializableComponentID>(readID);
		//if (readID != SerializableComponentID::CameraComponent) return false;
		//return true;

		stream->ReadRaw(component.IsActive);
		stream->ReadRaw(component.IsDefault);
		stream->ReadRaw(component.Cam);
		return true;
	}

	//void CameraComponent::on_construct(entt::registry& registry, const entt::entity entt)
	//{
	//	CameraComponent& component = registry.get<CameraComponent>(entt);
	//	TransformComponent* transform = registry.try_get<TransformComponent>(entt);

	//	if (transform)
	//	{
	//		component.ViewMatrix = glm::inverse(transform->GetObjectToWorldMatrix());
	//		//component.FrustumCornersWS = Frustum::CalcFrustumCornersWS(component.ViewMatrix, component.Cam.ProjectionMatrix());

	//	}
	//}
}