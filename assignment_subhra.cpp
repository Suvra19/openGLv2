#include <iostream>
#include <fstream>
#include <math.h>
#include <cmath>
#include <GL/freeglut.h>
#include "loadBMP.h"
#include "loadTGA.h"
#define GL_CLAMP_TO_EDGE 0x812F

using namespace std;

GLUquadric *q; 
float angle = 0, eye_x = 0, eye_z = 30, ref_x = 0, ref_z = 0, rot_angle = 0;
float horseAngle = 30, humanAngle = 20, walkAngle = 0, flyAngle = 360, wingAngle1 = 100, wingAngle2 = 80, human_cycle = 0;
float cdr = 3.14159265 / 180.0, flightHeight = 2;
float lpos[4] = { 0., 10, 10., 1.0 };  //light 1's position
float lpos_2[4] = { 0.0, -10, -10, 1.0 }; //light 2's position
GLuint txId[11]; 
const int N = 16;
bool isAntiClockWise = true, switchCamera = false;

float vx[N] = {0, 1, 0.75, 0.5, 0.25, 0.25, 0.25, 0.25, 0.5, 1, 1.5, 1.75, 2, 2, 2, 1.8};
float vy[N] = {0, 0, 0.25, 0.75, 1, 2, 3, 4, 5, 5.5, 6, 6.5, 7, 7.5, 7.75, 8};
float vz[N] = {0};

void humanCycleTimer(int value) {
	if (isAntiClockWise) {
		human_cycle = fmod((human_cycle + 1), 360);
	}
	else {
		human_cycle = fmod((human_cycle - 1), 360);
	}
	
	glutPostRedisplay();
	glutTimerFunc(500, humanCycleTimer, 0);
}

void humanWalkTimer(int value) {
	humanAngle = -humanAngle;
	glutPostRedisplay();
	glutTimerFunc(500, humanWalkTimer, 0);
}

void rotateTimer(int value) {
	rot_angle = fmod((rot_angle + 2), 360);
	glutPostRedisplay();
	glutTimerFunc(50, rotateTimer, 0);
}

void walkTimer(int value) {
	walkAngle = walkAngle + 4;
	if (walkAngle >= 360) {
		walkAngle = 0;
	}
	horseAngle = -horseAngle;
	glutPostRedisplay();
	glutTimerFunc(500, walkTimer, 0);
}

void flyTimer(int value) {
	flyAngle = flyAngle - 10;
	if (flyAngle <= 0) {
		flyAngle = 360;
	}
	glutPostRedisplay();
	glutTimerFunc(200, flyTimer, 0);
}

void wingTimer(int value) {
	if (wingAngle1 == 100 && wingAngle2 == 80) {
		wingAngle1 = 80;
		wingAngle2 = 100;
	} else if (wingAngle1 == 80 && wingAngle2 == 100) {
		wingAngle1 = 100;
		wingAngle2 = 80;
	}
	glutPostRedisplay();
	glutTimerFunc(150, wingTimer, 0);
}

void base()
{
    glColor4f(0.2, 0.2, 0.2, 1.0);   
    glPushMatrix();
      glTranslatef(0.0, 3.75, 0.0);
      glScalef(5.0, 0.5, 2.5);    
      glutSolidCube(1.0);
    glPopMatrix();

    //Wheels
    glColor4f(0.5, 0., 0., 1.0);
    glPushMatrix();
      glTranslatef(-2, 3.5, 1.2);
      gluDisk(q, 0.0, 0.5, 12, 2);
    glPopMatrix();
    glPushMatrix();
      glTranslatef(1.5, 3.5, 1.2);
      gluDisk(q, 0.0, 0.5, 12, 2);
    glPopMatrix();
    glPushMatrix();
      glTranslatef(-2, 3.5, -1.2);
      glRotatef(180.0, 0., 1., 0.);
      gluDisk(q, 0.0, 0.5, 12, 2);
    glPopMatrix();
    glPushMatrix();
      glTranslatef(1.5, 3.5, -1.2);
      glRotatef(180.0, 0., 1., 0.);
      gluDisk(q, 0.0, 0.5, 12, 2);
    glPopMatrix();
}

