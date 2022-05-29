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
		void rotate(const glm::vec4& quaternion);

		std::unique_ptr<glm::mat4> scale_mat() const;
		std::unique_ptr<glm::mat4> translation_mat() const;
		std::unique_ptr<glm::mat4> orientation_mat() const;

	private:
		float _scale;
		glm::vec3 _translation;
		glm::vec4 _quaternion;

		std::pair<glm::vec3, glm::vec3> minmax() const;
	};
}
