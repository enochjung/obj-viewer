// obj-viewer - github @enochjung

#define TINYOBJLOADER_IMPLEMENTATION
#include <iostream>
#include <memory>

#include "engine.h"
#include "object.h"

using namespace obj_viewer;

std::unique_ptr<object> read_obj(const std::string& file_directory) {
	const std::size_t found = file_directory.find_last_of("/\\");
	const std::string path = file_directory.substr(0, found + 1);
	const std::string file = file_directory.substr(found + 1);
	std::cout << "path:" << path << ", file:" << file << '\n';

	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = "";
	reader_config.triangulate = true;

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(file_directory, reader_config)) {
		if (!reader.Error().empty())
			std::cerr << "TinyObjReader: " << reader.Error();
		exit(1);
	}
	if (!reader.Warning().empty())
		std::cout << "TinyObjReader: " << reader.Warning();

	const auto& attrib = reader.GetAttrib();
	const auto& shapes = reader.GetShapes();
	const auto& materials = reader.GetMaterials();
	return std::make_unique<object>(attrib, shapes, materials, path);
}

int main(int argc, char** argv) {
	for (int i = 1; i < argc; ++i)
		std::cout << "argc[" << i << "] : " << argv[i] << '\n';

	engine& engine = engine::instance();

	if (argc == 1) {
		std::string obj_directory;

		std::cout << "input your .obj file directory : ";
		std::cin >> obj_directory;

		engine.init(&argc, argv, "obj viewer", 800, 800);
		engine.add_object(std::move(read_obj(obj_directory)));
	}
	else {
		engine.init(&argc, argv, "obj viewer", 800, 800);
		for (int i = 1; i < argc; ++i) {
			const std::string obj_directory = argv[i];
			std::unique_ptr<object> obj = read_obj(obj_directory);
			const float dx = (i - 1) * 1.0f - (argc - 2) * 0.5f;
			obj->move(glm::vec3(dx, 0, 0));
			engine.add_object(std::move(obj));
		}
	}

	engine.run();

	return 0;
}