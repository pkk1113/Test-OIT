#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
#include <cstdio>

#include "OITRenderer.h"

int g_width = 1000;
int g_height = 800;
float g_aspect = (float)g_width / (float)g_height;

int g_peelCount = 1;
bool g_pause = false;

void initContext(bool useDefault, int major = 3, int minor = 3, bool useCompatibility = false);
void framebufferSizeCallback(GLFWwindow*, int w, int h);
void mousebuttonCallback(GLFWwindow*, int btn, int act, int);

int main()
{
	/* 초기화, 에러 핸들링 등록, 이벤트 콜백 등록, OpenGL 초기화 */
	/* -------------------------------------------------------------------------------------- */
	glfwInit();
	glfwSetErrorCallback([](int err, const char* desc) { puts(desc); });
	initContext(/*use dafault = */ true);
	GLFWwindow *window = glfwCreateWindow(g_width, g_height, "Order Independent Transparency Rendering!", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetMouseButtonCallback(window, mousebuttonCallback);
	glfwMakeContextCurrent(window);
	glewInit();

	/* 객체 생성 및 초기화 */
	/* -------------------------------------------------------------------------------------- */
	OITRenderer * oit_renderer = new OITRenderer(1024, 1024);
	VAO vao;				vao.load("resources/objects/stanford_bunny_scaled.obj");
	Shader quadShader;		quadShader.load("resources/shaders/quad.vert", "resources/shaders/quad.frag");
	Texture helpTexture;	helpTexture.load("resources/textures/help.png");

	/* 객체 생성 및 초기화 검사 */
	/* -------------------------------------------------------------------------------------- */
	printAllErrors("객체 생성 및 초기화 검사");

	/* 메인 루프 */
	/* -------------------------------------------------------------------------------------- */
	while (!glfwWindowShouldClose(window))
	{
		if (g_pause) {
			// 이벤트 폴
			glfwPollEvents();
			continue;
		}

		glClearColor(0, 0, 0, 0);

		// OIT
		auto renderScene = [&]() {
			// 배경
			glm::mat4 mmat;
			vao.bind();

			// 물체 1
			mmat = glm::mat4(1);
			mmat *= glm::rotate((float)glfwGetTime(), glm::vec3(0.f, 1.f, 0.f));
			mmat *= glm::translate(glm::vec3(0.4f, -0.8f, 0.f));
			mmat *= glm::scale(glm::vec3(2.f, 2.f, 2.f));
			oit_renderer->setModelMatrix(&mmat[0][0]);
			oit_renderer->setAlpha(0.8f);
			vao.render();

			// 물체 2
			mmat = glm::mat4(1);
			mmat *= glm::rotate((float)glfwGetTime(), glm::vec3(0.f, 1.f, 0.f));
			mmat *= glm::translate(glm::vec3(-0.4f, -0.8f, 0.f));
			mmat *= glm::scale(glm::vec3(2.f, 2.f, 2.f));
			oit_renderer->setModelMatrix(&mmat[0][0]);
			oit_renderer->setAlpha(0.8f);
			vao.render();


			vao.unbind();
		};

		oit_renderer->begin();
		renderScene();

		for (int i = 0; i < g_peelCount; i++) {
			oit_renderer->swap();

			if(i%3 == 0) oit_renderer->setColor(1, 0, 0);
			else if (i % 3 == 1) oit_renderer->setColor(0, 1, 0);
			else if (i % 3 == 2) oit_renderer->setColor(0, 0, 1);

			oit_renderer->setAlpha(0.1f);

			renderScene();
		}

		oit_renderer->end();

		// 일반 렌더링
		glViewport(0, 0, g_width, g_height);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glClear(GL_COLOR_BUFFER_BIT);
		quadShader.use();
		glm::mat4 mat;
		{
			constexpr int row = 2;
			constexpr int col = 3;
			constexpr float w = 1.f / col;
			constexpr float h = 1.f / row;
			constexpr float offset_x = 1.f / col - 1.f;
			constexpr float offset_y = 1.f - 1.f / row;

			auto setPos = [&](int r, int c) {
				float pos_x = offset_x + 2.f * w * c;
				float pos_y = offset_y - 2.f * h * r;

				mat = glm::translate(glm::vec3(pos_x, pos_y, 0.f)) *
					glm::scale(glm::vec3(w, h, 1.f));

				glUniformMatrix4fv(0, 1, GL_FALSE, &mat[0][0]);
			};

			// A 버퍼 색
			setPos(0, 0);
			oit_renderer->getPeelFBO(0).bindColorTexture(0);
			glDrawArrays(GL_QUADS, 0, 4);

			// A 버퍼 깊이
			setPos(1, 0);
			oit_renderer->getPeelFBO(0).bindDepthTexture(0);
			glDrawArrays(GL_QUADS, 0, 4);

			// B 버퍼 색
			setPos(0, 1);
			oit_renderer->getPeelFBO(1).bindColorTexture(0);
			glDrawArrays(GL_QUADS, 0, 4);

			// B 버퍼 깊이
			setPos(1, 1);
			oit_renderer->getPeelFBO(1).bindDepthTexture(0);
			glDrawArrays(GL_QUADS, 0, 4);

			// 결과 색
			setPos(0, 2);
			oit_renderer->bindOITTexture();
			glDrawArrays(GL_QUADS, 0, 4);

			// 설명
			setPos(1, 2);
			helpTexture.bind();
			glDrawArrays(GL_QUADS, 0, 4);
		}
		quadShader.unuse();

		// 버퍼 스왑, 이벤트 폴
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	/* 루프 종료 검사 */
	/* -------------------------------------------------------------------------------------- */
	printAllErrors("루프 종료 검사");

	/* 객체 제거 */
	/* -------------------------------------------------------------------------------------- */
	delete oit_renderer;
	vao.unload();
	quadShader.unload();
	helpTexture.unload();

	/* 객체 제거 검사 */
	/* -------------------------------------------------------------------------------------- */
	printAllErrors("객체 제거 검사");

	/* 종료 */
	/* -------------------------------------------------------------------------------------- */
	glfwTerminate();
}

void initContext(bool useDefault, int major, int minor, bool useCompatibility)
{
	if (useDefault)
	{
		// 기본적으로  Default로 설정해주면
		// 가능한 최신의 OpenGL Vesion을 선택하며 (연구실 컴퓨터는 4.5)
		// Profile은 Legacy Function도 사용할 수 있게 해줍니다.(Compatibility 사용)
		glfwDefaultWindowHints();
	}
	else
	{
		// Major 와 Minor 결정
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);

		// Core 또는 Compatibility 선택
		if (useCompatibility)
		{
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
		}
		else
		{
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		}
	}
}

void framebufferSizeCallback(GLFWwindow*, int w, int h)
{
	g_width = w;
	g_height = h;
	g_aspect = (float)g_width / (float)g_height;
	printf("%d, %d\n", w, h);

	g_pause = false;
	puts("start !");
}

void mousebuttonCallback(GLFWwindow*, int button, int action, int mods)
{
	if (action == GLFW_PRESS) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			g_peelCount++;

			printf("peeling count: %d\n", g_peelCount);
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			g_peelCount--;

			if (g_peelCount < 0)
				g_peelCount = 0;

			printf("peeling count: %d\n", g_peelCount);

		}
		else {
			g_pause = !g_pause;

			puts(g_pause ? "pause !" : "start !");
		}

	}
}
