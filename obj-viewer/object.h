#pragma once

#include <vector> // vector
#include <memory> // unique_ptr
#include <GL/glew.h> // GLuint
#include <glm/vec3.hpp> // vec3
#include <glm/gtx/quaternion.hpp> // quat
#include "OBJ_Loader.h"

namespace obj_viewer {

	class mesh {
	public:
		GLuint vao;
		GLuint vbo;
		std::vector<glm::vec3> points;

		mesh(const std::vector<glm::vec3> points);
	};

	class object {
	public:
		std::vector<mesh> meshes;

		object(const std::vector<obj_loader::Mesh>& meshes);
		void rotate(const glm::quat& rotation);

		std::unique_ptr<glm::vec3> scale() const;
		std::unique_ptr<glm::vec3> position() const;
		std::unique_ptr<glm::quat> orientation() const;

	private:
		glm::vec3 _scale;
		glm::vec3 _position;
		glm::quat _orientation;

		std::pair<glm::vec3, glm::vec3> minmax() const;
	};
}
