#include <window/window.h>

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

struct cube_context {
	GLuint vao;
	GLuint program;
	GLuint texture;
};

struct imgui_context {
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	GLuint program;
	GLuint texture;
};

namespace cube {
	void create(cube_context *cube);
	void destroy(cube_context *cube);
	void render(cube_context *cube, GLuint per_frame_data_buffer, float ratio);
}

namespace psyimgui {
	void create(imgui_context *imgui, window_info *info);
	void destroy(imgui_context *imgui);
	void render(imgui_context *imgui, GLuint per_frame_data_buffer, window_info *info);
}

static window_state window;

void window_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void window_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
void window_mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

void create_shader_program(std::string vertex_file, std::string fragment_file, GLuint *program);
std::string read_text_from_file(std::string path);
int check_shader(unsigned int shader, const char *type);

int main() {
	window_info info = {};
	info.title = "PSYCHOTIC";
	info.width = 1920 / 2;
	info.height = 1080 / 2;

	psywindow::window_initialize(&info, &window);
	psywindow::window_set_callback(&window, WINDOW_CALLBACK_KEY, window_key_callback);
	psywindow::window_set_callback(&window, WINDOW_CALLBACK_CURSOR_POS, window_cursor_pos_callback);
	psywindow::window_set_callback(&window, WINDOW_CALLBACK_MOUSE_BUTTON, window_mouse_button_callback);

	GLuint per_frame_data_buffer;
	glCreateBuffers(1, &per_frame_data_buffer);
	glNamedBufferStorage(
		per_frame_data_buffer,
		sizeof(per_frame_data),
		nullptr,
		GL_DYNAMIC_STORAGE_BIT);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, per_frame_data_buffer, 0, sizeof(per_frame_data));

	//cube_context cube = {};
	//cube::create(&cube);

	imgui_context imgui = {};
	psyimgui::create(&imgui, &info);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	while (psywindow::window_alive(&window)) {
		glfwGetFramebufferSize(window.handle, &info.width, &info.height);
		const float ratio = info.width / (float)info.height;

		glViewport(0, 0, info.width, info.height);
		glClear(GL_COLOR_BUFFER_BIT);
		
		//cube::render(&cube, per_frame_data_buffer, ratio);
		psyimgui::render(&imgui, per_frame_data_buffer, &info);
		
		psywindow::end_frame(&window);
	}

	psyimgui::destroy(&imgui);
	//cube::destroy(&cube);

	glDeleteBuffers(1, &per_frame_data_buffer);

	psywindow::window_shutdown(&window);

	return 0;
}

