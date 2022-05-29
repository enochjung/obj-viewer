#include "object.h"

#include <ranges>
#include <numeric>
#include <algorithm>
#include <iostream>

namespace obj_viewer {

	object::object(const std::vector<Mesh>& meshes) {

		const auto sizes = meshes
			| std::views::transform([](const Mesh& mesh) { return mesh.Indices.size(); });
		const size_t all_points_num = std::accumulate(sizes.begin(), sizes.end(), 0);
		this->points = std::vector<glm::vec3>(all_points_num);

		int points_index = 0;
		for (int i = 0; i < (int)meshes.size(); ++i) {
			const Mesh& mesh = meshes[i];

			const auto points = mesh.Indices
				| std::views::transform([&mesh](unsigned int index) { return mesh.Vertices[index].Position; })
				| std::views::transform([](Vector3 position) { return glm::vec3(position.X, position.Y, position.Z); });

			for (const glm::vec3& point : points)
				this->points[points_index++] = point;
		}

		const auto minmax = this->minmax();

		const float sx = 2.0f / (minmax.second.x - minmax.first.x);
		const float sy = 2.0f / (minmax.second.y - minmax.first.y);
		const float sz = 2.0f / (minmax.second.z - minmax.first.z);
		_scale = std::min({ sx, sy, sz });

		const float dx = (minmax.first.x + minmax.second.x) * -0.5f;
		const float dy = (minmax.first.y + minmax.second.y) * -0.5f;
		const float dz = (minmax.first.z + minmax.second.z) * -0.5f;
		_translation = { dx, dy, dz };

		_quaternion = { 1.0f, 0.0f, 0.0f, 0.0f };
	}

	void object::rotate(const glm::vec4& quaternion) {
		const glm::vec4& a = _quaternion;
		const glm::vec4& b = quaternion;
		const glm::vec3 aa = { a.y, a.z, a.w };
		const glm::vec3 bb = { b.y, b.z, b.w };
		const glm::vec3 cc = a.x * bb + b.x * aa + glm::cross(bb, aa);
		_quaternion = { a.x * b.x - dot(aa, bb), cc.x, cc.y, cc.z };
	}

	std::unique_ptr<glm::mat4> object::scale_mat() const {
		const glm::mat4 result = {
			{_scale, 0, 0, 0},
			{0, _scale, 0, 0},
			{0, 0, _scale, 0},
			{0, 0, 0, 1} };
		return std::make_unique<glm::mat4>(result);
	}

	std::unique_ptr<glm::mat4> object::translation_mat() const {
		const glm::mat4 result = {
			{1, 0, 0, _translation.x},
			{0, 1, 0, _translation.y},
			{0, 0, 1, _translation.z},
			{0, 0, 0, 1} };
		return std::make_unique<glm::mat4>(result);
	}

	std::unique_ptr<glm::mat4> object::orientation_mat() const {
		const float a = _quaternion.x;
		const float x = _quaternion.y;
		const float y = _quaternion.z;
		const float z = _quaternion.w;
		const glm::mat4 result = {
			{a * a + x * x - y * y - z * z, 2 * x * y - 2 * a * z, 2 * x * z + 2 * a * y, 0},
			{2 * x * y + 2 * a * z, a * a - x * x + y * y - z * z, 2 * y * z - 2 * a * x, 0},
			{2 * x * z - 2 * a * y, 2 * y * z + 2 * a * x, a * a - x * x - y * y + z * z, 0},
			{0, 0, 0, 1} };
		return std::make_unique<glm::mat4>(result);
	}

	std::pair<glm::vec3, glm::vec3> object::minmax() const {
		static bool initialized = false;
		static std::pair<glm::vec3, glm::vec3> result;

		if (initialized)
			return result;

		for (const glm::vec3& point : this->points) {
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
		return result;
	}
}
