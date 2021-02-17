#include <ctime>
#include <cstdlib>
#include <GL/glut.h>
#include "BmpLoader.h"
#include <list>
#include <sstream>

using namespace std;

void init();

// tipe data komposit untuk bullets dan akan dipakai untuk list
struct Bullet
{
    int id;
    GLfloat xPosition;
    GLfloat zPosition;
};

struct Car
{
    GLfloat xPosition{};
    GLfloat zPosition{};
    int health = 100;
};

GLfloat cameraAngle;
GLfloat viewCam; // camera dari depan/belakang
GLfloat moveCamX; // pergerakan kamera selama perubahan dari sumbu x
float angle = 0.0;

// how long z position from first place
float mapDistance;

GLfloat manyClicks = 0.0;

// update loop dan overall speed
GLfloat speed;

// image load var
GLuint grassTexture, roadTexture, skyTexture, textureBricks, textureDoor, textureGrass, textureRoof, textureSky, textureWindowsLeft;

//lighting starts
GLfloat matSpecular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat matShininess[] = {100.0};
//GLfloat light_position[] = {.1, .1, 1, 0};
//GLfloat lmodel_ambient[] = {0.2, 0.2, 0.2, 0.2};

bool reversal = false;

struct Bullet playerBullet;
struct Bullet enemyBullet;
struct Car enemyCar;
struct Car playerCar;

list<Bullet> bullets;

// rotation
bool leftRotate;
bool rightRotate;
bool pause;
bool lookFront;
bool isFirstPerson;
GLfloat angleCAM;

#define SSTR( x ) (static_cast< std::ostringstream & >( \
	( std::ostringstream() << std::dec << (x) ) ).str())

// collision condition
bool isEnemyDead;
bool isPlayerDead;

void roundTree(double x, double y, double z);
void coneTree(double x, double y, double z);
void drawHome();

bool detectCollision(Bullet bulletPosition) {
    if(bulletPosition.id==0){ // player bullet
        // car area to be detected
        GLfloat maxRight = enemyCar.xPosition + .5;
        GLfloat maxLeft = enemyCar.xPosition - .5;
        GLfloat maxFront = enemyCar.zPosition - 4; // smaller far++
        GLfloat maxRear = enemyCar.zPosition;

        if(bulletPosition.zPosition >= maxFront && bulletPosition.zPosition <= maxRear)
        {
            if(bulletPosition.xPosition >= maxLeft && bulletPosition.xPosition <= maxRight)
                return true;
        } else
            return false;
    }
    else // enenmy bullet
    {
        // car area to be detected
        GLfloat maxRight = playerCar.xPosition + .4;
        GLfloat maxLeft = playerCar.xPosition - .4;
        GLfloat maxFront = playerCar.zPosition + 2.5; // smaller far++
        GLfloat maxRear = playerCar.zPosition + 4;

        if(bulletPosition.zPosition >= maxFront && bulletPosition.zPosition <= maxRear)
        {
            if(-(bulletPosition.xPosition) >= maxLeft && -(bulletPosition.xPosition) <= maxRight)
                return true;
        } else
            return false;
    }

    return false;
}

// menggerakkan semua peluru yang ditembakkan menjauh ke depan (player)
void moveBullets(list <Bullet> g)
{
    list <Bullet> :: iterator it;
//    list <Bullet> :: iterator begin = g.begin();
    // copy dulu ke variable berbeda karena posisi end list akan terus berubah karena insert new value
    auto begin = g.begin();
    auto end = g.end();

    for(it = begin; it != end; ++it)
    {
        // add Z bullet (player)
        if(it->id==0){
            if (detectCollision(*it)) {
                if(enemyCar.health > 0)
                    enemyCar.health -= 20;
//                speed = 100;
                if(enemyCar.health == 0){
                    isEnemyDead = true;
                    pause = true;
                }
            }
            // move bullet
            it->zPosition -= 0.5;

            // check max bullet position
            if (bullets.front().zPosition <= -15) { // if already at maximum
                bullets.pop_front();
            } else // if it's not
            {
                // copy last bullet and insert to new element before delete the old one
                playerBullet.zPosition=it->zPosition;
                playerBullet.xPosition=it->xPosition;
                bullets.push_back(playerBullet);
                bullets.pop_front();
            }
        }
        else // enemy bullet
        {
            if (detectCollision(*it)) {
                if(playerCar.health > 0)
                    playerCar.health -= 20;
//                speed = 100;
                if(playerCar.health == 0){
                    isPlayerDead = true;
                    pause = true;
                }
            }

            it->zPosition += 0.5;

            if (bullets.front().zPosition >= 2.5) {
                bullets.pop_front();
            } else
            {
                // copy last bullet and insert to new element before delete the old one
                enemyBullet.zPosition=it->zPosition;
                enemyBullet.xPosition=it->xPosition;
                bullets.push_back(enemyBullet);
                bullets.pop_front();
            }
        }
    }
}

void lightInit(GLfloat white_light[]) {
    // glClearColor (0.4,0.9,0.9 ,0.0);
    glShadeModel(GL_SMOOTH);

    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    //glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
}

void lightRed() {
    GLfloat white_light[] = {1, 0, 0, 0};
    //GLfloat mat_ambiant[]= {0.2f, 0.2f, 0.2f, 1.0f};
    lightInit(white_light);
}

