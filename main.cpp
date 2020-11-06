/*
*        Computer Graphics Course - Shenzhen University
*  Week 7 - Orthogonal Projection and Perspective Projection
* ============================================================
*
* - ����������ǲο����룬����Ҫ����ο���ҵ˵��������˳������ɡ�
* - ��������OpenGL�����������������У���ο���һ��ʵ��γ�����ĵ���
*
* ============================================================
* ���̣��ⲩ��(Bojian Wu)
* ���䣺bj.wu@siat.ac.cn
* ���������˵������ҵ���κ����⣬��ӭ���ʼ���ѯ��
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

// TODO �밴��ʵ������ݲ�ȫ�������
// �������
double Delta = M_PI / 2.0;
double Theta = M_PI / 2.0;
double R = 5.0;

// ͶӰ����
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
		// TODO �밴��ʵ������ݲ�ȫ����۲����ļ���
		// ƽ��ͶӰ������ppt
		vec4 v0 = vec4(2 / (right - left), 0, 0, -(right + left) / (right - left));
		vec4 v1 = vec4(0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom));
		vec4 v2 = vec4(0, 0, -2 / (zFar - zNear), -(zFar + zNear) / (zFar - zNear));
		vec4 v3 = vec4(0, 0, 0, 1.0f);
		return mat4(v0, v1, v2, v3);
	}

	mat4 perspective( const GLfloat fovy, const GLfloat aspect,
		const GLfloat zNear, const GLfloat zFar)
	{
		// TODO �밴��ʵ������ݲ�ȫ����۲����ļ���
		// ͸��ͶӰ������ppt
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
		// TODO �밴��ʵ������ݲ�ȫ����۲����ļ���
		// �������λ�ü����򣬲����Ͻڿ�����
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
// OpenGL ��ʼ��

void init()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// ����shader���һ�ȡ������λ��
	programID = InitShader("vshader.glsl", "fshader.glsl");
	vPositionID = glGetAttribLocation(programID, "vPosition");
	modelViewProjMatrixID = glGetUniformLocation(programID, "modelViewProjMatrix");

	// ���ⲿ��ȡ��άģ���ļ�
	mesh->read_off("cube.off");

	vector<vec3f> vs = mesh->v();
	vector<vec3i> fs = mesh->f();

	// ����VAO
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// ����VBO�����󶨶�������
	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, vs.size() * sizeof(vec3f), vs.data(), GL_STATIC_DRAW);

	// ����VBO�����󶨶�������
	glGenBuffers(1, &vertexIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, fs.size() * sizeof(vec3i), fs.data(), GL_STATIC_DRAW);

	// OpenGL��Ӧ״̬����
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

//////////////////////////////////////////////////////////////////////////
// ��Ⱦ

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);

	// TODO �����������
	vec4 eye(R * sin(Delta) * cos(Theta), R * cos(Delta), R * sin(Delta) * sin(Theta), 1.0);
	vec4 at(0.0, 0.0, 0.0, 1.0);
	vec4 up(0.0, 1.0, 0.0, 0.0);

	Camera::modelMatrix = mat4(1);  //����ʵ����Ϊ��λ��
	Camera::viewMatrix = Camera::lookAt(eye, at, up);   //LookAt
	if (mode == 0) {
		Camera::projMatrix = Camera::ortho(-Right, Right, -Top, Top, Near, Far);
	}
	else {
		Camera::projMatrix = Camera::perspective(Fovy, Aspect, Near, Far);   //ƽ�л�͸��ͶӰ����
	}
	

	mat4 modelViewProjMatrix = Camera::projMatrix * Camera::viewMatrix * Camera::modelMatrix;  //�������ˣ�ע��˳���ȳ˵Ŀ���ԭ���ꡣ
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
// �������ô���

void reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
}

//////////////////////////////////////////////////////////////////////////
// �����Ӧ����

void mouse(int button, int state, int x, int y)
{
	return;
}

//////////////////////////////////////////////////////////////////////////
// ������Ӧ����
// ����Delta
void updateDelta(int sign, double step) {
	Delta += sign * step;

}

// ����Theta
void updateTheta(int sign, double step) {
	Theta += sign * step;
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 033:	// ESC�� �� 'q' ���˳���Ϸ
		exit(EXIT_SUCCESS);
		break;
	case 'q':
		exit(EXIT_SUCCESS);
		break;
		// Todo�����̿��������λ�úͳ���
		// w, s --> R����
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
		// ����Ϊ2 * M_PI
		break;
	case 'f':
		updateTheta(-1, 0.1);
		break;

	// ͶӰ������������д��ʾ���ӣ���֮��С
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
	// ͶӰģʽѡ��
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