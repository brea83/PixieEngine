#include "BsPrecompileHeader.h"
#include "ApplicationEvent.h"
#include "Scene/Scene.h"

namespace Pixie
{

	std::string SceneChangedEvent::ToString() const 
	{
		std::stringstream ss;
		ss << "Scene Changed Event: " << m_NewScene->GetName();
		return ss.str();
	}
}