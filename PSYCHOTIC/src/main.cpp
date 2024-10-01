#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <imgui.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

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
void glfw_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
void glfw_mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

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

	GLFWwindow *handle = glfwCreateWindow(
		desc.width, 
		desc.height, 
		desc.title, 
		nullptr, 
		nullptr);
	if (!handle) {
		glfwTerminate();
		return -1;
	} else {
		window.handle = handle;
	}

	glfwSetWindowPos(
		window.handle, 
		(mode->width - desc.width) / 2, 
		(mode->height - desc.height) / 2);

	glfwSetKeyCallback(window.handle, glfw_key_callback);
	glfwSetCursorPosCallback(window.handle, glfw_cursor_pos_callback);
	glfwSetMouseButtonCallback(window.handle, glfw_mouse_button_callback);

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

	GLuint vbo_handle;
	glCreateBuffers(1, &vbo_handle);
	glNamedBufferStorage(vbo_handle, 256 * 1024, nullptr, GL_DYNAMIC_STORAGE_BIT);

	GLuint ebo_handle;
	glCreateBuffers(1, &ebo_handle);
	glNamedBufferStorage(ebo_handle, 256 * 1024, nullptr, GL_DYNAMIC_STORAGE_BIT);

	glVertexArrayElementBuffer(vao, ebo_handle);
	glVertexArrayVertexBuffer(vao, 0, vbo_handle, 0, sizeof(ImDrawVert));

	glEnableVertexArrayAttrib(vao, 0);
	glEnableVertexArrayAttrib(vao, 1);
	glEnableVertexArrayAttrib(vao, 2);

	glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, IM_OFFSETOF(ImDrawVert, pos));
	glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, IM_OFFSETOF(ImDrawVert, uv));
	glVertexArrayAttribFormat(vao, 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, IM_OFFSETOF(ImDrawVert, col));

	glVertexArrayAttribBinding(vao, 0, 0);
	glVertexArrayAttribBinding(vao, 1, 0);
	glVertexArrayAttribBinding(vao, 2, 0);

	glBindVertexArray(vao);

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
	
	//
	//
	const GLsizeiptr buffer_size = sizeof(per_frame_data);

	GLuint per_frame_data_buffer;
	glCreateBuffers(1, &per_frame_data_buffer);
	glNamedBufferStorage(
		per_frame_data_buffer, 
		buffer_size,
		nullptr, 
		GL_DYNAMIC_STORAGE_BIT);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, per_frame_data_buffer, 0, buffer_size);

	//
	//
	ImGui::CreateContext();

	ImGuiIO &io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

	ImFontConfig cfg = ImFontConfig();
	cfg.FontDataOwnedByAtlas = false;
	cfg.RasterizerMultiply = 1.5f;
	cfg.SizePixels = (float)desc.height / 32.0f;
	cfg.PixelSnapH = true;
	cfg.OversampleH = 4;
	cfg.OversampleV = 4;

	ImFont *font = io.Fonts->AddFontFromFileTTF(
		"res/fonts/liberation-mono.ttf", 
		cfg.SizePixels, 
		&cfg);;

	unsigned char *pixels = nullptr;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	GLuint texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	glTextureParameteri(texture, GL_TEXTURE_MAX_LEVEL, 0);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D(texture, 1, GL_RGBA8, width, height);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTextureSubImage2D(
		texture, 0, 
		0, 0,
		width, height, 
		GL_RGBA, 
		GL_UNSIGNED_BYTE, 
		pixels);

	glBindTextures(0, 1, &texture);

	io.Fonts->TexID = (ImTextureID)(intptr_t)texture;
	io.FontDefault = font;
	io.DisplayFramebufferScale = ImVec2(1, 1);

	//
	//
	/*
	int width, height, comp;
	const uint8_t *img = stbi_load("res/textures/goreshit.jpg", &width, &height, &comp, 3);

	GLuint texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	glTextureParameteri(texture, GL_TEXTURE_MAX_LEVEL, 0);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTextureStorage2D(texture, 1, GL_RGB8, width, height);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, img);
	glBindTextures(0, 1, &texture);

	stbi_image_free((void*)img);
	*/

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_POLYGON_OFFSET_LINE);
	//glPolygonOffset(-1.0f, -1.0f);

	while (!glfwWindowShouldClose(window.handle)) {
		glfwGetFramebufferSize(window.handle, &desc.width, &desc.height);
		const float ratio = desc.width / (float)desc.height;

		glViewport(0, 0, desc.width, desc.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//
		ImGuiIO &io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)desc.width, (float)desc.height);
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();
		ImGui::Render();

		const ImDrawData *draw_data = ImGui::GetDrawData();

		const float left = draw_data->DisplayPos.x;
		const float right = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
		const float top = draw_data->DisplayPos.y;
		const float bottom = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
		const glm::mat4 ortho_projection = glm::ortho(left, right, bottom, top);

		per_frame_data frame_data = {};
		frame_data.mvp = ortho_projection;
		frame_data.is_wire_frame = false;
		glNamedBufferSubData(per_frame_data_buffer, 0, buffer_size, &frame_data);

		for (int i = 0; i < draw_data->CmdListsCount; i++) {
			const ImDrawList *cmd_list = draw_data->CmdLists[i];
			glNamedBufferSubData(
				vbo_handle, 
				0, 
				(GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), 
				cmd_list->VtxBuffer.Data);
			glNamedBufferSubData(
				ebo_handle, 
				0, 
				(GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), 
				cmd_list->IdxBuffer.Data);

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
				const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
				const ImVec4 cr = pcmd->ClipRect;
				glScissor((int)cr.x, (int)(desc.height - cr.w), (int)(cr.z - cr.x), (int)(cr.w - cr.y));
				glBindTextureUnit(0, (GLuint)(intptr_t)pcmd->TextureId);
				glDrawElementsBaseVertex(
					GL_TRIANGLES, 
					(GLsizei)pcmd->ElemCount, 
					GL_UNSIGNED_SHORT,
					(void *)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)), 
					(GLint)pcmd->VtxOffset);
			}
		}

		/*
		const glm::mat4 model = glm::rotate(
			glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.5f)), 
			(float)glfwGetTime(), 
			glm::vec3(1.0f, 1.0f, 1.0f));
		const glm::mat4 projection = glm::perspective(45.0f, ratio, 0.1f, 10.0f);

		per_frame_data frame_data = {};
		frame_data.mvp = projection * model;
		frame_data.is_wire_frame = false;

		//
		/*
		glNamedBufferSubData(per_frame_data_buffer, 0, buffer_size, &frame_data);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		frame_data.is_wire_frame = true;
		glNamedBufferSubData(per_frame_data_buffer, 0, buffer_size, &frame_data);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		*/

		glScissor(0, 0, desc.width, desc.height);

		glfwSwapBuffers(window.handle);
		glfwPollEvents();
	}

	ImGui::DestroyContext();

	glDeleteTextures(1, &texture);
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
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		uint8_t *ptr = (uint8_t*)malloc(width * height * sizeof(int));
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, ptr);
		stbi_write_png("screenshot.png", width, height, 4, ptr, 0);
		free(ptr);
	}
}

void glfw_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
	ImGui::GetIO().MousePos = ImVec2((float)xpos, (float)ypos);
}

void glfw_mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
	auto &io = ImGui::GetIO();
	int index = button == GLFW_MOUSE_BUTTON_LEFT ? 0 : button == GLFW_MOUSE_BUTTON_RIGHT ? 2 : 1;
	io.MouseDown[index] = action == GLFW_PRESS;
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

