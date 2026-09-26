#define _CRT_SECURE_NO_WARNINGS 
#include <GL/glut.h> 
#include <cmath> 
#include <cstdlib> 
#include <cstdio> 
#include <iostream>
#include <ctime>
// ===== sound headers =====
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
// ==============================
// ===== GLOBAL VARIABLES ===== 
float x = 0.0f;
float y = 5.0f;
float z = 15.0f;
float Ix = 0.0f;
float Iy = 0.0f;
float Iz = -1.0f;
float ratio = 1.0f;
float carX = 0.0f;
float carY = 0.0f;
float carZ = 0.0f;
float carAngle = 90.0f;
float carSpeed = 0.0f;
float steerAngle = 0.0f;
float distanceTravelled = 0.0f;
float fuel = 100.0f;
int score = 0;
bool forwardPressed = false;
bool backwardPressed = false;
bool leftPressed = false;
bool rightPressed = false;
bool brakePressed = false;
bool headlightsOn = false;

bool isRaining = false;
int weatherTimer = 0;
int weatherDuration = 600;
int weatherType = 0;
int lightningTimer = 0;
int lightningFlashFrames = 0;
int shakeTime = 0;
float timeOfDay = 0.0f;   // permanently night

// ===== SOUND STATE ===== 
const char* currentAmbientFile = "";   // no ambient at start
int ambientRestartTimer = 0;
// ========================

int gameState = 0;
int countdownTimer = 180;
float gameOverAlpha = 0.0f;

#define NUM_TRAFFIC_CARS 25 
float trafficCarX[NUM_TRAFFIC_CARS];
float trafficCarZ[NUM_TRAFFIC_CARS];
float trafficCarSpeed[NUM_TRAFFIC_CARS];
int trafficCarType[NUM_TRAFFIC_CARS];
bool carPassed[NUM_TRAFFIC_CARS];
float trafficCarColorR[NUM_TRAFFIC_CARS];
float trafficCarColorG[NUM_TRAFFIC_CARS];
float trafficCarColorB[NUM_TRAFFIC_CARS];

#define NUM_STARS 300 
float starX[NUM_STARS];
float starY[NUM_STARS];

#define GROUND_Y 0.32f 

GLuint carDisplayList;
GLuint houseDisplayList;
GLuint lampPostDisplayList;
GLuint treeDisplayList;

// ===== SOUND HELPERS (PlaySound-based) =====
void playAmbientLoop(const char* filename) {
    currentAmbientFile = filename;
    ambientRestartTimer = 0;
    if (filename[0] == '\0') {
        PlaySoundA(NULL, NULL, 0);   // stop any sound
    }
    else {
        PlaySoundA(filename, NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    }
}

void playOneShot(const char* filename, int restartAfterFrames) {
    PlaySoundA(filename, NULL, SND_FILENAME | SND_ASYNC);
    ambientRestartTimer = restartAfterFrames;
}

void tickAmbientRestart() {
    if (ambientRestartTimer > 0) {
        ambientRestartTimer--;
        if (ambientRestartTimer == 0) {
            if (currentAmbientFile[0] != '\0') {
                PlaySoundA(currentAmbientFile, NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
            }
        }
    }
}
// ==========================================

// ===== FUNCTION PROTOTYPES ===== 
void changeSize(int w, int h);
void drawCar();
void drawPrivateCar(float r, float g, float b);
void drawBus(float r, float g, float b);
void drawTruck(float r, float g, float b);
void drawTrafficCar(int type, float r, float g, float b);
void drawHouse();
void drawLampPost();
void drawTree();
void initStars();
void drawSkyAndStars();
void drawRain();
GLuint createCarDisplayList();
GLuint createHouseDisplayList();
GLuint createLampPostDisplayList();
GLuint createTreeDisplayList();
void initScene();
void renderScene();
void updateGame(int value);
void processNormalKeys(unsigned char key, int xx, int yy);
void processNormalKeysUp(unsigned char key, int xx, int yy);
void processSpecialKeys(int key, int xx, int yy);
void processSpecialKeysUp(int key, int xx, int yy);
void updateCamera();
void drawGround();
void drawRoad();
void drawHUD();
void resetGame();

void initTrafficCars() {
    float startDist = -20.0f;
    for (int i = 0; i < NUM_TRAFFIC_CARS; i++) {
        int lane = rand() % 4;
        if (lane == 0) trafficCarX[i] = -4.0f;
        else if (lane == 1) trafficCarX[i] = -2.0f;
        else if (lane == 2) trafficCarX[i] = 2.0f;
        else trafficCarX[i] = 4.0f;
        trafficCarZ[i] = startDist;
        startDist -= 25.0f;
        trafficCarSpeed[i] = 0.50f + ((float)(rand() % 10) / 100.0f);
        trafficCarType[i] = rand() % 3;
        trafficCarColorR[i] = 0.15f + (float)((i * 37) % 85) / 100.0f;
        trafficCarColorG[i] = 0.15f + (float)((i * 53) % 80) / 100.0f;
        trafficCarColorB[i] = 0.15f + (float)((i * 71) % 90) / 100.0f;
        carPassed[i] = false;
    }
}

void initStars() {
    for (int i = 0; i < NUM_STARS; i++) {
        starX[i] = (float)rand() / RAND_MAX;
        starY[i] = (float)rand() / RAND_MAX;
    }
}

void drawSkyAndStars() {
    float dayFactor = 0.5f + 0.5f * sin(timeOfDay * 2.0f * 3.14159265f - 3.14159265f / 2.0f);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    float botR = 0.02f + dayFactor * (0.45f - 0.02f);
    float botG = 0.05f + dayFactor * (0.65f - 0.05f);
    float botB = 0.15f + dayFactor * (0.90f - 0.15f);
    float topR = 0.00f + dayFactor * 0.25f;
    float topG = 0.00f + dayFactor * 0.45f;
    float topB = 0.02f + dayFactor * 0.83f;
    glBegin(GL_QUADS);
    glColor3f(botR, botG, botB);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(1.0f, 0.0f);
    glColor3f(topR, topG, topB);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(0.0f, 1.0f);
    glEnd();
    float sb = 1.0f - dayFactor;
    if (sb < 0.0f) sb = 0.0f;
    glColor3f(sb, sb, sb);
    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i < NUM_STARS; i++) {
        if (rand() % 10 > 1) {
            glVertex2f(starX[i], starY[i]);
        }
    }
    glEnd();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void drawRain() {
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glColor3f(0.55f, 0.65f, 0.85f);
    glLineWidth(1.2f);
    glBegin(GL_LINES);
    int drops = (weatherType == 2) ? 900 : 500;
    for (int i = 0; i < drops; i++) {
        float rx = carX + ((rand() % 400 - 200) / 10.0f);
        float rz = carZ + ((rand() % 400 - 200) / 10.0f);
        float ry = 2.0f + ((rand() % 200) / 10.0f);
        glVertex3f(rx, ry, rz);
        glVertex3f(rx + 0.05f, ry - 0.6f, rz + 0.05f);
    }
    glEnd();
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
}

void drawCar() {
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glColor3f(0.85f, 0.05f, 0.05f);
    glBegin(GL_QUADS);
    glVertex3f(-1.9f, 0.35f, 0.95f); glVertex3f(1.9f, 0.35f, 0.95f);
    glVertex3f(1.9f, 0.35f, -0.95f); glVertex3f(-1.9f, 0.35f, -0.95f);
    glVertex3f(-1.9f, 0.35f, 0.95f); glVertex3f(1.9f, 0.35f, 0.95f);
    glVertex3f(1.9f, 0.95f, 0.95f); glVertex3f(-1.9f, 0.95f, 0.95f);
    glVertex3f(-1.9f, 0.35f, -0.95f); glVertex3f(1.9f, 0.35f, -0.95f);
    glVertex3f(1.9f, 0.95f, -0.95f); glVertex3f(-1.9f, 0.95f, -0.95f);
    glVertex3f(1.9f, 0.35f, 0.95f); glVertex3f(1.9f, 0.35f, -0.95f);
    glVertex3f(1.9f, 0.85f, -0.95f); glVertex3f(1.9f, 0.85f, 0.95f);
    glVertex3f(-1.9f, 0.35f, 0.95f); glVertex3f(-1.9f, 0.85f, 0.95f);
    glVertex3f(-1.9f, 0.85f, -0.95f); glVertex3f(-1.9f, 0.35f, -0.95f);
    glEnd();
    glColor3f(0.9f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
    glVertex3f(0.9f, 0.95f, 0.95f); glVertex3f(1.9f, 0.85f, 0.95f);
    glVertex3f(1.9f, 0.85f, -0.95f); glVertex3f(0.9f, 0.95f, -0.95f);
    glVertex3f(-1.9f, 0.85f, 0.95f); glVertex3f(-0.9f, 0.95f, 0.95f);
    glVertex3f(-0.9f, 0.95f, -0.95f); glVertex3f(-1.9f, 0.85f, -0.95f);
    glEnd();
    glColor3f(0.85f, 0.05f, 0.05f);
    glBegin(GL_QUADS);
    glVertex3f(-0.6f, 1.45f, 0.82f); glVertex3f(0.5f, 1.45f, 0.82f);
    glVertex3f(0.5f, 1.45f, -0.82f); glVertex3f(-0.6f, 1.45f, -0.82f);
    glVertex3f(0.5f, 1.45f, 0.82f); glVertex3f(0.5f, 1.45f, -0.82f);
    glVertex3f(0.9f, 0.95f, -0.82f); glVertex3f(0.9f, 0.95f, 0.82f);
    glVertex3f(-0.6f, 1.45f, 0.82f); glVertex3f(-0.6f, 1.45f, -0.82f);
    glVertex3f(-0.9f, 0.95f, -0.82f); glVertex3f(-0.9f, 0.95f, 0.82f);
    glEnd();
    glColor3f(0.1f, 0.12f, 0.18f);
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, 1.4f, 0.83f); glVertex3f(0.4f, 1.4f, 0.83f);
    glVertex3f(0.75f, 1.0f, 0.83f); glVertex3f(-0.75f, 1.0f, 0.83f);
    glVertex3f(-0.5f, 1.4f, -0.83f); glVertex3f(0.4f, 1.4f, -0.83f);
    glVertex3f(0.75f, 1.0f, -0.83f); glVertex3f(-0.75f, 1.0f, -0.83f);
    glEnd();
    GLfloat headLightEmit[] = { 4.0f, 4.0f, 3.5f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, headLightEmit);
    glColor3f(1.0f, 1.0f, 0.9f);
    glBegin(GL_QUADS);
    glVertex3f(1.91f, 0.55f, 0.55f); glVertex3f(1.91f, 0.72f, 0.55f);
    glVertex3f(1.91f, 0.72f, 0.85f); glVertex3f(1.91f, 0.55f, 0.85f);
    glVertex3f(1.91f, 0.55f, -0.85f); glVertex3f(1.91f, 0.72f, -0.85f);
    glVertex3f(1.91f, 0.72f, -0.55f); glVertex3f(1.91f, 0.55f, -0.55f);
    glEnd();
    GLfloat backLightEmit[] = { 3.0f, 0.2f, 0.2f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, backLightEmit);
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(-1.91f, 0.62f, 0.35f); glVertex3f(-1.91f, 0.78f, 0.35f);
    glVertex3f(-1.91f, 0.78f, 0.85f); glVertex3f(-1.91f, 0.62f, 0.85f);
    glVertex3f(-1.91f, 0.62f, -0.85f); glVertex3f(-1.91f, 0.78f, -0.85f);
    glVertex3f(-1.91f, 0.78f, -0.35f); glVertex3f(-1.91f, 0.62f, -0.35f);
    glEnd();
    GLfloat noEmit[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
    glColor3f(0.05f, 0.05f, 0.05f);
    glBegin(GL_QUADS);
    glVertex3f(1.91f, 0.45f, -0.35f); glVertex3f(1.91f, 0.65f, -0.35f);
    glVertex3f(1.91f, 0.65f, 0.35f); glVertex3f(1.91f, 0.45f, 0.35f);
    glEnd();
    glColor3f(0.15f, 0.15f, 0.15f);
    glPushMatrix();
    glTranslatef(1.1f, 0.35f, 0.96f);
    glRotatef(steerAngle, 0.0f, 1.0f, 0.0f);
    glutSolidTorus(0.1f, 0.28f, 12, 12);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1.1f, 0.35f, -0.96f);
    glRotatef(steerAngle, 0.0f, 1.0f, 0.0f);
    glutSolidTorus(0.1f, 0.28f, 12, 12);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-1.1f, 0.35f, 0.96f);
    glutSolidTorus(0.1f, 0.28f, 12, 12);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-1.1f, 0.35f, -0.96f);
    glutSolidTorus(0.1f, 0.28f, 12, 12);
    glPopMatrix();
}

