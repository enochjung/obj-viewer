#include "object.h"

#include <ranges> // ranges
#include <algorithm> // min max
#include <iostream> // cout

namespace obj_viewer {

	mesh::mesh(const std::vector<glm::vec3> points) {
		this->points.assign(points.begin(), points.end());

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao); 
		
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * this->points.size(), &(this->points[0]), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	}

	object::object(const std::vector<obj_loader::Mesh>& meshes) {
		for (const obj_loader::Mesh m : meshes) {
			const size_t indices_size = m.Indices.size();
			const auto r_points = m.Indices
				| std::views::transform([&m](unsigned int index) { return m.Vertices[index].Position; })
				| std::views::transform([](obj_loader::Vector3 position) { return glm::vec3(position.X, position.Y, position.Z); });
			const std::vector<glm::vec3> points(r_points.begin(), r_points.end());

			const mesh my_mesh(points);
			this->meshes.push_back(my_mesh);
		}

		const auto minmax = this->minmax();

		const float sx = 2.0f / (minmax.second.x - minmax.first.x);
		const float sy = 2.0f / (minmax.second.y - minmax.first.y);
		const float sz = 2.0f / (minmax.second.z - minmax.first.z);
		const float scale = std::min({ sx, sy, sz });
		_scale = { scale, scale, scale };

		const float dx = (minmax.first.x + minmax.second.x) * -0.5f;
		const float dy = (minmax.first.y + minmax.second.y) * -0.5f;
		const float dz = (minmax.first.z + minmax.second.z) * -0.5f;
		_position = { dx, dy, dz };

		_orientation = { 1.0f, 0.0f, 0.0f, 0.0f };
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
		static bool initialized = false;
		static std::pair<glm::vec3, glm::vec3> result;

		if (initialized)
			return result;

		for (const mesh m : meshes) {
			for (const glm::vec3& point : m.points) {
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
