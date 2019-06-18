#pragma once

#include "Application.h"
#include "Renderer2D.h"

#include"GOAPActionBase.h"
#include "GOAPWrapper.h"
#include "Unit.h"

class Application2D : public aie::Application {
public:

	Application2D();
	virtual ~Application2D();

	virtual bool startup();
	virtual void shutdown();

	virtual void update(float deltaTime);
	virtual void draw();

protected:

	aie::Renderer2D*	m_2dRenderer;
	aie::Texture*		m_texture;
	aie::Texture*		m_shipTexture;
	aie::Font*			m_font;

	std::vector<GOAPActionBase*> actionList;
	GOAPWrapper* wrapper;

	std::vector<Unit*> units;
	bool calculating = false;
	std::chrono::steady_clock::time_point m_startTime;
	double m_totalTime = 0;
	int m_count = 0;

	float m_timer;
};