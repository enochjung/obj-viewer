#pragma once

#include <memory>
#include <utility>
#include <GL/glew.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "OBJ_Loader.h"

namespace obj_viewer {

	class object {
	public:
		std::vector<glm::vec3> points;

		object(const std::vector<Mesh>& meshes);
		std::unique_ptr<glm::mat4> centralize() const;

	private:
		std::pair<glm::vec3, glm::vec3> minmax() const;
	};
}
