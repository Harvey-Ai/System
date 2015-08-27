#include <iostream>
#include <GL/glut.h>
using namespace std;

void displayFun(void)
{
	glClearColor(0, 0, 0, 1);
	cout << "This is display function" << endl;
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	//显示模式初始化
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);

	// 定义窗口大小
	glutInitWindowSize(300, 300);

	// 定义窗口位置
	glutInitWindowPosition(100, 100);

	// 创建窗口
	glutCreateWindow("Glut tester");

	// 指定渲染函数
	glutDisplayFunc(displayFun);

	glutMainLoop();
	return 0;
}