void lightYellow() {
    GLfloat white_light[] = {1, 1, 0, 0};

    lightInit(white_light);
}

void lightBlack() {
    GLfloat white_light[] = {0, 0, 0, 0};

    lightInit(white_light);
}

void lightWhite() {
    GLfloat white_light[] = {1, 1, 1, 0};

    lightInit(white_light);
}

void lightBlue() {
    GLfloat white_light[] = {0, 0, 1, 0};
    lightInit(white_light);
}

void lightOff() {
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);
}
//lighting ends

void lightWoodTree()
{
    GLfloat white_light[] = { 0.9,0.3,0 };
    lightInit(white_light);
}
void lightGreenTree()
{
    GLfloat white_light[] = { 0,1,0.3,1 };

    lightInit(white_light);
}

void initCarPosition() {
    enemyCar.xPosition = 0;
    enemyCar.zPosition = -15;
}

void drawEnemyCar() {
    glPushMatrix();
//    glTranslatef(0 + x, 0 + y, 0 + z);
    glTranslatef(enemyCar.xPosition, -1, enemyCar.zPosition);
    glRotatef(180,0,1,0);

    //main body
    glPushMatrix();
    glTranslatef(0, 0, -.4);
    lightRed();
    glScalef(1, .4, 2);
    glutSolidCube(1);
    glPopMatrix();

    //atap
    glPushMatrix();
    glTranslatef(0, .3, -.4);
    lightBlue();
    glScalef(2, .4, 2);
    glutSolidCube(.5);
    glPopMatrix();

    //kaca
    glPushMatrix();
    glTranslatef(0, .3, .13);
    lightWhite();
    glScalef(2, .4, .1);
    glutSolidCube(.5);
    glPopMatrix();

    //lampu kiri
    glPushMatrix();
    glTranslatef(-0.2, 0, 0.6);
    lightYellow();
    glRotatef(90, 0, 1, 0);
    glutSolidSphere(.1, 20, 20);
    glPopMatrix();

    //lampu kanan
    glPushMatrix();
    glTranslatef(0.2, 0, 0.6);
    lightYellow();
    glRotatef(90, 0, 1, 0);
    glutSolidSphere(.1, 20, 20);
    glPopMatrix();

    //ban depan kanan
    glPushMatrix();
    lightBlack();
    glTranslatef(-0.5, -.2, .1);
    glRotatef(90, 0, 1, 0);
    glutSolidTorus(.07, .15, 20, 30);
    glPopMatrix();

    //ban depan kiri
    glPushMatrix();
    lightBlack();
    glTranslatef(0.5, -.2, .1);
    glRotatef(90, 0, 1, 0);
    glutSolidTorus(.07, .15, 20, 30);
    glPopMatrix();

    //ban belakang kanan
    glPushMatrix();
    lightBlack();
    glTranslatef(-0.5, -.2, -.85);
    glRotatef(90, 0, 1, 0);
    glutSolidTorus(.07, .15, 20, 30);
    glPopMatrix();

    //ban belakang kiri
    glPushMatrix();
    lightBlack();
    glTranslatef(0.5, -.2, -.85);
    glRotatef(90, 0, 1, 0);
    glutSolidTorus(.07, .15, 20, 30);
    glPopMatrix();
    glPopMatrix();

    lightOff();
}

