#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "graphics_math.h"
#include "sphere_scene.h"

using namespace glm;

// -------------------------------------------------
// Global Variables
// -------------------------------------------------
int Width = 512;
int Height = 512;
std::vector<float> OutputImage;
// -------------------------------------------------

// 화면 픽셀 색 저장 구조체 (RGB 8비트)
struct Color { uint8_t r, g, b; };
// 재질·광원 파라미터
static const Vec3 ka{ 0,1,0 }, kd{ 0,0.5f,0 }, ks{ 0.5f,0.5f,0.5f };
static const float shininess = 32.0f, Ia = 0.2f;
static const Vec3 lightPos{ -4,4,-3 }, camPos{ 0,0,0 };

std::vector<Color> rasterizeScene(int NX, int NY) {
	// 1) 구 메시 생성
	create_scene();

	// 2) 화면·깊이 버퍼 준비
	std::vector<Color>  frameBuffer(NX * NY, { 0,0,0 });
	std::vector<float>  depthBuffer(NX * NY, std::numeric_limits<float>::infinity());

	// 3) 변환 행렬 세팅
	Mat4 M = mul(Translate(0, 0, -7), Scale(2, 2, 2));
	Mat4 V = Identity();
	Mat4 P = MakePerspective(-0.1f, 0.1f, -0.1f, 0.1f, -0.1f, -1000.0f);
	Mat4 W = MakeViewport(NX, NY);

	// 4) 월드 -> 스크린 좌표 변환
	auto transform_vertex = [&](const Vec3& v) {
		Vec4 v4{ v.x,v.y,v.z,1.0f };
		Vec4 clip = mul(P, mul(V, mul(M, v4)));
		Vec4 ndc{ clip.x / clip.w, clip.y / clip.w, clip.z / clip.w, 1.0f };
		return mul(W, ndc);
	};

	int drawn = 0;
	// 5) 삼각형 래스터화
	for (int tri = 0; tri < gNumTriangles; ++tri) {
		int i0 = gIndexBuffer[3 * tri + 0],
			i1 = gIndexBuffer[3 * tri + 1],
			i2 = gIndexBuffer[3 * tri + 2];
		Vec4 v0 = transform_vertex(gVertexArray[i0]);
		Vec4 v1 = transform_vertex(gVertexArray[i1]);
		Vec4 v2 = transform_vertex(gVertexArray[i2]);

		// back-face culling
		float crossZ = (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
		if (crossZ <= 0.0f) continue;

		// 1) 삼각형 Normal을 계산하기 위해 월드 공간 좌표로 변환
		auto to_world = [&](const Vec3& v) {
			Vec4 v4{ v.x,v.y,v.z,1.0f };
			Vec4 w4 = mul(M, v4);
			return Vec3{ w4.x, w4.y, w4.z };
			};
		Vec3 p0 = to_world(gVertexArray[i0]);
		Vec3 p1 = to_world(gVertexArray[i1]);
		Vec3 p2 = to_world(gVertexArray[i2]);


		// 2) 삼각형면 Normal 계산 및 카메라 방향과 일치 여부 확인
		Vec3 e1 = p1 - p0;
		Vec3 e2 = p2 - p0;
		Vec3 N = normalize(cross(e1, e2));
		//printf("Normal: (%.2f, %.2f, %.2f)\n", N.x, N.y, N.z);
		Vec3 centroid = (p0 + p1 + p2) * (1.0f / 3.0f);
		Vec3 viewDir = normalize(camPos - centroid);
		if (dot(N, viewDir) < 0) {
			N = N * -1.0f; // 카메라 반대 방향 노말 뒤집기
		}

		// 3) 조명 계산 (Blinn-Phong)
		Vec3 Vv = normalize(camPos - centroid);
		Vec3 L = normalize(lightPos - centroid);
		float diff = std::max(0.0f, dot(N, L));
		Vec3 H = normalize(L + Vv);
		float spec = std::pow(std::max(0.0f, dot(N, H)), shininess);

		Vec3 col = ka * Ia + kd * diff + ks * spec;

		// 4) 감마 보정 γ=2.2
		col.x = std::pow(col.x, 1.0f / 2.2f);
		col.y = std::pow(col.y, 1.0f / 2.2f);
		col.z = std::pow(col.z, 1.0f / 2.2f);

		// 5) 8bit 컬러로 변환
		Color faceColor = {
		  uint8_t(std::min(1.0f, std::max(0.0f, col.x)) * 255),
		  uint8_t(std::min(1.0f, std::max(0.0f, col.y)) * 255),
		  uint8_t(std::min(1.0f, std::max(0.0f, col.z)) * 255)
		};

		// 6) Bounding box 계산
		int x0i = int(v0.x + 0.5f), x1i = int(v1.x + 0.5f), x2i = int(v2.x + 0.5f);
		int y0i = int(v0.y + 0.5f), y1i = int(v1.y + 0.5f), y2i = int(v2.y + 0.5f);

		int xmin = std::max(0, std::min({ x0i, x1i, x2i }));
		int xmax = std::min(NX - 1, std::max({ x0i, x1i, x2i }));
		int ymin = std::max(0, std::min({ y0i, y1i, y2i }));
		int ymax = std::min(NY - 1, std::max({ y0i, y1i, y2i }));

		// 7) Barycentric, depth 검사
		for (int y = ymin; y <= ymax; ++y) {
			for (int x = xmin; x <= xmax; ++x) {
				float w0, w1, w2;
				barycentric(v0, v1, v2, x + 0.5f, y + 0.5f, w0, w1, w2);
				if (w0 < 0 || w1 < 0 || w2 < 0) continue;
				float z = w0 * v0.z + w1 * v1.z + w2 * v2.z;
				int idx = y * NX + x;
				if (z < depthBuffer[idx]) {
					depthBuffer[idx] = z;
					frameBuffer[idx] = faceColor;
					++drawn;
				}
			}
		}
	}

	printf("Total pixels drawn: %d\n", drawn);
	return frameBuffer;
}

void render()
{
	// 1) frameBuffer 생성
	std::vector<Color> frameBuffer = rasterizeScene(Width, Height);

	// 2) OutputImage.clear() & reserve
	OutputImage.clear();
	OutputImage.reserve(Width * Height * 3);

	// 3) frameBuffer → [0,1] float RGB 채우기
	for (int j = 0; j < Height; ++j) {
		for (int i = 0; i < Width; ++i) {
			const Color& c = frameBuffer[j * Width + i];
			OutputImage.push_back(c.r / 255.0f);
			OutputImage.push_back(c.g / 255.0f);
			OutputImage.push_back(c.b / 255.0f);
		}
	}
}

void resize_callback(GLFWwindow*, int nw, int nh)
{
	//This is called in response to the window resizing.
	//The new width and height are passed in so we make 
	//any necessary changes:
	Width = nw;
	Height = nh;
	//Tell the viewport to use all of our screen estate
	glViewport(0, 0, nw, nh);

	//This is not necessary, we're just working in 2d so
	//why not let our spaces reflect it?
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0, static_cast<double>(Width)
		, 0.0, static_cast<double>(Height)
		, 1.0, -1.0);

	//Reserve memory for our render so that we don't do 
	//excessive allocations and render the image
	OutputImage.reserve(Width * Height * 3);
	render();
}


int main(int argc, char* argv[])
{
	// -------------------------------------------------
	// Initialize Window
	// -------------------------------------------------

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(Width, Height, "Assignment6", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	//We have an opengl context now. Everything from here on out 
	//is just managing our window or opengl directly.

	//Tell the opengl state machine we don't want it to make 
	//any assumptions about how pixels are aligned in memory 
	//during transfers between host and device (like glDrawPixels(...) )
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	//We call our resize function once to set everything up initially
	//after registering it as a callback with glfw
	glfwSetFramebufferSizeCallback(window, resize_callback);
	resize_callback(NULL, Width, Height);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		//Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);
		// -------------------------------------------------------------
		//Rendering begins!
		glDrawPixels(Width, Height, GL_RGB, GL_FLOAT, &OutputImage[0]);
		//and ends.
		// -------------------------------------------------------------

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		//Close when the user hits 'q' or escape
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS
			|| glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
