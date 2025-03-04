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
	struct Animator
	{
		AnimationClip clip;
		bool isPlaying;
		bool isLooping;
		float playbackSpeed;
		float playbackTime;

		ew::Transform calcNewFrame()
		{
			ew::Transform newTransform;
			if (isPlaying)
			{
				playbackTime += playbackSpeed;
			}
			if (playbackTime > clip.duration)
			{
				if (isLooping)
				{
					playbackTime -= clip.duration;
				}
				else
				{
					playbackTime = clip.duration;
				}
			}
			if (playbackTime < 0.0f)
			{
				if (isLooping)
				{
					playbackTime += clip.duration;
				}
				else
				{
					playbackTime = 0.0f;
				}
			}

			clip.calcNextFrame(newTransform, playbackTime);
			
			return newTransform;
		}

	};



}