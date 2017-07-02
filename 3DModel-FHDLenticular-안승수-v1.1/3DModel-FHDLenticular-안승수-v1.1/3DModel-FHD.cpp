#include "stdafx.h"

using namespace std;

typedef struct _position
{
	float x, y, z, b, g, r,min;
} position;
// �������� �����ؾߵ� ��ǥ���� �� ����ü�� ����
int cnt = 0;

#define GET2D8U(IMAGE,X,Y)	(*( ( (uchar*)( ( (IMAGE) -> imageData ) + (Y) * ( (IMAGE) -> widthStep ) ) ) + (X) ))
#define GET2D8U3CH(IMAGE,X,Y) ( ( (uchar*)( ( (IMAGE) -> imageData ) + (Y) * ( (IMAGE) -> widthStep ) ) ) + ( 3 * (X) ) )
//use :  GET2D8U3CH(IMAGE,X,Y)[0], GET2D8U3CH(IMAGE,X,Y)[1], GET2D8U3CH(IMAGE,X,Y)[2]
#define GET2D16U(IMAGE,X,Y)	(*( ( (ushort*)( ( (IMAGE) -> imageData ) + (Y) * ( (IMAGE) -> widthStep ) ) ) + (X) ))
#define GET2D16S(IMAGE,X,Y)	(*( ( (short*)( ( (IMAGE) -> imageData ) + (Y) * ( (IMAGE) -> widthStep ) ) ) + (X) ))
#define GET2D32F(IMAGE,X,Y) (*( ( (float*)( ( (IMAGE) -> imageData ) + (Y) * ( (IMAGE) -> widthStep ) ) ) + (X) ))

//static bool mouseLeftDown;

static float theta;
static int direction;
static int rotate_x;
static int rotate_y;
static int rotate_z;
bool rt;
int cnt_points, depth_max, depth_min;
//////////////////////////////////////
//���콺�� ��ȸ��
GLint  mouseX = 0;
GLint  mouseY = 0;
GLint  mouseState = 0;
GLint  mouseButton = 0;
int g=1;
GLfloat  xTheta=0.0, yTheta=0.0;
GLfloat  scale=1.0, scaleDelta=1.01; 

///////////////////////////////////////
position *Render3D;
///////////////////////////////////////
//winapi window ȭ�� ���� ����
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM IParam);
DWORD __stdcall CreateStereoWindowsAPI(void* lpszClass);
HANDLE g_hThreadTvDisplay=0;
DWORD g_IDThreadTvDisplay;
HWND g_hWndTvDisplay=0;
void iplImageToDC(IplImage* pImgIpl, HDC hDC, CvRect rect);

/////////////////////////////////

int g_lenti_angle;//1.5
int g_colorenalbe=1;
int g_holefilling=1;


IplImage *g_color_image;
IplImage *g_depth_image;
IplImage *g_3dmodel[8];
IplImage* g_display_stereo;

int window1;
int window2;
int window3;
int window4;
int window5;
int window6;
int window7;
int window8;



char str_fps[20]="0.00fps";

void initProgram();
void readSettingsFromINI();
void exitProgram();
void exitCamera();
void SetupRC(void);
void init(void);

void ChangeSize1(int w, int h);
void ChangeSize2(int w, int h);
void ChangeSize3(int w, int h);
void ChangeSize4(int w, int h);
void ChangeSize5(int w, int h);
void ChangeSize6(int w, int h);
void ChangeSize7(int w, int h);
void ChangeSize8(int w, int h);

void Render1(void);
void Render2(void);
void Render3(void);
void Render4(void);
void Render5(void);
void Render6(void);
void Render7(void);
void Render8(void);

void lenticular();

void Renderright(void);
void mouseButtonfunc(int button, int state, int x, int y);
void mouseWheel(int button,int dir,int x,int y);
void myMotion(int x, int y);
void SaveInfo(IplImage *Color, IplImage *Depth);
void ChangeSize(int w, int h);
void ChangeSize2(int w, int h);
void Renderleft(void);
void Renderright(void);
void Key(int key, int x, int y);
void mouseWheel(int button, int dir, int x, int y);



