#include "stdafx.h"

using namespace std;

typedef struct _position
{
	float x, y, z, b, g, r,min;
} position;// 렌더링시 저장해야될 좌표값을 이 구조체에 저장

position *Render3D;
int cnt = 0;


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

int g_Transmin; // 18
int g_colorenable=1;
int g_holefilling=1;
int g_optitrack=0;


IplImage *g_color_image;
IplImage *g_color_image_half;
IplImage *g_depth_image;
IplImage *g_depth_image_half;
IplImage* g_bilateral;
IplImage* g_depthresize;
IplImage* g_display;
IplImage* g_display_left;
IplImage* g_display_right;
IplImage* g_displayzoomleft;
IplImage* g_displayzoomright;
IplImage* g_display_stereo;

int mainwindow;
int windowleft;
int windowright;

char str_fps[20]="0.00fps";
////////////////////////////
//optitrack 선언
using namespace CameraLibrary;

Camera *camera  ;
int cameraWidth ;
int cameraHeight;
IplImage* frame ;
IplImage* frame3c;
IplImage* showopti;
///////////////////////////
//opti_cv글자 띄우기
char str_model_y[50]="3d model y_axis 0";
char str_obj1x[50]="1 x 0.00";
char str_obj1y[50]="1 y 0.00";
char str_obj2x[50]="2 x 0.00";
char str_obj2y[50]="2 y 0.00";
//

#define GET2D8U(IMAGE,X,Y)	(*( ( (uchar*)( ( (IMAGE) -> imageData ) + (Y) * ( (IMAGE) -> widthStep ) ) ) + (X) ))
#define GET2D8U3CH(IMAGE,X,Y) ( ( (uchar*)( ( (IMAGE) -> imageData ) + (Y) * ( (IMAGE) -> widthStep ) ) ) + ( 3 * (X) ) )
//use :  GET2D8U3CH(IMAGE,X,Y)[0], GET2D8U3CH(IMAGE,X,Y)[1], GET2D8U3CH(IMAGE,X,Y)[2]
#define GET2D16U(IMAGE,X,Y)	(*( ( (ushort*)( ( (IMAGE) -> imageData ) + (Y) * ( (IMAGE) -> widthStep ) ) ) + (X) ))
#define GET2D16S(IMAGE,X,Y)	(*( ( (short*)( ( (IMAGE) -> imageData ) + (Y) * ( (IMAGE) -> widthStep ) ) ) + (X) ))
#define GET2D32F(IMAGE,X,Y) (*( ( (float*)( ( (IMAGE) -> imageData ) + (Y) * ( (IMAGE) -> widthStep ) ) ) + (X) ))

void initProgram();
void readSettingsFromINI();
void exitProgram();
void exitCamera();
void SetupRC(void);
void init(void);
void ChangeSize(int w, int h);
void ChangeSize2(int w, int h);
void Renderleft(void);
void Renderright(void);
void mouseButtonfunc(int button, int state, int x, int y);
void myMotion(int x, int y);
void SaveInfo(IplImage *Color, IplImage *Depth);
void ChangeSize(int w, int h);
void ChangeSize2(int w, int h);
void Renderleft(void);
void Renderright(void);
void Key(int key, int x, int y);
void mouseWheel(int button, int dir, int x, int y);
void timer(int value);
int ttt;
void main(int argc, char* argv[])
{  
	CameraLibrary_EnableDevelopment();
	CameraManager::X().WaitForInitialization();
	camera  = CameraManager::X().GetCamera();


	cameraWidth = camera->Width();
	cameraHeight = camera->Height();
	frame =cvCreateImage(cvSize(cameraWidth,cameraHeight),8,1);
	
	
	glutInit(&argc, argv);

	initProgram();

	cvResize(g_depth_image,g_depth_image_half,CV_INTER_CUBIC);//resize부분
	cvResize(g_color_image,g_color_image_half,CV_INTER_CUBIC);//resize부분

	cvSmooth(g_depth_image_half, g_depth_image_half, CV_MEDIAN,3,3);

	//ttt=GetTickCount();//걸리는 시간 재기 단위는 millisec
	Render3D = (position*)malloc(sizeof(position) * 1036800);			// Render3D 구조체 동적 할당 부분
	SaveInfo(g_color_image_half,g_depth_image_half);					// Depth image, Color Image 저장부분
	glutMainLoop();

}

