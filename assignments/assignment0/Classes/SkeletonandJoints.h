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
		Skeleton() {};
		//Skeleton(const MeshData& meshData);
		std::vector<ew::Transform> CalcChildrenPos();
		

	private:
		Joint* mTorso = new Joint;
		Joint* mShoulder = new Joint;
		Joint* mElbow = new Joint;
		Joint* mWrist = new Joint;
	};




}