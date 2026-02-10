#include "BsPrecompileHeader.h"
#include "SplineComponent.h"

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
			int startIndex = input.Segment * 2;
			if (startIndex >= spline.Points.size() && !spline.IsLoop)
			{
				// time has passed the end of the spline
				return spline.Points[spline.Points.size() - 1].ReadOnlyPosition();
			}

			glm::vec3 start = spline.Points[startIndex].ReadOnlyPosition();
			glm::vec3  end = spline.Points[startIndex + 1].ReadOnlyPosition();
			return glm::mix(start, end, input.SegmentT);
		}

		glm::vec3 DeCasteljauPos(const SplineComponent& spline, float t)
		{
			SegmentRelativeT input = GetTSegmentData(t);

			//cubic beziers share their last point so each segment sould start on a mult of 3 or on 0
			int startIndex = input.Segment * 3; 
			if (startIndex >= spline.Points.size() && !spline.IsLoop)
			{
				// time has passed the end of the spline
				return spline.Points[spline.Points.size() - 1].ReadOnlyPosition();
			}

			glm::vec3 pointA = spline.Points[startIndex + 0].ReadOnlyPosition();
			glm::vec3 pointB = spline.Points[startIndex + 1].ReadOnlyPosition(); // controll point
			glm::vec3 pointC = spline.Points[startIndex + 2].ReadOnlyPosition(); // controll point
			glm::vec3 pointD = spline.Points[startIndex + 3].ReadOnlyPosition();

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


	void SplineComponent::AddSegment()
	{
		int indexA = Points.size() - 2;
		int indexB = Points.size() - 1;
		glm::vec3 pointA = Points[indexA].GetPosition();
		glm::vec3 pointB = Points[indexB].GetPosition();

		glm::vec3 nextPointDirection = pointB - pointA;
		nextPointDirection = glm::normalize(nextPointDirection);

		switch (Type)
		{
		case Pixie::SplineType::Linear:
			TransformComponent newPosition = TransformComponent(pointB + nextPointDirection, Points[indexB].GetRotationEuler(), glm::vec3(1.0f));
			Points.push_back(newPosition);
			break;
		case Pixie::SplineType::CubicBezier:
			for (int i = 0; i < 3; i++)
			{
				TransformComponent newPosition = TransformComponent(pointB + nextPointDirection, Points[indexB].GetRotationEuler(), glm::vec3(1.0f));
				pointA = pointB;
				pointB = newPosition.GetPosition();

				Points.push_back(newPosition);
			}
			break;
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

	void SplineComponent::RemoveSegment()
	{}

	glm::vec3 SplineComponent::GetTangent(float T)
	{
		return glm::vec3();
	}

	int SplineComponent::GetNumSegments()
	{
		switch (Type)
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
	}

	
}

