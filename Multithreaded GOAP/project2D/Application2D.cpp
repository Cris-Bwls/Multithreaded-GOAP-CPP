#include "Application2D.h"
#include "Texture.h"
#include "Font.h"
#include "Input.h"
#include <chrono>
#include <iostream>

using namespace std::chrono;
#define THREAD_COUNT 1
#define UNIT_COUNT 50

Application2D::Application2D() {

}

Application2D::~Application2D() {

}

bool Application2D::startup() {
	
	m_2dRenderer = new aie::Renderer2D();

	m_font = new aie::Font("./font/consolas.ttf", 32);
	
	m_timer = 0;

	wrapper = new GOAPWrapper(THREAD_COUNT);

	for (int i = 0; i < UNIT_COUNT; ++i)
	{
		units.push_back(new Unit(wrapper, i));
	}

	return true;
}

void Application2D::shutdown() {
	
	delete m_font;
	delete m_2dRenderer;

	delete wrapper;

	while (units.size() > 0)
	{
		delete units.back();
		units.pop_back();
	}
}

void Application2D::update(float deltaTime) {

	bool complete = true;
	for (int i = 0; i < units.size(); ++i)
	{
		units[i]->Update();
		complete &= units[i]->planComplete;
	}
	if (complete && calculating)
	{
		auto endTime = high_resolution_clock::now();
		auto planTime = duration_cast<duration<double>>(endTime - m_startTime);

		// Print time that it took for this group to be finished and observed
		std::cout << "Time taken for this group = " <<planTime.count();
		std::cout << std::endl;

		m_totalTime += planTime.count();
		m_count++;

		// Print average time
		std::cout << "AVG Time Taken = " << m_totalTime / m_count;
		std::cout << std::endl;
		calculating = false;
		printf("Done");
	}

	m_timer += deltaTime;

	// input example
	aie::Input* input = aie::Input::getInstance();

	// exit the application
	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();

	if (complete)
	{
		if (input->isKeyDown(aie::INPUT_KEY_1))
		{
			calculating = true;
			m_startTime = high_resolution_clock::now();
			system("cls");
			for (int i = 0; i < units.size(); ++i)
			{
				units[i]->planComplete = false;
				units[i]->GetPlan();
			}
		}

		if (input->isKeyDown(aie::INPUT_KEY_2))
		{
			for (size_t i = 0; i < units.size(); ++i)
			{
				units[i]->m_bPrintToConsole = !units[i]->m_bPrintToConsole;
			}
		}
	}
}

void Application2D::draw() {

	// wipe the screen to the background colour
	clearScreen();

	// begin drawing sprites
	m_2dRenderer->begin();
	
	// output some text, uses the last used colour
	char fps[32];
	sprintf_s(fps, 32, "FPS: %i", getFPS());
	m_2dRenderer->drawText(m_font, fps, 0, getWindowHeight() - 32);
	m_2dRenderer->drawText(m_font, "Press ESC to quit!", 0, getWindowHeight() - 64);
	m_2dRenderer->drawText(m_font, "Press 1 to cause units to make plan", 0, getWindowHeight() - 128);
	m_2dRenderer->drawText(m_font, "Press 2 to toggle units printing to console", 0, getWindowHeight() - 160);
	m_2dRenderer->drawText(m_font, "(Printing to console substantially slows down results)", 0, getWindowHeight() - 192);

	// done drawing sprites
	m_2dRenderer->end();
}