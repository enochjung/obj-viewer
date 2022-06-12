// obj-viewer - github @enochjung

#define TINYOBJLOADER_IMPLEMENTATION
#include <iostream>
#include <memory>

#include "engine.h"
#include "object.h"

using namespace obj_viewer;

int main(int argc, char** argv) {
	std::string inputfile = "../sample_obj/tree.obj";
	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = "";
	reader_config.triangulate = true;

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(inputfile, reader_config)) {
		if (!reader.Error().empty())
			std::cerr << "TinyObjReader: " << reader.Error();
		exit(1);
	}
	if (!reader.Warning().empty())
		std::cout << "TinyObjReader: " << reader.Warning();

	engine& engine = engine::instance();
	engine.init(&argc, argv, "obj viewer", 800, 800);

	const auto& attrib = reader.GetAttrib();
	const auto& shapes = reader.GetShapes();
	const auto& materials = reader.GetMaterials();
	std::unique_ptr<object> obj = std::make_unique<object>(attrib, shapes, materials);

	engine.add_object(std::move(obj));

	engine.run();
	
	return 0;
}