#ifndef OPAQUE_H
#define OPAQUE_H

#include <renderObjects/compositing.h>


namespace BALL
{
	class System;
}

class Opaque
	: public Compositing
{
		Q_OBJECT
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		/**
		 * @brief Default constructor
		 * The renderer needs to be initialized before usage by calling init().
		 * @param camera The camera of the renderer.
		 *
		 **/
		Opaque(Camera* camera);

		virtual void prepare_rendering(RenderLevel level = RenderLevel::OPAQUE);

		/**
		 * @brief Starting point for rendering.
		 * This method should be called whenever the window needs to be redrawn. It starts the rendering
		 * process for the spheres.
		 * The opaque shader program is used.
		 *
		 * @return void
		 **/
		void render(RenderLevel level = RenderLevel::OPAQUE);

		/**
		 * @brief Setup the render targets.
		 *
		 * The textures for diffuse and specular color, the normal texture and the depth texture are
		 * setup (2D textures). All textures use RGBA and linear interpolation.
		 *
		 * The renderbuffer for the z-test is setup.
		 *
		 * The textures are attached to the framebuffer. If an error occurs during the attachment an
		 * error message is printed.
		 *
		 * The diffuse, specular, normal, and depth texture are used for drawing. The binding of the
		 * framebuffer is broken.
		 *
		 * @param w The width of the current render context.
		 * @param h The height of the current render context.
		 * @return void
		 **/
		void setupRenderTargets(unsigned int w, unsigned int h);

		GLuint getNormalTexture();

		GLuint getDepthTexture();

	public Q_SLOTS:

		virtual void setViewport(unsigned int w, unsigned int h, unsigned int x=0, unsigned int y=0 );

	protected:

		virtual void register_();

		virtual void unregister_();

		virtual void compileShaderPrograms_();

		virtual void initShaderPrograms_();

		/**
		 * @brief Initializes the system.
		 *
		 * Only initializes the system when the initialized_ flag is not already set to true.
		 * It performs the OpenGL generation calls for nedded buffers (vertex and index buffer objects).
		 * The initialized_ flag is set to true.
		 *
		 * @return void
		 **/
		void initialize_();

		/**
		 * @brief Performs an cleanup.
		 *
		 * Delets buffers.
		 * After calling cleanup_() the initialized_ flag is set to false.
		 *
		 * @return void
		 **/
		void cleanup_();

		/**
		 * @brief Render the spheres.
		 *
		 * The vertex attribut arrays (discribing the shap of the object) are enabled to draw the
		 * triangles and are disabled after that.
		 *
		 * @return void
		 **/
		void renderOpaque_();

		/**
		 * @brief Set the uniforms.
		 *
		 * The lightsource uniforms are updated and the locations for the textures are setup.
		 *
		 * @return void
		 **/
		void setOpaqueUniforms();

		/**
		 * @brief Sets up the opaque.
		 *
		 * @return void
		 **/
		virtual void initVBO_();

		virtual void initVBO_ ( std::list< const BALL::VIEW::GeometricObject* >& /*list*/ ) {};

		/**
		 * @brief Array of texture names generated by glGenTextures().
		 *
		 * The textures are identified as follows:
		 * 0: ambient
		 * 1: diffuse
		 * 2: specular + shininess
		 * 3: normal
		 * 4: depth
		 **/
		GLuint rendertarget_textures_[5];

		/**
		 * @brief Framebuffer object names (ID) generated by glGenFramebuffers().
		 **/
		GLuint rendertarget_fboid_;

		/**
		 * @brief Renderbuffer object names generated by glGenRenderbuffers().
		 **/
		GLuint renderbuffer_depth_;

		/**
		 * @brief Location of the uniform variable "quad_position".
		 *
		 * Needed to pass data to the shader program.
		 **/
		GLuint quad_position_idx;

		/**
		 * @brief Location of the uniform variable "quad_texcoord".
		 *
		 * Needed to pass data to the shader program.
		 **/
		GLuint quad_texcoord_idx;

};

#endif //OPAQUE_H