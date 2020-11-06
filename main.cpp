/*
*        Computer Graphics Course - Shenzhen University
*  Week 7 - Orthogonal Projection and Perspective Projection
* ============================================================
*
* - 本代码仅仅是参考代码，具体要求请参考作业说明，按照顺序逐步完成。
* - 关于配置OpenGL开发环境、编译运行，请参考第一周实验课程相关文档。
*
* ============================================================
* 助教：吴博剑(Bojian Wu)
* 邮箱：bj.wu@siat.ac.cn
* 如果对上述说明或作业有任何问题，欢迎发邮件咨询。
*/

#include "Angel.h"
#include "TriMesh.h"

#pragma comment(lib, "glew32.lib")

#include <cstdlib>
#include <iostream>
/*
#ifndef ENABLE_PROJECTION
#define ENABLE_PROJECTION 1
#endif // !ENABLE_PROJECTION
#if ENABLE_PROJECTION
int mode = 0; // 0 <--> ortho, 1 <--> perspective
GLfloat Right;
GLfloat Top;
GLfloat Near;
GLfloat Far;
GLfloat Fvoy;
GLfloat Aspect;
GLfloat delta = 0.1;
#endif
*/

using namespace std;

GLuint programID;
GLuint vertexArrayID;
GLuint vertexBufferID;
GLuint vertexIndexBuffer;

GLuint vPositionID;
GLuint modelViewProjMatrixID;

// TODO 请按照实验课内容补全相机参数
// 相机参数
double Delta = M_PI / 2.0;
double Theta = M_PI / 2.0;
double R = 5.0;

// 投影参数
int mode = 1; // 0 <--> ortho, 1 <--> perspective
GLfloat Right = 0.1;
GLfloat Top = 0.1;
GLfloat Near = 0.01;
GLfloat Far = 100;
GLfloat Fovy = M_PI / 4;
GLfloat Aspect = 1;

TriMesh* mesh = new TriMesh();

namespace Camera
{
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projMatrix;

	mat4 ortho( const GLfloat left, const GLfloat right,
		const GLfloat bottom, const GLfloat top,
		const GLfloat zNear, const GLfloat zFar )
	{
		// TODO 请按照实验课内容补全相机观察矩阵的计算
		// 平行投影，参照ppt
		vec4 v0 = vec4(2 / (right - left), 0, 0, -(right + left) / (right - left));
		vec4 v1 = vec4(0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom));
		vec4 v2 = vec4(0, 0, -2 / (zFar - zNear), -(zFar + zNear) / (zFar - zNear));
		vec4 v3 = vec4(0, 0, 0, 1.0f);
		return mat4(v0, v1, v2, v3);
	}

	mat4 perspective( const GLfloat fovy, const GLfloat aspect,
		const GLfloat zNear, const GLfloat zFar)
	{
		// TODO 请按照实验课内容补全相机观察矩阵的计算
		// 透视投影，参照ppt
		auto top = zNear * tan(fovy);
		auto right = top * aspect;
		vec4 v0 = vec4(zNear / right, 0, 0, 0);
		vec4 v1 = vec4(0, zNear / top, 0, 0);
		vec4 v2 = vec4(0, 0, -(zFar + zNear) / (zFar - zNear), -2 * zFar * zNear / (zFar - zNear));
		vec4 v3 = vec4(0, 0, -1, 0);
		return mat4(v0, v1, v2, v3);
	}

	mat4 lookAt( const vec4& eye, const vec4& at, const vec4& up )
	{
		// TODO 请按照实验课内容补全相机观察矩阵的计算
		// 控制相机位置及朝向，参照上节课内容
		vec4  n;
		vec4  u;
		vec4  v;

		n = eye - at;
		n = normalize(n);
		u = vec4(cross(up, n), 0.0);
		u = normalize(u);
		v = vec4(cross(n, u), 0.0);
		v = normalize(v);

		return mat4(u, v, n, vec4(0.0, 0.0, 0.0, 1.0)) *
			mat4(1.0, 0, 0, 0,
				0, 1.0, 0, 0,
				0, 0, 1.0, 0,
				-eye.x, -eye.y, -eye.z, 1.0);
	}
}

//////////////////////////////////////////////////////////////////////////
// OpenGL 初始化

