#include "window.h"

#include <stdio.h>

void glfw_error_callback(int error, const char *description);

bool psywindow::window_initialize(window_info *info, window_state *window) {
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *mode = glfwGetVideoMode(monitor);

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	GLFWwindow *handle = glfwCreateWindow(
		info->width,
		info->height,
		info->title,
		nullptr,
		nullptr);
	if (!handle) {
		glfwTerminate();
		return false;
	} else {
		window->handle = handle;
	}

	glfwSetWindowPos(
		window->handle,
		(mode->width - info->width) / 2,
		(mode->height - info->height) / 2);

	glfwMakeContextCurrent(window->handle);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwDestroyWindow(window->handle);
		glfwTerminate();
		return false;
	}

	glfwSwapInterval(1);

	return true;
}

void psywindow::window_shutdown(window_state *window) {
	if (window->handle) {
		glfwDestroyWindow(window->handle);
		window->handle = 0;
	}
	glfwTerminate();
}

void psywindow::end_frame(window_state *window) {
	glfwSwapBuffers(window->handle);
	glfwPollEvents();
}

bool psywindow::window_alive(window_state *window) {
	return !glfwWindowShouldClose(window->handle);
}

void psywindow::kill(window_state *window) {
	glfwSetWindowShouldClose(window->handle, 1);
}

void psywindow::window_set_callback(window_state *window, window_callback type, void(*function)) {
	if (type == WINDOW_CALLBACK_KEY) {
		glfwSetKeyCallback(window->handle, (GLFWkeyfun)function);
	}
	if (type == WINDOW_CALLBACK_CURSOR_POS) {
		glfwSetCursorPosCallback(window->handle, (GLFWcursorposfun)function);
	}
	if (type == WINDOW_CALLBACK_MOUSE_BUTTON) {
		glfwSetMouseButtonCallback(window->handle, (GLFWmousebuttonfun)function);
	}
}

void glfw_error_callback(int error, const char *description) {
	fprintf(stderr, "Error: %s\n", description);
}
