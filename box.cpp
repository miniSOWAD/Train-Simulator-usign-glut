
#include <windows.h>
#include <GL/glut.h>
#include <cmath>

// Transformation variables
float tx = 0.0f, ty = 0.0f;
float angle = 0.0f;
float sx = 1.0f, sy = 1.0f;

// Toggle rotation pivot using P
bool rotateAroundFixedPivot = false;

// Fixed pivot point for the fifth task
const float pivotX = 250.0f;
const float pivotY = 250.0f;

// Structure to store a 2D point
struct Point {
    float x, y;
};

// --------------------------------------------------
// USER-DEFINED FUNCTION 1: TRANSLATION
// --------------------------------------------------
void translatePoint(Point &p, float dx, float dy) {
    p.x = p.x + dx;
    p.y = p.y + dy;
}

// --------------------------------------------------
// USER-DEFINED FUNCTION 2: ROTATION
// Rotates a point around a specified pivot
// --------------------------------------------------
void rotatePoint(Point &p, float degrees,
                 float cx, float cy) {

    float radians = degrees * 3.14159265f / 180.0f;

    // Move pivot to the origin
    float x = p.x - cx;
    float y = p.y - cy;

    // Apply rotation equations
    float newX = x * cos(radians) - y * sin(radians);
    float newY = x * sin(radians) + y * cos(radians);

    // Move back to the original coordinate system
    p.x = newX + cx;
    p.y = newY + cy;
}

// --------------------------------------------------
// USER-DEFINED FUNCTION 3: SCALING
// Scales a point relative to a fixed point
// --------------------------------------------------
void scalePoint(Point &p, float scaleX, float scaleY,
                float cx, float cy) {

    p.x = cx + (p.x - cx) * scaleX;
    p.y = cy + (p.y - cy) * scaleY;
}

// --------------------------------------------------
// USER-DEFINED FUNCTION 4: ROTATION ABOUT ORIGIN
// --------------------------------------------------
void rotateAroundOrigin(Point &p, float degrees) {
    rotatePoint(p, degrees, 0.0f, 0.0f);
}

// --------------------------------------------------
// INITIALIZATION
// --------------------------------------------------
void init() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 500.0, 0.0, 500.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// --------------------------------------------------
// DISPLAY FUNCTION
// --------------------------------------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Original triangle vertices
    Point p1 = {150.0f, 150.0f};
    Point p2 = {250.0f, 150.0f};
    Point p3 = {200.0f, 250.0f};

    // Calculate the triangle's centroid
    float cx = (p1.x + p2.x + p3.x) / 3.0f;
    float cy = (p1.y + p2.y + p3.y) / 3.0f;

    // Apply scaling using the user-defined function
    scalePoint(p1, sx, sy, cx, cy);
    scalePoint(p2, sx, sy, cx, cy);
    scalePoint(p3, sx, sy, cx, cy);

    // Apply rotation using user-defined functions
    if (rotateAroundFixedPivot) {
        rotatePoint(p1, angle, pivotX, pivotY);
        rotatePoint(p2, angle, pivotX, pivotY);
        rotatePoint(p3, angle, pivotX, pivotY);
    } else {
        rotatePoint(p1, angle, cx, cy);
        rotatePoint(p2, angle, cx, cy);
        rotatePoint(p3, angle, cx, cy);
    }

    // Apply translation using the user-defined function
    translatePoint(p1, tx, ty);
    translatePoint(p2, tx, ty);
    translatePoint(p3, tx, ty);

    // Draw the transformed triangle
    glBegin(GL_TRIANGLES);

        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex2f(p1.x, p1.y);

        glColor3f(0.0f, 0.7f, 0.0f);
        glVertex2f(p2.x, p2.y);

        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex2f(p3.x, p3.y);

    glEnd();

    // Show the fixed pivot when pivot rotation is enabled
    if (rotateAroundFixedPivot) {
        glPointSize(7.0f);
        glColor3f(0.0f, 0.0f, 0.0f);

        glBegin(GL_POINTS);
            glVertex2f(pivotX, pivotY);
        glEnd();
    }

    glFlush();
}

// --------------------------------------------------
// KEYBOARD SPECIAL FUNCTION
// Arrow keys: Translation
// F1 / F3: Rotation
// --------------------------------------------------
void special(int key, int, int) {
    switch (key) {
        case GLUT_KEY_LEFT:
            tx -= 5.0f;
            break;

        case GLUT_KEY_RIGHT:
            tx += 5.0f;
            break;

        case GLUT_KEY_UP:
            ty += 5.0f;
            break;

        case GLUT_KEY_DOWN:
            ty -= 5.0f;
            break;

        case GLUT_KEY_F1:
            angle += 5.0f;
            break;

        case GLUT_KEY_F3:
            angle -= 5.0f;
            break;

        default:
            return;
    }

    glutPostRedisplay();
}

// --------------------------------------------------
// NORMAL KEYBOARD FUNCTION
// + / - : Scaling
// P     : Toggle rotation pivot
// R     : Reset all transformations
// ESC   : Exit
// --------------------------------------------------
void keyboard(unsigned char key, int, int) {
    switch (key) {
        case '+':
        case '=':
            sx *= 1.1f;
            sy *= 1.1f;
            break;

        case '-':
        case '_':
            sx *= 0.9f;
            sy *= 0.9f;
            break;

        case 'p':
        case 'P':
            rotateAroundFixedPivot = !rotateAroundFixedPivot;
            break;

        case 'r':
        case 'R':
            tx = 0.0f;
            ty = 0.0f;
            angle = 0.0f;
            sx = 1.0f;
            sy = 1.0f;
            rotateAroundFixedPivot = false;
            break;

        case 27:
            exit(0);
    }

    glutPostRedisplay();
}

// --------------------------------------------------
// MAIN FUNCTION
// --------------------------------------------------
int main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(80, 80);
    glutInitWindowSize(500, 500);

    glutCreateWindow("Computer Graphics Lab - 2D Transformations");

    init();

    glutDisplayFunc(display);
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);

    glutMainLoop();

    return 0;
}
