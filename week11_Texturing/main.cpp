#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <GL\GLU.h>
#include <GL\freeglut.h>
#include <vector>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <Geometry\TriangularSurface\StaticTriangularSurface.h>
#include <GL2\GL2_World.h>
#include <GL2\GL2_Object.h>
#include <GL2\GL2_Light.h>
#include <GL2\GL2_Material.h>
#include <BMPImage.h>

int width_window = 640;
int height_window = 480;

GL2_World gl_world;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	gl_world.camera_.UpdateDolly(yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		gl_world.camera_.StartMouseRotation(xpos, ypos);
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		gl_world.camera_.EndMouseRotation(xpos, ypos);
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		gl_world.camera_.StartMousePan(xpos, ypos);
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		gl_world.camera_.EndMousePan(xpos, ypos);
	}
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	gl_world.camera_.ProcessMouseMotion(xpos, ypos);
}

int main(void)
{
	GLFWwindow* window = nullptr;

	if (!glfwInit()) return -1;

	glfwWindowHint(GLFW_SAMPLES, 32);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	width_window = mode->width * 0.8f;
	height_window = mode->height * 0.8f;

	window = glfwCreateWindow(width_window, height_window, "Texturing", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	// callbacks here
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

	glfwMakeContextCurrent(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		return -1;
	}

	glClearColor(1, 1, 1, 1);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	const float aspect = (float)width / (float)height;
	glViewport(0, 0, width, height);
	glOrtho(-1.2*aspect, 1.2*aspect, -1.2, 1.2, -100.0, 100.0);
	gluLookAt(0.1, 0.1, 0.1, 0.0, 0.0, 0.0, 0, 1, 0);

	const float zNear = 0.001, zFar = 100.0, fov = 45.0;
	gl_world.camera_.Resize(width, height, fov, zNear, zFar);
	gl_world.initShaders();

	StaticTriangularSurface surface;
	surface.readObj("Dark_Star_Toad.obj", true, true);
	surface.vertex_uv_.init(surface.vertex_positions_.num_elements_);
	for (int i = 0; i < surface.vertex_positions_.num_elements_; i++)
	{
		float x = surface.vertex_positions_[i].x_;
		float z = surface.vertex_positions_[i].z_;

		float theta = std::atan2(z, x); // -PI ~ PI
		theta = ((theta / PI) + 1.0)*0.5f;

		// uv 좌표 설정
		surface.vertex_uv_[i].u_ = theta;
		surface.vertex_uv_[i].v_ = surface.vertex_positions_[i].y_ + 0.5;//  vertext position => 0.0 ~ 1.0

	}
	// week 11 texturing toy example
	//surface.vertex_positions_.init(4);
	//surface.vertex_positions_[0] = TV(-1.0, -1.0, 0.0);
	//surface.vertex_positions_[1] = TV(1.0, -1.0, 0.0);
	//surface.vertex_positions_[2] = TV(1.0, 1.0, 0.0);
	//surface.vertex_positions_[3] = TV(-1.0, 1.0, 0.0);

	//surface.vertex_normals_.init(4);
	//surface.vertex_normals_[0] = TV(0.0, 0.0, 1.0);
	//surface.vertex_normals_[1] = TV(0.0, 0.0, 1.0);
	//surface.vertex_normals_[2] = TV(0.0, 0.0, 1.0);
	//surface.vertex_normals_[3] = TV(0.0, 0.0, 1.0);

	//surface.vertex_uv_.init(4);
	//surface.vertex_uv_[0] = TV2(0.0, 0.0);
	//surface.vertex_uv_[1] = TV2(1.0, 0.0);
	//surface.vertex_uv_[2] = TV2(1.0, 1.0);
	//surface.vertex_uv_[3] = TV2(0.0, 1.0);

	//surface.triangles_.init(2);
	//surface.triangles_[0] = TV_INT(0, 1, 2);
	//surface.triangles_[1] = TV_INT(0, 2, 3);

	GL2_Object gl_obj;
	gl_obj.initPhongSurfaceWithTexture(surface);
	gl_obj.mat_.setGold();

	GLuint textures[2];
	glGenTextures(2, textures);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);

	// Black/white checkerboard
	/*const float pixels[] = { 1.0f, 0.0f, 0.0f,   0.3f, 0.3f, 1.0f,   0.5f, 0.5f, 0.0f,
			   0.5f, 0.5f, 1.0f,   0.0f, 0.0f, 0.0f ,   0.1f, 0.4f, 0.3f};*/

			   //표현할 행렬 개수 3, 2로 되어있음
			   //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 3, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_array);


	int texture_width, texture_height;
	unsigned char* rgb_array;

	//파일명 입력받아서 bmp파일 불러와서 지정되있는 width, height를 변수에 각각 저장, 1차원에 변형
	//unsigned라서 GL_FLOAT를 GL_UNSIGNED_BYTE로 변경
	readBMP24("20170108_071114.bmp", &texture_width, &texture_height, &rgb_array);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_array);

	// do not forget these options!
	// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);



	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_COLOR_MATERIAL);

	glLoadIdentity();

	GL2_Light light;

	while (!glfwWindowShouldClose(window))
	{
		gl_world.camera_.ContinueRotation();

		glm::mat4 vp = gl_world.camera_.GetWorldViewMatrix();

		for (int i = 0; i < surface.vertex_positions_.num_elements_; ++i)
		{
			surface.vertex_uv_[i].x_ += 0.001f;
		}

		// don't need to update if there is no change
		gl_obj.updatePhongSurfaceWithTexture(surface);


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw with shader
		glUseProgram(gl_world.shaders_.program_id_);

		gl_world.shaders_.sendUniform(vp, "mvp");

		gl_obj.applyLighting(light);
		//gl_obj.drawWithShaderTexture(gl_world.shaders_);

		gl_obj.drawWithTextureShader(gl_world.shaders_);
		glUseProgram(0);

		// draw with old style
		glLoadMatrixf(&vp[0][0]);

		glLineWidth(2);
		gl_world.drawAxes();

		glfwSwapBuffers(window);

		glfwPollEvents();

	}

	glfwTerminate();

	return 0;
}