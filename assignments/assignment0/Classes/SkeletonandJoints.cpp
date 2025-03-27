#include "SkeletonandJoints.h"

namespace ec
{
	Skeleton::Skeleton()
	{
		mTorso.position = glm::vec3(0, 0, 0);
		mTorso.eulerRotation = glm::vec3(0, 0, 0);
		mTorso.scale = glm::vec3(1, 1, 1);

		mShoulder.position = glm::vec3(1, 0, 0);
		mShoulder.eulerRotation = glm::vec3(0, 0, 0);
		mShoulder.scale = glm::vec3(1, 1, 1);

		mElbow.position = glm::vec3(1, 0, 0);
		mElbow.eulerRotation = glm::vec3(0, 0, 0);
		mElbow.scale = glm::vec3(1, 1, 1);

		mWrist.position = glm::vec3(1, 0, 0);
		mWrist.eulerRotation = glm::vec3(0, 0, 0);
		mWrist.scale = glm::vec3(1, 1, 1);
	}

	std::vector<ew::Transform> Skeleton::CalcChildrenPos()
	{
		std::vector<ew::Transform> newVector;

		ew::Transform torsoTransform;
		ew::Transform shoulderTransform;
		ew::Transform elbowTransform;
		ew::Transform wristTransform;

		torsoTransform.position = mTorso.position;
		torsoTransform.rotation = glm::radians(mTorso.eulerRotation);
		torsoTransform.scale = mTorso.scale;


		glm::vec3 torsoRot = glm::eulerAngles(torsoTransform.rotation);
		torsoRot = glm::degrees(torsoRot);
		shoulderTransform.position = torsoTransform.position + mShoulder.position;
		shoulderTransform.rotation = glm::radians(glm::vec3((torsoRot.x + mShoulder.eulerRotation.x), (torsoRot.y + mShoulder.eulerRotation.y), (torsoRot.z + mShoulder.eulerRotation.z)));
		shoulderTransform.scale = 0.8f * glm::vec3((torsoTransform.scale.x * mShoulder.scale.x), (torsoTransform.scale.y * mShoulder.scale.y), (torsoTransform.scale.z * mShoulder.scale.z));

		glm::vec3 shoulderRot = glm::eulerAngles(shoulderTransform.rotation);
		shoulderRot = glm::degrees(shoulderRot);
		elbowTransform.position = shoulderTransform.position + mElbow.position;
		elbowTransform.rotation = glm::radians(glm::vec3((shoulderRot.x + mElbow.eulerRotation.x), (shoulderRot.y + mElbow.eulerRotation.y), (shoulderRot.z + mElbow.eulerRotation.z)));
		elbowTransform.scale = 0.8f * glm::vec3((shoulderTransform.scale.x * mElbow.scale.x), (shoulderTransform.scale.y * mElbow.scale.y), (shoulderTransform.scale.z * mElbow.scale.z));

		glm::vec3 elbowRot = glm::eulerAngles(elbowTransform.rotation);
		elbowRot = glm::degrees(elbowRot);
		wristTransform.position = elbowTransform.position + mWrist.position;
		wristTransform.rotation = glm::radians(glm::vec3((elbowRot.x + mWrist.eulerRotation.x), (elbowRot.y + mWrist.eulerRotation.y), (elbowRot.z + mWrist.eulerRotation.z)));
		wristTransform.scale = 0.8f * glm::vec3((elbowTransform.scale.x * mWrist.scale.x), (elbowTransform.scale.y * mWrist.scale.y), (elbowTransform.scale.z * mWrist.scale.z));

		newVector.push_back(torsoTransform);
		newVector.push_back(shoulderTransform);
		newVector.push_back(elbowTransform);
		newVector.push_back(wristTransform);

		return newVector;
	}
}