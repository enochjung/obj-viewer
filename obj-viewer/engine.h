#pragma once

#include <string> // string
#include <vector> // vector
#include <memory> // unique_ptr
#include <GL/glew.h> // GLuint
#include "object.h" // object

namespace obj_viewer {
	
	class engine {
	public:
		static engine& instance();

		std::vector<std::unique_ptr<object>> objs;

		void init(int* argc, char** argv, const std::string& title, int width, int height);
		void add_object(std::unique_ptr<object> obj);
		void run();

		GLuint model_view_loc() const;
		GLuint model_loc() const;
		GLuint view_loc() const;
		GLuint projection_loc() const;
		GLuint light_loc() const;

		std::pair<int, int> window_size() const;

	private:
		GLuint _model_view_loc;
		GLuint _model_loc;
		GLuint _view_loc;
		GLuint _projection_loc;
		GLuint _light_loc;

		engine();
	};
}
