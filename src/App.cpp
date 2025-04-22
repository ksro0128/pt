#include "include/App.h"

App::App() {
	init();
}

App::~App() {
	cleanup();
}

void App::run() {
	std::cout << "App::run" << std::endl;
	auto lastTime = std::chrono::high_resolution_clock::now();

	while (!glfwWindowShouldClose(m_window->getWindow())) {

		auto currentTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;

		glfwPollEvents();
		if (!m_renderer->isBenchmarkRunning()) {
			m_renderer->update(deltaTime);
		}
		m_renderer->render(deltaTime);
	}
}

void App::init() {
	std::cout << "App::init" << std::endl;
	m_window = Window::createWindow();
	m_renderer = Renderer::createRenderer(m_window->getWindow());
}

void App::cleanup()
{
	std::cout << "App::cleanup" << std::endl;
}