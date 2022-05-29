// OBJ_Loader.h - A Single Header OBJ Model Loader
// by @Bly7 - https://github.com/Bly7/OBJ-Loader
// customized by @enochjung

#pragma once

#include <vector>
#include <string>
//#define OBJL_CONSOLE_OUTPUT

namespace obj_viewer {

	struct Vector2 {
		float X;
		float Y;

		Vector2();
		Vector2(float X_, float Y_);
		bool operator==(const Vector2& other) const;
		bool operator!=(const Vector2& other) const;
		Vector2 operator+(const Vector2& right) const;
		Vector2 operator-(const Vector2& right) const;
		Vector2 operator*(const float& other) const;
	};

	struct Vector3 {
		float X;
		float Y;
		float Z;

		Vector3();
		Vector3(float X_, float Y_, float Z_);
		bool operator==(const Vector3& other) const;
		bool operator!=(const Vector3& other) const;
		Vector3 operator+(const Vector3& right) const;
		Vector3 operator-(const Vector3& right) const;
		Vector3 operator*(const float& other) const;
		Vector3 operator/(const float& other) const;
	};

	struct Vertex {
		Vector3 Position;
		Vector3 Normal;
		Vector2 TextureCoordinate;
	};

	struct Material {
		// Material Name
		std::string name;
		// Ambient Color
		Vector3 Ka;
		// Diffuse Color
		Vector3 Kd;
		// Specular Color
		Vector3 Ks;
		// Specular Exponent
		float Ns;
		// Optical Density
		float Ni;
		// Dissolve
		float d;
		// Illumination
		int illum;
		// Ambient Texture Map
		std::string map_Ka;
		// Diffuse Texture Map
		std::string map_Kd;
		// Specular Texture Map
		std::string map_Ks;
		// Specular Hightlight Map
		std::string map_Ns;
		// Alpha Texture Map
		std::string map_d;
		// Bump Map
		std::string map_bump;

		Material();
	};

	struct Mesh {
		// Mesh Name
		std::string MeshName;
		// Vertex List
		std::vector<Vertex> Vertices;
		// Index List
		std::vector<unsigned int> Indices;
		// Material
		Material MeshMaterial;

		Mesh();
		Mesh(std::vector<Vertex>& _Vertices, std::vector<unsigned int>& _Indices);
	};

	namespace math {
		Vector3 CrossV3(const Vector3 a, const Vector3 b);
		float MagnitudeV3(const Vector3 in);
		float DotV3(const Vector3 a, const Vector3 b);
		float AngleBetweenV3(const Vector3 a, const Vector3 b);
		Vector3 ProjV3(const Vector3 a, const Vector3 b);
	}

	namespace algorithm {
		Vector3 operator*(const float& left, const Vector3& right);
		bool SameSide(Vector3 p1, Vector3 p2, Vector3 a, Vector3 b);
		Vector3 GenTriNormal(Vector3 t1, Vector3 t2, Vector3 t3);
		bool inTriangle(Vector3 point, Vector3 tri1, Vector3 tri2, Vector3 tri3);
		inline void split(const std::string& in, std::vector<std::string>& out, std::string token);
		inline std::string tail(const std::string& in);
		inline std::string firstToken(const std::string& in);
		template <class T>
		inline const T& getElement(const std::vector<T>& elements, std::string& index);
	}

	class Loader {
	public:
		// Loaded Mesh Objects
		std::vector<Mesh> LoadedMeshes;
		// Loaded Vertex Objects
		std::vector<Vertex> LoadedVertices;
		// Loaded Index Positions
		std::vector<unsigned int> LoadedIndices;
		// Loaded Material Objects
		std::vector<Material> LoadedMaterials;

		Loader();
		~Loader();
		bool LoadFile(std::string Path);

	private:
		void GenVerticesFromRawOBJ(
			std::vector<Vertex>& oVerts,
			const std::vector<Vector3>& iPositions,
			const std::vector<Vector2>& iTCoords,
			const std::vector<Vector3>& iNormals,
			std::string icurline);
		void VertexTriangluation(
			std::vector<unsigned int>& oIndices,
			const std::vector<Vertex>& iVerts);
		bool LoadMaterials(std::string path);
	};
}
