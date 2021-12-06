/*
NOU Framework - Created for INFR 2310 at Ontario Tech.
(c) Samantha Stahlke 2020

CMorphAnimator.cpp
Simple animator component for demonstrating morph target animation.

As a convention in NOU, we put "C" before a class name to signify
that we intend the class for use as a component with the ENTT framework.
*/

#include "CMorphAnimator.h"
#include "CMorphMeshRenderer.h"
#include "NOU/Mesh.h"
#include <iostream>

namespace nou
{
	CMorphAnimator::AnimData::AnimData()
	{
		frame0 = nullptr;
		frame1 = nullptr;
		frameTime = 1.0f;
	}

	CMorphAnimator::CMorphAnimator(Entity& owner)
	{
		m_owner = &owner;
		m_data = std::make_unique<AnimData>();
		m_timer = 0.0f;
		m_index = 0;
		m_forwards = true;
	}

	
	void CMorphAnimator::Update(float deltaTime)
	{

		std::cout << "deltatime " << deltaTime << std::endl;

		m_timer += deltaTime;

		if (m_timer > m_data->frameTime) {
			m_timer -= m_data->frameTime;

			m_index++;

			if (m_index >= m_data->frames.size()) {
				m_index = 0;
			}
		}

		float t = m_timer / m_data->frameTime;

		int framenuma, framenumb;
		framenuma = m_index;
		framenumb = (framenuma == 0) ? m_data->frames.size() - 1 : m_index - 1;

		//segments stuffand things
		// TODO: Complete this function
		std::cout << "Current Frame : " << m_index << " T: " << t << std::endl;
		std::cout << "a: " << framenuma << " b: " << framenumb << std::endl;
		m_owner->Get<CMorphMeshRenderer>().UpdateData(*m_data->frames[framenumb], *m_data->frames[framenuma], t); //frame i frame i+1 t
	}

	
	void CMorphAnimator::SetFrames(std::vector<std::unique_ptr <Mesh>>& frames) {

		for (int i = 0; i < frames.size(); i++) {
		
			m_data->frames.push_back(std::move(frames[i]));
		}
	}

	void CMorphAnimator::SetFrameTime(float num) {
		m_data->frameTime = num;
	}
}