void car()
{
    base();

    glColor4f(0.8, 0.8, 0.0, 1.0);
    glPushMatrix();
      glTranslatef(0.0, 4.25, 0.0);
      glScalef(5.0, 0.5, 2.5);
      glutSolidCube(1.0);
    glPopMatrix();
    
    glPushMatrix();
	  glColor4f(1.0, 0.0, 0.0, 1.0);
      glTranslatef(-2.55, 4.5, -0.5);
      glRotatef(90, 0., 1., 0.);
      gluDisk(q, 0.0, 0.25, 20,2);
    glPopMatrix();
    
    glPushMatrix();
	  glColor4f(1.0, 0.0, 0.0, 1.0);
      glTranslatef(-2.55, 4.5, 0.5);
      glRotatef(90, 0., 1., 0.);
      gluDisk(q, 0.0, 0.25, 20,2);
    glPopMatrix();
    
    glPushMatrix();
      glColor4f(0.0, 1.0, 0.0, 1.0);
      glTranslatef(0, 4.5, -1.0);
      glRotatef(0, 0., 1., 0.);
      gluCylinder(q, 1, 1, 2, 20, 5);
    glPopMatrix();
}

void normal(float x1, float y1, float z1, 
            float x2, float y2, float z2,
		      float x3, float y3, float z3 )
{
	  float nx, ny, nz;
	  nx = y1*(z2-z3)+ y2*(z3-z1)+ y3*(z1-z2);
	  ny = z1*(x2-x3)+ z2*(x3-x1)+ z3*(x1-x2);
	  nz = x1*(y2-y3)+ x2*(y3-y1)+ x3*(y1-y2);

      glNormal3f(nx, ny, nz);
}

void drawHumanoid(bool isShadow) 
{
	if (!isShadow) {
		glColor3f(1., 0.78, 0.06);
	}
	else {
		glColor4f(0.2, 0.2, 0.2, 1.0);
	}
	//Head
	glPushMatrix();
	glTranslatef(0, 7.7, 0);
	glutSolidCube(1.4);
	glPopMatrix();

	if (!isShadow) glColor3f(1., 0., 0.);
	//Torso
	glPushMatrix();
	glTranslatef(0, 5.5, 0);
	glScalef(3, 3, 1.4);
	glutSolidCube(1);
	glPopMatrix();

	if (!isShadow) glColor3f(0., 0., 1.);
	//Right leg
	glPushMatrix();
	glTranslatef(-0.8, 4, 0);
	glRotatef(-humanAngle, 1, 0, 0);
	glTranslatef(0.8, -4, 0);
	glTranslatef(-0.8, 2.2, 0);
	glScalef(1, 4.4, 1);
	glutSolidCube(1);
	glPopMatrix();

	if (!isShadow) glColor3f(0., 0., 1.);
	//Left leg
	glPushMatrix();
	glTranslatef(0.8, 4, 0);
	glRotatef(humanAngle, 1, 0, 0);
	glTranslatef(-0.8, -4, 0);
	glTranslatef(0.8, 2.2, 0);
	glScalef(1, 4.4, 1);
	glutSolidCube(1);
	glPopMatrix();

	if (!isShadow) glColor3f(0., 0., 1.);
	//Right arm
	glPushMatrix();
	glTranslatef(-2, 6.5, 0);
	glRotatef(humanAngle, 1, 0, 0);
	glTranslatef(2, -6.5, 0);
	glTranslatef(-2, 5, 0);
	glScalef(1, 4, 1);
	glutSolidCube(1);
	glPopMatrix();

	if (!isShadow) glColor3f(0., 0., 1.);
	//Left arm
	glPushMatrix();
	glTranslatef(2, 6.5, 0);
	glRotatef(-humanAngle, 1, 0, 0);
	glTranslatef(-2, -6.5, 0);
	glTranslatef(2, 5, 0);
	glScalef(1, 4, 1);
	glutSolidCube(1);
	glPopMatrix();
}

void drawGuard(int guardNumber)
{	
	float guardAngle = 90;
	float zPosition = 15;
	if (guardNumber == 2) {
		guardAngle = -guardAngle;
		zPosition = -20;
	}
	float shadowMatrix[16] = { lpos[1],0,0,0,-lpos[0],0,-lpos[2],-1,0,0,lpos[1],0,0,0,0,lpos[1] };
	glDisable(GL_LIGHTING);
	glPushMatrix();
		glMultMatrixf(shadowMatrix);
		glColor4f(0.2, 0.2, 0.2, 1.0);
		glRotatef(guardAngle, 0, 1, 0);
		glScalef(0.1, 0.1, 0.1);
		glTranslatef(0, 0, zPosition);
		drawHumanoid(true); 
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glPushMatrix(); 
		glRotatef(guardAngle, 0, 1, 0);
		glScalef(0.1, 0.1, 0.1);
		glTranslatef(0, 0, zPosition);
		drawHumanoid(false);
	glPopMatrix();
}