void drawPrivateCar(float r, float g, float b) {
    GLfloat noEmit[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex3f(-1.9f, 0.35f, 0.95f); glVertex3f(1.9f, 0.35f, 0.95f);
    glVertex3f(1.9f, 0.35f, -0.95f); glVertex3f(-1.9f, 0.35f, -0.95f);
    glVertex3f(-1.9f, 0.35f, 0.95f); glVertex3f(1.9f, 0.35f, 0.95f);
    glVertex3f(1.9f, 0.9f, 0.95f); glVertex3f(-1.9f, 0.9f, 0.95f);
    glVertex3f(-1.9f, 0.35f, -0.95f); glVertex3f(1.9f, 0.35f, -0.95f);
    glVertex3f(1.9f, 0.9f, -0.95f); glVertex3f(-1.9f, 0.9f, -0.95f);
    glEnd();
    glColor3f(r * 0.85f, g * 0.85f, b * 0.85f);
    glBegin(GL_QUADS);
    glVertex3f(0.9f, 0.9f, 0.95f); glVertex3f(1.9f, 0.8f, 0.95f);
    glVertex3f(1.9f, 0.8f, -0.95f); glVertex3f(0.9f, 0.9f, -0.95f);
    glVertex3f(-1.9f, 0.8f, 0.95f); glVertex3f(-0.9f, 0.9f, 0.95f);
    glVertex3f(-0.9f, 0.9f, -0.95f); glVertex3f(-1.9f, 0.8f, -0.95f);
    glEnd();
    glColor3f(r * 0.9f, g * 0.9f, b * 0.9f);
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, 1.4f, 0.82f); glVertex3f(0.5f, 1.4f, 0.82f);
    glVertex3f(0.5f, 1.4f, -0.82f); glVertex3f(-0.5f, 1.4f, -0.82f);
    glVertex3f(0.5f, 1.4f, 0.82f); glVertex3f(0.5f, 1.4f, -0.82f);
    glVertex3f(0.9f, 0.9f, -0.82f); glVertex3f(0.9f, 0.9f, 0.82f);
    glVertex3f(-0.5f, 1.4f, 0.82f); glVertex3f(-0.5f, 1.4f, -0.82f);
    glVertex3f(-0.9f, 0.9f, -0.82f); glVertex3f(-0.9f, 0.9f, 0.82f);
    glEnd();
    glColor3f(0.12f, 0.16f, 0.25f);
    glBegin(GL_QUADS);
    glVertex3f(-0.4f, 1.35f, 0.83f); glVertex3f(0.3f, 1.35f, 0.83f);
    glVertex3f(0.75f, 0.95f, 0.83f); glVertex3f(-0.75f, 0.95f, 0.83f);
    glVertex3f(-0.4f, 1.35f, -0.83f); glVertex3f(0.3f, 1.35f, -0.83f);
    glVertex3f(0.75f, 0.95f, -0.83f); glVertex3f(-0.75f, 0.95f, -0.83f);
    glVertex3f(-1.91f, 0.6f, -0.75f); glVertex3f(-1.91f, 1.0f, -0.75f);
    glVertex3f(-1.91f, 1.0f, 0.75f); glVertex3f(-1.91f, 0.6f, 0.75f);
    glEnd();
    GLfloat tailLightEmit[] = { 2.5f, 0.1f, 0.1f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, tailLightEmit);
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(-1.92f, 0.62f, 0.4f); glVertex3f(-1.92f, 0.78f, 0.4f);
    glVertex3f(-1.92f, 0.78f, 0.8f); glVertex3f(-1.92f, 0.62f, 0.8f);
    glVertex3f(-1.92f, 0.62f, -0.8f); glVertex3f(-1.92f, 0.78f, -0.8f);
    glVertex3f(-1.92f, 0.78f, -0.4f); glVertex3f(-1.92f, 0.62f, -0.4f);
    glEnd();

    // ===== Traffic car headlights (private car) =====
    GLfloat headLightEmit[] = { 4.0f, 4.0f, 3.5f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, headLightEmit);
    glColor3f(1.0f, 1.0f, 0.9f);
    glBegin(GL_QUADS);
    glVertex3f(1.91f, 0.5f, 0.55f); glVertex3f(1.91f, 0.7f, 0.55f);
    glVertex3f(1.91f, 0.7f, 0.85f); glVertex3f(1.91f, 0.5f, 0.85f);
    glVertex3f(1.91f, 0.5f, -0.85f); glVertex3f(1.91f, 0.7f, -0.85f);
    glVertex3f(1.91f, 0.7f, -0.55f); glVertex3f(1.91f, 0.5f, -0.55f);
    glEnd();
    // ================================================

    glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
    glColor3f(0.12f, 0.12f, 0.12f);
    glPushMatrix(); glTranslatef(1.1f, 0.35f, 0.96f); glutSolidTorus(0.09f, 0.27f, 12, 12);
    glPopMatrix();
    glPushMatrix(); glTranslatef(1.1f, 0.35f, -0.96f); glutSolidTorus(0.09f, 0.27f, 12, 12);
    glPopMatrix();
    glPushMatrix(); glTranslatef(-1.1f, 0.35f, 0.96f); glutSolidTorus(0.09f, 0.27f, 12, 12);
    glPopMatrix();
    glPushMatrix(); glTranslatef(-1.1f, 0.35f, -0.96f); glutSolidTorus(0.09f, 0.27f, 12, 12);
    glPopMatrix();
}