void readSettingsFromINI() // ini 파일에서 정보 읽어오는 함수 
{
	TCHAR path[512];

	GetCurrentDirectory(512,path);  //프로젝트 경로
	wcscat(path,L"\\program.ini");	

	g_Transmin=GetPrivateProfileInt(TEXT("Ytheta"),TEXT("Difference"),-1,path);
}  
void initProgram() //프로그램 초기화 함수
{
	readSettingsFromINI();
	//initCamera();
	g_color_image = cvLoadImage("color.bmp",3);
	g_depth_image = cvLoadImage("Depth Image fix3.bmp",CV_LOAD_IMAGE_UNCHANGED);
	g_color_image_half=cvCreateImage(cvSize(960,1080),8,3);
	g_depth_image_half=cvCreateImage(cvSize(960,1080),8,3);
	g_bilateral=cvCreateImage(cvSize(1920,1080),8,3);
	g_display_left=cvCreateImage(cvSize(960,1080),8,3);
	g_display_right=cvCreateImage(cvSize(960,1080),8,3);
	g_display_stereo=cvCreateImage(cvSize(1920,1080),8,3);
	g_displayzoomleft=cvCreateImage(cvSize(960,1080),8,3);
	g_displayzoomright=cvCreateImage(cvSize(960,1080),8,3);
	
	cvNamedWindow("Control",1);//trackbar 관련 설정 
	cvResizeWindow("Control",800,230);
	
	cvCreateTrackbar("Difference","Control",&g_Transmin,100,0);
	cvCreateTrackbar("Color","Control",&g_colorenable,1,0);
	cvCreateTrackbar("Filling","Control",&g_holefilling,1,0);
	cvCreateTrackbar("Optitrack","Control",&g_optitrack,1,0);//------------

	CameraLibrary_EnableDevelopment();//optitrack 관련 초기화
	CameraManager::X().WaitForInitialization();
	camera->SetVideoType(SegmentMode);
	camera->SetFrameRate(120);
	camera->SetExposure(40);
	camera->SetThreshold(150);
	camera->SetIntensity(35);
	camera->Start();

	frame=cvCreateImage(cvSize(cameraWidth,cameraHeight),8,1);
	frame3c=cvCreateImage(cvSize(cameraWidth,cameraHeight),8,3);
    showopti=cvCreateImage(cvSize(400,300),8,3);///-------------------------------------

	
	init();//opengl
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB );
	glutInitWindowPosition(1920,0);
	glutInitWindowSize(1920,1080);
	mainwindow=glutCreateWindow("main");
	glutFullScreen();

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB ); //왼쪽 glutsub window 관련 설정
	glutInitWindowPosition(0,0);
	glutInitWindowSize(960,1080);
	windowleft=glutCreateSubWindow(mainwindow,0,0,960,1080);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutMouseFunc(mouseButtonfunc);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(windowleft);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(Renderleft);
	glEnable(GL_DEPTH_TEST);//깊이 따라서 모델 배열 및 그려주기
	glEnable(GL_CULL_FACE);// 폴리곤 추출하기(보이지 않는면 안그리기)

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB ); //오른쪽 glutsub window 관련 설정
	glutInitWindowPosition(960,0);
	glutInitWindowSize(960,1080);
	windowright=glutCreateSubWindow(mainwindow,960,0,960,1080);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutMouseFunc(mouseButtonfunc);//freeglut
	glutMotionFunc(myMotion); 
	glutSpecialFunc(Key);
	glutSetWindow(windowright);
	glLoadIdentity();
	glutReshapeFunc(ChangeSize2);
	glutDisplayFunc(Renderright);
	glEnable(GL_DEPTH_TEST);//깊이 따라서 모델 배열 및 그려주기
	glEnable(GL_CULL_FACE);// 폴리곤 추출하기(보이지 않는면 안그리기)//------------
	
}

void timer(int value){
	if(g_optitrack==0){   //optitrack----------------------------------------------------------
		//optitrack 비활성화 하였을 때
	}
	else{//optitrack 활성화 하였을 때
		Frame *raw_frame = camera->GetLatestFrame();
		if(raw_frame)
		{
			int optivalue[10]={0};
			int optiyval[10]={0};
			int cntcnt=0;
			int midxvalue=0;
			int ytheta3d=0;
			raw_frame->Rasterize(cameraWidth,cameraHeight,cameraWidth,8,frame->imageData);

			cvMerge(frame,frame,frame,NULL,frame3c);
	

			for(int i=0;i<raw_frame->ObjectCount();i++)
			{
				cObject *obj=raw_frame->Object(i);
				if(obj->Radius()>4){

					optivalue[cntcnt]=obj->X();
					optiyval[cntcnt]=obj->Y();
					
				
					cvDrawCircle(frame3c, cvPoint(obj->X(),obj->Y()), obj->Radius(),CV_RGB(255,255,255),30);
					
					
					cntcnt++;
				}

			}
			for(int i=0; i<cntcnt;i++ ){
			
				midxvalue= (midxvalue + optivalue[i])/(i+1);
			}

			if(midxvalue>0){
				yTheta=(midxvalue/10)-70;
				ytheta3d=yTheta;
			}	

			cvResize(frame3c,showopti,CV_INTER_CUBIC);

			sprintf_s(str_model_y,"3d model y_axis %d",ytheta3d);

			sprintf_s(str_obj1x,"1_x %d",optivalue[0]);
			sprintf_s(str_obj1y,"1_y %d",optiyval[0]);
			sprintf_s(str_obj2x,"2_x %d",optivalue[1]);
			sprintf_s(str_obj2y,"2_y %d",optiyval[1]);

			cvPutText(showopti,str_model_y,cvPoint(10,20),&cvFont(1,1),CV_RGB(255, 255, 255));

			cvPutText(showopti,str_obj1x,cvPoint(10,35),&cvFont(1,1),CV_RGB(255, 255, 255));
			cvPutText(showopti,str_obj1y,cvPoint(10,50),&cvFont(1,1),CV_RGB(255, 255, 255));
			cvPutText(showopti,str_obj2x,cvPoint(10,65),&cvFont(1,1),CV_RGB(255, 255, 255));
			cvPutText(showopti,str_obj2y,cvPoint(10,80),&cvFont(1,1),CV_RGB(255, 255, 255));

			cvShowImage("사용자 위치",showopti);

			raw_frame->Release();

		}
		else{
			yTheta=yTheta;
		}
	}//------------------------------------------------------------------------------------//

	glutTimerFunc(1000/30, timer, 1);
}
void myMotion(int x, int y) //모델 움직일 때 필요한 함수 특히 마우스로 제어할 때
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