void main(int argc, char* argv[])
{  
	glutInit(&argc, argv); //glut �ʱ�ȭ 
	initProgram(); // ���α׷� ��ü �ʱ�ȭ
	
	cvSmooth(g_depth_image, g_depth_image, CV_MEDIAN,3,3); //depth image �߰��� ���ͷ� smoothing

	Render3D = (position*)malloc(sizeof(position) * 20000000);			// Render3D ����ü ���� �Ҵ� �κ�
	SaveInfo(g_color_image,g_depth_image);							// Depth image, Color Image ����κ�
	glutMainLoop();//glut rendering �� glut �Լ����� �ݺ� ��Ŵ 
	
}

void readSettingsFromINI() //ini ���Ͽ��� ���� ���� �޾ƿ��� 
{
	TCHAR path[512];
	GetCurrentDirectory(512,path);  //������Ʈ ���

	GetCurrentDirectory(512,path);  //������Ʈ ���
	wcscat(path,L"\\program.ini");	

	g_lenti_angle=GetPrivateProfileInt(TEXT("Angle"),TEXT("Lenti_angle"),-1,path);
}  
void initProgram() //���α׷� �ʱ�ȭ 
{
	readSettingsFromINI();
	
	g_color_image = cvLoadImage("color.bmp",3);
	g_depth_image = cvLoadImage("Depth Image fix3.bmp",CV_LOAD_IMAGE_UNCHANGED);
	g_3dmodel[0]=cvCreateImage(cvSize(1920,1080),8,3);
	g_3dmodel[1]=cvCreateImage(cvSize(1920,1080),8,3);
	g_3dmodel[2]=cvCreateImage(cvSize(1920,1080),8,3);
	g_3dmodel[3]=cvCreateImage(cvSize(1920,1080),8,3);
	g_3dmodel[4]=cvCreateImage(cvSize(1920,1080),8,3);
	g_3dmodel[5]=cvCreateImage(cvSize(1920,1080),8,3);
	g_3dmodel[6]=cvCreateImage(cvSize(1920,1080),8,3);
	g_3dmodel[7]=cvCreateImage(cvSize(1920,1080),8,3);

	g_display_stereo=cvCreateImage(cvSize(1920,1080),8,3);

	

	cvNamedWindow("Control",1);
	cvResizeWindow("Control",450,130);
	
	cvCreateTrackbar("Color","Control",&g_colorenalbe,1,0);
	cvCreateTrackbar("Filling","Control",&g_holefilling,1,0);
	cvCreateTrackbar("Viewangle","Control",&g_lenti_angle,8.0,0);
	
	init();//opengl
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB );
	glutInitWindowSize(1920,1080);

	window1=glutCreateWindow("1����");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);//freeglut
	//glutMouseWheelFunc(mouseWheel);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window1);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize1);
	glutDisplayFunc(Render1);
	glEnable(GL_DEPTH_TEST);// ���� ���� ���� �� �׷��ֱ�
	glEnable(GL_CULL_FACE);// ������ �����ϱ�(������ �ʴ¸� �ȱ׸���)

	window2=glutCreateWindow("2����");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window2);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize2);
	glutDisplayFunc(Render2);
	glEnable(GL_DEPTH_TEST);// ���� ���� ���� �� �׷��ֱ�
	glEnable(GL_CULL_FACE);// ������ �����ϱ�(������ �ʴ¸� �ȱ׸���)

	window3=glutCreateWindow("3����");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window3);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize3);
	glutSetWindow(window3);
	glutDisplayFunc(Render3);
	glEnable(GL_DEPTH_TEST);// ���� ���� ���� �� �׷��ֱ�
	glEnable(GL_CULL_FACE);// ������ �����ϱ�(������ �ʴ¸� �ȱ׸���)

	window4=glutCreateWindow("4����");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window4);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize4);
	glutDisplayFunc(Render4);
	glEnable(GL_DEPTH_TEST);// ���� ���� ���� �� �׷��ֱ�
	glEnable(GL_CULL_FACE);// ������ �����ϱ�(������ �ʴ¸� �ȱ׸���)

	window5=glutCreateWindow("5����");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window5);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize5);
	glutDisplayFunc(Render5);
	glEnable(GL_DEPTH_TEST);// ���� ���� ���� �� �׷��ֱ�
	glEnable(GL_CULL_FACE);// ������ �����ϱ�(������ �ʴ¸� �ȱ׸���)

	window6=glutCreateWindow("6����");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window6);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize6);
	glutDisplayFunc(Render6);
	glEnable(GL_DEPTH_TEST);// ���� ���� ���� �� �׷��ֱ�
	glEnable(GL_CULL_FACE);// ������ �����ϱ�(������ �ʴ¸� �ȱ׸���)

	window7=glutCreateWindow("7����");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window7);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize7);
	glutDisplayFunc(Render7);
	glEnable(GL_DEPTH_TEST);// ���� ���� ���� �� �׷��ֱ�
	glEnable(GL_CULL_FACE);// ������ �����ϱ�(������ �ʴ¸� �ȱ׸���)

	window8=glutCreateWindow("8����");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window8);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize8);
	glutDisplayFunc(Render8);
	glEnable(GL_DEPTH_TEST);// ���� ���� ���� �� �׷��ֱ�
	glEnable(GL_CULL_FACE);// ������ �����ϱ�(������ �ʴ¸� �ȱ׸���)


	g_hThreadTvDisplay=CreateThread(NULL,0,CreateStereoWindowsAPI, L"TVstereo",0,&g_IDThreadTvDisplay);
	//���ȭ�鿡 ��üȭ������ ���� ���ؼ� ���� 
	
}

