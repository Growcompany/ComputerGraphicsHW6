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

	// 정점별 노말 저장용 벡터 추가
	std::vector<Vec3> vertexNormals(gNumVertices, Vec3{ 0,0,0 });

	// 각 삼각형의 face normal을 정점에 누적
	auto to_world = [&](const Vec3& v) {
		Vec4 v4{ v.x,v.y,v.z,1.0f };
		Vec4 w4 = mul(M, v4);
		return Vec3{ w4.x, w4.y, w4.z };
		};
	for (int tri = 0; tri < gNumTriangles; ++tri) {
		int i0 = gIndexBuffer[3 * tri + 0],
			i1 = gIndexBuffer[3 * tri + 1],
			i2 = gIndexBuffer[3 * tri + 2];
		Vec3 p0 = to_world(gVertexArray[i0]);
		Vec3 p1 = to_world(gVertexArray[i1]);
		Vec3 p2 = to_world(gVertexArray[i2]);
		Vec3 Nface = normalize(cross(p1 - p0, p2 - p0));
		Vec3 cent = (p0 + p1 + p2) * (1.0f / 3.0f);
		if (dot(Nface, normalize(camPos - cent)) < 0) Nface = Nface - Nface;
		vertexNormals[i0] = vertexNormals[i0] + Nface;
		vertexNormals[i1] = vertexNormals[i1] + Nface;
		vertexNormals[i2] = vertexNormals[i2] + Nface;
	}
	// 누적된 노말 정규화
	for (auto& N : vertexNormals)
		N = normalize(N);

	// 정점 셰이딩(Blinn–Phong+감마) 람다
	auto shadeVertex = [&](const Vec3& pos, const Vec3& N) {
		Vec3 L = normalize(lightPos - pos);
		Vec3 Vv = normalize(camPos - pos);
		Vec3 H = normalize(L + Vv);
		float diff = std::max(0.0f, dot(N, L));
		float spec = std::pow(std::max(0.0f, dot(N, H)), shininess);
		Vec3 c = ka * Ia + kd * diff + ks * spec;
		c.x = std::pow(c.x, 1.0f / 2.2f);
		c.y = std::pow(c.y, 1.0f / 2.2f);
		c.z = std::pow(c.z, 1.0f / 2.2f);
		return c;
		};

	// 월드 -> 스크린 좌표 변환 람다
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

		// 정점별 world 위치 & 노말 가져오기
		Vec3 p0 = to_world(gVertexArray[i0]);
		Vec3 p1 = to_world(gVertexArray[i1]);
		Vec3 p2 = to_world(gVertexArray[i2]);
		Vec3 N0 = vertexNormals[i0];
		Vec3 N1 = vertexNormals[i1];
		Vec3 N2 = vertexNormals[i2];

		// 정점별 색 계산
		Vec3 c0 = shadeVertex(p0, N0);
		Vec3 c1 = shadeVertex(p1, N1);
		Vec3 c2 = shadeVertex(p2, N2);

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
					// Gouraud 보간 색
					Vec3 col = c0 * w0 + c1 * w1 + c2 * w2;
					frameBuffer[idx] = {
						uint8_t(std::min(1.0f, std::max(0.0f, col.x)) * 255),
						uint8_t(std::min(1.0f, std::max(0.0f, col.y)) * 255),
						uint8_t(std::min(1.0f, std::max(0.0f, col.z)) * 255)
					};
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
