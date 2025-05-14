# Computer Graphics Assignment6

## Overview

- 이 과제는 OpenGL을 사용하지 않고 직접 간단한 래스터화 파이프라인을 구현하여, 여러 개의 삼각형으로 표현된 하나의 음영 처리된 구(쉐이딩된 스피어)를 그리는 것
- Q1 - Flat shading 구현
- Q2 - Gouraud shading 구현
- Q3 - Phong shading 구현

---

## Table of Contents
- [Compilation Instructions](#compilation-instructions)
- [Run Instructions](#run-instructions)
- [Short Description](#short-description)

---

## Compilation Instructions

### Using Visual Studio
1. **솔루션 열기**  
   `Assignment6.sln` 파일을 Visual Studio에서 엽니다.
2. **프로젝트 설정 확인**  
   - **Include 디렉토리:** `include` 폴더 경로가 포함되어야 합니다.  
   - **Library 디렉토리:** `lib` 폴더 경로가 포함되어야 합니다.
3. **Clean & Rebuild**  
   상단 메뉴에서 **Build > Clean Solution**을 선택한 후, **Build > Rebuild Solution**을 실행하여 최신 코드를 빌드합니다. 또는 단축키(Ctrl + Shift + B)로 빌드 가능

---

## Run Instructions

### 1. 실행 파일로 실행

- 빌드가 완료되면 실행 파일이 `bin` 폴더에 생성됩니다:
  - `Assignment6_Q1.exe`
  - `Assignment6_Q2.exe`
  - `Assignment6_Q3.exe`
- 원하는 `.exe` 파일을 더블 클릭하여 실행합니다.

---

### 2. Visual Studio에서 실행

- 솔루션에는 총 3개의 프로젝트가 포함되어 있습니다.
- 특정 프로젝트만 실행하려면:
  1. **솔루션 탐색기**에서 원하는 프로젝트를 마우스 오른쪽 클릭
  2. **"Set as Startup Project"** 선택
  3. `Ctrl + F5`를 눌러 실행 (디버깅 없이 실행)

---

### 3. 실행 시 동작

- 프로그램 실행 후, 계산된 최종 이미지가 창에 출력됩니다.
- 프로그램 종료는 키보드에서 **ESC 키** 또는 **Q 키**를 누르면 됩니다.


---

## Short Description

### Q1: Flat Shading

1. **Create sphere mesh**  
   삼각형들로 이루어진 구의 기하 정보를 메모리에 생성하여 정점과 인덱스 배열에 저장합니다.

2. **Prepare color and depth buffers**  
   화면 해상도 크기만큼 비어 있는 색상 버퍼와 최댓값으로 초기화된 깊이 버퍼를 할당합니다.

3. **Set up transformation matrices**  
   모델→월드→카메라→투영→뷰포트 순으로 적용할 변환 행렬을 차례로 계산해 둡니다.

4. **Transform vertex coordinates**  
   각 정점을 위에서 만든 변환 행렬로 처리해 화면 픽셀 좌표와 깊이 값으로 변환합니다.

5. **Perform back-face culling**  
   화면상에서 보이지 않는 뒷면 삼각형을 걸러 내어 계산량을 줄입니다.

6. **Compute normal vectors**  
   월드 공간에서 삼각형 면의 법선 방향을 구하고, 카메라 방향에 맞춰 뒤집어 줍니다.

7. **Compute lighting**  
   법선, 광원 방향, 시점 방향을 이용해 Blinn–Phong 모델로 각 삼각형의 색상을 산출합니다.

8. **Rasterize and perform depth test**  
   삼각형의 바운딩 박스 안 픽셀마다 내부 여부를 검사하고, 깊이 비교를 거쳐 최종 색상을 기록합니다.

9. **Normalize color**  
   8비트 RGB 값을 0~1 범위의 실수로 변환해 최종 이미지 버퍼에 채웁니다.

10. **Handle window resize**  
    창 크기 변경 시 뷰포트와 투영을 재설정한 뒤, 렌더링 과정을 다시 실행합니다.

11. **Initialize render loop**  
    GLFW 초기화, 윈도우 생성, 픽셀 정렬 설정을 수행합니다.

12. **Draw frame**  
    매 프레임 `glClear` → `glDrawPixels(OutputImage)` → `glfwSwapBuffers` 순으로 이미지를 화면에 그립니다.

13. **Handle window resize**  
    `resize_callback`에서 뷰포트(`glViewport`)와 투영(`glOrtho`)을 재설정한 뒤 `render()`를 호출해 이미지를 띄워줍니다.

---

### Q2: Gouraud Shading

1. **Create sphere mesh**  
   삼각형들로 이루어진 구의 기하 정보를 메모리에 생성하여 정점과 인덱스 배열에 저장합니다.

2. **Prepare color and depth buffers**  
   화면 해상도 크기만큼 비어 있는 색상 버퍼와 최댓값으로 초기화된 깊이 버퍼를 할당합니다.

3. **Set up transformation matrices**  
   모델→월드→카메라→투영→뷰포트 순으로 적용할 변환 행렬을 차례로 계산해 둡니다.

4. **Accumulate vertex normals**  
   각 삼각형의 면 법선을 월드 공간에서 계산하여, 해당 삼각형에 속한 세 정점의 법선 벡터에 누적합니다.

5. **Normalize vertex normals**  
   누적된 정점 법선을 모두 정규화하여 매 정점마다 부드러운 법선 방향을 얻습니다.

6. **Shade vertices (Blinn–Phong + gamma)**  
   각 정점 위치와 법선을 이용해 Ambient/Diffuse/Specular 조명을 계산하고, 감마 보정까지 적용해 정점별 색상 벡터를 미리 구해둡니다.

7. **Transform vertex coordinates**  
   계산된 변환 행렬로 각 정점을 클립→NDC→화면 좌표와 깊이 값으로 변환합니다.

8. **Perform back-face culling**  
   화면상에서 보이지 않는 뒷면 삼각형을 걸러 내어 계산량을 줄입니다.

9. **Fetch triangle vertex data**  
   남은 삼각형에 대해 화면 좌표와 미리 셰이딩된 정점 색상(c0, c1, c2)을 가져옵니다.

10. **Compute bounding box**  
    삼각형이 차지하는 최소/최대 픽셀 범위를 구해 래스터화 영역을 제한합니다.

11. **Rasterize with barycentric coords + depth test**  
    각 픽셀에서 barycentric 좌표(w0, w1, w2)를 계산해 삼각형 내부를 판정하고, 깊이 버퍼 검사 후 픽셀 색상을 기록합니다.

12. **Interpolate colors**  
    내부 픽셀에 대해 정점 색상(c0, c1, c2)을 barycentric 가중치로 보간하여 최종 색상을 얻습니다.

13. **Convert framebuffer to float RGB**  
    8비트 RGB 값을 0~1 범위 실수로 정규화해 `OutputImage` 벡터에 채웁니다.

14. **Handle window resize**  
    `resize_callback`에서 뷰포트(`glViewport`)와 투영(`glOrtho`)을 재설정한 뒤 `render()`를 호출해 이미지를 띄워줍니다.

---

### Q3: Phong Shading

1. **Create sphere mesh**  
   삼각형 기반 구의 정점·인덱스 데이터를 초기화합니다.

2. **Prepare color and depth buffers**  
   화면 크기만큼 빈 프레임버퍼와 무한대 값의 깊이버퍼를 할당합니다.

3. **Set up transformation matrices**  
   모델(M), 뷰(V), 투영(P), 뷰포트(W) 행렬을 차례로 계산합니다.

4. **Accumulate and normalize vertex normals**  
   각 삼각형 면 법선을 정점 법선에 누적한 뒤 모두 정규화합니다.

5. **Transform vertex coordinates**  
   `transform_vertex` 람다로 월드→클립→NDC→화면 좌표와 깊이 값을 계산합니다.

6. **Perform back-face culling**  
   화면 교차값(crossZ)을 이용해 보이지 않는 뒷면 삼각형을 걸러냅니다.

7. **Compute rasterization bounds**  
   삼각형의 바운딩 박스를 계산해 픽셀 검사 범위를 제한합니다.

8. **Perform barycentric and depth tests**  
   각 픽셀에서 barycentric 좌표로 내부 여부를 판정하고 Z-버퍼를 비교합니다.

9. **Interpolate position and normal**  
   barycentric 가중치로 픽셀 위치(`Ppx`)와 법선(`Npx`)을 보간합니다.

10. **Compute Phong lighting**  
    보간된 위치·법선으로 Ambient/Diffuse/Specular를 Blinn–Phong 모델로 산출합니다.

11. **Apply gamma correction and convert to 8-bit**  
    γ=2.2로 감마 보정 후 0–255 범위의 8비트 RGB 색상으로 변환합니다.

12. **Convert to float RGB**  
    프레임버퍼의 8비트 색상을 0~1 실수로 정규화해 `OutputImage`에 채웁니다.

13. **Handle window resize**  
    `resize_callback`에서 뷰포트(`glViewport`)와 투영(`glOrtho`)을 재설정한 뒤 `render()`를 호출해 이미지를 띄워줍니다.
