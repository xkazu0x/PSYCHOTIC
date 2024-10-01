#include <stdio.h>
#include <fstream>
#include <string>

#include <glad/glad.h>
#include <glfw3.h>

struct window_desc {
	const char *title;
	int width;
	int height;
};

struct window_state {
	GLFWwindow *handle;
};

static window_state window;

void glfw_error_callback(int error, const char* description);
void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void glfw_window_size_callback(GLFWwindow *window, int width, int height);

std::string read_text_from_file(const char *path);

int main() {
	window_desc desc = {};
	desc.title = "PSYCHOTIC";
	desc.width = 1920 / 2;
	desc.height = 1080 / 2;

	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		return -1;
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

	GLFWwindow *handle = glfwCreateWindow(desc.width, desc.height, desc.title, nullptr, nullptr);
	if (!handle) {
		glfwTerminate();
		return -1;
	} else {
		window.handle = handle;
	}

	glfwSetWindowPos(window.handle, (mode->width - desc.width) / 2, (mode->height - desc.height) / 2);
	
	glfwSetKeyCallback(window.handle, glfw_key_callback);
	glfwSetWindowSizeCallback(window.handle, glfw_window_size_callback);

	glfwMakeContextCurrent(window.handle);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwDestroyWindow(window.handle);
		glfwTerminate();
		return -1;
	}

	glfwSwapInterval(1);

	//
	//
	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	std::string vertex_source = read_text_from_file("res/shaders/shader.vert");
	std::string fragment_source = read_text_from_file("res/shaders/shader.frag");

	const char *vertex_string = vertex_source.c_str();
	const char *fragment_string = fragment_source.c_str();

	const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_string, nullptr);
	glCompileShader(vertex_shader);

	const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_string, nullptr);
	glCompileShader(fragment_shader);

	const GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	
	while (!glfwWindowShouldClose(window.handle)) {
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window.handle);
		glfwPollEvents();
	}

	glDeleteProgram(program);
	glDeleteShader(fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteVertexArrays(1, &vao);

	glfwDestroyWindow(window.handle);
	glfwTerminate();

	return 0;
}

void glfw_error_callback(int error, const char *description) {
	fprintf(stderr, "Error: %s\n", description);
}

void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

std::string read_text_from_file(const char *path) {
	std::ifstream file(path);
	std::string str;
	std::string line;
	while (std::getline(file, line)) {
		str += line + "\n";
	}
	return str;
}

void glfw_window_size_callback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
}