//
//
//
// CREATIONS
namespace cube {
	void create(cube_context *cube) {
		create_shader_program("cube.vert", "cube.frag", &cube->program);
		glCreateVertexArrays(1, &cube->vao);
		{
			int width, height, comp;
			stbi_set_flip_vertically_on_load(1);
			uint8_t *data = stbi_load("res/textures/goreshit.jpg", &width, &height, &comp, 3);
			if (data) {
				GLenum format = GL_NONE;
				if (comp == 1) format = GL_RED;
				else if (comp == 3) format = GL_RGB;
				else if (comp == 4) format = GL_RGBA;

				glCreateTextures(GL_TEXTURE_2D, 1, &cube->texture);
				glTextureParameteri(cube->texture, GL_TEXTURE_MAX_LEVEL, 0);
				glTextureParameteri(cube->texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTextureParameteri(cube->texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glTextureStorage2D(cube->texture, 1, GL_RGB8, width, height);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

				glTextureSubImage2D(cube->texture, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
			}

			stbi_image_free(data);
		}
	}

	void destroy(cube_context *cube) {
		glDeleteTextures(1, &cube->texture);
		glDeleteProgram(cube->program);
		glDeleteVertexArrays(1, &cube->vao);
	}

	void render(cube_context *cube, GLuint per_frame_data_buffer, float ratio) {
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(-1.0f, -1.0f);

		glClear(GL_DEPTH_BUFFER_BIT);

		const glm::mat4 model = glm::rotate(
			glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.5f)),
			(float)glfwGetTime(),
			glm::vec3(0.0f, 1.0f, 0.0f));
		const glm::mat4 pers_projection = glm::perspective(45.0f, ratio, 0.1f, 10.0f);

		per_frame_data frame_data = {};
		frame_data.mvp = pers_projection * model;
		frame_data.is_wire_frame = false;

		glUseProgram(cube->program);
		glBindVertexArray(cube->vao);
		glBindTextures(0, 1, &cube->texture);

		glNamedBufferSubData(per_frame_data_buffer, 0, sizeof(per_frame_data), &frame_data);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		frame_data.is_wire_frame = true;
		glNamedBufferSubData(per_frame_data_buffer, 0, sizeof(per_frame_data), &frame_data);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

namespace psyimgui {
	void create(imgui_context *imgui, window_info *info) {
		glCreateVertexArrays(1, &imgui->vao);

		glCreateBuffers(1, &imgui->vbo);
		glNamedBufferStorage(imgui->vbo, 256 * 1024, nullptr, GL_DYNAMIC_STORAGE_BIT);

		glCreateBuffers(1, &imgui->ebo);
		glNamedBufferStorage(imgui->ebo, 256 * 1024, nullptr, GL_DYNAMIC_STORAGE_BIT);

		glVertexArrayElementBuffer(imgui->vao, imgui->ebo);
		glVertexArrayVertexBuffer(imgui->vao, 0, imgui->vbo, 0, sizeof(ImDrawVert));

		glEnableVertexArrayAttrib(imgui->vao, 0);
		glEnableVertexArrayAttrib(imgui->vao, 1);
		glEnableVertexArrayAttrib(imgui->vao, 2);

		glVertexArrayAttribFormat(imgui->vao, 0, 2, GL_FLOAT, GL_FALSE, IM_OFFSETOF(ImDrawVert, pos));
		glVertexArrayAttribFormat(imgui->vao, 1, 2, GL_FLOAT, GL_FALSE, IM_OFFSETOF(ImDrawVert, uv));
		glVertexArrayAttribFormat(imgui->vao, 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, IM_OFFSETOF(ImDrawVert, col));

		glVertexArrayAttribBinding(imgui->vao, 0, 0);
		glVertexArrayAttribBinding(imgui->vao, 1, 0);
		glVertexArrayAttribBinding(imgui->vao, 2, 0);

		create_shader_program("imgui.vert", "imgui.frag", &imgui->program);

		ImGui::CreateContext();

		ImGuiIO &io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

		ImFontConfig cfg = ImFontConfig();
		cfg.FontDataOwnedByAtlas = false;
		cfg.RasterizerMultiply = 1.5f;
		cfg.SizePixels = (float)info->height / 32.0f;
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

		glCreateTextures(GL_TEXTURE_2D, 1, &imgui->texture);
		glTextureParameteri(imgui->texture, GL_TEXTURE_MAX_LEVEL, 0);
		glTextureParameteri(imgui->texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(imgui->texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureStorage2D(imgui->texture, 1, GL_RGBA8, width, height);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTextureSubImage2D(
			imgui->texture, 0,
			0, 0,
			width, height,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			pixels);

		io.Fonts->TexID = (ImTextureID)(intptr_t)imgui->texture;
		io.FontDefault = font;
		io.DisplayFramebufferScale = ImVec2(1, 1);
	}

	void destroy(imgui_context *imgui) {
		ImGui::DestroyContext();
		glDeleteTextures(1, &imgui->texture);
		glDeleteProgram(imgui->program);
		glDeleteShader(imgui->ebo);
		glDeleteShader(imgui->vbo);
		glDeleteVertexArrays(1, &imgui->vao);
	}

	void render(imgui_context *imgui, GLuint per_frame_data_buffer, window_info *info) {
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_SCISSOR_TEST);

		ImGuiIO &io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)info->width, (float)info->height);
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

		glUseProgram(imgui->program);
		glBindVertexArray(imgui->vao);
		glBindTextures(0, 1, &imgui->texture);

		glNamedBufferSubData(per_frame_data_buffer, 0, sizeof(per_frame_data), &frame_data);

		for (int i = 0; i < draw_data->CmdListsCount; i++) {
			const ImDrawList *cmd_list = draw_data->CmdLists[i];
			glNamedBufferSubData(
				imgui->vbo,
				0,
				(GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
				cmd_list->VtxBuffer.Data);
			glNamedBufferSubData(
				imgui->ebo,
				0,
				(GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
				cmd_list->IdxBuffer.Data);

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
				const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
				const ImVec4 clip_rect = pcmd->ClipRect;
				glScissor((int)clip_rect.x, (int)(info->height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));
				glBindTextureUnit(0, (GLuint)(intptr_t)pcmd->TextureId);
				glDrawElementsBaseVertex(
					GL_TRIANGLES,
					(GLsizei)pcmd->ElemCount,
					GL_UNSIGNED_SHORT,
					(void *)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)),
					(GLint)pcmd->VtxOffset);
			}
		}

		glScissor(0, 0, info->width, info->height);
	}
}

//
// CALLBACKS
void window_key_callback(GLFWwindow *handle, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		psywindow::kill(&window);
	}
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
		int width, height;
		glfwGetFramebufferSize(handle, &width, &height);
		uint8_t *ptr = (uint8_t*)malloc(width * height * sizeof(int));
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, ptr);
		stbi_write_png("screenshot.png", width, height, 4, ptr, 0);
		free(ptr);
	}
}

void window_cursor_pos_callback(GLFWwindow *handle, double xpos, double ypos) {
	ImGui::GetIO().MousePos = ImVec2((float)xpos, (float)ypos);
}

void window_mouse_button_callback(GLFWwindow *handle, int button, int action, int mods) {
	auto &io = ImGui::GetIO();
	int index = button == GLFW_MOUSE_BUTTON_LEFT ? 0 : button == GLFW_MOUSE_BUTTON_RIGHT ? 2 : 1;
	io.MouseDown[index] = action == GLFW_PRESS;
}

//
// UTILS
void create_shader_program(std::string vertex_file, std::string fragment_file, GLuint *program) {
	std::string vertex_source = read_text_from_file("res/shaders/" + vertex_file);
	std::string fragment_source = read_text_from_file("res/shaders/" + fragment_file);

	const char *vertex_string = vertex_source.c_str();
	const char *fragment_string = fragment_source.c_str();

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_string, nullptr);
	glCompileShader(vertex_shader);
	check_shader(vertex_shader, "VERTEX SHADER");

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_string, nullptr);
	glCompileShader(fragment_shader);
	check_shader(fragment_shader, "FRAGMENT SHADER");

	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	check_shader(shader_program, "PROGRAM");

	glDeleteShader(vertex_shader);
	glDeleteShader(vertex_shader);

	*program = shader_program;
}

std::string read_text_from_file(std::string path) {
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

