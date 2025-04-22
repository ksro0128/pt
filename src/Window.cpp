#include "include/Window.h"

std::unique_ptr<Window> Window::createWindow() {
	std::unique_ptr<Window> window = std::unique_ptr<Window>(new Window());
	window->initWindow();
	return window;
}

Window::~Window() {
	cleanup();
}

void Window::cleanup() {
	std::cout << "Window::cleanup" << std::endl;
	glfwDestroyWindow(m_window);
	glfwTerminate();
}


void Window::initWindow() {
	std::cout << "Window::init" << std::endl;
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_window = glfwCreateWindow(1080, 720, "Vulkan", nullptr, nullptr);

}