void drawBus(float r, float g, float b) {
    GLfloat noEmit[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex3f(-3.5f, 0.35f, 1.2f); glVertex3f(3.5f, 0.35f, 1.2f);
    glVertex3f(3.5f, 0.35f, -1.2f); glVertex3f(-3.5f, 0.35f, -1.2f);
    glVertex3f(-3.5f, 0.35f, 1.2f); glVertex3f(3.5f, 0.35f, 1.2f);
    glVertex3f(3.5f, 2.4f, 1.2f); glVertex3f(-3.5f, 2.4f, 1.2f);
    glVertex3f(-3.5f, 0.35f, -1.2f); glVertex3f(3.5f, 0.35f, -1.2f);
    glVertex3f(3.5f, 2.4f, -1.2f); glVertex3f(-3.5f, 2.4f, -1.2f);
    glEnd();
    glColor3f(0.95f, 0.95f, 0.95f);
    glBegin(GL_QUADS);
    glVertex3f(-3.5f, 2.4f, 1.2f); glVertex3f(3.5f, 2.4f, 1.2f);
    glVertex3f(3.5f, 2.4f, -1.2f); glVertex3f(-3.5f, 2.4f, -1.2f);
    glEnd();
    glColor3f(0.12f, 0.18f, 0.28f);
    glBegin(GL_QUADS);
    glVertex3f(-3.1f, 1.3f, 1.21f); glVertex3f(3.0f, 1.3f, 1.21f);
    glVertex3f(3.0f, 2.1f, 1.21f); glVertex3f(-3.1f, 2.1f, 1.21f);
    glVertex3f(-3.1f, 1.3f, -1.21f); glVertex3f(3.0f, 1.3f, -1.21f);
    glVertex3f(3.0f, 2.1f, -1.21f); glVertex3f(-3.1f, 2.1f, -1.21f);
    glVertex3f(-3.52f, 1.5f, -0.9f); glVertex3f(-3.52f, 1.5f, 0.9f);
    glVertex3f(-3.52f, 2.25f, 0.9f); glVertex3f(-3.52f, 2.25f, -0.9f);
    glEnd();
    glColor3f(r * 0.85f, g * 0.85f, b * 0.85f);
    glBegin(GL_QUADS);
    glVertex3f(-3.51f, 0.35f, -1.1f); glVertex3f(-3.51f, 0.35f, 1.1f);
    glVertex3f(-3.51f, 1.4f, 1.1f); glVertex3f(-3.51f, 1.4f, -1.1f);
    glEnd();
    GLfloat busTailEmit[] = { 2.5f, 0.1f, 0.1f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, busTailEmit);
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(-3.53f, 0.7f, -1.05f); glVertex3f(-3.53f, 1.1f, -1.05f);
    glVertex3f(-3.53f, 1.1f, -0.7f); glVertex3f(-3.53f, 0.7f, -0.7f);
    glVertex3f(-3.53f, 0.7f, 0.7f); glVertex3f(-3.53f, 1.1f, 0.7f);
    glVertex3f(-3.53f, 1.1f, 1.05f); glVertex3f(-3.53f, 0.7f, 1.05f);
    glEnd();

    // ===== Traffic car headlights (bus) =====
    GLfloat headLightEmit[] = { 4.0f, 4.0f, 3.5f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, headLightEmit);
    glColor3f(1.0f, 1.0f, 0.9f);
    glBegin(GL_QUADS);
    glVertex3f(3.51f, 0.7f, 0.85f); glVertex3f(3.51f, 1.0f, 0.85f);
    glVertex3f(3.51f, 1.0f, 1.15f); glVertex3f(3.51f, 0.7f, 1.15f);
    glVertex3f(3.51f, 0.7f, -1.15f); glVertex3f(3.51f, 1.0f, -1.15f);
    glVertex3f(3.51f, 1.0f, -0.85f); glVertex3f(3.51f, 0.7f, -0.85f);
    glEnd();
    // =======================================

    glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
    glColor3f(0.12f, 0.12f, 0.12f);
    glPushMatrix(); glTranslatef(2.2f, 0.35f, 1.25f); glutSolidTorus(0.13f, 0.35f, 10, 10);
    glPopMatrix();
    glPushMatrix(); glTranslatef(2.2f, 0.35f, -1.25f); glutSolidTorus(0.13f, 0.35f, 10, 10);
    glPopMatrix();
    glPushMatrix(); glTranslatef(-2.0f, 0.35f, 1.25f); glutSolidTorus(0.13f, 0.35f, 10, 10);
    glPopMatrix();
    glPushMatrix(); glTranslatef(-2.0f, 0.35f, -1.25f); glutSolidTorus(0.13f, 0.35f, 10, 10);
    glPopMatrix();
}

void drawTruck(float r, float g, float b) {
    GLfloat noEmit[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex3f(-2.8f, 0.35f, 1.1f); glVertex3f(-1.0f, 0.35f, 1.1f);
    glVertex3f(-1.0f, 1.9f, 1.1f); glVertex3f(-2.8f, 1.9f, 1.1f);
    glVertex3f(-2.8f, 0.35f, -1.1f); glVertex3f(-1.0f, 0.35f, -1.1f);
    glVertex3f(-1.0f, 1.9f, -1.1f); glVertex3f(-2.8f, 1.9f, -1.1f);
    glEnd();
    glColor3f(0.15f, 0.18f, 0.25f);
    glBegin(GL_QUADS);
    glVertex3f(-2.1f, 1.1f, 1.11f); glVertex3f(-1.3f, 1.1f, 1.11f);
    glVertex3f(-1.3f, 1.7f, 1.11f); glVertex3f(-2.1f, 1.7f, 1.11f);
    glVertex3f(-2.1f, 1.1f, -1.11f); glVertex3f(-1.3f, 1.1f, -1.11f);
    glVertex3f(-1.3f, 1.7f, -1.11f); glVertex3f(-2.1f, 1.7f, -1.11f);
    glVertex3f(-1.01f, 1.1f, -0.8f); glVertex3f(-1.01f, 1.1f, 0.8f);
    glVertex3f(-1.01f, 1.8f, 0.8f); glVertex3f(-1.01f, 1.8f, -0.8f);
    glEnd();
    glColor3f(0.28f, 0.28f, 0.34f);
    glBegin(GL_QUADS);
    glVertex3f(-0.8f, 0.35f, 1.15f); glVertex3f(3.5f, 0.35f, 1.15f);
    glVertex3f(3.5f, 2.2f, 1.15f); glVertex3f(-0.8f, 2.2f, 1.15f);
    glVertex3f(-0.8f, 0.35f, -1.15f); glVertex3f(3.5f, 0.35f, -1.15f);
    glVertex3f(3.5f, 2.2f, -1.15f); glVertex3f(-0.8f, 2.2f, -1.15f);
    glVertex3f(-0.8f, 2.2f, 1.15f); glVertex3f(3.5f, 2.2f, 1.15f);
    glVertex3f(3.5f, 2.2f, -1.15f); glVertex3f(-0.8f, 2.2f, -1.15f);
    glEnd();
    glColor3f(0.22f, 0.22f, 0.28f);
    glBegin(GL_QUADS);
    glVertex3f(3.51f, 0.35f, -1.14f); glVertex3f(3.51f, 0.35f, 1.14f);
    glVertex3f(3.51f, 2.18f, 1.14f); glVertex3f(3.51f, 2.18f, -1.14f);
    glEnd();
    glColor3f(0.15f, 0.15f, 0.15f);
    glBegin(GL_QUADS);
    glVertex3f(3.52f, 0.35f, -0.05f); glVertex3f(3.52f, 0.35f, 0.05f);
    glVertex3f(3.52f, 2.18f, 0.05f); glVertex3f(3.52f, 2.18f, -0.05f);
    glEnd();
    GLfloat truckTailEmit[] = { 2.5f, 0.1f, 0.1f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, truckTailEmit);
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(3.53f, 0.5f, 1.0f); glVertex3f(3.53f, 0.9f, -1.0f);
    glVertex3f(3.53f, 0.9f, -0.6f); glVertex3f(3.53f, 0.5f, -0.6f);
    glVertex3f(3.53f, 0.5f, 0.6f); glVertex3f(3.53f, 0.9f, 0.6f);
    glVertex3f(3.53f, 0.9f, 1.0f); glVertex3f(3.53f, 0.5f, 1.0f);
    glEnd();

    // ===== Traffic car headlights (truck, at cab end) =====
    GLfloat headLightEmit[] = { 4.0f, 4.0f, 3.5f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, headLightEmit);
    glColor3f(1.0f, 1.0f, 0.9f);
    glBegin(GL_QUADS);
    glVertex3f(-2.81f, 0.5f, 0.75f); glVertex3f(-2.81f, 0.8f, 0.75f);
    glVertex3f(-2.81f, 0.8f, 1.05f); glVertex3f(-2.81f, 0.5f, 1.05f);
    glVertex3f(-2.81f, 0.5f, -1.05f); glVertex3f(-2.81f, 0.8f, -1.05f);
    glVertex3f(-2.81f, 0.8f, -0.75f); glVertex3f(-2.81f, 0.5f, -0.75f);
    glEnd();
    // =====================================================

    glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
    glColor3f(0.12f, 0.12f, 0.12f);
    glPushMatrix(); glTranslatef(2.0f, 0.35f, 1.2f); glutSolidTorus(0.14f, 0.35f, 10, 10);
    glPopMatrix();
    glPushMatrix(); glTranslatef(2.0f, 0.35f, -1.2f); glutSolidTorus(0.14f, 0.35f, 10, 10);
    glPopMatrix();
    glPushMatrix(); glTranslatef(-2.0f, 0.35f, 1.2f); glutSolidTorus(0.14f, 0.35f, 10, 10);
    glPopMatrix();
    glPushMatrix(); glTranslatef(-2.0f, 0.35f, -1.2f); glutSolidTorus(0.14f, 0.35f, 10, 10);
    glPopMatrix();
}

void drawTrafficCar(int type, float r, float g, float b) {
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    if (type == 0) {
        drawPrivateCar(r, g, b);
    }
    else if (type == 1) {
        drawBus(r, g, b);
    }
    else {
        drawTruck(r, g, b);
    }
}

void drawHouse() {
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glColor3f(0.65f, 0.55f, 0.45f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-2.0f, 0.0f, 2.0f); glVertex3f(2.0f, 0.0f, 2.0f);
    glVertex3f(2.0f, 4.0f, 2.0f); glVertex3f(-2.0f, 4.0f, 2.0f);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-2.0f, 0.0f, -2.0f); glVertex3f(-2.0f, 4.0f, -2.0f);
    glVertex3f(2.0f, 4.0f, -2.0f); glVertex3f(2.0f, 0.0f, -2.0f);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-2.0f, 0.0f, -2.0f); glVertex3f(-2.0f, 0.0f, 2.0f);
    glVertex3f(-2.0f, 4.0f, 2.0f); glVertex3f(-2.0f, 4.0f, -2.0f);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(2.0f, 0.0f, -2.0f); glVertex3f(2.0f, 4.0f, -2.0f);
    glVertex3f(2.0f, 4.0f, 2.0f); glVertex3f(2.0f, 0.0f, 2.0f);
    glEnd();
    GLfloat windowEmit[] = { 1.5f, 1.2f, 0.4f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, windowEmit);
    glColor3f(1.0f, 0.9f, 0.5f);
    glBegin(GL_QUADS);
    glVertex3f(-1.2f, 1.5f, 2.01f); glVertex3f(-0.3f, 1.5f, 2.01f); glVertex3f(-0.3f, 2.5f, 2.01f);
    glVertex3f(-1.2f, 2.5f, 2.01f);
    glVertex3f(0.3f, 1.5f, 2.01f); glVertex3f(1.2f, 1.5f, 2.01f); glVertex3f(1.2f, 2.5f, 2.01f);
    glVertex3f(0.3f, 2.5f, 2.01f);
    glVertex3f(-1.2f, 1.5f, -2.01f); glVertex3f(-0.3f, 1.5f, -2.01f); glVertex3f(-0.3f, 2.5f, -2.01f);
    glVertex3f(-1.2f, 2.5f, -2.01f);
    glVertex3f(0.3f, 1.5f, -2.01f); glVertex3f(1.2f, 1.5f, -2.01f); glVertex3f(1.2f, 2.5f, -2.01f);
    glVertex3f(0.3f, 2.5f, -2.01f);
    glVertex3f(-2.01f, 1.5f, -1.2f); glVertex3f(-2.01f, 1.5f, -0.3f); glVertex3f(-2.01f, 2.5f, -0.3f);
    glVertex3f(-2.01f, 2.5f, -1.2f);
    glVertex3f(-2.01f, 1.5f, 0.3f); glVertex3f(-2.01f, 1.5f, 1.2f); glVertex3f(-2.01f, 2.5f, 1.2f);
    glVertex3f(-2.01f, 2.5f, 0.3f);
    glVertex3f(2.01f, 1.5f, -1.2f); glVertex3f(2.01f, 1.5f, -0.3f); glVertex3f(2.01f, 2.5f, -0.3f);
    glVertex3f(2.01f, 2.5f, -1.2f);
    glVertex3f(2.01f, 1.5f, 0.3f); glVertex3f(2.01f, 1.5f, 1.2f); glVertex3f(2.01f, 2.5f, 1.2f);
    glVertex3f(2.01f, 2.5f, 0.3f);
    glEnd();
    GLfloat noEmit[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
    glColor3f(0.45f, 0.15f, 0.15f);
    glBegin(GL_TRIANGLES);
    glNormal3f(0.0f, 0.707f, 0.707f);
    glVertex3f(-2.0f, 4.0f, 2.0f); glVertex3f(2.0f, 4.0f, 2.0f); glVertex3f(0.0f, 6.0f, 2.0f);
    glNormal3f(0.0f, 0.707f, -0.707f);
    glVertex3f(-2.0f, 4.0f, -2.0f); glVertex3f(0.0f, 6.0f, -2.0f); glVertex3f(2.0f, 4.0f, -2.0f);
    glEnd();
}

void drawLampPost() {
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glColor3f(0.25f, 0.25f, 0.25f);
    glBegin(GL_QUADS);
    glVertex3f(-0.15f, 0.0f, -0.15f); glVertex3f(0.15f, 0.0f, -0.15f);
    glVertex3f(0.15f, 7.0f, -0.15f); glVertex3f(-0.15f, 7.0f, -0.15f);
    glVertex3f(-0.15f, 0.0f, 0.15f); glVertex3f(-0.15f, 7.0f, 0.15f);
    glVertex3f(0.15f, 7.0f, 0.15f); glVertex3f(0.15f, 0.0f, 0.15f);
    glEnd();
    GLfloat emission[] = { 2.5f, 2.5f, 1.4f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, emission);
    glColor3f(1.0f, 1.0f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, 6.8f, -0.5f); glVertex3f(0.5f, 6.8f, -0.5f);
    glVertex3f(0.5f, 7.3f, 0.5f); glVertex3f(-0.5f, 7.3f, 0.5f);
    glEnd();
    GLfloat noEmission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
}

void drawTree() {
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glColor3f(0.35f, 0.2f, 0.05f);
    glBegin(GL_QUADS);
    glVertex3f(-0.2f, 0.0f, 0.2f); glVertex3f(0.2f, 0.0f, 0.2f); glVertex3f(0.2f, 2.0f, 0.2f);
    glVertex3f(-0.2f, 2.0f, 0.2f);
    glVertex3f(-0.2f, 0.0f, -0.2f); glVertex3f(-0.2f, 2.0f, -0.2f); glVertex3f(0.2f, 2.0f, -0.2f);
    glVertex3f(0.2f, 0.0f, -0.2f);
    glVertex3f(-0.2f, 0.0f, -0.2f); glVertex3f(-0.2f, 0.0f, 0.2f); glVertex3f(-0.2f, 2.0f, 0.2f);
    glVertex3f(-0.2f, 2.0f, -0.2f);
    glVertex3f(0.2f, 0.0f, -0.2f); glVertex3f(0.2f, 2.0f, -0.2f); glVertex3f(0.2f, 2.0f, 0.2f);
    glVertex3f(0.2f, 0.0f, -0.2f);
    glEnd();
    glColor3f(0.1f, 0.4f, 0.1f);
    glBegin(GL_TRIANGLES);
    glNormal3f(0.0f, 0.7f, 0.7f); glVertex3f(-1.5f, 1.5f, 1.5f); glVertex3f(1.5f, 1.5f, 1.5f);
    glVertex3f(0.0f, 4.0f, 0.0f);
    glNormal3f(0.0f, 0.7f, -0.7f); glVertex3f(-1.5f, 1.5f, -1.5f); glVertex3f(0.0f, 4.0f, 0.0f);
    glVertex3f(1.5f, 1.5f, -1.5f);
    glNormal3f(-0.7f, 0.7f, 0.0f); glVertex3f(-1.5f, 1.5f, -1.5f); glVertex3f(-1.5f, 1.5f, 1.5f);
    glVertex3f(0.0f, 4.0f, 0.0f);
    glNormal3f(0.7f, 0.7f, 0.0f); glVertex3f(1.5f, 1.5f, -1.5f); glVertex3f(0.0f, 4.0f, 0.0f);
    glVertex3f(1.5f, 1.5f, 1.5f);
    glColor3f(0.15f, 0.45f, 0.15f);
    glNormal3f(0.0f, 0.7f, 0.7f); glVertex3f(-1.0f, 3.0f, 1.0f); glVertex3f(1.0f, 3.0f, 1.0f);
    glVertex3f(0.0f, 5.5f, 0.0f);
    glNormal3f(0.0f, 0.7f, -0.7f); glVertex3f(-1.0f, 3.0f, 1.0f); glVertex3f(0.0f, 5.5f, 0.0f);
    glVertex3f(1.0f, 3.0f, 1.0f);
    glNormal3f(-0.7f, 0.7f, 0.0f); glVertex3f(-1.0f, 3.0f, 1.0f); glVertex3f(-1.0f, 3.0f, 1.0f);
    glVertex3f(0.0f, 5.5f, 0.0f);
    glNormal3f(0.7f, 0.7f, 0.0f); glVertex3f(1.0f, 3.0f, 1.0f); glVertex3f(0.0f, 5.5f, 0.0f);
    glVertex3f(1.0f, 3.0f, -1.0f);
    glEnd();
}

GLuint createCarDisplayList() {
    GLuint listID = glGenLists(1);
    glNewList(listID, GL_COMPILE);
    drawCar();
    glEndList();
    return listID;
}
GLuint createHouseDisplayList() {
    GLuint listID = glGenLists(1);
    glNewList(listID, GL_COMPILE);
    drawHouse();
    glEndList();
    return listID;
}
GLuint createLampPostDisplayList() {
    GLuint listID = glGenLists(1);
    glNewList(listID, GL_COMPILE);
    drawLampPost();
    glEndList();
    return listID;
}
GLuint createTreeDisplayList() {
    GLuint listID = glGenLists(1);
    glNewList(listID, GL_COMPILE);
    drawTree();
    glEndList();
    return listID;
}

void initScene() {
    initStars();
    initTrafficCars();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    GLfloat globalAmbient[] = { 0.15f, 0.15f, 0.2f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    GLfloat moonDir[] = { -1.0f, 1.0f, -0.5f, 0.0f };
    GLfloat moonDiffuse[] = { 0.15f, 0.2f, 0.3f, 1.0f };
    glEnable(GL_LIGHT2);
    glLightfv(GL_LIGHT2, GL_POSITION, moonDir);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, moonDiffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, moonDiffuse);
    carDisplayList = createCarDisplayList();
    houseDisplayList = createHouseDisplayList();
    lampPostDisplayList = createLampPostDisplayList();
    treeDisplayList = createTreeDisplayList();
}

void changeSize(int w, int h) {
    if (h == 0) h = 1;
    ratio = (float)w / (float)h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    gluPerspective(45.0, ratio, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(x, y, z, x + Ix, y + Iy, z + Iz, 0.0f, 1.0f, 0.0f);
}

void renderScene() {
    float dayFactor = 0.5f + 0.5f * sin(timeOfDay * 2.0f * 3.14159265f - 3.14159265f / 2.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float sunAngle = (timeOfDay - 0.25f) * 2.0f * 3.14159265f;
    GLfloat moonDir[] = { cos(sunAngle), fabs(sin(sunAngle)) + 0.3f, -0.5f, 0.0f };
    GLfloat moonDiffuse[] = {
        0.15f + dayFactor * 0.75f,
        0.20f + dayFactor * 0.65f,
        0.30f + dayFactor * 0.45f,
        1.0f
    };
    glLightfv(GL_LIGHT2, GL_POSITION, moonDir);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, moonDiffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, moonDiffuse);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawSkyAndStars();

    if (weatherType == 2 && lightningFlashFrames > 0) {
        lightningFlashFrames--;
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, 1.0, 0.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f); glVertex2f(1.0f, 0.0f);
        glVertex2f(1.0f, 1.0f); glVertex2f(0.0f, 1.0f);
        glEnd();
        glDisable(GL_BLEND);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }

    if (weatherType != 0) {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, 1.0, 0.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if (weatherType == 1)      glColor4f(0.03f, 0.07f, 0.18f, 0.55f);
        else if (weatherType == 2) glColor4f(0.02f, 0.03f, 0.10f, 0.70f);
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f); glVertex2f(1.0f, 0.0f);
        glVertex2f(1.0f, 1.0f); glVertex2f(0.0f, 1.0f);
        glEnd();
        glDisable(GL_BLEND);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);

        GLfloat fogColor[4];
        float fogDensity = 0.035f;
        if (weatherType == 1) {
            fogColor[0] = 0.05f; fogColor[1] = 0.08f; fogColor[2] = 0.15f; fogColor[3] = 1.0f;
            fogDensity = 0.014f;
        }
        else if (weatherType == 2) {
            fogColor[0] = 0.02f; fogColor[1] = 0.04f; fogColor[2] = 0.09f; fogColor[3] = 1.0f;
            fogDensity = 0.024f;
        }
        glEnable(GL_FOG);
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogi(GL_FOG_MODE, GL_EXP2);
        glFogf(GL_FOG_DENSITY, fogDensity);
        glHint(GL_FOG_HINT, GL_NICEST);
    }

    glLoadIdentity();
    float shakeX = 0.0f, shakeY = 0.0f, shakeZ = 0.0f;
    if (shakeTime > 0) {
        shakeX = ((rand() % 100) - 50) / 200.0f;
        shakeY = ((rand() % 100) - 50) / 200.0f;
        shakeZ = ((rand() % 100) - 50) / 200.0f;
        shakeTime--;
    }
    gluLookAt(x + shakeX, y + shakeY, z + shakeZ,
        x + Ix, y + Iy, z + Iz, 0.0f, 1.0f, 0.0f);

    drawGround();
    drawRoad();
    int currentBlock = (int)floor((carZ + 200.0f) / 200.0f);
    for (int i = currentBlock - 4; i <= currentBlock + 4; i++) {
        float hZ = i * 200.0f;
        glPushMatrix();
        glTranslatef(15.0f, GROUND_Y, hZ);
        glCallList(houseDisplayList);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(-18.0f, GROUND_Y, hZ - 70.0f);
        glCallList(houseDisplayList);
        glPopMatrix();
    }
    int lampBlock = (int)floor((carZ + 100.0f) / 50.0f);
    int activeLightCount = GL_LIGHT3;
    for (int i = lampBlock - 6; i <= lampBlock + 6; i++) {
        float lZ = i * 50.0f;

        // ===== FIX: draw lamp posts WITHOUT fog, restore only if it was on =====
        GLboolean fogWasOn = glIsEnabled(GL_FOG);
        glDisable(GL_FOG);
        glPushMatrix();
        glTranslatef(7.0f, GROUND_Y, lZ);
        glCallList(lampPostDisplayList);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(-7.0f, GROUND_Y, lZ);
        glCallList(lampPostDisplayList);
        glPopMatrix();
        if (fogWasOn) glEnable(GL_FOG);
        // =====================================================================

        glPushMatrix(); glTranslatef(9.0f, GROUND_Y, lZ + 25.0f); glCallList(treeDisplayList);
        glPopMatrix();
        glPushMatrix(); glTranslatef(-9.0f, GROUND_Y, lZ + 25.0f); glCallList(treeDisplayList);
        glPopMatrix();
        if (fabs(carZ - lZ) < 80.0f && activeLightCount <= GL_LIGHT7) {
            glEnable(activeLightCount);
            GLfloat lightPosRight[] = { 7.0f, GROUND_Y + 7.0f, lZ, 1.0f };
            GLfloat lightColor[] = { 2.5f, 2.3f, 1.6f, 1.0f };
            glLightfv(activeLightCount, GL_POSITION, lightPosRight);
            glLightfv(activeLightCount, GL_DIFFUSE, lightColor);
            glLightfv(activeLightCount, GL_SPECULAR, lightColor);
            glLightf(activeLightCount, GL_CONSTANT_ATTENUATION, 1.0f);
            glLightf(activeLightCount, GL_LINEAR_ATTENUATION, 0.02f);
            glLightf(activeLightCount, GL_QUADRATIC_ATTENUATION, 0.002f);
            activeLightCount++;
        }
    }
    for (int l = activeLightCount; l <= GL_LIGHT7; l++) {
        glDisable(l);
    }
    float rad = carAngle * 3.14159265f / 180.0f;
    float fx = cos(rad);
    float fz = -sin(rad);

    GLfloat headLightPos[] = { carX, GROUND_Y + 1.6f, carZ, 1.0f };
    GLfloat headLightDir[] = { fx, -0.2f, fz };
    GLfloat headlightColor[] = { 30.0f, 30.0f, 28.0f, 1.0f };
    GLfloat specRef[] = { 15.0f, 15.0f, 15.0f, 1.0f };

    if (headlightsOn) {
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, headLightPos);
        glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, headLightDir);
        glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 65.0f);
        glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 10.0f);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, headlightColor);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specRef);
        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.001f);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.00001f);
        glEnable(GL_LIGHT1);
        glLightfv(GL_LIGHT1, GL_POSITION, headLightPos);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, headLightDir);
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 65.0f);
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 10.0f);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, headlightColor);
        glLightfv(GL_LIGHT1, GL_SPECULAR, specRef);
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.001f);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.00001f);
    }
    else {
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHT1);
    }
    glPushMatrix();
    glTranslatef(carX, GROUND_Y, carZ);
    glRotatef(carAngle, 0.0f, 1.0f, 0.0f);
    glCallList(carDisplayList);
    glPopMatrix();

    for (int i = 0; i < NUM_TRAFFIC_CARS; i++) {
        glPushMatrix();
        glTranslatef(trafficCarX[i], GROUND_Y, trafficCarZ[i]);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        drawTrafficCar(trafficCarType[i], trafficCarColorR[i], trafficCarColorG[i], trafficCarColorB[i]);
        glPopMatrix();
    }

    if (weatherType == 1 || weatherType == 2) {
        drawRain();
    }
    glDisable(GL_FOG);

    drawHUD();
    glutSwapBuffers();
}

