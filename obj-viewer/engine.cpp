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

#define BUFFER_OFFSET(offset) ((GLvoid*)(offset))

GLuint InitShader(const char* vShaderFile, const char* fShaderFiie);

namespace obj_viewer {

	static void display_callback();
	static void reshape_callback(int width, int height);
	static void keyboard_callback(unsigned char key, int x, int y);

	engine::engine() : _projection_loc(0), _translation_loc(0) {
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

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		//glBufferData(GL_ARRAY_BUFFER, _points.value + sizeof(colors), NULL, GL_STATIC_DRAW);
		/*
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);
		*/

		GLuint program = InitShader("vshader.glsl", "fshader.glsl");
		glUseProgram(program);

		GLuint position_loc = glGetAttribLocation(program, "vPosition");
		glEnableVertexAttribArray(position_loc);
		glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

		_projection_loc = glGetUniformLocation(program, "mProjection");
		_translation_loc = glGetUniformLocation(program, "mTranslation");

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glEnable(GL_DEPTH_TEST);
	}

	void engine::run() {

		/*
		GLuint vColor = glGetAttribLocation(program, "vColor");
		glEnableVertexAttribArray(vColor);
		glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));
		*/

		// rotation matrix
		/*
		rotation_loc = glGetUniformLocation(program, "mRotation");
		glUniformMatrix4fv(rotation_loc, 1, GL_FALSE, value_ptr(mRotation));
		*/

		/*
		glutIdleFunc(spinCube);
		glutMouseFunc(mouse);
		*/

		glutMainLoop();
	}

	void engine::draw(std::unique_ptr<object> obj) {
		_obj = std::move(obj);
		glBufferData(GL_ARRAY_BUFFER, _obj->points.size() * sizeof(_obj->points[0]), &_obj->points[0], GL_STATIC_DRAW);

		std::unique_ptr<glm::mat4> mTranslation = _obj->centralize();
		glUniformMatrix4fv(_translation_loc, 1, GL_FALSE, value_ptr(*mTranslation));
	}

	int engine::points_num() const {
		return _obj != nullptr ? _obj->points.size() : 0;
	}

	GLuint engine::translation_loc() const {
		return _translation_loc;
	}

	GLuint engine::projection_loc() const {
		return _projection_loc;
	}

	static void display_callback() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int points_num = engine::instance().points_num();
		glDrawArrays(GL_TRIANGLES, 0, points_num);

		glutSwapBuffers();
	}

	static void reshape_callback(int width, int height)
	{
		const int view_size = 80;

		glViewport(0, 0, width, height);

		GLfloat left = -view_size, right = view_size;
		GLfloat bottom = -view_size, top = view_size;
		GLfloat zNear = -view_size, zFar = view_size;

		GLfloat aspect = GLfloat(width == 0 ? 1 : width) / (height == 0 ? 1 : height);

		if (aspect > 1)
		{
			left *= aspect;
			right *= aspect;
		}
		else
		{
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
}