void drawWalkingGaurd(int guard)
{
	glPushMatrix();
		glRotatef(human_cycle, 0, 1, 0);
		glTranslatef(0, -0.5, 17);
		drawGuard(guard);
	glPopMatrix();
}

void drawVase() {
	float wx[N], wy[N], wz[N]; 
	float angStep = 10.0*3.1415926/180.0;
	glBindTexture(GL_TEXTURE_2D, txId[10]);
	for(int j=0; j<36;j++) {
		glBegin(GL_TRIANGLE_STRIP);	
		for (int i=0;i<N;i++) {		
			wx[i] = vx[i]*cos(angStep) + vz[i]*sin(angStep);
			wy[i] = vy[i];
			wz[i] = -1*vx[i]*sin(angStep) + vz[i]*cos(angStep);
			if(i>0) normal(wx[i-1],wy[i-1],wz[i-1], vx[i-1],vy[i-1],vz[i-1], vx[i],vy[i],vz[i]);
			if(j==0) glTexCoord2f(0.0, i/(float)N);
			else glTexCoord2f(j/36.0, i/(float)N); 
			glVertex3f(vx[i],vy[i],vz[i]);
			if(i>0) normal(wx[i-1],wy[i-1],wz[i-1], vx[i],vy[i],vz[i], wx[i],wy[i],wz[i]);
			glTexCoord2f((j+1)/36.0, i/(float)N);	  
			glVertex3f(wx[i],wy[i],wz[i]);		
		}
	    glEnd();		
		for (int i = 0; i < N; i++) {
			vx[i] = wx[i];
			vy[i] = wy[i];
			vz[i] = wz[i];	
		}	
	}	
}

void drawVehicle() {
   glPushMatrix();
	   glRotatef(45, 0.0, 1.0, 0.0);
       glTranslatef(3, -4, 1);  
	   car();
   glPopMatrix();  
}

//-- HORSE ------------------------------------------------
void drawHorse()
{
	glColor3f(1., 0.78, 0.06);		//BODY
	glPushMatrix();
	glTranslatef(0, 1, 0);
	glScalef(0.8, 0.4, 0.5);
	glutSolidCube(1);
	glPopMatrix();

	glColor3f(1., 0, 0);		//HEAD
	glPushMatrix();
	glTranslatef(-0.45, 1.25, 0);
	glRotatef(-45, 0, 0, 1);
	glScalef(2, 0.5, 0.5);
	glutSolidCube(0.2);
	glPopMatrix();

	glColor3f(1., 1., 1.);			//FACE
	glPushMatrix();
	glTranslatef(-0.5, 1.3, 0);
	glRotatef(70, 1, -1, 0);
	glutSolidCone(0.1, 0.3, 20, 20);
	glPopMatrix();
	
	glColor3f(0., 0., 1.);			//Hind leg
	glPushMatrix();
	glTranslatef(0.25, 0.7, 0.31);
	glRotatef(horseAngle, 0, 0, 1);
	glTranslatef(-0.25, -0.7, -0.31);
	glTranslatef(0.25, 0.65, 0.31);
	glScalef(0.2, 1, 0.2);
	glutSolidCube(0.5);
	glPopMatrix();

	glColor3f(0., 0., 1.);			//Hind leg
	glPushMatrix();
	glTranslatef(0.25, 0.7, -0.31);
	glRotatef(-horseAngle, 0, 0, 1);
	glTranslatef(-0.25, -0.7, 0.31);
	glTranslatef(0.25, 0.65, -0.31);
	glScalef(0.2, 1, 0.2);
	glutSolidCube(0.5);
	glPopMatrix();

	glColor3f(0., 0., 1.);			//Front leg
	glPushMatrix();
	glTranslatef(-0.25, 0.7, -0.31);
	glRotatef(horseAngle, 0, 0, 1);
	glTranslatef(0.25, -0.7, 0.31);
	glTranslatef(-0.25, 0.65, -0.31);
	glScalef(0.2, 1, 0.2);
	glutSolidCube(0.5);
	glPopMatrix();

	glColor3f(0., 0., 1.);			//Front leg
	glPushMatrix();
	glTranslatef(-0.25, 0.7, 0.31);
	glRotatef(-horseAngle, 0, 0, 1);
	glTranslatef(0.25, -0.7, -0.31);
	glTranslatef(-0.25, 0.65, 0.31);
	glScalef(0.2, 1, 0.2);
	glutSolidCube(0.5);
	glPopMatrix();

	glColor3f(1., 0, 0);		//HEAD
	glPushMatrix();
	glTranslatef(0.45, 1, 0);
	glRotatef(-45, 0, 0, 1);
	glScalef(2, 0.5, 0.5);
	glutSolidCube(0.2);
	glPopMatrix();
}

