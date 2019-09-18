#pragma once
#include <gl/GL.h>
#include <initializer_list>

void printAllErrors(const char * caption = nullptr);
const char * getGLErrorStr(GLenum err);

//Vertex Array Object
class VAO
{
	GLuint m_vao = 0;
	GLuint m_vbo = 0;
	int m_faceCount = 0;

public:
	VAO() = default;
	~VAO();

	bool load(const char *obj_file);
	void unload();
	bool isLoaded() const;

	/*
		다음이 순서대로 1번씩 실행됩니다.
		bind()
		render()
		unbind()
	*/
	void render_once();
	/*
		바인드하고 렌더링합니다.
		언바이드는 하지 않습니다.
	*/
	void bind_render();
	void bind();
	void render();
	static void unbind();

	GLuint getVAO() const;
	GLuint getVBO() const;
};

class Shader
{
	GLuint m_program = 0;

public:
	Shader() = default;
	~Shader();

	bool load(const char *vert_file, const char *frag_File);
	void unload();
	bool isLoaded();

	void use();
	static void unuse();

	GLuint getProgram() const;

private:
	GLuint genShader(GLenum type, const char * file);
};

//Frame buffer object
class FBO
{
	static constexpr int MAX_COLOR_TEXTURE = 15;

	GLuint m_fbo = 0;
	GLuint m_depthTex = 0;
	GLuint m_colorTex[MAX_COLOR_TEXTURE];
	int m_colorTexCount = 0;
	int m_width;
	int m_height;

public:
	FBO() = default;
	~FBO();

	bool create(int width, int height, int colorTextureCount, bool hasDepthTexture = true);
	void destroy();
	bool isCreated();

	/*
		내부에서 viewport를 호출합니다.
	*/
	void bind();
	static void unbind();
	void bindColorTexture(int texture_index = 0, int unit = 0);
	void bindDepthTexture(int unit = 0);
	static void unbindTexture();

	/*
		{} 중괄호를 사용하여 출력을 정할 수 있습니다.
		ex)	GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT1 를 순서대로 출력으로 쓰려면
		setDrawbuffers({0, 4, 1});
	*/
	void setDrawbuffers(const std::initializer_list<GLenum>& buffer_list);

	/*
		만들어진 모든 색상 버퍼를 출력으로 사용합니다. 순서는 만들어진 순서를 따릅니다.
		create() 를 해서 성공할 경우 기본적으로 호출됩니다.
	*/
	void setAllDrawbuffers();
	
	GLuint getFBO() const;
	GLuint getDepthTex() const;
	GLuint getColorTex(int texture_index = 0) const;
	int getWidth() const;
	int getHeight() const;
	int getColorTexCount() const;
};

class Texture
{
	GLuint m_texture;
	int m_width;
	int m_height;

public:
	Texture() = default;
	~Texture();

	bool load(const char *image_file);
	void unload();
	bool isLoaded() const;

	void bind(int unit = 0);
	static void unbind();

	GLuint getTexture() const;
};