void resetGame() {
    carX = 0.0f;
    carZ = 0.0f;
    carSpeed = 0.0f;
    carAngle = 90.0f;
    steerAngle = 0.0f;
    distanceTravelled = 0.0f;
    fuel = 100.0f;
    score = 0;
    gameState = 0;
    countdownTimer = 180;
    gameOverAlpha = 0.0f;
    initTrafficCars();
    shakeTime = 0;
    weatherType = 0;
    isRaining = false;
    weatherTimer = 0;
    weatherDuration = 600;
    lightningTimer = 0;
    lightningFlashFrames = 0;
    playAmbientLoop("");   // stop ambient on reset
}

void updateGame(int value) {
    tickAmbientRestart();

    if (gameState == 3) {
        glutPostRedisplay();
        glutTimerFunc(16, updateGame, 0);
        return;
    }
    if (gameState == 0) {
        countdownTimer--;
        score = 0;
        if (countdownTimer <= 0) {
            score = 0;
            gameState = 1;
        }
        glutPostRedisplay();
        glutTimerFunc(16, updateGame, 0);
        return;
    }
    if (gameState == 2) {
        if (gameOverAlpha < 1.0f) {
            gameOverAlpha += 0.05f;
        }
        glutPostRedisplay();
        glutTimerFunc(16, updateGame, 0);
        return;
    }
    if (forwardPressed) {
        carSpeed += 0.035f;
        if (carSpeed > 1.875f) carSpeed = 1.875f;
    }
    else if (backwardPressed) {
        carSpeed -= 0.02f;
        if (carSpeed < -0.5f) carSpeed = -0.5f;
    }
    else {
        carSpeed *= 0.94f;
        if (fabs(carSpeed) < 0.005f) {
            carSpeed = 0.0f;
        }
    }
    if (brakePressed) {
        carSpeed *= 0.80f;
        if (fabs(carSpeed) < 0.01f) carSpeed = 0.0f;
    }
    if (fuel <= 0 && forwardPressed) {
        carSpeed = 0;
    }
    if (leftPressed) {
        steerAngle += 2.0f;
        if (steerAngle > 30.0f) steerAngle = 30.0f;
    }
    else if (rightPressed) {
        steerAngle -= 2.0f;
        if (steerAngle < -30.0f) steerAngle = -30.0f;
    }
    else {
        steerAngle *= 0.85f;
        if (fabs(carSpeed) < 0.1f) {
            steerAngle = 0.0f;
        }
    }
    if (fabs(carSpeed) > 0.005f) {
        float dir = (carSpeed > 0) ? 1.0f : -1.0f;
        carAngle += (steerAngle * 0.038f) * dir;
    }
    float rad = carAngle * 3.14159265f / 180.0f;
    float fx = cos(rad);
    float fz = -sin(rad);
    float movementFactor = 0.40f;
    carX += fx * carSpeed * movementFactor;
    carZ += fz * carSpeed * movementFactor;

    for (int i = 0; i < NUM_TRAFFIC_CARS; i++) {
        trafficCarZ[i] -= trafficCarSpeed[i];

        float dx = carX - trafficCarX[i];
        float dz = carZ - trafficCarZ[i];
        if (fabs(dx) < 2.0f && fabs(dz) < 3.2f) {
            // ===== FIX: crash sound plays, then ALL sound stops =====
            if (gameState != 2) {
                shakeTime = 30;
                PlaySoundA("crash.wav", NULL, SND_FILENAME | SND_ASYNC);
                ambientRestartTimer = 0;   // no ambient restart after crash
                currentAmbientFile = "";   // clear so nothing resumes
            }
            // ========================================================
            gameState = 2;
        }

        if (!carPassed[i] && carZ < trafficCarZ[i]) {
            score += 5;
            carPassed[i] = true;
        }

        if (trafficCarZ[i] > carZ + 15.0f) {
            float minZ = carZ;
            for (int j = 0; j < NUM_TRAFFIC_CARS; j++) {
                if (trafficCarZ[j] < minZ) {
                    minZ = trafficCarZ[j];
                }
            }
            trafficCarZ[i] = minZ - (25.0f + (float)(rand() % 20));
            int lane = rand() % 4;
            if (lane == 0) trafficCarX[i] = -4.0f;
            else if (lane == 1) trafficCarX[i] = -2.0f;
            else if (lane == 2) trafficCarX[i] = 2.0f;
            else trafficCarX[i] = 4.0f;
            carPassed[i] = false;
        }
    }

    distanceTravelled += fabs(carSpeed) * movementFactor * 0.001f;
    if (fabs(carSpeed) > 0.01f) {
        fuel -= fabs(carSpeed) * 0.0005f;
    }
    if (fuel < 0) {
        fuel = 0;
    }
    if (carX > 5.0f) carX = 5.0f;
    else if (carX < -5.0f) carX = -5.0f;

    weatherTimer++;
    if (weatherTimer >= weatherDuration) {
        weatherType = rand() % 3;
        isRaining = (weatherType == 1 || weatherType == 2);
        weatherTimer = 0;
        weatherDuration = 600 + rand() % 900;

        if (isRaining) {
            playAmbientLoop("rain.wav");
        }
        else {
            playAmbientLoop("");   // no ambient when clear
        }
    }
    if (weatherType == 2) {
        lightningTimer++;
        if (lightningTimer > 120 + (int)(rand() % 240)) {
            lightningFlashFrames = 3;
            lightningTimer = 0;
            playOneShot("thunder.wav", 150);
        }
    }
    else {
        lightningTimer = 0;
    }

    if (weatherType == 1 || weatherType == 2) {
        headlightsOn = true;
    }

    updateCamera();
    glutPostRedisplay();
    glutTimerFunc(16, updateGame, 0);
}

