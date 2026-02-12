#include "BsPrecompileHeader.h"
#include "SplineComponent.h"
#include "EngineContext.h"
#include "Scene/GameObject.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

namespace Pixie
{
	namespace Spline
	{
		SegmentRelativeT GetTSegmentData(float t)
		{
			SegmentRelativeT data;

			data.Segment = glm::floor(t);
			data.SegmentT = t - data.Segment;

			return data;
		}

		glm::vec3 LinearPos(const SplineComponent& spline, float t)
		{
			SegmentRelativeT input = GetTSegmentData(t);
			// glm::mix = (1 - t)pointA + (t * pointB)

			//linear splines share end points so start at indexes 0, and mult of 2
			int startIndex = input.Segment;
			if (startIndex + 1 >= spline.Points.size() && !spline.IsLoop)
			{
				// time has passed the end of the spline
				return glm::vec3(spline.Points[spline.Points.size() - 1]->GetObjectToWorldMatrix()[3]);
			}

			
			glm::vec3 start = spline.Points[startIndex]->GetObjectToWorldMatrix()[3];
			glm::vec3  end = spline.Points[startIndex + 1]->GetObjectToWorldMatrix()[3];
			return glm::mix(start, end, input.SegmentT);
		}

		//TODO: add catch for negative time t trying to reach past the start of points array
		glm::vec3 DeCasteljauPos(const SplineComponent& spline, float t)
		{
			SegmentRelativeT input = GetTSegmentData(t);

			//cubic beziers share their last point so each segment sould start on a mult of 3 or on 0
			int startIndex = input.Segment * 3; 
			if (startIndex +3 >= spline.Points.size() && !spline.IsLoop)
			{
				// time has passed the end of the spline
				return spline.Points[spline.Points.size() - 1]->GetObjectToWorldMatrix()[3];
			}

			glm::vec3 pointA = spline.Points[startIndex + 0]->GetObjectToWorldMatrix()[3];
			glm::vec3 pointB = spline.Points[startIndex + 1]->GetObjectToWorldMatrix()[3]; // controll point
			glm::vec3 pointC = spline.Points[startIndex + 2]->GetObjectToWorldMatrix()[3]; // controll point
			glm::vec3 pointD = spline.Points[startIndex + 3]->GetObjectToWorldMatrix()[3];

			glm::vec3 lerpA = glm::mix(pointA, pointB, input.SegmentT);
			glm::vec3 lerpB = glm::mix(pointB, pointC, input.SegmentT);
			glm::vec3 lerpC = glm::mix(pointC, pointD, input.SegmentT);

			glm::vec3 lerpD = glm::mix(lerpA, lerpB, input.SegmentT);
			glm::vec3 lerpE = glm::mix(lerpB, lerpC, input.SegmentT);

			return glm::mix(lerpD, lerpE, input.SegmentT);
		}
	}

	const char* SplineComponent::TypeNames[] = {
				"Linear",
				"Cubiec Bezier",
				"Cardinal",
				"Catmul-Rom",
				"B"
	};

	const glm::mat4 SplineComponent::m_CubicBezierCharacteristic = 
	{
		{ 1, -3,  3, -1},
		{ 0,  3, -6,  3},
		{ 0,  0,  3, -3},
		{ 0,  0,  0,  1}
	};

	void SplineComponent::on_construct(entt::registry& registry, const entt::entity entt)
	{
		SplineComponent& component = registry.get<SplineComponent>(entt);
		component.m_SplineEntity = entt;//GameObject(entt, EngineContext::GetEngine()->GetScene());
	}

