// obj-viewer - github @enochjung

#include <iostream>
#include <memory>

#include "engine.h"
#include "OBJ_Loader.h"
#include "object.h"

using namespace obj_viewer;
using namespace obj_loader;

int main(int argc, char** argv) {
	engine& engine = engine::instance();
	engine.init(&argc, argv, "obj viewer", 800, 800);

	Loader obj_loader = Loader();
	if (!obj_loader.LoadFile("../sample_obj/tree.obj")) {
		std::cout << "failed to read obj file.";
		return 1;
	}
	std::unique_ptr<object> fox = std::make_unique<object>(obj_loader.LoadedMeshes);
	engine.add_object(std::move(fox));

	engine.run();

	return 0;
}