void myMotion(int x, int y)//�� ������ �� �ʿ��� �Լ� Ư�� ���콺�� ������ ��
{
 if(mouseButton == GLUT_LEFT_BUTTON && mouseState == GLUT_DOWN) 
 {
    yTheta -= (GLfloat)(mouseX - x);
    xTheta -= (GLfloat)(mouseY - y);
 }
 else if(mouseButton == GLUT_RIGHT_BUTTON && mouseState == GLUT_DOWN) 
 {
    if(mouseY!=y) 
		scale = scale * pow((double)scaleDelta, (double)(mouseY - y));
 }
 else return;

 mouseX = x;
 mouseY = y;
 
 //glutPostRedisplay();
}

void mouseButtonfunc(int button, int state, int x, int y)// ���콺 ��ư ���� �Լ�
{
	if(button==GLUT_LEFT_BUTTON && state ==GLUT_DOWN) {
		mouseState=state;
		mouseButton=button;
		mouseX=x;
		mouseY=y;
	}
	else if(button==GLUT_LEFT_BUTTON && state == GLUT_UP) {
		mouseState=-1;
	
	}
	else if(button==GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		mouseState=state;
		mouseButton=button;
		mouseX=x;
		mouseY=y;
	}
	else if(button==GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		mouseState=-1;
	}
	else if(button==GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) {
		xTheta=yTheta=0.0;
		scale=1.0;
	}
	else return;

	//glutPostRedisplay();
}
void Key(int key, int x, int y){ // Ű���� �̺�Ʈ

	switch(key){

	case GLUT_KEY_LEFT :
		yTheta -=10;
		break;
	case GLUT_KEY_RIGHT : 
		yTheta +=10;
		break;
	case GLUT_KEY_END:
		exitProgram();
		break;
	default :
		break;
	}
	
}

void SetupRC(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
}

void init(void)//modeldml x,y,z �� �ʱ�ȭ
{
	
	rotate_x = 1;
	rotate_y = 0;
	rotate_z = 0;

	direction = 1;

	rt = false;

	theta = 0.0f;

}