void drawCart() {
	glColor4f(0.2, 0.2, 0.2, 1.0); 
	glPushMatrix();
	glTranslatef(0.0, -0.6, 0.0);
	glScalef(5.0, 0.25, 2.5);    
	glutSolidCube(0.25);
	glPopMatrix();

	glColor3f(0., 0., 0.);			//Connectors
	glPushMatrix();
	glRotatef(-7, 0, 0, 1);
	glTranslatef(-1.2, -0.6, 0.3);
	glScalef(6, 0.2, 0.2);
	glutSolidCube(0.25);
	glPopMatrix();

	glColor3f(0., 0., 0.);			//Connectors
	glPushMatrix();
	glRotatef(-7, 0, 0, 1);
	glTranslatef(-1.2, -0.6, -0.3);
	glScalef(6, 0.2, 0.2);
	glutSolidCube(0.25);
	glPopMatrix();

	glColor3f(0., 0., 0.);			//Connectors
	glPushMatrix();
	glRotatef(0, 0, 0, 1);
	glTranslatef(-2, -0.35, 0);
	glScalef(0.2, 0.2, 3);
	glutSolidCube(0.25);
	glPopMatrix();

	//Wheels
	glColor4f(1, 0, 1, 1);
	glPushMatrix();
	glTranslatef(-0.5, -0.8, 0.33);
	gluDisk(q, 0.0, 0.25, 20, 20);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.5, -0.8, 0.33);
	gluDisk(q, 0.0, 0.25, 20, 20);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(-0.5, -0.8, -0.33);
	glRotatef(180.0, 0., 1., 0.);
	gluDisk(q, 0.0, 0.25, 20, 20);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.5, -0.8, -0.33);
	glRotatef(180.0, 0., 1., 0.);
	gluDisk(q, 0.0, 0.25, 20, 20);
	glPopMatrix();
}

void drawBird() {

	glPushMatrix();
		glColor3f(1, 1, 0); 
		glutSolidSphere(0.1, 20, 20);
	glPopMatrix();
	glPushMatrix();
		glColor3f(0, 1, 0); 
		glTranslatef(0.2, -0.2, 0);
		glutSolidSphere(0.2, 20, 20);
	glPopMatrix();
	glPushMatrix();
		glColor3f(1, 0, 0); 
		glRotatef(-70, 0, 1, 0);
		glTranslatef(0, 0, 0.1);
		glutSolidCone(0.05, 0.1, 20, 20);
	glPopMatrix();
	glPushMatrix();
		glColor3f(1, 0, 0);
		glRotatef(wingAngle1, 1, 0, 0);
		glTranslatef(0.2, 0.4, -0.05);
		glBegin(GL_TRIANGLES);
		glVertex3f(-0.1, -0.5, 0.1);
		glVertex3f(0.1, -0.5, 0.1);
		glVertex3f(0, 0.5, 0.1);
		glEnd();
	glPopMatrix();
	glPushMatrix();
		glColor3f(1, 0, 0);
		glRotatef(wingAngle2, 1, 0, 0);
		glTranslatef(0.15, -0.4, 0.15);
		glBegin(GL_TRIANGLES);
		glVertex3f(-0.1, 0.5, -0.1);
		glVertex3f(0.1, 0.5, -0.1);
		glVertex3f(0, -0.5, -0.1);
		glEnd();
	glPopMatrix();
}

