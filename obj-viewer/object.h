#pragma once

#include <GL/glew.h>
#include <glm/vec3.hpp>
#include "OBJ_Loader.h"

namespace obj_viewer {

	class object {
	public:
		std::vector<glm::vec3> points;

		object(std::vector<Mesh> meshes);

	private:
	};
}