void ChangeSize1(int w, int h) // 1 ���� window ����
{
	glutSetWindow(window1);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ChangeSize2(int w, int h) // 2 ���� window ����
{
	glutSetWindow(window2);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ChangeSize3(int w, int h) // 3 ���� window ����
{
	glutSetWindow(window3);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ChangeSize4(int w, int h) // 4 ���� window ����
{
	glutSetWindow(window4);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ChangeSize5(int w, int h) // 5 ���� window ����
{
	glutSetWindow(window5);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ChangeSize6(int w, int h) // 6 ���� window ����
{
	glutSetWindow(window6);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ChangeSize7(int w, int h) // 7 ���� window ����
{
	glutSetWindow(window7);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ChangeSize8(int w, int h) // 8 ���� window ����
{
	glutSetWindow(window8);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

////////////////////////3D�𵨸� ���ִ� �κ�
void Render1(void)// 1���� �� �׸���
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y�� ȸ��    
	glRotatef(yTheta-(g_lenti_angle+0.5)*3, 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling ������
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// ��ǥ ȯ�� �ϴ� �κ�
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//�Ƚ¼� holefilling �κ�
		if(g_holefilling==1){
		if(min-z < 0) // ��������ŭ Hole Filling �ϴ� �κ� 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// �⺻���� ������
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();
	
	glutPostWindowRedisplay(window8);
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[0]->imageData);

}

void Render2(void)// 2���� �� �׸���
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y�� ȸ��    
	glRotatef(yTheta-(g_lenti_angle+0.5)*2, 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling ������
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// ��ǥ ȯ�� �ϴ� �κ�
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//�Ƚ¼� holefilling �κ�
		if(g_holefilling==1){
		if(min-z < 0) // ��������ŭ Hole Filling �ϴ� �κ� 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// �⺻���� ������
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();

	glutPostWindowRedisplay(window1);
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[1]->imageData);
	
}

void Render3(void)// 3���� �� �׸���
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y�� ȸ��    
	glRotatef(yTheta-(g_lenti_angle+0.5), 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling ������
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// ��ǥ ȯ�� �ϴ� �κ�
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//�Ƚ¼� holefilling �κ�
		if(g_holefilling==1){
		if(min-z < 0) // ��������ŭ Hole Filling �ϴ� �κ� 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// �⺻���� ������
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();

	glutPostWindowRedisplay(window2);	
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[2]->imageData);
	
}

void Render4(void)// 4���� �� �׸���
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y�� ȸ��    
	glRotatef(yTheta, 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling ������
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// ��ǥ ȯ�� �ϴ� �κ�
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//�Ƚ¼� holefilling �κ�
		if(g_holefilling==1){
		if(min-z < 0) // ��������ŭ Hole Filling �ϴ� �κ� 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// �⺻���� ������
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();

	glutPostWindowRedisplay(window3);
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[3]->imageData);
	
}

void Render5(void)// 5���� �� �׸���
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y�� ȸ��    
	glRotatef(yTheta+(g_lenti_angle+0.5), 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling ������
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// ��ǥ ȯ�� �ϴ� �κ�
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//�Ƚ¼� holefilling �κ�
		if(g_holefilling==1){
		if(min-z < 0) // ��������ŭ Hole Filling �ϴ� �κ� 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// �⺻���� ������
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();
	
	glutPostWindowRedisplay(window4);
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[4]->imageData);
	
}

void Render6(void)// 6���� �� �׸���
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y�� ȸ��    
	glRotatef(yTheta+(g_lenti_angle+0.5)*2, 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling ������
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// ��ǥ ȯ�� �ϴ� �κ�
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//�Ƚ¼� holefilling �κ�
		if(g_holefilling==1){
		if(min-z < 0) // ��������ŭ Hole Filling �ϴ� �κ� 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// �⺻���� ������
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();
	
	glutPostWindowRedisplay(window5);
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[5]->imageData);
	
}

void Render7(void)// 7���� �� �׸���
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y�� ȸ��    
	glRotatef(yTheta+(g_lenti_angle+0.5)*3, 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling ������
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// ��ǥ ȯ�� �ϴ� �κ�
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//�Ƚ¼� holefilling �κ�
		if(g_holefilling==1){
		if(min-z < 0) // ��������ŭ Hole Filling �ϴ� �κ� 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// �⺻���� ������
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();

	glutPostWindowRedisplay(window6);
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[6]->imageData);
	
}

void Render8(void)// 8���� �� �׸���
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y�� ȸ��    
	glRotatef(yTheta+(g_lenti_angle+0.5)*3, 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling ������
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// ��ǥ ȯ�� �ϴ� �κ�
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//�Ƚ¼� holefilling �κ�
		if(g_holefilling==1){
		if(min-z < 0) // ��������ŭ Hole Filling �ϴ� �κ� 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// �⺻���� ������
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// ���� ��
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();

	glutPostWindowRedisplay(window7);
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[6]->imageData);
	
	lenticular();
}


void lenticular(){ //8������ alioscopy �˰��� ���ؼ� lenticular lens���� �� �� �ְ� �ϴ� �Լ�

	int image_number = 0;
	int i, j, rgb;
	int rgb_line_start_number = 0;

	for(i = 0; i < g_display_stereo->height; i++)
	{
		image_number = rgb_line_start_number;
		for(j = 0; j < g_display_stereo->width; j++)
		{
			for(rgb = 2 ; rgb >=0 ; rgb--)
			{
				g_display_stereo->imageData[i*g_display_stereo->widthStep + j*g_display_stereo->nChannels + rgb] =  
					(BYTE)g_3dmodel[image_number]->imageData[i*g_3dmodel[image_number]->widthStep + j*g_3dmodel[image_number]->nChannels + rgb];
				image_number = (image_number + 1)%8;
			}
		}
		if( rgb_line_start_number > 0 )
			rgb_line_start_number--;
		else rgb_line_start_number = 7;
	}

	iplImageToDC(g_display_stereo,GetDC(g_hWndTvDisplay),cvRect(0,0,1920,1080));
}


void SaveInfo(IplImage *Color, IplImage *Depth) //iplimage�� ��� ���� �޾ƿ��� hole filling�� ���� �ּҰ��� �޾ƿ�
{
	CvScalar value;		// Depth ������ ������ ���� ����
	CvScalar value2;	// Color ������ ������ ���� ����
	CvScalar TempVal;	// min ������ ���� �ӽ� ����
	for(int j = 0; j < Color->height; j++){
		for(int i = 0; i < Color->width; i++){
			value = cvGet2D(Depth,j,i);		// Depth ������ Value�� ����
			value2 = cvGet2D(Color,j,i);	// Color ������ Value2�� ����

			if(value.val[0] <= 255)		//Depth �̹����� Depth
			{
				Render3D[cnt].x = i;
				Render3D[cnt].y = j;
				Render3D[cnt].z = (float)value.val[0];
			}
			if(value2.val[0] <= 255 && value2.val[1] <= 255 && value2.val[2] <= 255) // Color �̹����� ���� ���� �������� �κ�
			{																		 
				Render3D[cnt].b = value2.val[0];	//Render3D[cnt].b = value.val[0]; 
				Render3D[cnt].g = value2.val[1];	//Render3D[cnt].b = value.val[0];
				Render3D[cnt].r = value2.val[2];	//Render3D[cnt].b = value.val[0]; �� �����Depth �̹����� 3D Modeling ����
			}
			if(j>1 && j< 1079 && i>1 && i < 1919)	//���� �ȼ����� �ֺ��� �ȼ� �˻��� ������ min�� �����Ѵ�.
			{	//�Ƚ¼� holefilling �κ�
				TempVal = cvGet2D(Depth,j,i);
				if(TempVal.val[0] == value.val[0]) Render3D[cnt].min = TempVal.val[0];
				TempVal = cvGet2D(Depth,j-1,i-1);
				if(TempVal.val[0] < value.val[0]) Render3D[cnt].min = TempVal.val[0];
				TempVal = cvGet2D(Depth,j-1,i);
				if(TempVal.val[0] < value.val[0]) Render3D[cnt].min = TempVal.val[0];
				TempVal = cvGet2D(Depth,j-1,i+1);
				if(TempVal.val[0] < value.val[0]) Render3D[cnt].min = TempVal.val[0];
				TempVal = cvGet2D(Depth,j,i-1);
				if(TempVal.val[0] < value.val[0]) Render3D[cnt].min = TempVal.val[0];
				TempVal = cvGet2D(Depth,j,i+1);
				if(TempVal.val[0] < value.val[0]) Render3D[cnt].min = TempVal.val[0];
				TempVal = cvGet2D(Depth,j+1,i-1);
				if(TempVal.val[0] < value.val[0]) Render3D[cnt].min = TempVal.val[0];
				TempVal = cvGet2D(Depth,j+1,i);
				if(TempVal.val[0] < value.val[0]) Render3D[cnt].min = TempVal.val[0];
				TempVal = cvGet2D(Depth,j+1,i+1);
				if(TempVal.val[0] < value.val[0]) Render3D[cnt].min = TempVal.val[0];
				//3x3 �ȼ� Ž���Ͽ� �ּҰ� �ֱ� �Ƚ¼� holefilling �κ�
			
				}
			else
			{
				Render3D[cnt].min = (float)value.val[0];
			}
			cnt++;
		}
	}
	
}

//win api ��üȭ������ ���� ���� �Լ�
DWORD __stdcall CreateStereoWindowsAPI(void* lpszClass)
{
 DWORD  exitcode; 
 HINSTANCE g_Inst;
 HWND hWnd;
 MSG Message;
 WNDCLASS WndClass;
 HINSTANCE hInstance = GetModuleHandle(NULL);
 g_Inst = hInstance;
 WndClass.cbClsExtra = 0;
 WndClass.cbWndExtra = 0;
 WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
 WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
 WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
 WndClass.hInstance = hInstance;
 WndClass.lpszClassName = (LPCWSTR)lpszClass;
 WndClass.lpszMenuName = NULL;
 WndClass.style = CS_HREDRAW|CS_VREDRAW;
 WndClass.lpfnWndProc = WndProc;
 RegisterClass(&WndClass);
 g_hWndTvDisplay = CreateWindowW((LPCWSTR)lpszClass, (LPCWSTR)lpszClass, WS_POPUP|WS_BORDER, 
  1920, 0, 1920, 1080,
  NULL, (HMENU)NULL, hInstance, NULL);
  
 
ShowWindow(g_hWndTvDisplay, SW_SHOW);
 while(GetMessage(&Message, NULL, 0, 0))
 {
  TranslateMessage(&Message);
  DispatchMessage(&Message);
 }
 GetExitCodeThread(g_hThreadTvDisplay, &exitcode);
 ExitThread(exitcode);
 CloseHandle(g_hThreadTvDisplay);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM IParam)
{
 switch(iMessage)
 {
 case WM_DESTROY:
  PostQuitMessage(0);
  return 0;
 }
 return(DefWindowProc(hWnd, iMessage, wParam, IParam));
} 

void iplImageToDC(IplImage* pImgIpl, HDC hDC, CvRect rect)  //IplImage�� dc�� �׸��� �Լ�
{
 BITMAPINFO bitmapInfo;
 bitmapInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
 bitmapInfo.bmiHeader.biPlanes=1;
 bitmapInfo.bmiHeader.biCompression=BI_RGB;
 bitmapInfo.bmiHeader.biXPelsPerMeter=100;
 bitmapInfo.bmiHeader.biYPelsPerMeter=100;
 bitmapInfo.bmiHeader.biClrUsed=0;
 bitmapInfo.bmiHeader.biClrImportant=0;
 bitmapInfo.bmiHeader.biSizeImage=0;
 bitmapInfo.bmiHeader.biWidth=pImgIpl->width;
 bitmapInfo.bmiHeader.biHeight=-pImgIpl->height;
 IplImage* tempImage;
 if(pImgIpl->nChannels == 3)
 {
  tempImage = (IplImage*)cvClone(pImgIpl);
  bitmapInfo.bmiHeader.biBitCount=tempImage->depth * tempImage->nChannels;
 }
 else if(pImgIpl->nChannels == 1)
 {
  tempImage =  cvCreateImage(cvGetSize(pImgIpl), IPL_DEPTH_8U, 3);
  cvCvtColor(pImgIpl, tempImage, CV_GRAY2BGR);
  bitmapInfo.bmiHeader.biBitCount=tempImage->depth * tempImage->nChannels;
 }
 ::SetStretchBltMode(hDC,COLORONCOLOR);
 ::StretchDIBits(hDC, rect.x, rect.y, rect.width, rect.height, 
  0, 0, tempImage->width, tempImage->height, tempImage->imageData, &bitmapInfo, 
  DIB_RGB_COLORS, SRCCOPY);
 cvReleaseImage(&tempImage);
}
//------------------------------------------------------------------

void exitProgram() //���α׷� ���� �Լ� 
{
	free(Render3D);
	cvReleaseImage(&g_color_image);
	cvReleaseImage(&g_depth_image);
	cvReleaseImage(&g_3dmodel[0]);
	cvReleaseImage(&g_3dmodel[1]);
	cvReleaseImage(&g_3dmodel[2]);
	cvReleaseImage(&g_3dmodel[3]);
	cvReleaseImage(&g_3dmodel[4]);
	cvReleaseImage(&g_3dmodel[5]);
	cvReleaseImage(&g_3dmodel[6]);
	cvReleaseImage(&g_3dmodel[7]);
	cvDestroyAllWindows();
	cvReleaseImage(&g_display_stereo);

	
	exit(0);
}



