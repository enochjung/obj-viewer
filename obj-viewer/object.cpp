#include "object.h"

#include <ranges>
#include <numeric>
#include <algorithm>

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
	}

	std::unique_ptr<glm::mat4> object::centralize() const {
		const auto minmax = this->minmax();

		const float dx = (minmax.first.x + minmax.second.x) * -0.5f;
		const float dy = (minmax.first.y + minmax.second.y) * -0.5f;
		const float dz = (minmax.first.z + minmax.second.z) * -0.5f;

		const glm::mat4 result = {
			{1, 0, 0, dx},
			{0, 1, 0, dy},
			{0, 0, 1, dz},
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