	void SplineComponent::SetType(SplineType type)
	{
		m_Type = type;
	
		if (Points.size() == 0)
			return;
			

		switch (type)
		{
		case Pixie::SplineType::Linear:
			return;
		case Pixie::SplineType::CubicBezier:
		{
			GameObject splineEntity = GameObject(m_SplineEntity, EngineContext::GetEngine()->GetScene());
			int sizeModThree = glm::mod<float>((float)(Points.size() - 1), 3.0f);
			if (sizeModThree == 0)
			{
				return;
			}

			int pointsNeeded = 3 - sizeModThree;

			glm::vec3 pointA{ 0.0f };
			glm::vec3 pointB{ 0.0f, 0.0f, 0.0f };
			int indexA{ -1 };
			int indexB{ -1 };
			glm::vec3 nextPointDirection{ 0.0f, 0.0f, 0.0f };

			if (Points.size() >= 2)
			{
				indexA = Points.size() - 2;
				indexB = Points.size() - 1;
				pointA = Points[indexA]->GetPosition();
				pointB = Points[indexB]->GetPosition();
				nextPointDirection = pointB - pointA;
				nextPointDirection = glm::normalize(nextPointDirection);
			}
			glm::vec3 nextPointRotation = indexB >= 0 ? Points[indexB]->GetRotationEuler() : glm::vec3(0.0f);

			for (int i = 0; i < pointsNeeded; i++)
			{

				if (i == 1 && nextPointDirection.x == 0.0f && nextPointDirection.y == 0.0f && nextPointDirection.z == 0.0f)
				{
					nextPointDirection.x = 1.0f;
				}

				int newIndex = indexB >= 0 ? indexB + 1 : 0;
				glm::vec3 newPosition = pointB + nextPointDirection;
				AddPoint(splineEntity, newIndex, newPosition, nextPointRotation);

				pointB = newPosition;
				indexB++;
			}
			
		}
		case Pixie::SplineType::Cardinal:
			break;
		case Pixie::SplineType::CatmulRom:
			break;
		case Pixie::SplineType::B:
			break;
		default:
			break;
		}
	}

	void SplineComponent::AddSegment(GameObject& splineObject)
	{
		//std::shared_ptr<Scene> scene = EngineContext::GetEngine()->GetScene();
		GameObject splineEntity = GameObject(m_SplineEntity, EngineContext::GetEngine()->GetScene());
		glm::vec3 pointA{ 0.0f };
		glm::vec3 pointB{ 0.0f, 0.0f, 0.0f };
		int indexA{ -1 };
		int indexB{ -1 };
		glm::vec3 nextPointDirection{ 0.0f, 0.0f, 0.0f };

		if (Points.size() >= 2)
		{
			indexA = Points.size() - 2;
			indexB = Points.size() - 1;
			pointA = Points[indexA]->GetPosition();
			pointB = Points[indexB]->GetPosition();
			nextPointDirection = pointB - pointA;
			nextPointDirection = glm::normalize(nextPointDirection);
		}
		glm::vec3 nextPointRotation = indexB >= 0 ? Points[indexB]->GetRotationEuler() : glm::vec3(0.0f);



		switch (m_Type)
		{
		case Pixie::SplineType::Linear:
		{
			int newIndex = indexB >= 0 ? indexB + 1 : 0;
			glm::vec3 newPosition = pointB + nextPointDirection;
			AddPoint( splineObject, newIndex, newPosition, nextPointRotation);

			if (Points.size() < 2)
			{
				nextPointDirection = glm::vec3(1.0f, 0.0f, 0.0f);
				newPosition = pointB + nextPointDirection;
				newIndex++;
				AddPoint( splineObject, newIndex, newPosition, nextPointRotation);
			}
			break;
		}
		case Pixie::SplineType::CubicBezier:
		{
			int pointsToAdd = 3;
			if (Points.size() < 4)
			{
				pointsToAdd++;
			}
			for (int i = 0; i < pointsToAdd; i++)
			{
				if (i == 1 && nextPointDirection.x == 0.0f && nextPointDirection.y == 0.0f && nextPointDirection.z == 0.0f)
				{
					nextPointDirection.x = 1.0f;
				}

				int newIndex = indexB >= 0 ? indexB + 1 : 0;
				glm::vec3 newPosition = pointB + nextPointDirection;
				AddPoint(splineObject, newIndex, newPosition, nextPointRotation);

				pointB = newPosition;
				indexB++;
			}

			break;
		}
		case Pixie::SplineType::Cardinal:
			break;
		case Pixie::SplineType::CatmulRom:
			break;
		case Pixie::SplineType::B:
			break;
		default:
			break;
		}
	}

