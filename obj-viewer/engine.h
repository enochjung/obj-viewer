#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <GL/glew.h>
#include <glm/vec3.hpp>
#include "OBJ_Loader.h"
#include "object.h"

namespace obj_viewer {
	
	class engine {
	public:
		static engine& instance();

		void init(int* argc, char** argv, const std::string& title, int width, int height);
		void draw(std::unique_ptr<object> obj);
		void run();

		int points_num() const;
		GLuint projection_loc() const;
		GLuint translation_loc() const;

	private:
		std::unique_ptr<object> _obj;
		GLuint _projection_loc;
		GLuint _translation_loc;

		engine();
	};
}