void loadTexture()				
{
	glGenTextures(11, txId); 	// Create 11 texture ids

	glBindTexture(GL_TEXTURE_2D, txId[0]);  
    loadTGA("Wall2.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	

	glBindTexture(GL_TEXTURE_2D, txId[1]);
    loadTGA("a_down.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	
	glBindTexture(GL_TEXTURE_2D, txId[2]);
    loadTGA("Floor.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	
	glBindTexture(GL_TEXTURE_2D, txId[3]);
	loadTGA("a_left.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	

	// *** front ***
	glBindTexture(GL_TEXTURE_2D, txId[4]);
	loadTGA("a_front.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	

	// *** right ***
	glBindTexture(GL_TEXTURE_2D, txId[5]);
	loadTGA("a_right.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	

	// *** back***
	glBindTexture(GL_TEXTURE_2D, txId[6]);
	loadTGA("a_back.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	

	// *** top ***
	glBindTexture(GL_TEXTURE_2D, txId[7]);
	loadTGA("a_top.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glBindTexture(GL_TEXTURE_2D, txId[8]); 
    loadTGA("Wall1.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	
	glBindTexture(GL_TEXTURE_2D, txId[9]);
    loadTGA("Wall.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	
	glBindTexture(GL_TEXTURE_2D, txId[10]);
    loadBMP("ice.bmp");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);			
	
	
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
}

void skybox(){
  
	///////////////////// LEFT WALL ///////////////////////
	glBindTexture(GL_TEXTURE_2D, txId[3]);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(-500, -1, 500);
		glTexCoord2f(1, 0);
		glVertex3f(-500, -1, -500);
		glTexCoord2f(1, 1);
		glVertex3f(-500, 500., -500);
		glTexCoord2f(0, 1);
		glVertex3f(-500, 500, 500);
	glEnd();

	////////////////////// FRONT WALL ///////////////////////
	glBindTexture(GL_TEXTURE_2D, txId[4]);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(-500, -1, -500);
		glTexCoord2f(1, 0);
		glVertex3f(500, -1, -500);
		glTexCoord2f(1, 1);
		glVertex3f(500, 500,  -500);
		glTexCoord2f(0, 1);
		glVertex3f(-500, 500, -500);
	glEnd();

	////////////////////// RIGHT WALL ///////////////////////
	glBindTexture(GL_TEXTURE_2D, txId[5]);
	glBegin(GL_QUADS);
		glTexCoord2f(1, 0);
		glVertex3f(500,  -1, 500);
		glTexCoord2f(0, 0);
		glVertex3f(500, -1, -500);
		glTexCoord2f(0, 1);
		glVertex3f(500, 500,  -500);
		glTexCoord2f(1, 1);
		glVertex3f(500,  500,  500);
  glEnd();

	////////////////////// REAR WALL ////////////////////////
	glBindTexture(GL_TEXTURE_2D, txId[6]);
	glBegin(GL_QUADS);
		glTexCoord2f(1, 0);
		glVertex3f(-500, -1, 500);
		glTexCoord2f(0, 0);
		glVertex3f(500, -1, 500);
		glTexCoord2f(0, 1);
		glVertex3f(500, 500,  500);
		glTexCoord2f(1, 1);
		glVertex3f(-500, 500, 500);
	glEnd();
  
	/////////////////////// TOP //////////////////////////
	glBindTexture(GL_TEXTURE_2D, txId[7]);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex3f(-500, 500, 500);
		glTexCoord2f(0, 0);
		glVertex3f(-500, 500, -500);
		glTexCoord2f(1, 0);
		glVertex3f(500, 500,  -500);
		glTexCoord2f(1, 1);
		glVertex3f(500, 500,  500);

	glEnd();
}

void drawBuildingUpper() 
{
	glBindTexture(GL_TEXTURE_2D, txId[8]);
 	
 	glBegin(GL_TRIANGLES);
		///////////////FRONT ROOF//////////////////
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-12, 2.5, 10);
		glTexCoord2f(0.0, 10.0);
		glVertex3f(12, 2.5, 10);
		glTexCoord2f(5, 10.0);
		glVertex3f(0, 5, 10);
		
		////////////////BACK ROOF//////////////////
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-12, 2.5, -10);
		glTexCoord2f(0.0, 10.0);
		glVertex3f(12, 2.5, -10);
		glTexCoord2f(5, 10.0);
		glVertex3f(0, 5, -10);
	glEnd();
 		
 	glBegin(GL_QUADS);
		///////////////LEFT ROOF///////////////////
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-12, 2.5, 10);
		glTexCoord2f(0.0, 10.0);
		glVertex3f(-12, 2.5, -10);
		glTexCoord2f(10, 10);
		glVertex3f(0, 5, -10);
		glTexCoord2f(10, 0);
		glVertex3f(0, 5, 10);
		
		///////////////RIGHT ROOF///////////////////
		glTexCoord2f(0.0, 0.0);
		glVertex3f(12, 2.5, 10);
		glTexCoord2f(0.0, 10.0);
		glVertex3f(12, 2.5, -10);
		glTexCoord2f(10, 10);
		glVertex3f(0, 5, -10);
		glTexCoord2f(10, 0);
		glVertex3f(0, 5, 10);
 	glEnd();
}

void drawBuildingWalls()
{
	glBindTexture(GL_TEXTURE_2D, txId[0]);
 	glBegin(GL_QUADS);
 	

	////////////////////// BACK WALLS ///////////////////////

	 glTexCoord2f(0.0, 2.0);
     glVertex3f(-10, 2.5, -10);
     glTexCoord2f(0.0, 0.0);
 	 glVertex3f(-10, -1, -10);
 	 glTexCoord2f(6.0, 0.0);
 	 glVertex3f(-1.25, -1, -10);
 	 glTexCoord2f(6.0, 2.0);
     glVertex3f(-1.25, 2.5, -10);
     
     glTexCoord2f(0.0, 2.0);
     glVertex3f(1.25, 2.5, -10);
     glTexCoord2f(0.0, 0.0);
 	 glVertex3f(1.25, -1, -10);
 	 glTexCoord2f(6.0, 0.0);
 	 glVertex3f(10, -1, -10);
 	 glTexCoord2f(6.0, 2.0);
     glVertex3f(10, 2.5, -10);
	
	////////////////////// FRONT WALLS ///////////////////////
	 glTexCoord2f(0.0, 2.0);
     glVertex3f(-10, 2.5, 10);
     glTexCoord2f(0.0, 0.0);
 	 glVertex3f(-10, -1, 10);
 	 glTexCoord2f(6.0, 0.0);
 	 glVertex3f(-1.25, -1, 10);
 	 glTexCoord2f(6.0, 2.0);
     glVertex3f(-1.25, 2.5, 10);
     
     glTexCoord2f(0.0, 2.0);
     glVertex3f(1.25, 2.5, 10);
     glTexCoord2f(0.0, 0.0);
 	 glVertex3f(1.25, -1, 10);
 	 glTexCoord2f(6.0, 0.0);
 	 glVertex3f(10, -1, 10);
 	 glTexCoord2f(6.0, 2.0);
     glVertex3f(10, 2.5, 10);
     
    ///////////////////// LEFT WALL ///////////////////////
     glTexCoord2f(0.0, 2.0);
	 glVertex3f(-10, 2.5, -10);
	 glTexCoord2f(0.0, 0.0);
  	 glVertex3f(-10, -1, -10);
  	 glTexCoord2f(12.0, 0.0);
	 glVertex3f(-10, -1, 10);
	 glTexCoord2f(12.0, 2.0);
 	 glVertex3f(-10, 2.5, 10);
 
 	////////////////////// RIGHT WALL ///////////////////////
	 glTexCoord2f(0.0, 2.0);
	 glVertex3f(10, 2.5, -10);
	 glTexCoord2f(0.0, 0.0);
	 glVertex3f(10, -1, -10);
	 glTexCoord2f(12.0, 0.0);
 	 glVertex3f(10, -1, 10);
 	 glTexCoord2f(12.0, 2.0);
 	 glVertex3f(10, 2.5, 10);
 	 
   glEnd();
}

void drawDoor() {
	glBindTexture(GL_TEXTURE_2D, txId[9]);
 	glBegin(GL_QUADS);
 	////// FRONT //////////////////////////
 	 glTexCoord2f(0.0, 1.0);
     glVertex3f(-1.25, 2.5, 10);
     glTexCoord2f(0.0, 0.0);
 	 glVertex3f(-1.25, 2.25, 10);
 	 glTexCoord2f(3.0, 0.0);
 	 glVertex3f(1.25, 2.25, 10);
 	 glTexCoord2f(3.0, 1.0);
     glVertex3f(1.25, 2.5, 10);
     
     glTexCoord2f(0.0, 3.0);
     glVertex3f(-1.25, 2.25, 10);
     glTexCoord2f(0.0, 0.0);
 	 glVertex3f(-1.25, -1, 10);
 	 glTexCoord2f(1.0, 0.0);
 	 glVertex3f(-1.00, -1, 10);
 	 glTexCoord2f(1.0, 3.0);
     glVertex3f(-1.00, 2.25, 10);
     
     glTexCoord2f(0.0, 3.0);
     glVertex3f(1.0, 2.25, 10);
     glTexCoord2f(0.0, 0.0);
 	 glVertex3f(1, -1, 10);
 	 glTexCoord2f(1.0, 0.0);
 	 glVertex3f(1.25, -1, 10);
 	 glTexCoord2f(1.0, 3.0);
     glVertex3f(1.25, 2.25, 10);
     
     ////// BACK /////////////////////////
     glTexCoord2f(0.0, 1.0);
     glVertex3f(-1.25, 2.5, -10);
     glTexCoord2f(0.0, 0.0);
 	 glVertex3f(-1.25, 2.25, -10);
 	 glTexCoord2f(3.0, 0.0);
 	 glVertex3f(1.25, 2.25, -10);
 	 glTexCoord2f(3.0, 1.0);
     glVertex3f(1.25, 2.5, -10);
     
     glTexCoord2f(0.0, 3.0);
     glVertex3f(-1.25, 2.25, -10);
     glTexCoord2f(0.0, 0.0);
 	 glVertex3f(-1.25, -1, -10);
 	 glTexCoord2f(1.0, 0.0);
 	 glVertex3f(-1.00, -1, -10);
 	 glTexCoord2f(1.0, 3.0);
     glVertex3f(-1.00, 2.25, -10);
     
     glTexCoord2f(0.0, 3.0);
     glVertex3f(1.0, 2.25, -10);
     glTexCoord2f(0.0, 0.0);
 	 glVertex3f(1, -1, -10);
 	 glTexCoord2f(1.0, 0.0);
 	 glVertex3f(1.25, -1, -10);
 	 glTexCoord2f(1.0, 3.0);
     glVertex3f(1.25, 2.25, -10);
     
    glEnd();
}

void drawBuildingFloor()
{
	glColor3f(0, 0, 0);	

	for(int i = -10; i <= 10; i ++)
	{
		glBegin(GL_LINES);		
			glVertex3f(-10, -0.9, i);
			glVertex3f(10, -0.9, i);
			glVertex3f(i, -0.9, -10);
			glVertex3f(i, -0.9, 10);
		glEnd();
	}
}

void drawSkyBoxFloor()
{
	glBindTexture(GL_TEXTURE_2D, txId[1]);
	glBegin(GL_QUADS);
	     glTexCoord2f(0.0, 0.0);
		 glVertex3f(-500, -1, 500);
		 glTexCoord2f(0.0, 1.0);
		 glVertex3f(500, -1, 500);
		 glTexCoord2f(1.0, 1.0);
		 glVertex3f(500, -1, -500);
		 glTexCoord2f(1.0, 0.0);
		 glVertex3f(-500, -1, -500);
	glEnd();
}

void drawScene() {
	glDisable(GL_LIGHTING);
		drawBuildingWalls();
		drawBuildingUpper();
		drawDoor();
		drawBuildingFloor();
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
		glTranslatef(-2, 0, -1);
		glRotatef(rot_angle, 0, 1, 1);
		glTranslatef(2, 0, 1);
		glTranslatef(-2, 0, -1);
		glScalef(0.15, 0.1, 0.15);
		drawVase();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
		glPushMatrix();
			drawWalkingGaurd(1);
		glPopMatrix();
		glPushMatrix();
			drawWalkingGaurd(2);
		glPopMatrix();
		glPushMatrix();
			glTranslatef(2, -0.4, -1);
			glRotatef(rot_angle, 0, 1, 0);
			glTranslatef(-2, 0.4, 1);
			glTranslatef(2, -0.4, -1);
			glScalef(0.35, 0.35, 0.35);
			drawVehicle();
		glPopMatrix();
		glPushMatrix();
			glRotatef(walkAngle, 0, 1, 0);
			glTranslatef(0, -1.25, -6);
			drawHorse();
		glPopMatrix();
		glPushMatrix();
			glRotatef(walkAngle, 0, 1, 0);
			glTranslatef(1.5, 0.25, -6);
			drawCart();
		glPopMatrix();
		glPushMatrix();
			glRotatef(flyAngle, 0, 1, 0);
			glTranslatef(0, flightHeight, 5);
			drawBird();
		glPopMatrix();
	glEnable(GL_TEXTURE_2D);
}

void display(void) 
{ 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(eye_x, 0, eye_z, ref_x, 0, ref_z, 0, 1, 0);
   
	glLightfv(GL_LIGHT0,GL_POSITION, lpos);
	glLightfv(GL_LIGHT1,GL_POSITION, lpos_2); 
	
	glDisable(GL_LIGHTING);
	skybox();
	drawSkyBoxFloor();
	glEnable(GL_LIGHTING);
	glPushMatrix();
		if (switchCamera) {
			glTranslatef(0, 0, 25);
			glRotatef(180, 0, 1, 0);
			glRotatef(-90, 0, 1, 0);
			glTranslatef(0, 0, -6);
			glRotatef(-walkAngle, 0, 1, 0);
		}
		drawScene();
	glPopMatrix();
	glutSwapBuffers();
	glFlush(); 
} 

void key(unsigned char key, int x, int y)
{
	cout << key << endl;
	if ((key == 'q') || (key == 'Q')) {
		isAntiClockWise = !isAntiClockWise;
	} else if ((key == 'w') || (key == 'W')) {
		flightHeight += 0.05;
	} else if ((key == 's') || (key == 'S')) {
		isAntiClockWise = !isAntiClockWise;
		flightHeight -= 0.05;
	} 
	glutPostRedisplay();
}

void special(int key, int x, int y)
{
	if(key == GLUT_KEY_LEFT) {
		angle -= 0.1;
		ref_x = eye_x + sin(angle);
		ref_z = eye_z - cos(angle);
	} 
	else if(key == GLUT_KEY_RIGHT) {
		angle += 0.1;
		ref_x = eye_x + sin(angle);
		ref_z = eye_z - cos(angle);
	}
	else if(key == GLUT_KEY_DOWN)
	{
		eye_x -= 0.1*sin(angle);
		eye_z += 0.1*cos(angle);
		ref_x = eye_x + sin(angle);
		ref_z = eye_z - cos(angle);
	}
	else if(key == GLUT_KEY_UP)
	{   
		eye_x += 0.1*sin(angle);
		eye_z -= 0.1*cos(angle);
		ref_x = eye_x + sin(angle);
		ref_z = eye_z - cos(angle);
	} 
	else if (key == GLUT_KEY_F1)
	{
		switchCamera = !switchCamera;
	}
	
	glutPostRedisplay();
}

//----------------------------------------------------------------------
void initialize(void)
{
	float grey[4] = { 0.2, 0.2, 0.2, 1.0 };
	float white[4] = { 1.0, 1.0, 1.0, 1.0 };
	glClearColor(0, 0, 0, 0);
	q = gluNewQuadric();
	loadTexture();	
	glEnable(GL_TEXTURE_2D);
	gluQuadricDrawStyle(q, GLU_FILL);

	glEnable(GL_LIGHTING);		//Enable OpenGL states
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glLightfv(GL_LIGHT0, GL_AMBIENT, grey);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);

	glLightfv(GL_LIGHT1, GL_AMBIENT, grey);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT1, GL_SPECULAR, white);

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	glMaterialf(GL_FRONT, GL_SHININESS, 50);

	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80, 1, 1, 1000);
}


int main(int argc, char **argv) 
{ 
	glutInit(&argc, argv);            
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);  
	glutInitWindowSize(700, 700);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("SUBHRA | COSC363 ASSIGNMENT 2018");
	initialize();
	glutDisplayFunc(display);
	glutSpecialFunc(special);
	glutKeyboardFunc(key);
	glutTimerFunc(500, humanCycleTimer, 0);
	glutTimerFunc(500, humanWalkTimer, 0);
	glutTimerFunc(50, rotateTimer, 0);
	glutTimerFunc(500, walkTimer, 0);
	glutTimerFunc(150, wingTimer, 0);
	glutTimerFunc(200, flyTimer, 0);
	glutMainLoop();
	return 0; 
}
