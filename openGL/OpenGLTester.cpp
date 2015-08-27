#include <stdio.h>
#include <GL/glut.h>

int main(int argc, char** argv)
{
	// 返回负责当前OpenGL实现厂商的名字
	const GLubyte* name = glGetString(GL_VENDOR);

	// 返回一个渲染器标识符，通常是个硬件平台
	const GLubyte* renderID = glGetString(GL_RENDERER);

	// 返回当前OpenGL实现的版本号
	const GLubyte* OpenGLVersion = glGetString(GL_VERSION);

	// 返回当前GLU工具库版本
	const GLubyte* gluVersion = gluGetString(GLU_VERSION);

	printf("OpenGL实现厂商的名字：%s\n", name);
	printf("渲染器标识符：%s\n", renderID);
	printf("OpenGL实现的版本号：%s\n", OpenGLVersion);
	printf("GLU工具库版本：%s\n", gluVersion);

	return 0;
}
