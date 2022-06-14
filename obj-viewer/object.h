#pragma once

#include <string> // string
#include <vector> // vector
#include <memory> // unique_ptr
#include <GL/glew.h> // GLuint
#include <glm/vec3.hpp> // vec3
#include <glm/gtx/quaternion.hpp> // quat
#include "tiny_obj_loader.h"

namespace obj_viewer {

	class vertices {
	public:
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texture_coordinates;

		vertices(size_t size);
	};

	class material {
	public:
		glm::vec3 diffuse;
		glm::vec3 specular;
		glm::vec3 ambient;
		float shininess;

		material();
	};

	class mesh {
	public:
		GLuint vao;
		GLuint vertex_buffer;
		GLuint uv_buffer;
		GLuint normal_buffer;
		GLuint texture_id;
		vertices vertices;
		material material;

		mesh(size_t vertices_size);
		void load_texture(const std::string& texture_name, const std::string texture_directory);
		void bind_buffer();
	};

	class object {
	public:
		std::vector<mesh> meshes;

		object(const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes, const std::vector<tinyobj::material_t>& materials, const std::string texture_directory);
		void rotate(const glm::quat& rotation);
		void scaling(float scale);

		std::unique_ptr<glm::vec3> scale() const;
		std::unique_ptr<glm::vec3> position() const;
		std::unique_ptr<glm::quat> orientation() const;

	private:
		glm::vec3 _scale;
		glm::vec3 _position;
		glm::quat _orientation;

		std::pair<glm::vec3, glm::vec3> minmax() const;
		int load_diffuse_texture(const tinyobj::material_t& material, const std::string texture_directory);
	};
}