void init(void) //opengl window창 초기화 
{
	rotate_x = 1;
	rotate_y = 0;
	rotate_z = 0;

	direction = 1;

	rt = false;

	theta = 0.0f;

	glutTimerFunc(1000/30, timer, 1);
}

void ChangeSize(int w, int h) //왼쪽 glutsubwindow 설정
{
	glutSetWindow(windowleft);
	glViewport(0, 0, 960, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-480.0, 480.0, -540.0, 540.0, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ChangeSize2(int w, int h)//오른쪽 glutsubwindow 설정
{
	glutSetWindow(windowright);
	glViewport(0, 0, 960, 1080);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	
	glOrtho(-480.0, 480.0, -540.0, 540.0, -100000.0, 10000.0);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Renderleft(void) //좌측 glut subwindow 그려주는 부분
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);//y축 회전    
	glRotatef(yTheta-g_Transmin*0.1, 0.0, 1.0, 0.0);

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
		x = Render3D[i].x -480;
		y = -(Render3D[i].y-540);
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
				if(g_colorenable==1){
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
		if(g_colorenable==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();
	//printf("makeing point time: %d\n",GetTickCount()-ttt);
	//ttt=GetTickCount();
	glutPostWindowRedisplay(windowright);//오른쪽 glutsubwindow 다시 그리기
}

void Renderright(void) //오른쪽 glut subwindow 그려주는 부분
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
	
	glRotatef(xTheta, 1.0, 0.0, 0.0);    
	glRotatef(yTheta+g_Transmin*0.1, 0.0, 1.0, 0.0);

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
		x = Render3D[i].x -480;
		y = -(Render3D[i].y-540);
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
				if(g_colorenable==1){
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
		if(g_colorenable==1){
		glColor3f(r, g, b);//r,g,b
		}
		else{
		glColor3f(Render3D[i].z/255,Render3D[i].z/255,Render3D[i].z/255);// 깊이 값
		}
		glVertex3f(x, y, z);

	}
	glEnd();
	glutSwapBuffers();
	
	glutPostWindowRedisplay(windowleft);// 왼쪽 glutsubwindow 다시 그리기
}

void SaveInfo(IplImage *Color, IplImage *Depth)//iplimage에서 좌표 추출해 구조체 배열에 저장
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
				Render3D[cnt].b = value2.val[0];	 
				Render3D[cnt].g = value2.val[1];	
				Render3D[cnt].r = value2.val[2];	
			 
			}
			if(g_holefilling==1){
			if(j>1 && j< 1079 && i>1 && i < 959)	//기준 픽셀에서 주변부 픽셀 검색후 최저값 min을 저장한다.
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
			}
			cnt++;
		}
	}
}

void exitProgram()
{
	free(Render3D);
	cvReleaseImage(&g_color_image);
	cvReleaseImage(&g_depth_image);
	cvReleaseImage(&g_depth_image_half);
	cvReleaseImage(&g_color_image_half);
	cvDestroyAllWindows();
	cvReleaseImage(&g_display);
	cvReleaseImage(&g_displayzoomleft);
	cvReleaseImage(&g_displayzoomright);
	cvReleaseImage(&g_display_left);
	cvReleaseImage(&g_display_right);
	cvReleaseImage(&g_display_stereo);
	cvReleaseImage(&g_bilateral);
	exitCamera();
	
	exit(0);
}

void exitCamera()
{	
	cvReleaseImage(&frame);//optitrack
	cvReleaseImage(&frame3c);
	cvReleaseImage(&showopti);
	camera->Stop();
	camera->Release();
	CameraManager::X().Shutdown();////////////
}

