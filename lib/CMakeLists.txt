execute_process(COMMAND git submodule update --init --recursive)
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

set(BUILD_SHARED_LIBS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_HUNTER_ENABLED OFF)
set(ASSIMP_BUILD_STATIC_LIB ON)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_SAMPLES OFF)
set(ASSIMP_BUILD_ZLIB ON)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_OBJ_IMPORTER ON)
add_subdirectory("assimp")
