#pragma once
#include "AnimationClip.h"

#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>
#include <ew/procGen.h>

#include <math.h>

namespace ec
{
	
	struct Joint {
		glm::vec3 position;
		glm::vec3 eulerRotation;
		glm::vec3 scale;
	};
	
	class Skeleton {
	public:
		Skeleton();
		std::vector<ew::Transform> CalcChildrenPos();

		Joint mTorso;
		Joint mShoulder;
		Joint mElbow;
		Joint mWrist;

	private:
		
	};




}