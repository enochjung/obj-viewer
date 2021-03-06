#include "engine.h"

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h> // glutMouseWheelFunc
#include <glm/vec3.hpp> // vec3
#include <glm/mat4x4.hpp> // mat4
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp> // scale translate
#include <glm/gtx/quaternion.hpp> // quat

#include <iostream> // cout
#include <cmath> // sin cos

#define BUFFER_OFFSET(offset) ((GLvoid*)(offset))

GLuint InitShader(const char* vShaderFile, const char* fShaderFiie);

namespace obj_viewer {

	static bool mouse_pressing;
	static std::unique_ptr<glm::vec3> last_cursor_vec3;
	static glm::quat camera_orientation = { 1.0f, 0.0f, 0.0f, 0.0f };
	static glm::vec3 light_position = { 100.0f, 100.0f, 100.0f };

	static void display_callback();
	static void idle_callback();
	static void reshape_callback(int width, int height);
	static void keyboard_callback(unsigned char key, int x, int y);
	static void mouse_button_callback(int button, int state, int x, int y);
	static void mouse_motion_callback(int x, int y);
	static void wheel_callback(int wheel, int direction, int x, int y);
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
		glutIdleFunc(idle_callback);
		glutReshapeFunc(reshape_callback);
		glutKeyboardFunc(keyboard_callback);
		glutMouseFunc(mouse_button_callback);
		glutMotionFunc(mouse_motion_callback);
		glutMouseWheelFunc(wheel_callback);

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);

		GLuint program = InitShader("vshader.glsl", "fshader.glsl");
		glUseProgram(program);

		_model_view_loc = glGetUniformLocation(program, "ModelView");
		_model_loc = glGetUniformLocation(program, "Model");
		_view_loc = glGetUniformLocation(program, "View");
		_projection_loc = glGetUniformLocation(program, "Projection");
		_light_loc = glGetUniformLocation(program, "lightPosition");

		_diffuse_loc = glGetUniformLocation(program, "diffuse");
		_specular_loc = glGetUniformLocation(program, "specular");
		_ambient_loc = glGetUniformLocation(program, "ambient");
		_shininess_loc = glGetUniformLocation(program, "shininess");
		_texture_loc = glGetUniformLocation(program, "textureSampler");

		glUniform3fv(_light_loc, 1, glm::value_ptr(light_position));

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
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

	GLuint engine::model_loc() const {
		return _model_loc;
	}

	GLuint engine::view_loc() const {
		return _view_loc;
	}

	GLuint engine::projection_loc() const {
		return _projection_loc;
	}

	GLuint engine::light_loc() const {
		return _light_loc;
	}

	GLuint engine::diffuse_loc() const {
		return _diffuse_loc;
	}

	GLuint engine::specular_loc() const {
		return _specular_loc;
	}

	GLuint engine::ambient_loc() const {
		return _ambient_loc;
	}

	GLuint engine::shininess_loc() const {
		return _shininess_loc;
	}

	GLuint engine::texture_loc() const {
		return _texture_loc;
	}

	std::pair<int, int> engine::window_size() const {
		int width = glutGet(GLUT_WINDOW_WIDTH);
		int height = glutGet(GLUT_WINDOW_HEIGHT);
		return std::pair<int, int>(width, height);
	}

	static void display_callback() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const engine& engine = engine::instance();
		const auto model_view_loc = engine.model_view_loc();
		const auto model_loc = engine.model_loc();
		const auto view_loc = engine.view_loc();
		const auto diffuse_loc = engine.diffuse_loc();
		const auto specular_loc = engine.specular_loc();
		const auto ambient_loc = engine.ambient_loc();
		const auto shininess_loc = engine.shininess_loc();
		const auto texture_loc = engine.texture_loc();

		const auto camera_origin_position = glm::vec3(0.0f, 0.0f, 4.0f);
		const auto m_camera_origin_view = glm::lookAt(camera_origin_position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		const auto m_camera_rotation = glm::toMat4(camera_orientation);
		const auto m_view = m_camera_origin_view * m_camera_rotation;

		for (auto& obj : engine.objs) {
			const auto m_scale = glm::scale(*(obj->scale()));
			const auto m_translation = glm::translate(glm::mat4(1), *(obj->position()));
			const auto m_rotation = glm::toMat4(*(obj->orientation()));
			const auto m_model = m_translation * m_scale * m_rotation;
			const auto m_model_view = m_view * m_model;

			glUniformMatrix4fv(model_view_loc, 1, GL_FALSE, glm::value_ptr(m_model_view));
			glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(m_model));
			glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(m_view));

			for (auto& mesh : obj.get()->meshes) {
				glBindVertexArray(mesh.vao);

				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_buffer);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

				glEnableVertexAttribArray(1);
				glBindBuffer(GL_ARRAY_BUFFER, mesh.uv_buffer);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

				glEnableVertexAttribArray(2);
				glBindBuffer(GL_ARRAY_BUFFER, mesh.normal_buffer);
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

				glUniformMatrix4fv(model_view_loc, 1, GL_FALSE, glm::value_ptr(m_model_view));

				glUniform3fv(diffuse_loc, 1, glm::value_ptr(mesh.material.diffuse));
				glUniform3fv(specular_loc, 1, glm::value_ptr(mesh.material.specular));
				glUniform3fv(ambient_loc, 1, glm::value_ptr(mesh.material.ambient));
				glUniform1f(shininess_loc, mesh.material.shininess);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, mesh.texture_id);
				glUniform1i(texture_loc, 0);

				glDrawArrays(GL_TRIANGLES, 0, mesh.vertices.positions.size());
			}
		}

		glutSwapBuffers();
	}

	static void idle_callback() {
		const float speed = 0.0002f;
		const glm::vec3 axis = { 0.0f, 1.0f, 0.0f };
		const glm::quat rotation = glm::angleAxis(speed, axis);
		const engine& engine = engine::instance();
		for (const auto& obj : engine.objs)
			obj->rotate(rotation);
		glutPostRedisplay();
	}

	static void reshape_callback(int width, int height) {
		const GLfloat FoV = 50.0f;
		const GLfloat aspect = GLfloat(width == 0 ? 1 : width) / (height == 0 ? 1 : height);
		const GLuint projection_loc = engine::instance().projection_loc();
		const glm::mat4 m_projection = glm::perspective(glm::radians(FoV), aspect, 0.1f, 100.0f);
		glUniformMatrix4fv(projection_loc, 1, GL_FALSE, value_ptr(m_projection));

		glViewport(0, 0, width, height);
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
			}
			else {
				mouse_pressing = false;
			}
		}
	}

	static void mouse_motion_callback(int x, int y) {
		if (!mouse_pressing)
			return;

		std::unique_ptr<glm::vec3> cursor_vec3 = point_to_trackball_vec3(x, y);

		const float speed = -0.05f;
		const float angle = glm::dot(*cursor_vec3, *last_cursor_vec3);
		const glm::vec3 axis = glm::cross(*cursor_vec3, *last_cursor_vec3);
		if (glm::length(axis) < 0.0001f)
			return;
		const glm::vec3 norm = glm::normalize(axis);
		const glm::quat quaternion = glm::angleAxis(speed * angle, norm);

		const engine& engine = engine::instance();
		camera_orientation = quaternion * camera_orientation;

		last_cursor_vec3 = std::move(cursor_vec3);
		glutPostRedisplay();
	}

	static void wheel_callback(int wheel, int direction, int x, int y)
	{
		const float scale_delta = 0.95f;
		const engine& engine = engine::instance();
		for (const auto& obj : engine.objs) {
			if (direction > 0)
				obj->scaling(1 / scale_delta);
			else
				obj->scaling(scale_delta);
		}
		glutPostRedisplay();
	}

	static std::unique_ptr<glm::vec3> point_to_trackball_vec3(int x, int y) {
		int width, height;
		std::tie(width, height) = engine::instance().window_size();

		const int criteria = std::min(width, height);
		const float _x = (2.0f * x - width) / criteria;
		const float _y = (height - 2.0f * y) / criteria;
		const float distance = sqrt(_x * _x + _y * _y);

		glm::vec3 v;
		if (distance < 1.0f) {
			const float _z = sin(acos(distance));
			v = { _x, _y, _z };
		}
		else
			v = { _x / distance, _y / distance, 0 };

		return std::make_unique<glm::vec3>(v);
	}
}
