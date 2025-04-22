#pragma once

#include "Window.h"
#include "Renderer.h"

class App {
public:
	App();
	~App();
	void run();

private:
	std::unique_ptr<Window> m_window;
	std::unique_ptr<Renderer> m_renderer;

	void init();
	void cleanup();
};