void drawSideRoads() {
    glPushMatrix();
    glTranslatef(0.0, 0.0, 5.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, grassTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glColor3f(1.0, 1.0, 1.0); // normalising the colour
    float ym = -2;

    // left side
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-50, ym, -300.0);

    glTexCoord2f(0.0, 5.0);
    glVertex3f(-5.0, ym, -300.0);

    glTexCoord2f(5.0, 5.0);
    glVertex3f(-5.0, ym, 100.0);

    glTexCoord2f(5.0, 0.0);
    glVertex3f(-50, ym, 100.0);
    glEnd();

    // right side
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(5, ym, -300.0);

    glTexCoord2f(0.0, 5.0);
    glVertex3f(50.0, ym, -300.0);

    glTexCoord2f(5.0, 5.0);
    glVertex3f(50.0, ym, 100.0);

    glTexCoord2f(5.0, 0.0);
    glVertex3f(5, ym, 100.0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // pepohonan
    glPushMatrix();
    coneTree(10, -1, -10);
    roundTree(-10, -1, 40);
    coneTree(-10, -1, 80);
    coneTree(-10, -1, -40);

    roundTree(10, -1, 90);
    coneTree(10, -1, 60);
    roundTree(10, -1, -60);
    glPopMatrix();

    // rumah kiri
    glPushMatrix();
    glTranslatef(0.0, 0.0, -20.0);
    glRotatef(80, 0.0, 1.0, 0.0); //Rotate the camera

    drawHome();

    glPopMatrix();

    // rumah kanan
    glPushMatrix();
    glTranslatef(0.0, 0.0, -50.0);
    glRotatef(290, 0.0, 1.0, 0.0); //Rotate the camera

    drawHome();

    glPopMatrix();
}

void drawMainRoad() {
    glPushMatrix();

    glTranslatef(0.0, 0.0, 5.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, roadTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glColor3f(1.0, 1.0, 1.0);
    float ym = -2;
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-5.0, ym, -300.0);

    glTexCoord2f(1.0, 0.0);
    glVertex3f(5.0, ym, -300.0);

    glTexCoord2f(1.0, 1.0);
    glVertex3f(5.0, ym, 100.0);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(-5.0, ym, 100.0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // garis tengah jalan
    glPushMatrix();
    glTranslatef(0,-1.5,0);
    glColor3f(1,1,1);
    //	lightWhite();
    glScalef(.70,.2,7);

    glTranslatef(0.0,0.0,-20);
    for(int i = 0; i < 20 ; i++)
    {
        glutSolidCube(.5);
        glTranslatef(0.0,0.0,2);
    }
    glPopMatrix();
}

void drawSky() {
    glPushMatrix();

    glTranslatef(0.0, 0.0, -mapDistance);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, skyTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glColor3f(1.0,1.0,1.0);
    glRotatef(180, 0, 0, 1);
//    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS); //Belakang
    glTexCoord3f(1.0,1.0,0);  glVertex3f(-150,150,-150);
    glTexCoord3f(0.0,0.0,0);  glVertex3f(-150,-150,-150);
    glTexCoord3f(1.0,0.0,0);  glVertex3f(150,-150,-150);
    glTexCoord3f(0.0,1.0,0);  glVertex3f(150,150,-150);
    glEnd();
    glBegin(GL_QUADS); //Depan
    glTexCoord3f(1.0,1.0,0);  glVertex3f(-150,150,150);
    glTexCoord3f(0.0,0.0,0);  glVertex3f(-150,-150,150);
    glTexCoord3f(1.0,0.0,0);  glVertex3f(150,-150,150);
    glTexCoord3f(0.0,1.0,0);  glVertex3f(150,150,150);
    glEnd();
    glBegin(GL_QUADS); //Kanan
    glTexCoord3f(1.0,1.0,0);  glVertex3f(100,150,-150);
    glTexCoord3f(0.0,0.0,0);  glVertex3f(100,-150,-150);
    glTexCoord3f(1.0,0.0,0);  glVertex3f(100,-150,150);
    glTexCoord3f(0.0,1.0,0);  glVertex3f(100,150,150);
    glEnd();
    glBegin(GL_QUADS); //Kiri
    glTexCoord3f(1.0,1.0,0);  glVertex3f(-100,150,-150);
    glTexCoord3f(0.0,0.0,0);  glVertex3f(-100,-150,-150);
    glTexCoord3f(1.0,0.0,0);  glVertex3f(-100,-150,150);
    glTexCoord3f(0.0,1.0,0);  glVertex3f(-100,150,150);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawRoad() {
    drawSideRoads();
    drawMainRoad();
}

void drawPlayerCar() {
    glPushMatrix();
    glTranslatef(.1, -.63, 1);
    glRotatef(180, 0.0, 1.0, 0.0);
    glScalef(.7, .5, .5);
//    glTranslatef(carX, 0.0, 0.0);
    glTranslatef(playerCar.xPosition, 0.0, 0.0);
//    drawPlayerCar();



    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    glTranslatef(0, -1.0, -.5);
    glScalef(2.4, 1.4, 1.5);

    glPushMatrix();
    glBegin(GL_QUADS);                /* OBJECT MODULE*/

    /* top of cube*/
    //FRONT BODY
    glColor3f(255, 0, 0);
    glVertex3f(0.2, 0.4, 0.6);
    glVertex3f(0.6, 0.5, 0.6);
    glVertex3f(0.6, 0.5, 0.2);
    glVertex3f(0.2, 0.4, 0.2);

    /* bottom of cube*/
    glVertex3f(0.2, 0.2, 0.6);
    glVertex3f(0.6, 0.2, 0.6);
    glVertex3f(0.6, 0.2, 0.2);
    glVertex3f(0.2, 0.2, 0.2);

    /* front of cube*/
    glVertex3f(0.2, 0.2, 0.6);
    glVertex3f(0.2, 0.4, 0.6);
    glVertex3f(0.2, 0.4, 0.2);
    glVertex3f(0.2, 0.2, 0.2);

    /* back of cube.*/
    glVertex3f(0.6, 0.2, 0.6);
    glVertex3f(0.6, 0.5, 0.6);
    glVertex3f(0.6, 0.5, 0.2);
    glVertex3f(0.6, 0.2, 0.2);

    /* left of cube*/
    glVertex3f(0.2, 0.2, 0.6);
    glVertex3f(0.6, 0.2, 0.6);
    glVertex3f(0.6, 0.5, 0.6);
    glVertex3f(0.2, 0.4, 0.6);

    /* Right of cube */
    glVertex3f(0.2, 0.2, 0.2);
    glVertex3f(0.6, 0.2, 0.2);
    glVertex3f(0.6, 0.5, 0.2);
    glVertex3f(0.2, 0.4, 0.2);
//------------------------------------------------------------------
    glVertex3f(0.7, 0.65, 0.6);
    glVertex3f(0.7, 0.65, 0.2);
    glVertex3f(1.2, 0.65, 0.2);        //top cover
    glVertex3f(1.2, 0.65, 0.6);
//back guard
    glColor3f(255, 0, 0);            /* Set The Color To Red*/
    glVertex3f(1.2, 0.5, 0.6);
    glVertex3f(1.2, 0.5, 0.2);
    glVertex3f(1.5, 0.4, 0.2);
    glVertex3f(1.5, 0.4, 0.6);

    /* bottom of cube*/
    glVertex3f(1.5, 0.2, 0.6);
    glVertex3f(1.5, 0.2, 0.2);
    glVertex3f(1.2, 0.2, 0.6);
    glVertex3f(1.2, 0.2, 0.6);

    /* back of cube.*/
    glVertex3f(1.5, 0.4, 0.6);
    glVertex3f(1.5, 0.4, 0.2);
    glVertex3f(1.5, 0.2, 0.2);
    glVertex3f(1.5, 0.2, 0.6);

    /* left of cube*/
    glVertex3f(1.2, 0.2, 0.2);
    glVertex3f(1.2, 0.5, 0.2);
    glVertex3f(1.5, 0.4, 0.2);
    glVertex3f(1.5, 0.2, 0.2);

    /* Right of cube */
    glVertex3f(1.2, 0.2, 0.6);
    glVertex3f(1.2, 0.5, 0.6);
    glVertex3f(1.5, 0.4, 0.6);
    glVertex3f(1.5, 0.2, 0.6);
//MIDDLE BODY
    glVertex3f(0.6, 0.5, 0.6);
    glVertex3f(0.6, 0.2, 0.6);
    glVertex3f(1.2, 0.2, 0.6);
    glVertex3f(1.2, 0.5, 0.6);

    /* bottom of cube*/
    glVertex3f(0.6, 0.2, 0.6);
    glVertex3f(0.6, 0.2, 0.2);
    glVertex3f(1.2, 0.2, 0.2);
    glVertex3f(1.2, 0.2, 0.6);

    /* back of cube.*/
    glVertex3f(0.6, 0.5, 0.2);
    glVertex3f(0.6, 0.2, 0.2);
    glVertex3f(1.2, 0.2, 0.2);
    glVertex3f(1.2, 0.5, 0.2);
//ENTER WINDOW
    glColor3f(0.3, 0.3, 0.3);
    glVertex3f(0.77, 0.63, 0.2);
    glVertex3f(0.75, 0.5, 0.2);        //quad front window
    glVertex3f(1.2, 0.5, 0.2);
    glVertex3f(1.22, 0.63, 0.2);

    glColor3f(255, 0, 0);
    glVertex3f(0.7, 0.65, 0.2);
    glVertex3f(0.7, 0.5, .2);       //first separation
    glVertex3f(0.75, 0.5, 0.2);
    glVertex3f(0.77, 0.65, 0.2);

    glVertex3f(0.75, 0.65, 0.2);
    glVertex3f(0.75, 0.63, 0.2);        //line strip
    glVertex3f(1.2, 0.63, 0.2);
    glVertex3f(1.2, 0.65, 0.2);

    glVertex3f(0.75, 0.65, 0.6);
    glVertex3f(0.75, 0.63, 0.6);        //line strip
    glVertex3f(1.2, 0.63, 0.6);
    glVertex3f(1.2, 0.65, 0.6);

    glColor3f(0.3, 0.3, 0.3);
    glVertex3f(0.77, 0.63, 0.6);
    glVertex3f(0.75, 0.5, 0.6);        //quad front window
    glVertex3f(1.2, 0.5, 0.6);
    glVertex3f(1.22, 0.63, 0.6);

    glColor3f(255, 0, 0);
    glVertex3f(0.7, 0.65, 0.6);
    glVertex3f(0.7, 0.5, .6);       //first separation/sambungan jendela
    glVertex3f(0.75, 0.5, 0.6);
    glVertex3f(0.77, 0.65, 0.6);

    glEnd();

    //----------------------------------------------------------------
    glBegin(GL_QUADS);

    /* top of cube*/
    glColor3f(0.3, 0.3, 0.3);
    glVertex3f(0.6, 0.5, 0.6);
    glVertex3f(0.6, 0.5, 0.2);        //quad front window
    glVertex3f(0.7, 0.65, 0.2);
    glVertex3f(0.7, 0.65, 0.6);

    glVertex3f(1.2, 0.65, .6);
    glVertex3f(1.2, 0.65, 0.2);        //quad back window
    glVertex3f(1.3, 0.5, 0.2);
    glVertex3f(1.3, 0.5, 0.6);

//---------------------------------------------------------------------
    glBegin(GL_TRIANGLES);                /* start drawing the cube.*/

    /* top of cube*/
    glColor3f(0.3, 0.3, 0.3);
    glVertex3f(0.6, 0.5, 0.6);
    glVertex3f(0.7, 0.65, 0.6);       //tri front window
    glVertex3f(0.7, 0.5, 0.6);

    glVertex3f(0.6, 0.5, 0.2);
    glVertex3f(0.7, 0.65, 0.2);       //tri front window
    glVertex3f(0.7, 0.5, 0.2);

    glVertex3f(1.2, 0.65, 0.2);
    glVertex3f(1.3, 0.5, 0.2);       //tri back window
    glVertex3f(1.2, 0.5, 0.2);

    glVertex3f(1.2, 0.65, 0.6);
    glVertex3f(1.3, 0.5, 0.6);       //tri back window
    glVertex3f(1.2, 0.5, 0.6);

    glEnd();

    glTranslatef(0.6, 0.2, 0.6); //ban
    glColor3f(0, 0, 0);
    glutSolidTorus(0.025, 0.07, 10, 25);

    glTranslatef(0, 0, -0.4);
    glutSolidTorus(0.025, 0.07, 10, 25);

    glTranslatef(0.6, 0, 0);
    glutSolidTorus(0.025, 0.07, 10, 25);

    glTranslatef(0, 0, 0.4);
    glutSolidTorus(0.025, 0.07, 10, 25);
    glPopMatrix();

//-------------------------------------------------------------------------------
    glPopMatrix();

    glPopMatrix();
}

// display all bullets
void drawBullet(list <Bullet> g) {
    list <Bullet> :: iterator it;
    for(it = g.begin(); it != g.end(); ++it)
    {
        glPushMatrix();
        glTranslatef(it->xPosition, -1.23, it->zPosition);
        if(it->id==0)
            glColor3f(0.5, 1, 1);
        else
            glColor3f(1, 0, 0);

        glBegin(GL_QUADS);
        glVertex3f(0.1, 0.5, 0);
        glVertex3f(-0.1, 0.5, 0);
        glVertex3f(-0.1, 0.5, 0.5);
        glVertex3f(0.1, 0.5, 0.5);
        glEnd();
        glPopMatrix();
    }
}

// text display starts
void drawText(const char *text, int length, int x, int y)
{
    glMatrixMode(GL_PROJECTION);
    auto* matrix=new double[16];
    glGetDoublev(GL_PROJECTION_MATRIX,matrix);
    glLoadIdentity();
    glOrtho(0,800,0,600,-5,5);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRasterPos2i(x,y);
    for(int i=0; i<length; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,(int)text[i]);
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(matrix);
    glMatrixMode(GL_MODELVIEW);
}

void drawAllTheText()
{
    glPushMatrix();
    glColor3f(1,0,0);
    string text;

    int x = 600;
    int y = 580;

    text="Enemy health : " +SSTR( enemyCar.health );
    drawText(text.data(), text.size(), x, y);

    glColor3f(0,1,0);
    text="Player health : " +SSTR( playerCar.health );
    drawText(text.data(), text.size(), x -= 500, y);

    glPopMatrix();
}

void roundTree(double x, double y, double z)
{
    glPushMatrix();
    lightWoodTree();
    //glColor3f(0.9,0.3,0);
    glTranslatef(0+x,0+y,0+z);
    glRotatef(90,1,0,0);
    glScalef(3,.75,20);
    glutSolidTorus(.03,.03,90,100);
    glPopMatrix();

    glPushMatrix();
    lightGreenTree();
    //glColor3f(0,1,0.3);
    glTranslatef(0+x,.6+y,0+z);
    glScalef(6,6,6);
    glutSolidSphere(.1,30,30);
    glPopMatrix();

    lightOff();
}

void coneTree(double x, double y, double z)
{
    glPushMatrix();
    //glColor3f(0.9,0.3,0);
    lightWoodTree();
    glTranslatef(0+x,0+y,0+z);
    glRotatef(90,1,0,0);
    glScalef(2,.75,15);
    glutSolidTorus(.03,.03,90,100);
    glPopMatrix();

    glPushMatrix();
    //glColor3f(0,1,0.3);
    lightGreenTree();
    glTranslatef(0+x,.3+y,0+z);
    glRotatef(90,-1,0,0);
    glutSolidCone(.6,1.5,30,30);
    glPopMatrix();
    lightOff();
    glPopMatrix();
}

void drawHome()
{

//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Windows Front Left
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureWindowsLeft);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,0,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,1.0,10001);    glVertex3f(-1.2,-0.4,1.0001);
    glTexCoord3f(1.0,1.0,10001);    glVertex3f(-0.7,-0.4,1.0001);
    glTexCoord3f(1.0,0.0,10001);    glVertex3f(-0.7,-1.0,1.0001);
    glTexCoord3f(0.0,0.0,10001);    glVertex3f(-1.2,-1.0,1.0001);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();


    // Windows Front top Left
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureWindowsLeft);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,0,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,1.0,10001);    glVertex3f(-1.2,1.0,1.0001);
    glTexCoord3f(1.0,1.0,10001);    glVertex3f(-0.7,1.0,1.0001);
    glTexCoord3f(1.0,0.0,10001);    glVertex3f(-0.7,0.4,1.0001);
    glTexCoord3f(0.0,0.0,10001);    glVertex3f(-1.2,0.4,1.0001);
    glEnd();
    glDisable(GL_TEXTURE_2D);


    glPopMatrix();


    // Windows Front Right
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureWindowsLeft);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,0,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,1.0,10001);    glVertex3f(0.7,-0.4,1.0001);
    glTexCoord3f(1.0,1.0,10001);    glVertex3f(1.2,-0.4,1.0001);
    glTexCoord3f(1.0,0.0,10001);    glVertex3f(1.2,-1.0,1.0001);
    glTexCoord3f(0.0,0.0,10001);    glVertex3f(0.7,-1.0,1.0001);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

    // Windows Top Front Right
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureWindowsLeft);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,0,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,1.0,10001);    glVertex3f(0.7,1.0,1.0001);
    glTexCoord3f(1.0,1.0,10001);    glVertex3f(1.2,1.0,1.0001);
    glTexCoord3f(1.0,0.0,10001);    glVertex3f(1.2,0.4,1.0001);
    glTexCoord3f(0.0,0.0,10001);    glVertex3f(0.7,0.4,1.0001);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();


    // Windows Back Right
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureWindowsLeft);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,0,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,1.0,-10001);    glVertex3f(0.7,-0.4,-6.0001);
    glTexCoord3f(1.0,1.0,-10001);    glVertex3f(1.2,-0.4,-6.0001);
    glTexCoord3f(1.0,0.0,-10001);    glVertex3f(1.2,-1.0,-6.0001);
    glTexCoord3f(0.0,0.0,-10001);    glVertex3f(0.7,-1.0,-6.0001);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

    // Windows Back Left
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureWindowsLeft);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,0,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,1.0,-10001);    glVertex3f(-1.2,-0.4,-6.0001);
    glTexCoord3f(1.0,1.0,-10001);    glVertex3f(-0.7,-0.4,-6.0001);
    glTexCoord3f(1.0,0.0,-10001);    glVertex3f(-0.7,-1.0,-6.0001);
    glTexCoord3f(0.0,0.0,-10001);    glVertex3f(-1.2,-1.0,-6.0001);
    glEnd();
    glDisable(GL_TEXTURE_2D);


    glPopMatrix();

    // Bagian Depan
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,0,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,2.0,0.1);  glVertex3f(-2,0,1);
    glTexCoord3f(4.0,2.0,0.1);  glVertex3f(2,0,1);
    glTexCoord3f(4.0,0.0,0.1);  glVertex3f(2,-1.5,1);
    glTexCoord3f(0.0,0.0,0.1);  glVertex3f(-2,-1.5,1);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

    // Bagian Belakang
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,0,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,2.0,-1);  glVertex3f(-2,0,-6);
    glTexCoord3f(4.0,2.0,-1);  glVertex3f(2,0,-6);
    glTexCoord3f(4.0,0.0,-1);  glVertex3f(2,-1.5,-6);
    glTexCoord3f(0.0,0.0,-1);  glVertex3f(-2,-1.5,-6);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // Kanan
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,0,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,2.0,1); glVertex3f(2,0,1);
    glTexCoord3f(2.0,2.0,-1); glVertex3f(2,0,-6);
    glTexCoord3f(2.0,0.0,-1); glVertex3f(2,-1.5,-6);
    glTexCoord3f(0.0,0.0,1); glVertex3f(2,-1.5,1);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // Kiri
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,0,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,2.0,1);    glVertex3f(-2,0,1);
    glTexCoord3f(2.0,2.0,-1);    glVertex3f(-2,0,-6);
    glTexCoord3f(2.0,0.0,-1);    glVertex3f(-2,-1.5,-6);
    glTexCoord3f(0.0,0.0,1);    glVertex3f(-2,-1.5,1);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

    // second wall
    // Bagian Depan
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,2.0,0.1);  glVertex3f(-2,0,1);
    glTexCoord3f(4.0,2.0,0.1);  glVertex3f(2,0,1);
    glTexCoord3f(4.0,0.0,0.1);  glVertex3f(2,-1.5,1);
    glTexCoord3f(0.0,0.0,0.1);  glVertex3f(-2,-1.5,1);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

    // Bagian Belakang
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,2.0,-1);  glVertex3f(-2,0,-1);
    glTexCoord3f(4.0,2.0,-1);  glVertex3f(2,0,-1);
    glTexCoord3f(4.0,0.0,-1);  glVertex3f(2,-1.5,-1);
    glTexCoord3f(0.0,0.0,-1);  glVertex3f(-2,-1.5,-1);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // Kanan
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,2.0,1); glVertex3f(2,0,1);
    glTexCoord3f(2.0,2.0,-1); glVertex3f(2,0,-1);
    glTexCoord3f(2.0,0.0,-1); glVertex3f(2,-1.5,-1);
    glTexCoord3f(0.0,0.0,1); glVertex3f(2,-1.5,1);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // Kiri
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,2.0,1);    glVertex3f(-2,0,1);
    glTexCoord3f(2.0,2.0,-1);    glVertex3f(-2,0,-1);
    glTexCoord3f(2.0,0.0,-1);    glVertex3f(-2,-1.5,-1);
    glTexCoord3f(0.0,0.0,1);    glVertex3f(-2,-1.5,1);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

    //third wall
    // Bagian Depan
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,2.0,0.1);  glVertex3f(-2,0,-4);
    glTexCoord3f(4.0,2.0,0.1);  glVertex3f(2,0,-4);
    glTexCoord3f(4.0,0.0,0.1);  glVertex3f(2,-1.5,-4);
    glTexCoord3f(0.0,0.0,0.1);  glVertex3f(-2,-1.5,-4);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

    // Bagian Belakang
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,2.0,-1);  glVertex3f(-2,0,-6);
    glTexCoord3f(4.0,2.0,-1);  glVertex3f(2,0,-6);
    glTexCoord3f(4.0,0.0,-1);  glVertex3f(2,-1.5,-6);
    glTexCoord3f(0.0,0.0,-1);  glVertex3f(-2,-1.5,-6);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // Kanan
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,2.0,1); glVertex3f(2,0,-4);
    glTexCoord3f(2.0,2.0,-1); glVertex3f(2,0,-6);
    glTexCoord3f(2.0,0.0,-1); glVertex3f(2,-1.5,-6);
    glTexCoord3f(0.0,0.0,1); glVertex3f(2,-1.5,-4);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // Kiri
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,2.0,1);    glVertex3f(-2,0,-4);
    glTexCoord3f(2.0,2.0,-1);    glVertex3f(-2,0,-6);
    glTexCoord3f(2.0,0.0,-1);    glVertex3f(-2,-1.5,-6);
    glTexCoord3f(0.0,0.0,1);    glVertex3f(-2,-1.5,-4);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

    //Pintu
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureDoor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,0,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS);  // Wall
    glTexCoord3f(0.0,1.0,10001);    glVertex3f(-0.3,-0.4,1.0001);
    glTexCoord3f(1.0,1.0,10001);    glVertex3f(0.3,-0.4,1.0001);
    glTexCoord3f(1.0,0.0,10001);    glVertex3f(0.3,-1.5,1.0001);
    glTexCoord3f(0.0,0.0,10001);    glVertex3f(-0.3,-1.5,1.0001);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

    // Roof 1
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureRoof);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS); //Atap Depan
    glTexCoord3f(0.0,2.0,0); glVertex3f(-2.2,0.5,0);
    glTexCoord3f(4.0,2.0,0); glVertex3f(2.2,0.5,0);
    glTexCoord3f(4.0,0.0,1.25); glVertex3f(2.2,-0.1,1.25);
    glTexCoord3f(0.0,.00,1.25); glVertex3f(-2.2,-0.1,1.25);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureRoof);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS); //Atap Belakang
    glTexCoord3f(0.0,2.0,0); glVertex3f(-2.2,0.5,0);
    glTexCoord3f(4.0,2.0,0); glVertex3f(2.2,0.5,0);
    glTexCoord3f(4.0,0.0,-1.25); glVertex3f(2.2,-0.1,-1.25);
    glTexCoord3f(0.0,.00,-1.25); glVertex3f(-2.2,-0.1,-1.25);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // Roof 2
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureRoof);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS); //Atap Depan
    glTexCoord3f(0.0,2.0,0); glVertex3f(-2.2,0.5,-5);
    glTexCoord3f(4.0,2.0,0); glVertex3f(2.2,0.5,-5);
    glTexCoord3f(4.0,0.0,1.25); glVertex3f(2.2,-0.1,-3.75);
    glTexCoord3f(0.0,.00,1.25); glVertex3f(-2.2,-0.1,-3.75);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureRoof);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_QUADS); //Atap Belakang
    glTexCoord3f(0.0,2.0,0); glVertex3f(-2.2,0.5,-5);
    glTexCoord3f(4.0,2.0,0); glVertex3f(2.2,0.5,-5);
    glTexCoord3f(4.0,0.0,-1.25); glVertex3f(2.2,-0.1,-6.25);
    glTexCoord3f(0.0,.00,-1.25); glVertex3f(-2.2,-0.1,-6.25);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    //wall roof 1
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_TRIANGLES); //Atap Kanan
    glTexCoord3f(0.0,1.0,0); glVertex3f(2,0.5,0);
    glTexCoord3f(1.0,0.0,1); glVertex3f(2,0,1);
    glTexCoord3f(-1.0,0.0,-1); glVertex3f(2,0,-1);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_TRIANGLES); //Atap Kiri
    glTexCoord3f(0.0,1.0,0); glVertex3f(-2,0.5,0);
    glTexCoord3f(1.0,0.0,1); glVertex3f(-2,0,1);
    glTexCoord3f(-1.0,0.0,-1); glVertex3f(-2,0,-1);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    //wall roof 2
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_TRIANGLES); //Atap Kanan
    glTexCoord3f(0.0,1.0,0); glVertex3f(2,0.5,-5);
    glTexCoord3f(1.0,0.0,1); glVertex3f(2,0,-4);
    glTexCoord3f(-1.0,0.0,-1); glVertex3f(2,0,-6);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureBricks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTranslatef(0,1.5,-6-4);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glBegin(GL_TRIANGLES); //Atap Kiri
    glTexCoord3f(0.0,1.0,0); glVertex3f(-2,0.5,-5);
    glTexCoord3f(1.0,0.0,1); glVertex3f(-2,0,-4);
    glTexCoord3f(-1.0,0.0,-1); glVertex3f(-2,0,-6);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

