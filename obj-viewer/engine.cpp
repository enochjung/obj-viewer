#include "engine.h"

#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <numeric>
#include <ranges>
#include <cmath>

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

	engine::engine() : _projection_loc(0), _model_loc(0) {
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

		GLuint position_loc = glGetAttribLocation(program, "vPosition");
		glEnableVertexAttribArray(position_loc);
		glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

		_projection_loc = glGetUniformLocation(program, "mProjection");
		_model_loc = glGetUniformLocation(program, "mModel");

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glEnable(GL_DEPTH_TEST);
	}

	void engine::run() {
		glutMainLoop();
	}

	void engine::draw(std::unique_ptr<object> obj) {
		glBufferData(GL_ARRAY_BUFFER, obj->points.size() * sizeof(obj->points[0]), &obj->points[0], GL_STATIC_DRAW);
		this->obj = std::move(obj);
	}

	int engine::points_num() const {
		return this->obj != nullptr ? this->obj->points.size() : 0;
	}

	GLuint engine::model_loc() const {
		return _model_loc;
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

		const engine& engine = engine::instance();
		const auto& obj = engine.obj;
		const std::unique_ptr<glm::mat4> m_scale = obj->scale_mat();
		const std::unique_ptr<glm::mat4> m_translation = obj->translation_mat();
		const std::unique_ptr<glm::mat4> m_orientation = obj->orientation_mat();
		const glm::mat4 m_model = (*m_orientation) * (*m_translation) * (*m_scale);
		glUniformMatrix4fv(engine.model_loc(), 1, GL_FALSE, value_ptr(m_model));

		const int points_num = engine.points_num();
		glDrawArrays(GL_TRIANGLES, 0, points_num);

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
			const glm::vec4 quaternion = {
				cos(angle / 2),
				sin(angle / 2) * norm.x,
				sin(angle / 2) * norm.y,
				sin(angle / 2) * norm.z
			};
			engine::instance().obj.get()->rotate(quaternion);

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