void updateCamera() {
    float dist = 10.0f;
    float height = 4.0f;
    float lookHeight = 1.5f;
    float rad = carAngle * 3.14159265f / 180.0f;
    float fx = cos(rad);
    float fz = -sin(rad);
    x = carX - fx * dist;
    y = GROUND_Y + height;
    z = carZ - fz * dist;
    Ix = carX - x;
    Iy = (GROUND_Y + lookHeight) - y;
    Iz = carZ - z;
}

void processNormalKeys(unsigned char key, int xx, int yy) {
    if (key == 'p' || key == 'P') {
        if (gameState == 1) gameState = 3;
        else if (gameState == 3) gameState = 1;
        glutPostRedisplay();
        return;
    }

    if (key == 'h' || key == 'H') {
        if (gameState == 1) playOneShot("horn.wav", 30);
        return;
    }

    if (gameState != 1) return;
    switch (key) {
    case 27:
        exit(0);
        break;
    case 'w': case 'W': forwardPressed = true; break;
    case 'z': case 'Z': backwardPressed = true; break;
    case 'a': case 'A': leftPressed = true; break;
    case 'd': case 'D': rightPressed = true; break;
    case 'o': case 'O': headlightsOn = true; break;
    case 'f': case 'F': headlightsOn = false; break;
    case 's': case 'S': brakePressed = true; break;
    }
    glutPostRedisplay();
}