	void SplineComponent::RemoveSegment(GameObject& splineObject)
	{
		if (Points.size() == 0)
			return;

		std::shared_ptr<Scene> scene = EngineContext::GetEngine()->GetScene();
		switch (m_Type)
		{
		case Pixie::SplineType::Linear:
		{
			Points.pop_back();
			scene->RemoveGameObject(GameObject(PointEnttIds.back(), scene));
			PointEnttIds.pop_back();
			PointIDs.pop_back();
			if (Points.size() == 1)
			{
				Points.pop_back();
				scene->RemoveGameObject(GameObject(PointEnttIds.back(), scene));
				PointEnttIds.pop_back();
				PointIDs.pop_back();
			}
			return;
		}
		case Pixie::SplineType::CubicBezier:
		{
			int numToRemove = 3;
			if (Points.size() <= 4)
			{
				numToRemove = Points.size();
			}
			for (int i = 0; i < numToRemove; i++)
			{
				Points.pop_back();
				scene->RemoveGameObject(GameObject(PointEnttIds.back(), scene));
				PointEnttIds.pop_back();
				PointIDs.pop_back();
			}
			return;
		}
		case Pixie::SplineType::Cardinal:
			break;
		case Pixie::SplineType::CatmulRom:
			break;
		case Pixie::SplineType::B:
			break;
		default:
			break;
		}
	}

	glm::vec3 SplineComponent::GetTangent(float T)
	{
		return glm::vec3();
	}

	int SplineComponent::GetNumSegments()
	{
		if (Points.size() <= 0)
			return 0;

		switch (m_Type)
		{
		case Pixie::SplineType::Linear:
			return Points.size() - 1;
		case Pixie::SplineType::CubicBezier:
			return (int)((Points.size() - 1) / 3);
		case Pixie::SplineType::Cardinal:
			break;
		case Pixie::SplineType::CatmulRom:
			break;
		case Pixie::SplineType::B:
			break;
		default:
			break;
		}

		return -1;
	}

	glm::vec3 SplineComponent::GetPostionT(float T)
	{
		if (Points.size() <= 0)
			return glm::vec3(0.0f);

		switch (m_Type)
		{
		case Pixie::SplineType::Linear:
			return Spline::LinearPos(*this, T);
		case Pixie::SplineType::CubicBezier:
			return Spline::DeCasteljauPos(*this, T);
		case Pixie::SplineType::Cardinal:
			break;
		case Pixie::SplineType::CatmulRom:
			break;
		case Pixie::SplineType::B:
			break;
		default:
			break;
		}

		return glm::vec3(-1.0f);
	}

	// currently brute forces it
	glm::vec3 SplineComponent::GetClosestPosition(glm::vec3 worldSpacePos, float granularity)
	{
		float minDistance = FLT_MAX;
		glm::vec3 closestPoint{ 0.0f };
		float t = 0.0f;
		while (t < GetNumSegments())
		{
			t += granularity;

			glm::vec3 position = GetPostionT(t);
			
			float distanceSquared = glm::distance2( position , worldSpacePos);
			if (distanceSquared < minDistance)
			{
				minDistance = distanceSquared;
				closestPoint = position;
			}
		}
		return closestPoint;
	}

	void SplineComponent::AddPoint( GameObject& parentSpline, int index, glm::vec3 position, glm::vec3 rotation)
	{
		std::shared_ptr<Scene> scene = parentSpline.GetScene();
		std::string name = "Spline Point " + std::to_string(index);
		GameObject newPoint = scene->CreateEmptyGameObject(name);
		parentSpline.AddChild(newPoint);

		PointIDs.push_back(newPoint.GetGUID());
		PointEnttIds.push_back(newPoint);
		TransformComponent* transform = newPoint.TryGetComponent<TransformComponent>();
		transform->SetPosition(position);
		transform->SetRotationEuler(rotation);
		Points.push_back(transform);
	}

	

	
}

