#pragma once

#include "Common.h"

class Window {
public:
	static std::unique_ptr<Window> createWindow();
	~Window();


	GLFWwindow* getWindow() { return m_window; }

private:
	Window() {}

	GLFWwindow* m_window;

	void initWindow();
	void cleanup();
};