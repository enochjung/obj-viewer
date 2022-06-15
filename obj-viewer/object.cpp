#include "object.h"

#include <algorithm> // min max

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace obj_viewer {

	vertices::vertices(size_t size) : positions(size), normals(size), texture_coordinates(size) {
		// nop
	}

	material::material() : diffuse({ 0, 0, 0 }), specular({ 0, 0, 0 }), ambient({ 0, 0, 0 }), shininess(0) {
		// nop
	}

	mesh::mesh(size_t vertices_size) : vao(0), vertex_buffer(0), uv_buffer(0), normal_buffer(0), texture_id(0), vertices(vertices_size), material() {
		// nop
	}

	void mesh::load_texture(const std::string& texture_name, const std::string texture_directory) {
		int w, h;
		int comp;

		if (texture_name.length() > 0) {
			const std::string texture_filepath = texture_directory + texture_name;
			
			stbi_set_flip_vertically_on_load(true); 
			unsigned char* image = stbi_load(texture_filepath.c_str(), &w, &h, &comp, STBI_default);
			if (image != NULL) {
				glGenTextures(1, &texture_id);
				glBindTexture(GL_TEXTURE_2D, texture_id);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				if (comp == 3)
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
				else if (comp == 4)
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
				glBindTexture(GL_TEXTURE_2D, 0);
				glGenerateMipmap(GL_TEXTURE_2D);
				stbi_image_free(image);

				return;
			}
		}

		unsigned char white[3] = { 255, 255, 255 };
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, white);
		glBindTexture(GL_TEXTURE_2D, 0);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	void mesh::bind_buffer() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.positions.size(), &vertices.positions[0], GL_STATIC_DRAW);

		glGenBuffers(1, &uv_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertices.texture_coordinates.size(), &vertices.texture_coordinates[0], GL_STATIC_DRAW);

		glGenBuffers(1, &normal_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.normals.size(), &vertices.normals[0], GL_STATIC_DRAW);
	}

	object::object(const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes, const std::vector<tinyobj::material_t>& materials, const std::string texture_directory) {
		for (size_t s = 0; s < shapes.size(); s++) {
			const size_t vertices_size = shapes[s].mesh.num_face_vertices.size() * 3;
			mesh mesh(vertices_size);

			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

				for (size_t v = 0; v < fv; v++) {
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
					tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
					tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
					mesh.vertices.positions[index_offset + v] = glm::vec3(vx, vy, vz);

					if (idx.normal_index >= 0) {
						tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
						tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
						tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
						mesh.vertices.normals[index_offset + v] = glm::vec3(nx, ny, nz);
					}

					if (idx.texcoord_index >= 0) {
						tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
						tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
						mesh.vertices.texture_coordinates[index_offset + v] = glm::vec2(tx, ty);
					}
				}
				index_offset += fv;
			}

			const int mat_idx = shapes[s].mesh.material_ids[0];
			const auto& material = materials[mat_idx];
			mesh.material.diffuse = { material.diffuse[0], material.diffuse[1], material.diffuse[2] };
			mesh.material.specular = { material.specular[0], material.specular[1], material.specular[2] };
			mesh.material.ambient = { material.ambient[0], material.ambient[1], material.ambient[2] };
			mesh.material.shininess = material.shininess;

			mesh.bind_buffer();
			mesh.load_texture(material.diffuse_texname, texture_directory);
			this->meshes.push_back(mesh);
		}

		const auto minmax = this->minmax();

		const float sx = 2.0f / (minmax.second.x - minmax.first.x);
		const float sy = 2.0f / (minmax.second.y - minmax.first.y);
		const float sz = 2.0f / (minmax.second.z - minmax.first.z);
		const float scale = std::min({ sx, sy, sz });
		_scale = { scale, scale, scale };

		const float dx = ((minmax.first.x + minmax.second.x) * -0.5f) * scale;
		const float dy = ((minmax.first.y + minmax.second.y) * -0.5f) * scale;
		const float dz = ((minmax.first.z + minmax.second.z) * -0.5f) * scale;
		_position = { dx, dy, dz };

		_orientation = { 1.0f, 0.0f, 0.0f, 0.0f };
	}

	void object::scaling(float scale) {
		_scale *= scale;
	}

	void object::move(const glm::vec3& distance) {
		_position += distance;
	}

	void object::rotate(const glm::quat& rotation) {
		_orientation = rotation * _orientation;
	}

	std::unique_ptr<glm::vec3> object::scale() const {
		return std::make_unique<glm::vec3>(_scale);
	}

	std::unique_ptr<glm::vec3> object::position() const {
		return std::make_unique<glm::vec3>(_position);
	}

	std::unique_ptr<glm::quat> object::orientation() const {
		return std::make_unique<glm::quat>(_orientation);
	}

	std::pair<glm::vec3, glm::vec3> object::minmax() const {
		bool initialized = false;
		std::pair<glm::vec3, glm::vec3> result;

		for (const mesh m : meshes) {
			for (const glm::vec3& point : m.vertices.positions) {
				if (!initialized) {
					initialized = true;
					result.first.x = result.second.x = point.x;
					result.first.y = result.second.y = point.y;
					result.first.z = result.second.z = point.z;
				}
				else {
					result.first.x = std::min(result.first.x, point.x);
					result.first.y = std::min(result.first.y, point.y);
					result.first.z = std::min(result.first.z, point.z);
					result.second.x = std::max(result.second.x, point.x);
					result.second.y = std::max(result.second.y, point.y);
					result.second.z = std::max(result.second.z, point.z);
				}
			}
		}
		return result;
	}
}
