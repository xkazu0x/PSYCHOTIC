#include <glad/glad.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>

struct per_frame_data {
	glm::mat4 mvp;
	int is_wire_frame;
};

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

std::string read_text_from_file(const char *path);
int check_shader(unsigned int shader, const char *type);

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
	glfwMakeContextCurrent(window.handle);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwDestroyWindow(window.handle);
		glfwTerminate();
		return -1;
	}

	glfwSwapInterval(1);

	//
	//
	std::string vertex_source = read_text_from_file("res/shaders/shader.vert");
	std::string fragment_source = read_text_from_file("res/shaders/shader.frag");

	const char *vertex_string = vertex_source.c_str();
	const char *fragment_string = fragment_source.c_str();

	const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_string, nullptr);
	glCompileShader(vertex_shader);
	check_shader(vertex_shader, "VERTEX SHADER");

	const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_string, nullptr);
	glCompileShader(fragment_shader);
	check_shader(fragment_shader, "FRAGMENT SHADER");

	const GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	check_shader(program, "PROGRAM");
	glUseProgram(program);

	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	//
	//
	const GLsizeiptr buffer_size = sizeof(per_frame_data);

	GLuint per_frame_data_buffer;
	glCreateBuffers(1, &per_frame_data_buffer);
	glNamedBufferStorage(per_frame_data_buffer, buffer_size, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, per_frame_data_buffer, 0, buffer_size);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.0f, -1.0f);

	while (!glfwWindowShouldClose(window.handle)) {
		int width, height;
		glfwGetFramebufferSize(window.handle, &width, &height);
		const float ratio = width / (float)height;

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const glm::mat4 model = glm::rotate(
			glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.5f)), 
			(float)glfwGetTime(), 
			glm::vec3(1.0f, 1.0f, 1.0f));
		const glm::mat4 projection = glm::perspective(45.0f, ratio, 0.1f, 10.0f);

		per_frame_data frame_data = {};
		frame_data.mvp = projection * model;
		frame_data.is_wire_frame = false;
		glNamedBufferSubData(per_frame_data_buffer, 0, buffer_size, &frame_data);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		frame_data.is_wire_frame = true;
		glNamedBufferSubData(per_frame_data_buffer, 0, buffer_size, &frame_data);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window.handle);
		glfwPollEvents();
	}

	glDeleteBuffers(1, &per_frame_data_buffer);
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

int check_shader(unsigned int shader, const char *type) {
	int success;
	char infoLog[1024];
	if (type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			printf("Shader compilation error type: %s\n%s", type, infoLog);
		}
	} else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			printf("Program linkning error type: %s\n%s", type, infoLog);
		}
	}
	return success;
}

