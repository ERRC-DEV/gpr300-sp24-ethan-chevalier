#pragma once

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

	struct Vec3Key {
		float timestamp;
		glm::vec3 keyFrame;
	};


	struct AnimationClip {
		float duration;

		Vec3Key positionKeys[3];
		Vec3Key rotationKeys[3];
		Vec3Key scaleKeys[3];
		
		void calcNextFrame(ew::Transform& theTransform, float currentTime)
		{
			int counter = 0;
			float percentageDistance = 0;
			for each(Vec3Key instance in positionKeys)
			{
				if (instance.timestamp > currentTime)
				{
					break;
				}
				else
				{
					counter++;
				}
			}
			// Perform the lerp on the cooresponding position keyframe
			percentageDistance = (float)((currentTime - positionKeys[counter - 1].timestamp) / (positionKeys[counter].timestamp - positionKeys[counter - 1].timestamp));
			theTransform.position = (percentageDistance * positionKeys[counter].keyFrame) + ((1.0f - percentageDistance) * positionKeys[counter - 1].keyFrame);
			counter = 0;

			for each(Vec3Key instance in rotationKeys)
			{
				if (instance.timestamp > currentTime)
				{
					break;
				}
				else
				{
					counter++;
				}
			}
			// Perform the slerp on the cooresponding rotation keyframe
			// Convert the keyframes to quaternion before interpolation
			glm::quat rotationOrigin = glm::quat(glm::vec3(glm::radians(rotationKeys[counter - 1].keyFrame.x), glm::radians(rotationKeys[counter - 1].keyFrame.y), glm::radians(rotationKeys[counter - 1].keyFrame.z)));
			glm::quat rotationDestination = glm::quat(glm::vec3(glm::radians(rotationKeys[counter].keyFrame.x), glm::radians(rotationKeys[counter].keyFrame.y), glm::radians(rotationKeys[counter].keyFrame.z)));
			percentageDistance = (float)((currentTime - rotationKeys[counter - 1].timestamp) / (rotationKeys[counter].timestamp - rotationKeys[counter - 1].timestamp));
			glm::quat finalQuaternion = glm::slerp(rotationOrigin, rotationDestination, percentageDistance);
			theTransform.rotation = finalQuaternion;
			counter = 0;

			for each(Vec3Key instance in scaleKeys)
			{
				if (instance.timestamp > currentTime)
				{
					break;
				}
				else
				{
					counter++;
				}
			}
			// Perform the lerp on the cooresponding scale keyframe
			percentageDistance = (float)((currentTime - scaleKeys[counter - 1].timestamp) / (scaleKeys[counter].timestamp - scaleKeys[counter - 1].timestamp));
			theTransform.scale = (percentageDistance * scaleKeys[counter].keyFrame) + ((1.0f - percentageDistance) * scaleKeys[counter - 1].keyFrame);
			counter = 0;
		}

	};

}