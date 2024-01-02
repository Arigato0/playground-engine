#pragma once
#include <filesystem>
#include <optional>

#include "../graphics/model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace pge
{

    // values taken from assimp
    enum MODEL_PP
    {
        MODEL_PP_CalcTangentSpace = 0x1,
        MODEL_PP_JoinIdenticalVertices = 0x2,
        MODEL_PP_MakeLeftHanded = 0x4,
        MODEL_PP_Triangulate = 0x8,
        MODEL_PP_RemoveComponent = 0x10,
        MODEL_PP_GenNormals = 0x20,
        MODEL_PP_GenSmoothNormals = 0x40,
        MODEL_PP_SplitLargeMeshes = 0x80,
        MODEL_PP_PreTransformVertices = 0x100,
        MODEL_PP_LimitBoneWeights = 0x200,
        MODEL_PP_ValidateDataStructure = 0x400,
        MODEL_PP_ImproveCacheLocality = 0x800,
        MODEL_PP_RemoveRedundantMaterials = 0x1000,
        MODEL_PP_FixInfacingNormals = 0x2000,
        MODEL_PP_PopulateArmatureData = 0x4000,
        MODEL_PP_SortByPType = 0x8000,
        MODEL_PP_FindDegenerates = 0x10000,
        MODEL_PP_FindInvalidData = 0x20000,
        MODEL_PP_GenUVCoords = 0x40000,
        MODEL_PP_TransformUVCoords = 0x80000,
        MODEL_PP_FindInstances = 0x100000,
        MODEL_PP_OptimizeMeshes  = 0x200000,
        MODEL_PP_OptimizeGraph  = 0x400000,
        MODEL_PP_FlipUVs = 0x800000,
        MODEL_PP_FlipWindingOrder = 0x1000000,
        MODEL_PP_SplitByBoneCount = 0x2000000,
        MODEL_PP_Debone = 0x4000000,
        MODEL_PP_GlobalScale = 0x8000000,
        MODEL_PP_EmbedTextures  = 0x10000000,
        MODEL_PP_ForceGenNormals = 0x20000000,
        MODEL_PP_DropNormals = 0x40000000,
        MODEL_PP_GenBoundingBoxes = 0x80000000
    };

#define DEFAULT_MODEL_PP_FLAGS MODEL_PP_Triangulate | \
    MODEL_PP_JoinIdenticalVertices | MODEL_PP_OptimizeGraph | \
    MODEL_PP_OptimizeMeshes | MODEL_PP_GenUVCoords \

    class ModelLoader
    {
    public:
        std::optional<Model> load(std::string_view path, int flags = DEFAULT_MODEL_PP_FLAGS);

    private:
        std::filesystem::path m_path;

        glm::mat4 process_node(Model &model, aiNode *node, const aiScene *scene);

        std::vector<Vertex> load_mesh_vertices(aiMesh *mesh);

        std::vector<uint32_t> load_mesh_indices(aiMesh *mesh);

        Mesh process_mesh(aiMesh *mesh, const aiScene *scene);

        Texture load_material(aiMaterial *material, aiTextureType type);

		pge::Material load_mesh_material(const aiMesh *mesh, const aiScene *scene);
	};
}