//    glutSwapBuffers();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawAllTheText();

    drawSky();

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW); //Switch to the drawing perspective
    glLoadIdentity(); //Reset the drawing perspective

    glRotatef(-cameraAngle, 0.0, 1.0, 0.0); //Rotate the camera

    if(!isFirstPerson)
        glTranslatef(moveCamX, -0.5, -7.0); //init perspective cam car
    else
        glTranslatef(moveCamX, 0.2, -3.0); // completely dissapear z = -2.5

    // camera bullet
    glRotatef(viewCam, 0.0, 1.0, 0.0);
    glRotatef(angleCAM, 0.0, 1.0, 0.0);

    // draw bullet as long as bullet is fired
    if (!bullets.empty())
        drawBullet(bullets);

    // draw car as long as the health is not 0
    if (!isEnemyDead)
        drawEnemyCar();
    if (!isPlayerDead)
        drawPlayerCar();

    glTranslatef(0.0, 0.0, mapDistance); // move car by z

    drawRoad();

    glFlush();
    glutSwapBuffers();
}
/* drawing ends */

GLuint loadTexture(const char *filename) {
    BmpLoader bl(filename);
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 bl.iWidth, bl.iHeight,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 bl.textureData);
    return textureId;
}

void initialiseTexture() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0, 1.00, 1.0, 200.0);

    grassTexture = loadTexture("images/fence.bmp");
    roadTexture = loadTexture("images/road2.bmp");
    skyTexture = loadTexture("images/skybox-top.bmp");

    textureSky = loadTexture("images/sky.bmp");
    textureBricks = loadTexture("images/pager(1).bmp");
    textureRoof = loadTexture("images/atap.bmp");
    textureDoor = loadTexture("images/door.bmp");
    textureWindowsLeft = loadTexture("images/window_reds.bmp");
}

