#pragma once
#include "GLObject.h"
#include <glm/gtx/transform.hpp>

/*
	Order Independent Transparency Renderer

	ex) How to use

	OITRenderer * oit = new OITRenderer();

	oit->begin();
	renderScene();

	for(int i = 0; i < N; i++) { 
		oit->swap(); 
		renderScene(); 
	}

	oit->end();

	GLuint result_texture = oit->getOITTexture();

	..

	delete oit;
*/
class OITRenderer
{
	enum ShaderLocation
	{
		// peel shader
		SL_pmat = 0, // projection matrix
		SL_vmat = 1, // view matrix
		SL_mmat = 2, // model matrix
		SL_inv_screen_size = 3, // inversed screen size
		SL_alpha = 4, // alpha for object
		SL_color = 5, // color for object
	};

	int m_width;
	int m_height;
	int m_curr; // current fbo index
	int m_prev; // previous fbo index
	FBO m_peelFBO[2];
	Shader m_peelShader;
	FBO m_blendFBO;
	Shader m_blendShader;

public:
	OITRenderer(int width, int height)
	{
		m_width = width;
		m_height = height;

		m_peelFBO[0].create(m_width, m_height, 1, true);
		m_peelFBO[1].create(m_width, m_height, 1, true);
		m_peelShader.load("resources/shaders/peel.vert", "resources/shaders/peel.frag");
		m_blendFBO.create(m_width, m_height, 1, false);
		m_blendShader.load("resources/shaders/blend.vert", "resources/shaders/blend.frag");
	}

	~OITRenderer()
	{
		m_peelFBO[0].destroy();
		m_peelFBO[1].destroy();
		m_peelShader.unload();
		m_blendFBO.destroy();
		m_blendShader.unload();
	}

	/*
		Call these.
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	*/
	void begin()
	{
		// clear blend fbo.
		glDisable(GL_DEPTH_TEST);
		m_blendFBO.bind();
		glClear(GL_COLOR_BUFFER_BIT);

		// enable depth test, set depth test function and blend funcion.
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD); // default
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_curr = 0;
		m_prev = 1;

		// set zero to previous depth buffer and clear it.
		glClearDepth(0.f);
		m_peelFBO[m_prev].bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// set one to current depth buffer and clear it.
		glClearDepth(1.f);
		m_peelFBO[m_curr].bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// use peeling shader.
		m_peelShader.use();
		setDefaultUniforms();

		// bind previous depth texture.
		m_peelFBO[m_prev].bindDepthTexture();
	}

	void swap()
	{
		// render current peeling color buffer to blending fbo.
		blend();

		// swap index each peeling fbo.
		m_prev = m_curr;
		m_curr ^= 1;

		// clear current peeling fbo and bind previous peeling depth buffer.
		m_peelFBO[m_curr].bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_peelShader.use();
		m_peelFBO[m_prev].bindDepthTexture();
	}

	void end()
	{
		// render current peeling color buffer to blending fbo.
		blend();

		// relase current fbo, shader, and texture.
		FBO::unbind();
		Shader::unuse();
		Texture::unbind();
	}

	// Shader Config
	void setPojectionMatrix(const float* value) {
		glUniformMatrix4fv(SL_pmat, 1, GL_FALSE, value);
	}
	void setViewMatrix(const float* value) {
		glUniformMatrix4fv(SL_vmat, 1, GL_FALSE, value);
	}
	void setModelMatrix(const float* value) {
		glUniformMatrix4fv(SL_mmat, 1, GL_FALSE, value);
	}
	void setAlpha(float value) {
		glUniform1f(SL_alpha, value);
	}
	void setColor(float r, float g, float b) {
		glUniform3f(SL_color, r, g, b);
	}

	// Result
	GLuint getOITTexture() const {
		return m_blendFBO.getColorTex();
	}
	/*
		ex) bind texture to unit0(GL_TEXTURE0)
		.bindResultTexture(0);
	*/
	void bindOITTexture(int unit = 0) {
		glActiveTexture(unit + GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, getOITTexture());
	}

	// For Debug
	FBO& getPeelFBO(int fbo_index) {
		return m_peelFBO[fbo_index];
	}
	FBO& getBlendFBO() {
		return m_blendFBO;
	}

private:
	void setDefaultUniforms() {
		auto pmat = glm::ortho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
		setPojectionMatrix(&pmat[0][0]);
		
		auto vmat = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1),
			glm::vec3(0, 1, 0));
		setViewMatrix(&vmat[0][0]);
		
		auto mmat = glm::mat4(1.f);
		setModelMatrix(&mmat[0][0]);

		glUniform2f(SL_inv_screen_size, 1.f/m_width, 1.f/m_height);

		setAlpha(0.2f);

		setColor(1, 1, 1);
	}

	void blend() {
		glDisable(GL_DEPTH_TEST);

		m_blendFBO.bind();
		m_blendShader.use();

		auto mat = glm::mat4(1.f);
		glUniformMatrix4fv(0, 1, GL_FALSE, &mat[0][0]);

		m_peelFBO[m_curr].bindColorTexture();

		// enable blend and render 
		glEnable(GL_BLEND);
		glDrawArrays(GL_QUADS, 0, 4);
		glDisable(GL_BLEND);

		glEnable(GL_DEPTH_TEST);
	}
};