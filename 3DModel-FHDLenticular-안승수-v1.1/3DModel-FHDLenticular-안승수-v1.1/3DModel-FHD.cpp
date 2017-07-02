#include "stdafx.h"

using namespace std;

typedef struct _position
{
	float x, y, z, b, g, r,min;
} position;
// 렌더링시 저장해야될 좌표값을 이 구조체에 저장
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
//마우스로 축회전
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
//winapi window 화면 띄우기 선언
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
	glutInit(&argc, argv); //glut 초기화 
	initProgram(); // 프로그램 전체 초기화
	
	cvSmooth(g_depth_image, g_depth_image, CV_MEDIAN,3,3); //depth image 중간값 필터로 smoothing

	Render3D = (position*)malloc(sizeof(position) * 20000000);			// Render3D 구조체 동적 할당 부분
	SaveInfo(g_color_image,g_depth_image);							// Depth image, Color Image 저장부분
	glutMainLoop();//glut rendering 및 glut 함수들을 반복 시킴 
	
}

void readSettingsFromINI() //ini 파일에서 변수 값을 받아오기 
{
	TCHAR path[512];
	GetCurrentDirectory(512,path);  //프로젝트 경로

	GetCurrentDirectory(512,path);  //프로젝트 경로
	wcscat(path,L"\\program.ini");	

	g_lenti_angle=GetPrivateProfileInt(TEXT("Angle"),TEXT("Lenti_angle"),-1,path);
}  
void initProgram() //프로그램 초기화 
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

	window1=glutCreateWindow("1시점");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);//freeglut
	//glutMouseWheelFunc(mouseWheel);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window1);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize1);
	glutDisplayFunc(Render1);
	glEnable(GL_DEPTH_TEST);// 깊이 값에 따라 모델 그려주기
	glEnable(GL_CULL_FACE);// 폴리곤 추출하기(보이지 않는면 안그리기)

	window2=glutCreateWindow("2시점");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window2);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize2);
	glutDisplayFunc(Render2);
	glEnable(GL_DEPTH_TEST);// 깊이 값에 따라 모델 그려주기
	glEnable(GL_CULL_FACE);// 폴리곤 추출하기(보이지 않는면 안그리기)

	window3=glutCreateWindow("3시점");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window3);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize3);
	glutSetWindow(window3);
	glutDisplayFunc(Render3);
	glEnable(GL_DEPTH_TEST);// 깊이 값에 따라 모델 그려주기
	glEnable(GL_CULL_FACE);// 폴리곤 추출하기(보이지 않는면 안그리기)

	window4=glutCreateWindow("4시점");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window4);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize4);
	glutDisplayFunc(Render4);
	glEnable(GL_DEPTH_TEST);// 깊이 값에 따라 모델 그려주기
	glEnable(GL_CULL_FACE);// 폴리곤 추출하기(보이지 않는면 안그리기)

	window5=glutCreateWindow("5시점");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window5);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize5);
	glutDisplayFunc(Render5);
	glEnable(GL_DEPTH_TEST);// 깊이 값에 따라 모델 그려주기
	glEnable(GL_CULL_FACE);// 폴리곤 추출하기(보이지 않는면 안그리기)

	window6=glutCreateWindow("6시점");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window6);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize6);
	glutDisplayFunc(Render6);
	glEnable(GL_DEPTH_TEST);// 깊이 값에 따라 모델 그려주기
	glEnable(GL_CULL_FACE);// 폴리곤 추출하기(보이지 않는면 안그리기)

	window7=glutCreateWindow("7시점");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window7);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize7);
	glutDisplayFunc(Render7);
	glEnable(GL_DEPTH_TEST);// 깊이 값에 따라 모델 그려주기
	glEnable(GL_CULL_FACE);// 폴리곤 추출하기(보이지 않는면 안그리기)

	window8=glutCreateWindow("8시점");
	glutFullScreen();
	glutMouseFunc(mouseButtonfunc);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(window8);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize8);
	glutDisplayFunc(Render8);
	glEnable(GL_DEPTH_TEST);// 깊이 값에 따라 모델 그려주기
	glEnable(GL_CULL_FACE);// 폴리곤 추출하기(보이지 않는면 안그리기)


	g_hThreadTvDisplay=CreateThread(NULL,0,CreateStereoWindowsAPI, L"TVstereo",0,&g_IDThreadTvDisplay);
	//출력화면에 전체화면으로 띄우기 위해서 선언 
	
}