void update(int value) {
    if (!bullets.empty())
        moveBullets(bullets);

    if (!pause) {
        // move z map positon
        mapDistance += .5;

        // enemy random bullets position to be fired
        int randomBullets = rand()%6;
        if(randomBullets == 0){
            enemyBullet.id=1;
            enemyBullet.zPosition=-13;
            enemyBullet.xPosition=enemyCar.xPosition;
            bullets.push_back(enemyBullet);
        }

        // enemy movement on X axis
        if(reversal){
            enemyCar.xPosition += 0.25; // move to the right
            if (enemyCar.xPosition == 4.0) // check if is it on the rightest road
                reversal = false;
        }
        else{
            enemyCar.xPosition -= 0.25; // // move to the left
            if (enemyCar.xPosition == -4.0)
                reversal = true;
        }

        // check if path end & if yes then restart path
        if (mapDistance > 100)
            mapDistance = 0;
    }

    if (leftRotate)
        angleCAM += 5.0, leftRotate = false;
    else if (rightRotate)
        angleCAM -= 5.0, rightRotate = false;

    if (angleCAM > 360)
        angleCAM -= 360;

    if (angleCAM < 0)
        angleCAM += 360;

    glutPostRedisplay();
    glutTimerFunc(speed, update, 0);
}

void keyboardHandle(unsigned char key, int x, int y) {
    if (key == 'f' || key == 'F') {
        GLfloat tempPos;
        playerBullet.id=0;
        playerBullet.zPosition=2;

        tempPos = manyClicks * 0.17;

        playerBullet.xPosition=tempPos;
        bullets.push_back(playerBullet);
    }

    if (key == 'h' || key == 'H') // pause or play
    {
        speed = 100;
        pause = true;
    }

    if ((key == 'w' || key == 'W') && speed > 5)
        speed -= 2;

    if ((key == 's' || key == 'S') && speed < 100)
        speed += 2;

    if (100.0 - speed == 0.0) {
        pause = true;
    } else
        pause = false;

    if ((key == 'a' || key == 'A')) // &&!pause
    {
        if (playerCar.xPosition < 6.0) {
            playerCar.xPosition += 0.25;
            manyClicks -= 1.0;
//            xPosition += .25;
            moveCamX += .12;
            angleCAM += .2;
        }
    }

    if ((key == 'd' || key == 'D')) //&&!pause
    {
        if (playerCar.xPosition > -6.0) {
            playerCar.xPosition -= 0.25;
            manyClicks += 1.0;
//            xPosition -= .25;
            moveCamX -= .12;
            angleCAM -= .2;
        }
    }

    if (key == 'b' || key == 'B') // look back or look front
    {
        if (viewCam != 180)
            viewCam = 180, lookFront = false;
        else
            viewCam = 0, lookFront = true;
    }

    if (key == 'l' || key == 'L') {
        if (!leftRotate)
            leftRotate = true;
        else
            leftRotate = false;
    }

    if (key == 'j' || key == 'J') {
        if (!rightRotate)
            rightRotate = true;
        else
            rightRotate = false;
    }

    if (key == 'z' || key== 'Z'){
        isFirstPerson = !isFirstPerson;
    }

    glutPostRedisplay();
}

//Called when the window is resized
void handleResize(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double) w / (double) h, 1.0, 200.0);
}

void init() {
    pause = true;

    initCarPosition();

    isEnemyDead = false;
    isPlayerDead = false;

    leftRotate = false;
    rightRotate = false;
    lookFront = true;
    isFirstPerson = false;
    angleCAM = 0.0;

    cameraAngle = 0.0;
    playerCar.zPosition = 0.0;
    mapDistance = 0.0;
    viewCam = 0;

    playerCar.xPosition = 0.0, moveCamX = 0.0;

    speed = 100;

    playerBullet.zPosition = -2;
    playerBullet.xPosition = 0.0;
    enemyBullet.zPosition = 20;
    enemyBullet.xPosition = 0.0;
}

int main(int argc, char **argv) {
    //Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1224, 568);
//    srand(time(nullptr));

    //Create the window
    glutCreateWindow("Battle Car");

    //Set handler functions
    glEnable(GL_DEPTH_TEST);
    initialiseTexture();

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(handleResize);

    glutKeyboardFunc(keyboardHandle);

    glutTimerFunc(speed, update, 0); //Add a timer
    glutMainLoop();

    return 0;
}