#include "tk_platform.h"

void opengl_init(struct api api)
{
    s32 version_major = 0;
    s32 version_minor = 0;
    s32 uniform_blocks_max_vertex = 0;
    s32 uniform_blocks_max_geometry = 0;
    s32 uniform_blocks_max_fragment = 0;
    s32 uniform_blocks_max_combined = 0;
    s32 uniform_buffer_max_bindings = 0;
    s32 uniform_block_max_size = 0;
    s32 vertex_attribs_max = 0;

    api.gl.glGetIntegerv(GL_MAJOR_VERSION, &version_major);
    api.gl.glGetIntegerv(GL_MINOR_VERSION, &version_minor);
    api.gl.glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS,
        &uniform_blocks_max_vertex);
    api.gl.glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS,
        &uniform_blocks_max_geometry);
    api.gl.glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS,
        &uniform_blocks_max_fragment);
    api.gl.glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS,
        &uniform_blocks_max_combined);
    api.gl.glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS,
        &uniform_buffer_max_bindings);
    api.gl.glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE,
        &uniform_block_max_size);
    api.gl.glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &vertex_attribs_max);

    api.log("OpenGL %i.%i\n", version_major, version_minor);
    api.log("Uniform blocks max vertex: %d\n", uniform_blocks_max_vertex);
    api.log("Uniform blocks max gemoetry: %d\n", uniform_blocks_max_geometry);
    api.log("Uniform blocks max fragment: %d\n", uniform_blocks_max_fragment);
    api.log("Uniform blocks max combined: %d\n", uniform_blocks_max_combined);
    api.log("Uniform buffer max bindings: %d\n", uniform_buffer_max_bindings);
    api.log("Uniform block max size: %d\n", uniform_block_max_size);
    api.log("Vertex attribs max: %d\n", vertex_attribs_max);

    api.gl.glEnable(GL_DEPTH_TEST);
    api.gl.glEnable(GL_BLEND);
    api.gl.glEnable(GL_CULL_FACE);
    api.gl.glDepthFunc(GL_LESS);
    api.gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    api.gl.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}
