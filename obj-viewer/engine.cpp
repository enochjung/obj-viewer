#include "engine.h"

#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/vec3.hpp> // vec3
#include <glm/mat4x4.hpp> // mat4
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp> // scale translate
#include <glm/gtx/quaternion.hpp> // quat
//#include "OBJ_Loader.h"

#include <iostream> // cout
#include <numeric>
#include <ranges>
#include <cmath> // sin cos

#define BUFFER_OFFSET(offset) ((GLvoid*)(offset))

GLuint InitShader(const char* vShaderFile, const char* fShaderFiie);

namespace obj_viewer {

	static bool mouse_pressing;
	static std::unique_ptr<glm::vec3> last_cursor_vec3;

	static void display_callback();
	static void reshape_callback(int width, int height);
	static void keyboard_callback(unsigned char key, int x, int y);
	static void mouse_button_callback(int button, int state, int x, int y);
	static void mouse_motion_callback(int x, int y);
	static std::unique_ptr<glm::vec3> point_to_trackball_vec3(int x, int y);

	engine::engine() : _model_view_loc(0), _projection_loc(0) {
		// nop
	}

	engine& engine::instance() {
		static engine* instance = new engine();
		return *instance;
	}

	void engine::init(int* argc, char** argv, const std::string& title, int width, int height) {
		glutInit(argc, argv);

		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
		glutInitWindowSize(width, height);
		glutCreateWindow(title.c_str());
		glewInit();

		glutDisplayFunc(display_callback);
		glutReshapeFunc(reshape_callback);
		glutKeyboardFunc(keyboard_callback);
		glutMouseFunc(mouse_button_callback);
		glutMotionFunc(mouse_motion_callback);

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);

		GLuint program = InitShader("vshader.glsl", "fshader.glsl");
		glUseProgram(program);

		_model_view_loc = glGetUniformLocation(program, "mModelView");
		_projection_loc = glGetUniformLocation(program, "mProjection");

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glEnable(GL_DEPTH_TEST);
	}

	void engine::add_object(std::unique_ptr<object> obj) {
		this->objs.push_back(std::move(obj));
	}

	void engine::run() {
		glutMainLoop();
	}

	GLuint engine::model_view_loc() const {
		return _model_view_loc;
	}

	GLuint engine::projection_loc() const {
		return _projection_loc;
	}

	std::pair<int, int> engine::window_size() const {
		int width = glutGet(GLUT_WINDOW_WIDTH);
		int height = glutGet(GLUT_WINDOW_HEIGHT);
		return std::pair<int, int>(width, height);
	}

	static void display_callback() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);

		const engine& engine = engine::instance();
		const auto model_view_loc = engine.model_view_loc();
		// TODO: add view matrix

		for (auto& obj : engine.objs) {
			const auto m_scale = glm::scale(*(obj->scale()));
			const auto m_translation = glm::translate(glm::mat4(1), *(obj->position()));
			const auto m_rotation = glm::toMat4(*(obj->orientation()));
			const auto m_model = m_rotation * m_scale * m_translation;

			for (auto& mesh : obj.get()->meshes) {
				glBindVertexArray(mesh.vao);

				glUniformMatrix4fv(model_view_loc, 1, GL_FALSE, glm::value_ptr(m_model));
				// TODO: bind view matrix

				glDrawArrays(GL_TRIANGLES, 0, mesh.points.size());
			}
		}

		glutSwapBuffers();
	}

	static void reshape_callback(int width, int height) {
		const int view_size = 1;

		glViewport(0, 0, width, height);

		GLfloat left = -view_size, right = view_size;
		GLfloat bottom = -view_size, top = view_size;
		GLfloat zNear = -view_size, zFar = view_size;

		GLfloat aspect = GLfloat(width == 0 ? 1 : width) / (height == 0 ? 1 : height);

		if (aspect > 1) {
			left *= aspect;
			right *= aspect;
		}
		else {
			bottom /= aspect;
			top /= aspect;
		}

		GLuint projection_loc = engine::instance().projection_loc();
		glm::mat4 projection = glm::ortho(left, right, bottom, top, zNear, zFar);
		glUniformMatrix4fv(projection_loc, 1, GL_FALSE, value_ptr(projection));
	}

	static void keyboard_callback(unsigned char key, int x, int y) {
		if (key == 27)
			exit(0);
	}

	static void mouse_button_callback(int button, int state, int x, int y) {
		if (button == GLUT_LEFT_BUTTON) {
			if (state == GLUT_DOWN) {
				mouse_pressing = true;
				last_cursor_vec3 = point_to_trackball_vec3(x, y);
			} else {
				mouse_pressing = false;
			}
		}
	}

	static void mouse_motion_callback(int x, int y) {
		if (!mouse_pressing)
			return;

		std::unique_ptr<glm::vec3> cursor_vec3 = point_to_trackball_vec3(x, y);
		const glm::vec3 movement = *cursor_vec3 - *last_cursor_vec3;
		float distance_pow = dot(movement, movement);

		if (distance_pow) {
			const float angle = (3.141592f / 2.0f) * sqrt(distance_pow);
			const glm::vec3 axis = glm::cross(*last_cursor_vec3, *cursor_vec3);
			const glm::vec3 norm = axis / sqrt(dot(axis, axis));
			const glm::quat quaternion = glm::angleAxis(angle, norm);
			engine::instance().objs[0].get()->rotate(quaternion);
			// TODO: rotate view

			last_cursor_vec3 = std::move(cursor_vec3);
			glutPostRedisplay();
		}
	}

	static std::unique_ptr<glm::vec3> point_to_trackball_vec3(int x, int y) {
		int width, height;
		std::tie(width, height) = engine::instance().window_size();

		const int criteria = std::min(width, height);

		glm::vec3 v;
		v[0] = (2.0f * x - width) / criteria;
		v[1] = (height - 2.0f * y) / criteria;

		float distance = sqrt(v[0] * v[0] + v[1] * v[1]);
		v[2] = distance < 1.0f ? cos((3.141592f / 2.0f) * distance) : 0.0f;
		v *= 1.0 / sqrt(glm::dot(v, v));

		return std::make_unique<glm::vec3>(v);
	}
}