void myMotion(int x, int y)//모델 움직일 때 필요한 함수 특히 마우스로 제어할 때
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

void mouseButtonfunc(int button, int state, int x, int y)// 마우스 버튼 관련 함수
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
void Key(int key, int x, int y){ // 키보드 이벤트

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

void init(void)//modeldml x,y,z 축 초기화
{
	
	rotate_x = 1;
	rotate_y = 0;
	rotate_z = 0;

	direction = 1;

	rt = false;

	theta = 0.0f;

}

void ChangeSize1(int w, int h) // 1 시점 window 설정
{
	glutSetWindow(window1);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ChangeSize2(int w, int h) // 2 시점 window 설정
{
	glutSetWindow(window2);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ChangeSize3(int w, int h) // 3 시점 window 설정
{
	glutSetWindow(window3);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ChangeSize4(int w, int h) // 4 시점 window 설정
{
	glutSetWindow(window4);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ChangeSize5(int w, int h) // 5 시점 window 설정
{
	glutSetWindow(window5);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ChangeSize6(int w, int h) // 6 시점 window 설정
{
	glutSetWindow(window6);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ChangeSize7(int w, int h) // 7 시점 window 설정
{
	glutSetWindow(window7);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ChangeSize8(int w, int h) // 8 시점 window 설정
{
	glutSetWindow(window8);
	glViewport(0, 0, 1920, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-960, 960, -540, 540, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

////////////////////////3D모델링 해주는 부분
void Render1(void)// 1시점 모델 그리기
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y축 회전    
	glRotatef(yTheta-(g_lenti_angle+0.5)*3, 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling 미적용
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// 좌표 환산 하는 부분
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//안승수 holefilling 부분
		if(g_holefilling==1){
		if(min-z < 0) // 최저값만큼 Hole Filling 하는 부분 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// 기본적인 랜더링
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();
	
	glutPostWindowRedisplay(window8);
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[0]->imageData);

}

void Render2(void)// 2시점 모델 그리기
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y축 회전    
	glRotatef(yTheta-(g_lenti_angle+0.5)*2, 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling 미적용
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// 좌표 환산 하는 부분
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//안승수 holefilling 부분
		if(g_holefilling==1){
		if(min-z < 0) // 최저값만큼 Hole Filling 하는 부분 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// 기본적인 랜더링
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();

	glutPostWindowRedisplay(window1);
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[1]->imageData);
	
}

void Render3(void)// 3시점 모델 그리기
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y축 회전    
	glRotatef(yTheta-(g_lenti_angle+0.5), 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling 미적용
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// 좌표 환산 하는 부분
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//안승수 holefilling 부분
		if(g_holefilling==1){
		if(min-z < 0) // 최저값만큼 Hole Filling 하는 부분 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// 기본적인 랜더링
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();

	glutPostWindowRedisplay(window2);	
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[2]->imageData);
	
}

void Render4(void)// 4시점 모델 그리기
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y축 회전    
	glRotatef(yTheta, 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling 미적용
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// 좌표 환산 하는 부분
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//안승수 holefilling 부분
		if(g_holefilling==1){
		if(min-z < 0) // 최저값만큼 Hole Filling 하는 부분 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// 기본적인 랜더링
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();

	glutPostWindowRedisplay(window3);
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[3]->imageData);
	
}

void Render5(void)// 5시점 모델 그리기
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y축 회전    
	glRotatef(yTheta+(g_lenti_angle+0.5), 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling 미적용
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// 좌표 환산 하는 부분
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//안승수 holefilling 부분
		if(g_holefilling==1){
		if(min-z < 0) // 최저값만큼 Hole Filling 하는 부분 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// 기본적인 랜더링
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();
	
	glutPostWindowRedisplay(window4);
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[4]->imageData);
	
}

void Render6(void)// 6시점 모델 그리기
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y축 회전    
	glRotatef(yTheta+(g_lenti_angle+0.5)*2, 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling 미적용
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// 좌표 환산 하는 부분
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//안승수 holefilling 부분
		if(g_holefilling==1){
		if(min-z < 0) // 최저값만큼 Hole Filling 하는 부분 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// 기본적인 랜더링
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();
	
	glutPostWindowRedisplay(window5);
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[5]->imageData);
	
}

void Render7(void)// 7시점 모델 그리기
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y축 회전    
	glRotatef(yTheta+(g_lenti_angle+0.5)*3, 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling 미적용
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// 좌표 환산 하는 부분
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//안승수 holefilling 부분
		if(g_holefilling==1){
		if(min-z < 0) // 최저값만큼 Hole Filling 하는 부분 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// 기본적인 랜더링
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();

	glutPostWindowRedisplay(window6);
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[6]->imageData);
	
}

void Render8(void)// 8시점 모델 그리기
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y축 회전    
	glRotatef(yTheta+(g_lenti_angle+0.5)*3, 0.0, 1.0, 0.0);

    if(g_holefilling==0){//holefilling 미적용
		glPointSize(1);
	}
	else {
		glPointSize(3);
	}
	glBegin(GL_POINTS);
	for(int i =0 ; i < cnt; i++)
	{

		float x, y, z, b, g, r, min; 

		// 좌표 환산 하는 부분
		x = Render3D[i].x-960;
		y = (Render3D[i].y-540);
		z = (Render3D[i].z-255);

		b = Render3D[i].b/255.0;
		g = Render3D[i].g/255.0;
		r = Render3D[i].r/255.0; 
		min = (Render3D[i].min-255);

		//안승수 holefilling 부분
		if(g_holefilling==1){
		if(min-z < 0) // 최저값만큼 Hole Filling 하는 부분 
		{
			for(int j = z; j > min; j--)
			{
				if(g_colorenalbe==1){
				glColor3f(r, g, b);//r,g,b
				}
				else{
				glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
				}
				glVertex3f(x, y, j);
			}
		}
		}
		
		// 기본적인 랜더링
		if(g_colorenalbe==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();

	glutPostWindowRedisplay(window7);
	glReadPixels(0,0,1920,1080,GL_BGR_EXT,GL_UNSIGNED_BYTE,g_3dmodel[6]->imageData);
	
	lenticular();
}


void lenticular(){ //8시점을 alioscopy 알고리즘 통해서 lenticular lens에서 볼 수 있게 하는 함수

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


void SaveInfo(IplImage *Color, IplImage *Depth) //iplimage의 출력 값을 받아오고 hole filling을 위한 최소값을 받아옴
{
	CvScalar value;		// Depth 데이터 저장을 위한 변수
	CvScalar value2;	// Color 데이터 저장을 위한 변수
	CvScalar TempVal;	// min 저장을 위한 임시 변수
	for(int j = 0; j < Color->height; j++){
		for(int i = 0; i < Color->width; i++){
			value = cvGet2D(Depth,j,i);		// Depth 데이터 Value에 저장
			value2 = cvGet2D(Color,j,i);	// Color 데이터 Value2에 저장

			if(value.val[0] <= 255)		//Depth 이미지의 Depth
			{
				Render3D[cnt].x = i;
				Render3D[cnt].y = j;
				Render3D[cnt].z = (float)value.val[0];
			}
			if(value2.val[0] <= 255 && value2.val[1] <= 255 && value2.val[2] <= 255) // Color 이미지의 색상 값을 가져오는 부분
			{																		 
				Render3D[cnt].b = value2.val[0];	//Render3D[cnt].b = value.val[0]; 
				Render3D[cnt].g = value2.val[1];	//Render3D[cnt].b = value.val[0];
				Render3D[cnt].r = value2.val[2];	//Render3D[cnt].b = value.val[0]; 로 변경시Depth 이미지의 3D Modeling 가능
			}
			if(j>1 && j< 1079 && i>1 && i < 1919)	//기준 픽셀에서 주변부 픽셀 검색후 최저값 min을 저장한다.
			{	//안승수 holefilling 부분
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
				//3x3 픽셀 탐색하여 최소값 넣기 안승수 holefilling 부분
			
				}
			else
			{
				Render3D[cnt].min = (float)value.val[0];
			}
			cnt++;
		}
	}
	
}

//win api 전체화면으로 띄우기 위한 함수
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

void iplImageToDC(IplImage* pImgIpl, HDC hDC, CvRect rect)  //IplImage를 dc에 그리는 함수
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

void exitProgram() //프로그램 종료 함수 
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



