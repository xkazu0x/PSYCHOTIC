#pragma once

#include <glad/glad.h>
#include <glfw/glfw3.h>

enum window_callback {
	WINDOW_CALLBACK_KEY,
	WINDOW_CALLBACK_CURSOR_POS,
	WINDOW_CALLBACK_MOUSE_BUTTON
};

struct window_info {
	const char *title;
	int width;
	int height;
};

struct window_state {
	GLFWwindow *handle;
};

namespace psywindow {
	bool window_initialize(window_info *info, window_state *window);
	void window_shutdown(window_state *window);
	void end_frame(window_state *window);
	bool window_alive(window_state *window);
	void kill(window_state *window);
	void window_set_callback(window_state *window, window_callback type, void(*function));
}