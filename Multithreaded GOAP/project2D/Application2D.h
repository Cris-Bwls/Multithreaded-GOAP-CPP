#pragma once

#include "Application.h"
#include "Renderer2D.h"

#include"GOAPActionBase.h"
#include "GOAPWrapper.h"

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

	bool m_bNotifier = false;
	GOAPPlan m_plan;

	float m_timer;
};