#pragma once

#include <list>
#include <map>
#include <set>

#include "gl_framebuffer.hpp"
#include "opengl_shader.hpp"
#include "../renderer_interface.hpp"
#include "../renderer_structs.hpp"
#include "../../data/id_table.hpp"
#include "../../data/string.hpp"
#include "gl_buffers.hpp"
#include "../render_view.hpp"
#include "shadow_map.hpp"
#include "gaussian_blur.hpp"

namespace pge
{
    class OpenglRenderer : public IRenderer
    {
    public:
        uint32_t init() override;

        IShader* create_shader(ShaderList shaders) override;

        void create_buffers(Mesh &mesh) override;

        void delete_buffers(Mesh &mesh) override;

        void set_visualize_depth(bool value) override;

        void new_frame() override;

        void end_frame() override;

        void draw(const MeshView&mesh, glm::mat4 model, DrawOptions options = {}) override;

        uint32_t
		create_texture_from_path(std::string_view path, uint32_t &out_texture, TextureOptions options) override;

        uint32_t create_texture(ustring_view data, int width, int height, int channels, uint32_t &out_texture,
			TextureWrapMode wrap_mode,
			bool gamma_correct) override;

        uint32_t create_cubemap_from_path(std::array<std::string_view, 6> faces, uint32_t& out_texture) override;

        Image get_image() override;

        void delete_texture(uint32_t id) override;

        void set_wireframe_mode(bool value) override
        {
            glPolygonMode(GL_FRONT_AND_BACK, value ? GL_LINE : GL_FILL);
            m_wireframe = value;
        }

        void set_clear_color(glm::vec4 value) override
        {
            glClearColor(EXPAND_VEC4(value));
        }

        glm::vec4 get_clear_color() override
        {
            float buffer[4];

            glGetFloatv(GL_COLOR_CLEAR_VALUE, buffer);

            return glm::make_vec4(buffer);
        }

        void set_offline(bool value) override
        {
            m_is_offline = value;
        }

        void wait() override
        {
            glFinish();
        }

        IFramebuffer* get_framebuffer() override
        {
            return &m_out_buffer;
        }

		IFramebuffer* get_render_framebuffer() override
        {
            return &m_render_buffer;
        }

		RenderView* add_view(Camera *camera) override;

		void remove_view(RenderView *view) override;

        void set_skybox(uint32_t id) override
        {
            m_skybox_texture = id;
        }

        std::string_view error_message(uint32_t code) override
        {
            return opengl_error_message((OpenGlErrorCode)code);
        }

		void set_gamma(float value) override
		{
			m_screen_shader.use();
			m_screen_shader.set("gamma", value);
		}

        RendererProperties properties() override;

		void set_shadow_settings(pge::ShadowSettings settings) override;

		ShadowSettings get_shadow_settings() override
		{
			return m_shadow_settings;
		}

		void set_screen_space_settings(pge::ScreenSpaceSettings settings) override;

		ScreenSpaceSettings get_color_settings() override
		{
			return m_screen_space_settings;
		}

		RenderStats get_stats() override
		{
			return m_stats;
		}

    private:
        // the default missing texture to use when unable to create a texture
        uint32_t m_missing_texture;
        // the texture id for the skybox
        uint32_t m_skybox_texture = UINT32_MAX;
        // the opengl buffers every mesh needs to be drawn.
        IdTable<GlBuffers> m_buffers;
        // the base shader
        GlShader m_lighting_shader;
        // a shader used for outlining
        GlShader m_outline_shader;
        GlShader m_screen_shader;
        GlShader m_skybox_shader;
		GlShader m_shadow_map_shader;
        // the screen plane where framebuffer textures are drawn to
        GlBuffers m_screen_plane;
        // the cube that will be used to draw the skybox
        GlBuffers m_skybox_cube;
        // possibly transparent meshes that are sorted by their distance from the camera
        std::multimap<float, DrawData> m_sorted_meshes;
        // meshes that are queued for drawing in all render passes
        std::vector<DrawData> m_render_queue;
        // the shaders that will be used for different render passes such as post processing stuff
        std::list<GlShader> m_shaders;
        // the queue for the buffers that are supposed to be deleted
        std::vector<uint32_t> m_delete_queue;
        // list of cameras to render to. these cameras will always render to the attached framebuffer
        RenderViewList m_render_views;
		// the buffer all the meshes will get rendered to
		GlFramebuffer m_render_buffer;

        // the screen buffer used for screen space rendering
        GlFramebuffer m_screen_buffer;
        // the buffer for the render output if offline renders are enabled
        GlFramebuffer m_out_buffer;
        bool m_is_offline = false;
        bool m_wireframe = false;
		IdTable<GlFramebuffer> m_shadow_maps;
		//int shadow_map_texture = GL_TEXTURE4;
		int sampler_start = 4;
		GaussianBlur m_gaussian_blur;

		ShadowSettings m_shadow_settings;
		ScreenSpaceSettings m_screen_space_settings;
		RenderStats m_stats;

        void draw_shaded_wireframe(const Mesh &mesh, glm::mat4 model);

        void handle_lighting();

		void draw_mesh(const MeshView &mesh);

        uint32_t handle_draw(const DrawData&data);

        void draw_passes();

        void draw_everything(bool calculate_shadows);

        void clear_buffers();

        void draw_outline(const DrawData &data);

        void handle_gl_buffer_delete();

        void set_base_uniforms(const DrawData &data);

        void create_screen_plane();

        void create_skybox_cube();

		void apply_bloom_blur();

		void draw_quad(GlBuffers &buffers);

        void draw_screen_plane();

        void draw_skybox();

		void render_to_framebuffer(pge::GlFramebuffer &fb);

		void render_to_shadow_map(IFramebuffer *fb, glm::vec3 position);
    };
}
