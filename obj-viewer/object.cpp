#include "object.h"

#include <ranges>
#include <numeric>
#include <iostream>

namespace obj_viewer {

	object::object(std::vector<Mesh> meshes) {

		const auto sizes = meshes
			| std::views::transform([](const Mesh& mesh) { return mesh.Indices.size(); });
		const size_t all_points_num = std::accumulate(sizes.begin(), sizes.end(), 0);
		this->points = std::vector<glm::vec3>(all_points_num);

		int points_index = 0;
		for (int i = 0; i < (int)meshes.size(); ++i) {
			Mesh& mesh = meshes[i];

			const auto points = mesh.Indices
				| std::views::transform([&mesh](unsigned int index) { return mesh.Vertices[index].Position; })
				| std::views::transform([](Vector3 position) { return glm::vec3(position.X, position.Y, position.Z); });

			for (glm::vec3 point : points)
				this->points[points_index++] = point;
		}
	}
}