void init()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// 加载shader并且获取变量的位置
	programID = InitShader("vshader.glsl", "fshader.glsl");
	vPositionID = glGetAttribLocation(programID, "vPosition");
	modelViewProjMatrixID = glGetUniformLocation(programID, "modelViewProjMatrix");

	// 从外部读取三维模型文件
	mesh->read_off("cube.off");

	vector<vec3f> vs = mesh->v();
	vector<vec3i> fs = mesh->f();

	// 生成VAO
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// 生成VBO，并绑定顶点坐标
	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, vs.size() * sizeof(vec3f), vs.data(), GL_STATIC_DRAW);

	// 生成VBO，并绑定顶点索引
	glGenBuffers(1, &vertexIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, fs.size() * sizeof(vec3i), fs.data(), GL_STATIC_DRAW);

	// OpenGL相应状态设置
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

//////////////////////////////////////////////////////////////////////////
// 渲染

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);

	// TODO 设置相机参数
	vec4 eye(R * sin(Delta) * cos(Theta), R * cos(Delta), R * sin(Delta) * sin(Theta), 1.0);
	vec4 at(0.0, 0.0, 0.0, 1.0);
	vec4 up(0.0, 1.0, 0.0, 0.0);

	Camera::modelMatrix = mat4(1);  //本次实验设为单位阵
	Camera::viewMatrix = Camera::lookAt(eye, at, up);   //LookAt
	if (mode == 0) {
		Camera::projMatrix = Camera::ortho(-Right, Right, -Top, Top, Near, Far);
	}
	else {
		Camera::projMatrix = Camera::perspective(Fovy, Aspect, Near, Far);   //平行或透视投影矩阵
	}
	

	mat4 modelViewProjMatrix = Camera::projMatrix * Camera::viewMatrix * Camera::modelMatrix;  //矩阵连乘，注意顺序，先乘的靠近原坐标。
	glUniformMatrix4fv(modelViewProjMatrixID, 1, GL_TRUE, &modelViewProjMatrix[0][0]);

	glEnableVertexAttribArray(vPositionID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glVertexAttribPointer(
		vPositionID,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);

	glDrawElements(
		GL_TRIANGLES,
		int(mesh->f().size() * 3),
		GL_UNSIGNED_INT,
		(void*)0
	);

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	glBegin(GL_POINTS);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glEnd();

	glutSwapBuffers();
}

//////////////////////////////////////////////////////////////////////////
// 重新设置窗口

void reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
}

//////////////////////////////////////////////////////////////////////////
// 鼠标响应函数

void mouse(int button, int state, int x, int y)
{
	return;
}

//////////////////////////////////////////////////////////////////////////
// 键盘响应函数
// 更新Delta
void updateDelta(int sign, double step) {
	Delta += sign * step;

}

// 更新Theta
void updateTheta(int sign, double step) {
	Theta += sign * step;
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 033:	// ESC键 和 'q' 键退出游戏
		exit(EXIT_SUCCESS);
		break;
	case 'q':
		exit(EXIT_SUCCESS);
		break;
		// Todo：键盘控制相机的位置和朝向
		// w, s --> R距离
		// e, d --> Delta
		// r, f --> Theta
	case 'w':
		R += 0.1;
		break;
	case 's':
		R -= 0.1;
		break;
	case 'e':
		updateDelta(1, 0.1);
		break;
	case 'd':
		updateDelta(-1, 0.1);
		break;
	case 'r':
		updateTheta(1, 0.1);
		// 周期为2 * M_PI
		break;
	case 'f':
		updateTheta(-1, 0.1);
		break;

	// 投影参数调整，大写表示增加，反之减小
	// Z -- Top
	// Right -- X
	// Far -- C
	// Fvoy -- V
	// Asperc -- B
	// Near -- N
	case 'Z':
		Top += 0.01;
		break;
	case 'z':
		Top -= 0.01;
		break;
	case 'X':
		Right += 0.01;
		break;
	case 'x':
		Right -= 0.01;
		break;
	case 'C':
		Far += 1;
		break;
	case 'c':
		Far -= 1;
		break;
	case 'V':
		Fovy += 0.1;
		break;
	case 'v':
		Fovy -= 0.1;
	case 'B':
		Aspect += 0.01;
		break;
	case 'b':
		Aspect -= 0.01;
		break;
	case 'N':
		Near += 0.01;
		break;
	case 'n':
		Near -= 0.01;
		break;
	// 投影模式选择
	case 'o':
		mode = 0;
		break;
	case 'p':
		mode = 1;
		break;
	}
	glutPostRedisplay();
}

//////////////////////////////////////////////////////////////////////////

void idle(void)
{
	glutPostRedisplay();
}

//////////////////////////////////////////////////////////////////////////

void clean()
{
	glDeleteBuffers(1, &vertexBufferID);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &vertexArrayID);

	if (mesh) {
		delete mesh;
		mesh = NULL;
	}
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(500, 500);
	glutCreateWindow("OpenGL-Tutorial");

	glewInit();
	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop();

	clean();

	return 0;
}