void processNormalKeysUp(unsigned char key, int xx, int yy) {
    switch (key) {
    case 'w': case 'W': forwardPressed = false; break;
    case 'z': case 'Z': backwardPressed = false; break;
    case 'a': case 'A': leftPressed = false; break;
    case 'd': case 'D': rightPressed = false; break;
    case 's': case 'S': brakePressed = false; break;
    case 'r': case 'R':
        if (gameState == 2) {
            resetGame();
        }
        break;
    }
}

void processSpecialKeys(int key, int xx, int yy) {
    if (gameState != 1) return;
    switch (key) {
    case GLUT_KEY_UP: forwardPressed = true; break;
    case GLUT_KEY_DOWN: backwardPressed = true; break;
    case GLUT_KEY_LEFT: leftPressed = true; break;
    case GLUT_KEY_RIGHT: rightPressed = true; break;
    }
}

void processSpecialKeysUp(int key, int xx, int yy) {
    switch (key) {
    case GLUT_KEY_UP: forwardPressed = false; break;
    case GLUT_KEY_DOWN: backwardPressed = false; break;
    case GLUT_KEY_LEFT: leftPressed = false; break;
    case GLUT_KEY_RIGHT: rightPressed = false; break;
    }
}

void drawGround() {
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glColor3f(0.12f, 0.4f, 0.12f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    float startX = -100.0f;
    float endX = 100.0f;
    float startZ = carZ - 100.0f;
    float endZ = carZ + 100.0f;
    for (float i = startX; i < endX; i += 5.0f) {
        for (float j = startZ; j < endZ; j += 5.0f) {
            glVertex3f(i, GROUND_Y, j);
            glVertex3f(i + 5.0f, GROUND_Y, j);
            glVertex3f(i + 5.0f, GROUND_Y, j + 5.0f);
            glVertex3f(i, GROUND_Y, j + 5.0f);
        }
    }
    glEnd();
}

void drawRoad() {
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    float segmentLength = 400.0f;
    int currentBlock = (int)floor((carZ + segmentLength / 2.0f) / segmentLength);
    for (int i = currentBlock - 2; i <= currentBlock + 2; i++) {
        float startZ = i * segmentLength - (segmentLength / 2.0f);
        float endZ = startZ + segmentLength;
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        for (float j = startZ; j < endZ; j += 5.0f) {
            glVertex3f(-6.0f, GROUND_Y + 0.01f, j);
            glVertex3f(6.0f, GROUND_Y + 0.01f, j);
            glVertex3f(6.0f, GROUND_Y + 0.01f, j + 5.0f);
            glVertex3f(-6.0f, GROUND_Y + 0.01f, j + 5.0f);
        }
        glEnd();
        glColor3f(0.9f, 0.9f, 0.0f);
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        for (float j = startZ; j < endZ; j += 12.0f) {
            glVertex3f(-0.2f, GROUND_Y + 0.02f, j);
            glVertex3f(0.2f, GROUND_Y + 0.02f, j);
            glVertex3f(0.2f, GROUND_Y + 0.02f, j + 6.0f);
            glVertex3f(-0.2f, GROUND_Y + 0.02f, j + 6.0f);
        }
        glEnd();
        glColor3f(0.9f, 0.9f, 0.9f);
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        for (float j = startZ; j < endZ; j += 5.0f) {
            glVertex3f(-5.8f, GROUND_Y + 0.02f, j);
            glVertex3f(-5.6f, GROUND_Y + 0.02f, j);
            glVertex3f(-5.6f, GROUND_Y + 0.02f, j + 5.0f);
            glVertex3f(-5.8f, GROUND_Y + 0.02f, j + 5.0f);
            glVertex3f(5.6f, GROUND_Y + 0.02f, j);
            glVertex3f(5.8f, GROUND_Y + 0.02f, j);
            glVertex3f(5.8f, GROUND_Y + 0.02f, j + 5.0f);
            glVertex3f(5.6f, GROUND_Y + 0.02f, j + 5.0f);
        }
        glEnd();
        glColor3f(0.6f, 0.6f, 0.65f);
        glBegin(GL_QUADS);
        glVertex3f(-6.2f, GROUND_Y + 0.6f, startZ); glVertex3f(-6.0f, GROUND_Y + 0.6f, startZ);
        glVertex3f(-6.0f, GROUND_Y + 0.6f, endZ); glVertex3f(-6.2f, GROUND_Y + 0.6f, endZ);
        glVertex3f(-6.2f, GROUND_Y + 0.8f, startZ); glVertex3f(-6.0f, GROUND_Y + 0.8f, startZ);
        glVertex3f(-6.0f, GROUND_Y + 0.8f, endZ); glVertex3f(-6.2f, GROUND_Y + 0.8f, endZ);
        glVertex3f(6.0f, GROUND_Y + 0.6f, startZ); glVertex3f(6.2f, GROUND_Y + 0.6f, startZ);
        glVertex3f(6.2f, GROUND_Y + 0.6f, endZ); glVertex3f(6.0f, GROUND_Y + 0.6f, endZ);
        glVertex3f(6.0f, GROUND_Y + 0.8f, startZ); glVertex3f(6.2f, GROUND_Y + 0.8f, startZ);
        glVertex3f(6.2f, GROUND_Y + 0.8f, endZ); glVertex3f(6.0f, GROUND_Y + 0.8f, endZ);
        for (float j = startZ; j < endZ; j += 15.0f) {
            glVertex3f(-6.15f, GROUND_Y, j - 0.2f); glVertex3f(-5.95f, GROUND_Y, j - 0.2f);
            glVertex3f(-5.95f, GROUND_Y + 0.6f, j - 0.2f); glVertex3f(-6.15f, GROUND_Y + 0.6f, j -
                0.2f);
            glVertex3f(-6.15f, GROUND_Y, j + 0.2f); glVertex3f(-5.95f, GROUND_Y, j + 0.2f);
            glVertex3f(-5.95f, GROUND_Y + 0.6f, j + 0.2f); glVertex3f(-6.15f, GROUND_Y + 0.6f, j +
                0.2f);
            glVertex3f(5.95f, GROUND_Y, j - 0.2f); glVertex3f(6.15f, GROUND_Y, j - 0.2f);
            glVertex3f(6.15f, GROUND_Y + 0.6f, j - 0.2f); glVertex3f(5.95f, GROUND_Y + 0.6f, j -
                0.2f);
            glVertex3f(5.95f, GROUND_Y, j + 0.2f); glVertex3f(6.15f, GROUND_Y, j + 0.2f);
            glVertex3f(6.15f, GROUND_Y + 0.6f, j + 0.2f); glVertex3f(5.95f, GROUND_Y + 0.6f, j +
                0.2f);
        }
        glEnd();
    }
}

void drawHUD() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 900, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(20, 565);
    glVertex2f(220, 565);
    glVertex2f(220, 425);
    glVertex2f(20, 425);
    glEnd();

    glBegin(GL_QUADS);
    glVertex2f(680, 570); glVertex2f(880, 570); glVertex2f(880, 500); glVertex2f(680, 500);
    glEnd();
    glDisable(GL_BLEND);
    glColor3f(1.0f, 1.0f, 1.0f);

    char speedText[50];
    sprintf_s(speedText, sizeof(speedText), "SPEED: %.0f km/h", fabs(carSpeed) * 80);
    glRasterPos2f(35, 535);
    for (int i = 0; speedText[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, speedText[i]);

    char lightText[50];
    sprintf_s(lightText, sizeof(lightText), "LIGHT: %s", headlightsOn ? "ON" : "OFF");
    glRasterPos2f(35, 505);
    for (int i = 0; lightText[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, lightText[i]);

    char distanceText[50];
    sprintf_s(distanceText, sizeof(distanceText), "DIST: %.2f km", distanceTravelled);
    glRasterPos2f(35, 475);
    for (int i = 0; distanceText[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, distanceText[i]);

    char fuelText[50];
    sprintf_s(fuelText, sizeof(fuelText), "FUEL: %.0f%%", fuel);
    glRasterPos2f(35, 445);
    for (int i = 0; fuelText[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, fuelText[i]);

    char scoreText[50];
    sprintf_s(scoreText, sizeof(scoreText), "SCORE: %d", score);
    glColor3f(1.0f, 0.85f, 0.0f);
    glRasterPos2f(705, 525);
    for (int i = 0; scoreText[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreText[i]);

    const char* weatherName = "CLEAR";
    if (weatherType == 1) weatherName = "RAIN";
    else if (weatherType == 2) weatherName = "STORM";
    char weatherText[50];
    sprintf_s(weatherText, sizeof(weatherText), "WEATHER: %s", weatherName);
    glColor3f(0.8f, 0.9f, 1.0f);
    glRasterPos2f(705, 480);
    for (int i = 0; weatherText[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, weatherText[i]);

    if (gameState == 0) {
        glEnable(GL_BLEND);
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f(900, 0); glVertex2f(900, 600); glVertex2f(0, 600);
        glEnd();
        glDisable(GL_BLEND);
        int secLeft = (countdownTimer / 60) + 1;
        const char* msg = "";
        if (secLeft > 3) msg = "GET READY!";
        else if (secLeft == 3) msg = "3";
        else if (secLeft == 2) msg = "2";
        else if (secLeft == 1) msg = "1";
        glColor3f(1.0f, 0.2f, 0.2f);
        glRasterPos2f(382, 308);
        for (int i = 0; msg[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, msg[i]);
        glColor3f(1.0f, 1.0f, 0.0f);
        glRasterPos2f(380, 310);
        for (int i = 0; msg[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, msg[i]);
        if (secLeft <= 1) {
            glColor3f(0.0f, 1.0f, 0.4f);
            char startMsg[] = "LET'S START!";
            glRasterPos2f(375, 250);
            for (int i = 0; startMsg[i] != '\0'; i++)
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, startMsg[i]);
        }
    }
    else if (gameState == 3) {
        glEnable(GL_BLEND);
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f(900, 0); glVertex2f(900, 600); glVertex2f(0, 600);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(1.0f, 1.0f, 1.0f);
        char pauseTitle[] = "PAUSED";
        glRasterPos2f(400, 320);
        for (int i = 0; pauseTitle[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, pauseTitle[i]);

        glColor3f(0.8f, 0.85f, 1.0f);
        char resumeText[] = "Press 'P' to Resume";
        glRasterPos2f(375, 275);
        for (int i = 0; resumeText[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, resumeText[i]);

        glColor3f(0.7f, 0.7f, 0.7f);
        char quitText[] = "Press 'ESC' to Quit";
        glRasterPos2f(378, 245);
        for (int i = 0; quitText[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, quitText[i]);
    }
    else if (gameState == 2) {
        glEnable(GL_BLEND);
        glColor4f(0.8f, 0.0f, 0.0f, gameOverAlpha * 0.4f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f(900, 0); glVertex2f(900, 600); glVertex2f(0, 600);
        glEnd();
        glDisable(GL_BLEND);
        glEnable(GL_BLEND);
        glColor4f(0.1f, 0.1f, 0.1f, 0.85f);
        glBegin(GL_QUADS);
        glVertex2f(250, 390); glVertex2f(650, 390); glVertex2f(650, 180); glVertex2f(250, 180);
        glEnd();
        glDisable(GL_BLEND);
        glColor3f(1.0f, 0.2f, 0.2f);
        char crashText[] = "CRASHED !";
        glRasterPos2f(395, 340);
        for (int i = 0; crashText[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, crashText[i]);
        glColor3f(1.0f, 1.0f, 1.0f);
        char overText[] = "GAME OVER";
        glRasterPos2f(390, 300);
        for (int i = 0; overText[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, overText[i]);
        char finalScoreText[50];
        sprintf_s(finalScoreText, sizeof(finalScoreText), "FINAL SCORE: %d", score);
        glColor3f(1.0f, 0.85f, 0.0f);
        glRasterPos2f(370, 250);
        for (int i = 0; finalScoreText[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, finalScoreText[i]);
        glColor3f(0.2f, 0.9f, 0.3f);
        char restartText[] = "Press 'R' to Restart Race";
        glRasterPos2f(355, 205);
        for (int i = 0; restartText[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, restartText[i]);
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

int main(int argc, char** argv) {
    srand((unsigned)time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(900, 600);
    glutCreateWindow("Celestial Drive 3D - Ultimate Speed & Overtake Edition");
    initScene();
    // (no engine.wav — no ambient started at launch)
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutTimerFunc(0, updateGame, 0);
    glutKeyboardFunc(processNormalKeys);
    glutKeyboardUpFunc(processNormalKeysUp);
    glutSpecialFunc(processSpecialKeys);
    glutSpecialUpFunc(processSpecialKeysUp);
    glutIgnoreKeyRepeat(1);
    updateCamera();
    glutMainLoop();
    return 0;
}
