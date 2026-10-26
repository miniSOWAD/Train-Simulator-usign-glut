#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

const float PI = 3.14159265358979323846f;

// ============================================================
// PROCEDURAL TEXTURE SYSTEM (VISUAL UPGRADE)
// ============================================================
GLuint texNoise, texConcrete, texGlassPanel, texBuilding, texGrass;

void generateTextures()
{
    // 1. Dark Noise/Grain (Used for Asphalt, Track Bed)
    glGenTextures(1, &texNoise);
    glBindTexture(GL_TEXTURE_2D, texNoise);
    unsigned char dataN[128][128][3];
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            unsigned char c = 60 + rand() % 40;
            dataN[i][j][0] = dataN[i][j][1] = dataN[i][j][2] = c;
        }
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, dataN);

    // 2. Concrete (Used for modern Metro slab tracks and pillars)
    glGenTextures(1, &texConcrete);
    glBindTexture(GL_TEXTURE_2D, texConcrete);
    unsigned char dataC[128][128][3];
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            unsigned char c = 160 + (rand() % 20) - 10;
            dataC[i][j][0] = dataC[i][j][1] = dataC[i][j][2] = c;
        }
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 128, 128, GL_RGB, GL_UNSIGNED_BYTE, dataC);

    // 3. Modern Glass/Steel Panel (Used for Station Buildings)
    glGenTextures(1, &texGlassPanel);
    glBindTexture(GL_TEXTURE_2D, texGlassPanel);
    unsigned char dataP[128][128][3];
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            bool edgeX = (i % 64 < 3);
            bool edgeY = (j % 64 < 3);
            if (edgeX || edgeY) {
                dataP[i][j][0] = 30; dataP[i][j][1] = 35; dataP[i][j][2] = 40;
            } else {
                float gradient = (float)j / 128.0f;
                dataP[i][j][0] = (unsigned char)(60 + gradient * 40);
                dataP[i][j][1] = (unsigned char)(90 + gradient * 60);
                dataP[i][j][2] = (unsigned char)(120 + gradient * 80);
            }
        }
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, dataP);

    // 4. Skyscraper Windows (Used for City)
    glGenTextures(1, &texBuilding);
    glBindTexture(GL_TEXTURE_2D, texBuilding);
    unsigned char dataBldg[128][128][3];
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            bool windowX = (i % 16 > 2);
            bool windowY = (j % 16 > 2);
            if (windowX && windowY) {
                // Randomly light up some windows
                if ((i / 16 * 7 + j / 16 * 11) % 5 == 0) {
                    dataBldg[i][j][0] = 220; dataBldg[i][j][1] = 230; dataBldg[i][j][2] = 255; // Lit
                } else {
                    dataBldg[i][j][0] = 20; dataBldg[i][j][1] = 25; dataBldg[i][j][2] = 30; // Dark
                }
            } else {
                dataBldg[i][j][0] = 10; dataBldg[i][j][1] = 10; dataBldg[i][j][2] = 15; // Frame
            }
        }
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, dataBldg);

    // 5. Grass/Rock Terrain (Used for Mountains)
    glGenTextures(1, &texGrass);
    glBindTexture(GL_TEXTURE_2D, texGrass);
    unsigned char dataG[128][128][3];
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            unsigned char noise = rand() % 40;
            dataG[i][j][0] = 40 + noise; // R
            dataG[i][j][1] = 70 + noise * 2; // G
            dataG[i][j][2] = 35 + noise; // B
        }
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 128, 128, GL_RGB, GL_UNSIGNED_BYTE, dataG);
}

// ============================================================
// BASIC STRUCTURES
// ============================================================

struct Vec2
{
    float x;
    float z;
};

struct Station
{
    const char* name;
    float x;
    float z;
    float sizeX;
    float sizeZ;
};

// ============================================================
// TRACK DATA
// ============================================================

std::vector<Vec2> trackPoints;
std::vector<float> cumulativeLength;
float totalTrackLength = 0.0f;

// ============================================================
// TRAIN VARIABLES
// ============================================================

float trainDistance = 0.0f;
float trainSpeed = 0.0f;
float throttle = 0.0f;
float brake = 0.0f;
bool manualBrakeApplied = false;

const float MAX_SPEED = 100.0f / 3.6f;
const float ENGINE_ACCELERATION = 7.0f;
const float THROTTLE_RAMP_RATE = 1.25f;
const float MAX_BRAKE_DECELERATION = 8.5f;
int trainDirection = 1;

const float METRO_CAB_OFFSET = 5.35f;
const float INITIAL_TRAIN_DISTANCE = 7.0f;

bool waitingAtTerminal = false;
bool terminalThrottleArmed = true;

const float TERMINAL_BRAKE_LOOKAHEAD = 82.0f;

// ============================================================
// KEY STATES
// ============================================================

bool keyW = false;
bool keyS = false;

// ============================================================
// CAMERA
// ============================================================

int cameraMode = 2;

float camPanX[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
float camPanZ[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
float camLift[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

float droneCamX = -5.0f;
float droneCamY = 12.0f;
float droneCamZ = 48.0f;
float droneYaw = 180.0f;
float dronePitch = -12.0f;

float personMoveSpeed = 3.0f;
float personVerticalSpeed = 2.5f;
float personRotationStep = 10.0f;

// Third-person human player, initially spawned at Station 1.
float humanX = -36.0f;
float humanY = 0.0f;
float humanZ = -43.0f;
float humanYaw = 180.0f;
float humanCameraYaw = 180.0f;
float humanCameraPitch = -8.0f;
const float humanMoveSpeed = 2.0f;
const float humanCameraDistance = 4.5f;
const float humanCameraHeight = 2.6f;

// Boarding system for the middle metro car only.
bool metroDoorsOpen = true;
bool humanOnboard = false;
float humanLocalX = 0.0f; // Position inside the car, in car-local coordinates.
float humanLocalZ = 0.0f;
float lastOnboardTrainYaw = 0.0f;

float activeCameraX = 0.0f;
float activeCameraY = 96.0f;
float activeCameraZ = 0.0f;

int lastTime = 0;

// ============================================================
// STATIONS
// ============================================================

Station stations[4] =
{
    { "Station 1", -36.0f, -43.0f, 6.0f, 3.0f },
    { "Station 2", -36.0f,  -8.0f, 7.0f, 3.5f },
    { "Station 3",  32.0f,  16.0f, 7.0f, 3.5f },
    { "Station 4", -38.0f,  54.0f, 7.0f, 3.5f }
};

// ============================================================
// STATION PASS COUNTERS
// ============================================================

int stationPassCount[4] = {0, 0, 0, 0};
float stationTrackDistance[4] = {0.0f, 0.0f, 0.0f, 0.0f};

// ============================================================
// METRO CARD UI / STATE
// ============================================================

bool metroCardOwned = false;
bool metroCardVisible = false;
int cardStationCount[4] = {0, 0, 0, 0};

int selectedFromStation = -1;
int selectedToStation = -1;
int cardFromStation = -1;
int cardToStation = -1;

bool cardJourneyReachedTarget = false;
int selectedFoilOption = -1;

struct Rect2D
{
    float x;
    float y;
    float width;
    float height;
};

// ============================================================
// UTILITY
// ============================================================

float clampValue(float value, float minimum, float maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

float distance2D(Vec2 a, Vec2 b)
{
    float dx = a.x - b.x;
    float dz = a.z - b.z;
    return sqrtf(dx * dx + dz * dz);
}

Vec2 normalize2D(Vec2 v)
{
    float length = sqrtf(v.x * v.x + v.z * v.z);
    if (length < 0.0001f) return {0.0f, 0.0f};
    return { v.x / length, v.z / length };
}

// ============================================================
// TEXTURED & DETAILED CUBE
// ============================================================

void drawCube(
    float x, float y, float z,
    float sx, float sy, float sz,
    float r, float g, float b,
    GLuint texID = 0
)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(r, g, b);

    if (texID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texID);
    }

    float hx = sx * 0.5f;
    float hy = sy * 0.5f;
    float hz = sz * 0.5f;

    glBegin(GL_QUADS);
    
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-hx, -hy, hz);
    glTexCoord2f(sx, 0.0f);   glVertex3f(hx, -hy, hz);
    glTexCoord2f(sx, sy);     glVertex3f(hx, hy, hz);
    glTexCoord2f(0.0f, sy);   glVertex3f(-hx, hy, hz);
    
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(hx, -hy, -hz);
    glTexCoord2f(sx, 0.0f);   glVertex3f(-hx, -hy, -hz);
    glTexCoord2f(sx, sy);     glVertex3f(-hx, hy, -hz);
    glTexCoord2f(0.0f, sy);   glVertex3f(hx, hy, -hz);
    
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-hx, hy, hz);
    glTexCoord2f(sx, 0.0f);   glVertex3f(hx, hy, hz);
    glTexCoord2f(sx, sz);     glVertex3f(hx, hy, -hz);
    glTexCoord2f(0.0f, sz);   glVertex3f(-hx, hy, -hz);
    
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-hx, -hy, -hz);
    glTexCoord2f(sx, 0.0f);   glVertex3f(hx, -hy, -hz);
    glTexCoord2f(sx, sz);     glVertex3f(hx, -hy, hz);
    glTexCoord2f(0.0f, sz);   glVertex3f(-hx, -hy, hz);
    
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(hx, -hy, hz);
    glTexCoord2f(sz, 0.0f);   glVertex3f(hx, -hy, -hz);
    glTexCoord2f(sz, sy);     glVertex3f(hx, hy, -hz);
    glTexCoord2f(0.0f, sy);   glVertex3f(hx, hy, hz);
    
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-hx, -hy, -hz);
    glTexCoord2f(sz, 0.0f);   glVertex3f(-hx, -hy, hz);
    glTexCoord2f(sz, sy);     glVertex3f(-hx, hy, hz);
    glTexCoord2f(0.0f, sy);   glVertex3f(-hx, hy, -hz);

    glEnd();

    if (texID != 0) {
        glDisable(GL_TEXTURE_2D);
    }

    glPopMatrix();
}

// ============================================================
// TRACK GENERATION FUNCTIONS
// ============================================================

void pushTrackPoint(float x, float z)
{
    if (!trackPoints.empty()) {
        Vec2 last = trackPoints.back();
        if (distance2D(last, {x, z}) < 0.01f) return;
    }
    trackPoints.push_back({x, z});
}

void addLine(float x1, float z1, float x2, float z2, int samples)
{
    for (int i = 0; i <= samples; i++) {
        float t = (float)i / (float)samples;
        pushTrackPoint(x1 + (x2 - x1) * t, z1 + (z2 - z1) * t);
    }
}

void addArc(float centerX, float centerZ, float radius, float startDegrees, float endDegrees, int samples)
{
    for (int i = 0; i <= samples; i++) {
        float t = (float)i / (float)samples;
        float degrees = startDegrees + (endDegrees - startDegrees) * t;
        float radians = degrees * PI / 180.0f;
        pushTrackPoint(centerX + radius * cosf(radians), centerZ + radius * sinf(radians));
    }
}

void buildTrack()
{
    trackPoints.clear();

    const float START_X = -58.0f;
    const float LEFT_TURN_X = -48.0f;
    const float RIGHT_TURN_X = 48.0f;
    const float END_X = -58.0f;
    const float TURN_RADIUS = 6.0f;

    const float Z0 = -36.0f;
    const float Z1 = -24.0f;
    const float Z2 = -12.0f;
    const float Z3 =   0.0f;
    const float Z4 =  12.0f;
    const float Z5 =  24.0f;
    const float Z6 =  36.0f;
    const float Z7 =  48.0f;

    pushTrackPoint(START_X, Z0);
    addLine(START_X, Z0, RIGHT_TURN_X, Z0, 110);
    addArc(RIGHT_TURN_X, (Z0 + Z1) * 0.5f, TURN_RADIUS, -90.0f, 90.0f, 36);
    addLine(RIGHT_TURN_X, Z1, LEFT_TURN_X, Z1, 100);
    addArc(LEFT_TURN_X, (Z1 + Z2) * 0.5f, TURN_RADIUS, -90.0f, -270.0f, 36);
    addLine(LEFT_TURN_X, Z2, RIGHT_TURN_X, Z2, 100);
    addArc(RIGHT_TURN_X, (Z2 + Z3) * 0.5f, TURN_RADIUS, -90.0f, 90.0f, 36);
    addLine(RIGHT_TURN_X, Z3, LEFT_TURN_X, Z3, 100);
    addArc(LEFT_TURN_X, (Z3 + Z4) * 0.5f, TURN_RADIUS, -90.0f, -270.0f, 36);
    addLine(LEFT_TURN_X, Z4, RIGHT_TURN_X, Z4, 100);
    addArc(RIGHT_TURN_X, (Z4 + Z5) * 0.5f, TURN_RADIUS, -90.0f, 90.0f, 36);
    addLine(RIGHT_TURN_X, Z5, LEFT_TURN_X, Z5, 100);
    addArc(LEFT_TURN_X, (Z5 + Z6) * 0.5f, TURN_RADIUS, -90.0f, -270.0f, 36);
    addLine(LEFT_TURN_X, Z6, RIGHT_TURN_X, Z6, 100);
    addArc(RIGHT_TURN_X, (Z6 + Z7) * 0.5f, TURN_RADIUS, -90.0f, 90.0f, 36);
    addLine(RIGHT_TURN_X, Z7, END_X, Z7, 110);

    cumulativeLength.clear();
    cumulativeLength.push_back(0.0f);

    for (size_t i = 1; i < trackPoints.size(); i++) {
        float segment = distance2D(trackPoints[i - 1], trackPoints[i]);
        cumulativeLength.push_back(cumulativeLength.back() + segment);
    }
    totalTrackLength = cumulativeLength.back();
}

float clampTrackDistance(float distance)
{
    if (distance < 0.0f) return 0.0f;
    if (distance > totalTrackLength) return totalTrackLength;
    return distance;
}

Vec2 sampleTrackPosition(float distance)
{
    distance = clampTrackDistance(distance);
    for (size_t i = 0; i < cumulativeLength.size() - 1; i++) {
        if (distance >= cumulativeLength[i] && distance <= cumulativeLength[i + 1]) {
            float segmentLength = cumulativeLength[i + 1] - cumulativeLength[i];
            float t = (segmentLength < 0.0001f) ? 0.0f : (distance - cumulativeLength[i]) / segmentLength;
            Vec2 a = trackPoints[i];
            Vec2 b = trackPoints[i + 1];
            return { a.x + (b.x - a.x) * t, a.z + (b.z - a.z) * t };
        }
    }
    return trackPoints.back();
}

Vec2 sampleTrackTangent(float distance)
{
    Vec2 before = sampleTrackPosition(distance - 0.35f);
    Vec2 after = sampleTrackPosition(distance + 0.35f);
    return normalize2D({ after.x - before.x, after.z - before.z });
}

float findNearestTrackDistance(float x, float z)
{
    float bestDistance = 0.0f;
    float bestError = 1.0e30f;
    for (size_t i = 0; i < trackPoints.size(); i++) {
        float dx = trackPoints[i].x - x;
        float dz = trackPoints[i].z - z;
        float error = dx * dx + dz * dz;
        if (error < bestError) {
            bestError = error;
            bestDistance = cumulativeLength[i];
        }
    }
    return bestDistance;
}

bool crossedTrackMarker(float previousDistance, float currentDistance, float markerDistance)
{
    if (fabsf(currentDistance - previousDistance) < 0.00001f) return false;
    if (currentDistance > previousDistance) return markerDistance > previousDistance && markerDistance <= currentDistance;
    return markerDistance < previousDistance && markerDistance >= currentDistance;
}

void updateStationPassCounters(float previousDistance, float currentDistance)
{
    if (!metroCardOwned) return;
    bool cardCounterFrozen = selectedFoilOption == 2;
    for (int i = 0; i < 4; i++) {
        if (crossedTrackMarker(previousDistance, currentDistance, stationTrackDistance[i])) {
            stationPassCount[i]++;
            if (i == cardToStation) cardJourneyReachedTarget = true;
            if (!cardCounterFrozen) cardStationCount[i]++;
        }
    }
}

// ============================================================
// PROCEDURAL TERRAIN (VISUAL UPGRADE)
// ============================================================

float getTerrainHeight(float x, float z) {
    // The track area is flat to preserve logic. Bounding box ~ (-65 to 65, -55 to 55)
    float dX = fabsf(x) - 65.0f;
    float dZ = fabsf(z) - 55.0f;
    float distToTrackEdge = 0.0f;
    if (dX > 0) distToTrackEdge += dX;
    if (dZ > 0) distToTrackEdge += dZ;

    if (distToTrackEdge <= 0.0f) {
        return -0.15f; // Flat track area
    }

    // Generate rolling hills outside the track area
    float height = distToTrackEdge * 0.35f; // Base slope up
    height += sinf(x * 0.08f) * 6.0f;
    height += cosf(z * 0.07f) * 5.0f;
    height += sinf((x + z) * 0.04f) * 8.0f;

    if (height < -0.15f) height = -0.15f;
    return height;
}

void drawGround()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texGrass);
    
    // Draw a grid of quads to create dynamic terrain
    const float STEP = 5.0f;
    const float LIMIT = 180.0f;

    for (float x = -LIMIT; x < LIMIT; x += STEP) {
        glBegin(GL_QUAD_STRIP);
        for (float z = -LIMIT; z <= LIMIT; z += STEP) {
            float h1 = getTerrainHeight(x, z);
            float h2 = getTerrainHeight(x + STEP, z);

            // Simple UP normal for terrain shading
            glNormal3f(0.0f, 1.0f, 0.0f); 

            // Dark, rich green tint for the grassy mountains
            glColor3f(0.40f, 0.55f, 0.35f); 
            
            glTexCoord2f((x + LIMIT), (z + LIMIT));
            glVertex3f(x, h1, z);

            glTexCoord2f((x + STEP + LIMIT), (z + LIMIT));
            glVertex3f(x + STEP, h2, z);
        }
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);
}

// ============================================================
// CITY / SKYSCRAPERS (VISUAL UPGRADE)
// ============================================================

void drawBuilding(float x, float z, float width, float height, float depth) {
    float groundY = getTerrainHeight(x, z);
    // Draw the skyscraper rising from the terrain
    drawCube(x, groundY + height/2.0f, z, width, height, depth, 0.8f, 0.8f, 0.85f, texBuilding);
}

void drawCity() {
    // Cluster 1: Top Right Background
    drawBuilding( 85, -70, 18, 75, 18);
    drawBuilding(105, -85, 25, 120, 25);
    drawBuilding( 70, -95, 20, 90, 20);
    drawBuilding(115, -60, 22, 100, 22);
    
    // Cluster 2: Top Left Background
    drawBuilding(-85, -75, 22, 95, 22);
    drawBuilding(-105, -65, 18, 110, 18);
    drawBuilding(-75, -95, 25, 80, 25);

    // Cluster 3: Bottom Left Background
    drawBuilding(-80,  70, 20, 85, 20);
    drawBuilding(-100,  85, 30, 130, 30);
    drawBuilding(-65,  95, 18, 70, 18);

    // Cluster 4: Bottom Right Background
    drawBuilding( 85,  75, 25, 105, 25);
    drawBuilding( 70,  90, 18, 80, 18);
    drawBuilding(105,  65, 20, 90, 20);
}

// ============================================================
// TRACK NORMAL / TANGENT
// ============================================================

Vec2 pointTangent(size_t index)
{
    size_t count = trackPoints.size();
    size_t previous = (index == 0) ? 0 : index - 1;
    size_t next = (index >= count - 1) ? count - 1 : index + 1;
    return normalize2D({ trackPoints[next].x - trackPoints[previous].x, trackPoints[next].z - trackPoints[previous].z });
}

// ============================================================
// DRAW TRACK (MODERN METRO RAILWAY)
// ============================================================

void drawTrack()
{
    // Concrete Slab Track Bed
    glDisable(GL_LIGHTING);
    glColor3f(0.65f, 0.65f, 0.65f);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texConcrete);
    glBegin(GL_QUAD_STRIP);
    for (size_t i = 0; i < trackPoints.size(); i++) {
        Vec2 tangent = pointTangent(i);
        Vec2 normal = { -tangent.z, tangent.x };
        float width = 1.6f;
        float dist = cumulativeLength[i];
        glTexCoord2f(0.0f, dist * 0.5f);
        glVertex3f(trackPoints[i].x + normal.x * width, 0.01f, trackPoints[i].z + normal.z * width);
        glTexCoord2f(2.0f, dist * 0.5f);
        glVertex3f(trackPoints[i].x - normal.x * width, 0.01f, trackPoints[i].z - normal.z * width);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);

    // Concrete Ties
    for (float d = 0.0f; d < totalTrackLength; d += 1.2f) {
        Vec2 position = sampleTrackPosition(d);
        Vec2 tangent = sampleTrackTangent(d);
        float angle = atan2f(tangent.x, tangent.z) * 180.0f / PI;
        glPushMatrix();
        glTranslatef(position.x, 0.06f, position.z);
        glRotatef(angle, 0.0f, 1.0f, 0.0f);
        drawCube(0.0f, 0.0f, 0.0f, 2.0f, 0.12f, 0.35f, 0.7f, 0.7f, 0.7f, texConcrete);
        glPopMatrix();
    }

    // Modern Steel Rails & Electric Third Rail
    glDisable(GL_LIGHTING);
    glLineWidth(4.0f);
    glColor3f(0.7f, 0.75f, 0.8f);
    for (int side = -1; side <= 1; side += 2) {
        glBegin(GL_LINE_STRIP);
        for (size_t i = 0; i < trackPoints.size(); i++) {
            Vec2 tangent = pointTangent(i);
            Vec2 normal = { -tangent.z, tangent.x };
            float offset = 0.60f * side;
            glVertex3f(trackPoints[i].x + normal.x * offset, 0.16f, trackPoints[i].z + normal.z * offset);
        }
        glEnd();
    }
    
    // Safety Orange Electric Third Rail
    glLineWidth(6.0f);
    glColor3f(0.85f, 0.45f, 0.15f);
    glBegin(GL_LINE_STRIP);
    for (size_t i = 0; i < trackPoints.size(); i++) {
        Vec2 tangent = pointTangent(i);
        Vec2 normal = { -tangent.z, tangent.x };
        float offset = 1.15f; 
        glVertex3f(trackPoints[i].x + normal.x * offset, 0.22f, trackPoints[i].z + normal.z * offset);
    }
    glEnd();
    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

// ============================================================
// PROFESSIONAL METRO TRAIN MODEL
// ============================================================

void drawMetroWheel(float x, float y, float z)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(1.0f, 1.0f, 0.42f);
    glColor3f(0.2f, 0.2f, 0.25f);
    glutSolidSphere(0.37f, 18, 14);
    glPopMatrix();
}

void drawMetroBogie(float z)
{
    drawCube(0.0f, 0.48f, z, 1.70f, 0.26f, 1.15f, 0.15f, 0.16f, 0.18f, texConcrete);
    drawMetroWheel(-1.02f, 0.43f, z + 0.34f);
    drawMetroWheel( 1.02f, 0.43f, z + 0.34f);
    drawMetroWheel(-1.02f, 0.43f, z - 0.34f);
    drawMetroWheel( 1.02f, 0.43f, z - 0.34f);
}

void drawMetroUnderbody(float carLength)
{
    drawCube(0.0f, 0.68f, 0.0f, 2.12f, 0.28f, carLength, 0.2f, 0.22f, 0.25f, texConcrete);
    drawCube(-0.48f, 0.56f, 0.0f, 0.70f, 0.30f, 1.45f, 0.25f, 0.27f, 0.3f, texConcrete);
    drawCube(0.48f, 0.56f, 0.0f, 0.70f, 0.30f, 1.45f, 0.25f, 0.27f, 0.3f, texConcrete);
}

void drawMetroRoofEquipment(float carLength)
{
    drawCube(0.0f, 2.68f, 0.0f, 2.06f, 0.18f, carLength - 0.18f, 0.8f, 0.82f, 0.85f, texConcrete);
    drawCube(0.0f, 2.82f, -0.72f, 1.12f, 0.18f, 0.72f, 0.6f, 0.62f, 0.65f, texConcrete);
    drawCube(0.0f, 2.82f, 0.72f, 1.12f, 0.18f, 0.72f, 0.6f, 0.62f, 0.65f, texConcrete);
}

void drawMetroSideDoor(float x, float z)
{
    drawCube(x, 1.67f, z, 0.055f, 1.62f, 0.78f, 0.15f, 0.18f, 0.22f, texConcrete);
    drawCube(x + (x < 0.0f ? -0.012f : 0.012f), 1.55f, z, 0.035f, 1.24f, 0.67f, 0.85f, 0.88f, 0.9f, texConcrete);
    drawCube(x + (x < 0.0f ? -0.030f : 0.030f), 1.98f, z, 0.025f, 0.48f, 0.46f, 0.05f, 0.15f, 0.2f);
}

// Double sliding doors for the passenger car. When open, the panels slide
// toward the ends of the doorway, leaving a clear opening for the human.
void drawMetroSlidingDoor(float x, float centerZ)
{
    float side = (x < 0.0f) ? -1.0f : 1.0f;
    float panelOffset = metroDoorsOpen ? 0.39f : 0.18f;
    float panelWidth = 0.34f;
    float surfaceX = x + side * 0.035f;

    // Door frame and header.
    drawCube(surfaceX, 1.55f, centerZ - 0.42f, 0.075f, 1.48f, 0.075f, 0.18f, 0.22f, 0.27f);
    drawCube(surfaceX, 1.55f, centerZ + 0.42f, 0.075f, 1.48f, 0.075f, 0.18f, 0.22f, 0.27f);
    drawCube(surfaceX, 2.30f, centerZ, 0.075f, 0.08f, 0.90f, 0.18f, 0.22f, 0.27f);

    if (!metroDoorsOpen) {
        for (int panel = -1; panel <= 1; panel += 2) {
            float panelZ = centerZ + panel * panelOffset;
            drawCube(surfaceX + side * 0.018f, 1.53f, panelZ,
                     0.045f, 1.38f, panelWidth, 0.62f, 0.72f, 0.78f, texConcrete);
            drawCube(surfaceX + side * 0.045f, 1.60f, panelZ,
                     0.018f, 0.82f, panelWidth - 0.06f, 0.08f, 0.22f, 0.30f);
            drawCube(surfaceX + side * 0.055f, 1.08f, panelZ,
                     0.018f, 0.06f, panelWidth - 0.06f, 0.95f, 0.72f, 0.12f);
        }
    }
}

void drawMetroSideWindow(float x, float z, float width)
{
    drawCube(x, 2.02f, z, 0.052f, 0.64f, width, 0.1f, 0.1f, 0.12f);
    drawCube(x + (x < 0.0f ? -0.018f : 0.018f), 2.02f, z, 0.022f, 0.53f, width - 0.10f, 0.05f, 0.15f, 0.25f);
}

void drawMetroGangway(float z)
{
    drawCube(0.0f, 1.58f, z, 1.46f, 1.56f, 0.18f, 0.1f, 0.1f, 0.12f);
    drawCube(0.0f, 0.62f, z, 0.62f, 0.22f, 0.30f, 0.15f, 0.15f, 0.18f);
}

void drawMetroCabCarModel()
{
    const float carLength = 4.80f;
    drawMetroUnderbody(carLength);

    drawCube(0.0f, 1.28f, -0.10f, 2.06f, 1.05f, 4.35f, 0.95f, 0.95f, 0.95f, texConcrete);
    drawCube(0.0f, 2.07f, -0.18f, 2.00f, 0.70f, 4.08f, 0.9f, 0.9f, 0.92f, texConcrete);

    glDisable(GL_LIGHTING);
    drawCube(-1.035f, 1.50f, -0.12f, 0.045f, 0.22f, 4.28f, 0.0f, 0.8f, 1.0f);
    drawCube( 1.035f, 1.50f, -0.12f, 0.045f, 0.22f, 4.28f, 0.0f, 0.8f, 1.0f);
    glEnable(GL_LIGHTING);

    glPushMatrix();
    glTranslatef(0.0f, 1.66f, 2.10f);
    glScalef(1.04f, 1.18f, 0.70f);
    glColor3f(0.92f, 0.92f, 0.95f);
    glutSolidSphere(1.0f, 28, 20);
    glPopMatrix();

    drawCube(0.0f, 2.05f, 2.69f, 1.62f, 0.72f, 0.07f, 0.1f, 0.1f, 0.12f);
    drawCube(0.0f, 2.12f, 2.735f, 1.44f, 0.50f, 0.035f, 0.05f, 0.15f, 0.25f);
    drawCube(0.0f, 2.12f, 2.765f, 0.065f, 0.52f, 0.025f, 0.1f, 0.1f, 0.12f);
    
    glDisable(GL_LIGHTING);
    drawCube(0.0f, 2.48f, 2.75f, 0.75f, 0.17f, 0.035f, 1.0f, 0.6f, 0.0f);
    drawCube(0.0f, 1.37f, 2.72f, 1.48f, 0.19f, 0.045f, 0.0f, 0.8f, 1.0f);
    drawCube(-0.62f, 1.15f, 2.745f, 0.24f, 0.16f, 0.04f, 1.0f, 1.0f, 0.8f);
    drawCube( 0.62f, 1.15f, 2.745f, 0.24f, 0.16f, 0.04f, 1.0f, 1.0f, 0.8f);
    drawCube(-0.82f, 1.10f, 2.73f, 0.10f, 0.12f, 0.04f, 1.0f, 0.1f, 0.1f);
    drawCube( 0.82f, 1.10f, 2.73f, 0.10f, 0.12f, 0.04f, 1.0f, 0.1f, 0.1f);
    glEnable(GL_LIGHTING);

    drawMetroSideWindow(-1.025f,  0.80f, 0.68f);
    drawMetroSideWindow( 1.025f,  0.80f, 0.68f);
    drawMetroSideWindow(-1.025f, -0.10f, 0.68f);
    drawMetroSideWindow( 1.025f, -0.10f, 0.68f);
    drawMetroSideDoor(-1.030f, -1.15f);
    drawMetroSideDoor( 1.030f, -1.15f);
    drawMetroSideWindow(-1.025f, 1.63f, 0.48f);
    drawMetroSideWindow( 1.025f, 1.63f, 0.48f);
    drawMetroGangway(-2.33f);
    drawMetroBogie( 1.35f);
    drawMetroBogie(-1.35f);
    drawMetroRoofEquipment(carLength);
}

void drawMetroMiddleCarModel()
{
    const float carLength = 4.95f;
    drawMetroUnderbody(carLength);

    // Passenger floor and roof. The side walls are built in sections so the
    // sliding doorways are genuinely open when the doors retract.
    drawCube(0.0f, 0.84f, 0.0f, 1.92f, 0.16f, 4.68f, 0.42f, 0.45f, 0.48f, texConcrete);
    drawCube(0.0f, 2.43f, 0.0f, 1.94f, 0.16f, 4.70f, 0.82f, 0.84f, 0.86f, texConcrete);

    // Side wall sections leave openings around the two doors on each side.
    for (int side = -1; side <= 1; side += 2) {
        float x = side * 0.98f;
        drawCube(x, 1.52f, -2.08f, 0.12f, 1.32f, 0.52f, 0.88f, 0.89f, 0.91f, texConcrete);
        drawCube(x, 1.52f,  0.00f, 0.12f, 1.32f, 1.76f, 0.88f, 0.89f, 0.91f, texConcrete);
        drawCube(x, 1.52f,  2.08f, 0.12f, 1.32f, 0.52f, 0.88f, 0.89f, 0.91f, texConcrete);

        // Dark glazed panels on the wall sections.
        drawMetroSideWindow((float)side * 1.055f, -2.00f, 0.40f);
        drawMetroSideWindow((float)side * 1.055f, -0.56f, 0.42f);
        drawMetroSideWindow((float)side * 1.055f,  0.56f, 0.42f);
        drawMetroSideWindow((float)side * 1.055f,  2.00f, 0.40f);

        // Door centers at z = +/-1.28, on both sides of this car only.
        drawMetroSlidingDoor((float)side * 1.035f, -1.28f);
        drawMetroSlidingDoor((float)side * 1.035f,  1.28f);
    }

    // End walls and gangways retain the existing three-car train connection.
    drawCube(0.0f, 1.55f, -2.31f, 1.90f, 1.40f, 0.12f, 0.78f, 0.80f, 0.83f, texConcrete);
    drawCube(0.0f, 1.55f,  2.31f, 1.90f, 1.40f, 0.12f, 0.78f, 0.80f, 0.83f, texConcrete);
    drawMetroGangway( 2.39f);
    drawMetroGangway(-2.39f);

    // Simple seats and grab rails make the interior visibly usable.
    for (int side = -1; side <= 1; side += 2) {
        for (int row = -1; row <= 1; row += 2) {
            float z = row * 0.48f;
            drawCube(side * 0.64f, 1.02f, z, 0.38f, 0.18f, 0.62f, 0.08f, 0.22f, 0.42f);
            drawCube(side * 0.79f, 1.28f, z, 0.10f, 0.42f, 0.62f, 0.08f, 0.22f, 0.42f);
        }
        drawCube(side * 0.52f, 1.92f, 0.0f, 0.045f, 0.06f, 1.45f, 0.75f, 0.78f, 0.80f);
    }

    drawMetroBogie( 1.38f);
    drawMetroBogie(-1.38f);
    drawMetroRoofEquipment(carLength);
}

void drawTrainUnit(float distance, int carType, int facingDirection)
{
    Vec2 position = sampleTrackPosition(distance);
    Vec2 tangent = sampleTrackTangent(distance);
    float angle = atan2f(tangent.x, tangent.z) * 180.0f / PI;
    if (facingDirection < 0) angle += 180.0f;

    glPushMatrix();
    glTranslatef(position.x, 0.18f, position.z);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);
    
    if (carType == 0) drawMetroCabCarModel();
    else drawMetroMiddleCarModel();
    
    glPopMatrix();
}

float getLeadingCabDistance()
{
    return clampTrackDistance(trainDistance + (float)trainDirection * METRO_CAB_OFFSET);
}

void drawTrain()
{
    float leadingCabDistance = trainDistance + (float)trainDirection * METRO_CAB_OFFSET;
    float trailingCabDistance = trainDistance - (float)trainDirection * METRO_CAB_OFFSET;

    drawTrainUnit(leadingCabDistance, 0, trainDirection);
    drawTrainUnit(trainDistance, 1, trainDirection);
    drawTrainUnit(trailingCabDistance, 0, -trainDirection);
}

// ============================================================
// STATION & ENVIRONMENT (Visual Upgrades)
// ============================================================

void drawText3D(float x, float y, float z, const char* text)
{
    glDisable(GL_LIGHTING);
    glColor3f(0.9f, 0.9f, 1.0f); 
    glRasterPos3f(x, y, z);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
    glEnable(GL_LIGHTING);
}

void drawCircle2D(float radius, int segments)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * PI * (float)i / (float)segments;
        glVertex3f(cosf(angle) * radius, sinf(angle) * radius, 0.0f);
    }
    glEnd();
}

void drawCircleOutline(float radius, int segments)
{
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * PI * (float)i / (float)segments;
        glVertex3f(cosf(angle) * radius, sinf(angle) * radius, 0.0f);
    }
    glEnd();
}

void drawCounterNumber(int value)
{
    char text[32];
    sprintf(text, "%d", value);
    float textWidth = 0.0f;
    for (const char* p = text; *p; p++) textWidth += (float)glutStrokeWidth(GLUT_STROKE_ROMAN, *p);

    const float textHeight = 119.05f;
    float scaleByWidth = 2.15f / (textWidth > 1.0f ? textWidth : 1.0f);
    float scaleByHeight = 1.65f / textHeight;
    float textScale = scaleByWidth < scaleByHeight ? scaleByWidth : scaleByHeight;

    glPushMatrix();
    glTranslatef(-textWidth * textScale * 0.5f, -textHeight * textScale * 0.42f, 0.10f);
    glScalef(textScale, textScale, textScale);

    glColor3f(1.0f, 1.0f, 1.0f); 
    glLineWidth(4.0f);
    for (const char* p = text; *p; p++) glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
    glLineWidth(1.0f);
    glPopMatrix();
}

void drawStationCounter(const Station& station, int stationIndex)
{
    float postX = station.x + station.sizeX * 0.5f + 2.7f;
    float postZ = station.z;
    const float postHeight = 5.4f;
    const float circleY = 6.1f;

    glDisable(GL_LIGHTING);
    drawCube(postX, postHeight * 0.5f, postZ, 0.20f, postHeight, 0.20f, 0.2f, 0.2f, 0.25f);

    float dx = activeCameraX - postX;
    float dy = activeCameraY - circleY;
    float dz = activeCameraZ - postZ;
    float horizontalDistance = sqrtf(dx * dx + dz * dz);

    float yaw = atan2f(dx, dz) * 180.0f / PI;
    float pitch = -atan2f(dy, horizontalDistance) * 180.0f / PI;

    glPushMatrix();
    glTranslatef(postX, circleY, postZ);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
    glRotatef(pitch, 1.0f, 0.0f, 0.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f, 0.5f, 1.0f, 0.25f);
    drawCircle2D(1.75f, 48);
    glTranslatef(0.0f, 0.0f, 0.02f);
    glColor4f(0.0f, 0.4f, 0.8f, 0.95f);
    drawCircle2D(1.42f, 48);
    glTranslatef(0.0f, 0.0f, 0.02f);
    glColor4f(0.2f, 0.8f, 1.0f, 1.0f);
    glLineWidth(5.0f);
    drawCircleOutline(1.48f, 64);
    glLineWidth(1.0f);
    drawCounterNumber(stationPassCount[stationIndex]);

    glDisable(GL_BLEND);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawStation(const Station& station)
{
    drawCube(station.x, 0.16f, station.z, station.sizeX, 0.35f, station.sizeZ, 0.7f, 0.72f, 0.75f, texConcrete);
    drawCube(station.x, 0.35f, station.z + station.sizeZ*0.5f - 0.25f, station.sizeX, 0.02f, 0.3f, 0.9f, 0.8f, 0.1f);
    drawCube(station.x, 1.15f, station.z, 2.8f, 1.8f, 2.0f, 0.8f, 0.9f, 1.0f, texGlassPanel);
    drawCube(station.x + 0.5f, 2.18f, station.z, 4.4f, 0.15f, 2.8f, 0.2f, 0.22f, 0.25f, texConcrete);
    
    glDisable(GL_LIGHTING);
    drawCube(station.x - 2.2f, 3.1f, station.z, 2.5f, 0.5f, 0.1f, 0.0f, 0.6f, 0.9f);
    glEnable(GL_LIGHTING);

    drawText3D(station.x - 3.2f, 3.0f, station.z + 0.06f, station.name);
}

void drawStations()
{
    for (int i = 0; i < 4; i++) {
        drawStation(stations[i]);
        drawStationCounter(stations[i], i);
    }
}

void drawTree(float x, float z)
{
    float groundY = getTerrainHeight(x, z);
    
    drawCube(x, groundY + 0.9f, z, 0.5f, 1.8f, 0.5f, 0.3f, 0.2f, 0.15f);

    glPushMatrix();
    glTranslatef(x, groundY + 1.6f, z);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    
    glColor3f(0.15f, 0.35f, 0.20f);
    glutSolidCone(1.6f, 2.2f, 16, 8);
    glTranslatef(0.0f, 0.0f, 1.2f);
    glColor3f(0.18f, 0.40f, 0.23f);
    glutSolidCone(1.3f, 1.8f, 16, 8);
    glTranslatef(0.0f, 0.0f, 1.0f);
    glColor3f(0.20f, 0.45f, 0.26f);
    glutSolidCone(0.9f, 1.4f, 16, 8);
    
    glPopMatrix();
}

void drawEnvironment()
{
    drawTree(-72.0f, 42.0f); drawTree(70.0f, 42.0f);
    drawTree(-70.0f, 12.0f); drawTree(72.0f, 8.0f);
    drawTree(-70.0f, -22.0f); drawTree(72.0f, -26.0f);
    drawTree(-66.0f, -48.0f); drawTree(70.0f, -47.0f);
}

// ============================================================
// THIRD-PERSON HUMAN PLAYER MODEL
// ============================================================

void drawHuman()
{
    glPushMatrix();
    glTranslatef(humanX, humanY, humanZ);
    glRotatef(humanYaw, 0.0f, 1.0f, 0.0f);

    // Legs
    drawCube(-0.16f, 0.48f, 0.0f, 0.18f, 0.72f, 0.22f, 0.12f, 0.18f, 0.32f);
    drawCube( 0.16f, 0.48f, 0.0f, 0.18f, 0.72f, 0.22f, 0.12f, 0.18f, 0.32f);
    // Shoes
    drawCube(-0.16f, 0.10f, 0.06f, 0.22f, 0.14f, 0.34f, 0.08f, 0.08f, 0.09f);
    drawCube( 0.16f, 0.10f, 0.06f, 0.22f, 0.14f, 0.34f, 0.08f, 0.08f, 0.09f);
    // Torso and arms
    drawCube(0.0f, 1.12f, 0.0f, 0.58f, 0.72f, 0.30f, 0.10f, 0.35f, 0.82f);
    drawCube(-0.39f, 1.12f, 0.0f, 0.16f, 0.62f, 0.18f, 0.82f, 0.58f, 0.40f);
    drawCube( 0.39f, 1.12f, 0.0f, 0.16f, 0.62f, 0.18f, 0.82f, 0.58f, 0.40f);
    // Head
    glPushMatrix();
    glTranslatef(0.0f, 1.72f, 0.0f);
    glColor3f(0.82f, 0.64f, 0.48f);
    glutSolidSphere(0.22f, 16, 12);
    glPopMatrix();

    glPopMatrix();
}

// ============================================================
// CAMERA
// ============================================================

void setCamera()
{
    Vec2 position = sampleTrackPosition(trainDistance);
    Vec2 tangent = sampleTrackTangent(trainDistance);

    tangent.x *= (float)trainDirection;
    tangent.z *= (float)trainDirection;

    float panX = camPanX[cameraMode];
    float panZ = camPanZ[cameraMode];
    float lift = camLift[cameraMode];

    if (cameraMode == 0) {
        activeCameraX = position.x - tangent.x * 11.0f + panX;
        activeCameraY = 6.0f + lift;
        activeCameraZ = position.z - tangent.z * 11.0f + panZ;
        gluLookAt(activeCameraX, activeCameraY, activeCameraZ, position.x + tangent.x * 3.0f + panX, 1.8f + lift, position.z + tangent.z * 3.0f + panZ, 0.0f, 1.0f, 0.0f);
    } else if (cameraMode == 1) {
        activeCameraX = position.x + tangent.x * 0.8f + panX;
        activeCameraY = 2.55f + lift;
        activeCameraZ = position.z + tangent.z * 0.8f + panZ;
        gluLookAt(activeCameraX, activeCameraY, activeCameraZ, position.x + tangent.x * 20.0f + panX, 2.2f + lift, position.z + tangent.z * 20.0f + panZ, 0.0f, 1.0f, 0.0f);
    } else if (cameraMode == 2) {
        float topHeight = 96.0f + lift;
        if (topHeight < 25.0f) topHeight = 25.0f;
        activeCameraX = panX; activeCameraY = topHeight; activeCameraZ = panZ;
        gluLookAt(activeCameraX, activeCameraY, activeCameraZ, panX, 0.0f, panZ, 0.0f, 0.0f, -1.0f);
    } else if (cameraMode == 3) {
        float yawRadians = droneYaw * PI / 180.0f;
        float pitchRadians = dronePitch * PI / 180.0f;
        float directionX = cosf(pitchRadians) * sinf(yawRadians);
        float directionY = sinf(pitchRadians);
        float directionZ = cosf(pitchRadians) * cosf(yawRadians);
        activeCameraX = droneCamX; activeCameraY = droneCamY; activeCameraZ = droneCamZ;
        gluLookAt(activeCameraX, activeCameraY, activeCameraZ, droneCamX + directionX, droneCamY + directionY, droneCamZ + directionZ, 0.0f, 1.0f, 0.0f);
    } else if (cameraMode == 4) {
        // Camera follows behind the human; arrow keys rotate this third-person view.
        float yawRadians = humanCameraYaw * PI / 180.0f;
        float pitchRadians = humanCameraPitch * PI / 180.0f;
        // Keep the follow camera inside the passenger cabin while onboard.
        float followDistance = humanOnboard ? 1.2f : humanCameraDistance;
        float followHeight = humanOnboard ? 1.45f : humanCameraHeight;
        float horizontalDistance = followDistance * cosf(pitchRadians);
        activeCameraX = humanX - sinf(yawRadians) * horizontalDistance;
        activeCameraY = humanY + followHeight + followDistance * sinf(pitchRadians);
        activeCameraZ = humanZ - cosf(yawRadians) * horizontalDistance;
        gluLookAt(activeCameraX, activeCameraY, activeCameraZ,
                  humanX, humanY + 1.0f, humanZ, 0.0f, 1.0f, 0.0f);
    } else if (cameraMode == 5) {
        // First-person view: camera is positioned at the human's eye level.
        float yawRadians = humanCameraYaw * PI / 180.0f;
        float pitchRadians = humanCameraPitch * PI / 180.0f;
        float directionX = cosf(pitchRadians) * sinf(yawRadians);
        float directionY = sinf(pitchRadians);
        float directionZ = cosf(pitchRadians) * cosf(yawRadians);
        activeCameraX = humanX;
        activeCameraY = humanY + 1.62f;
        activeCameraZ = humanZ;
        gluLookAt(activeCameraX, activeCameraY, activeCameraZ,
                  activeCameraX + directionX,
                  activeCameraY + directionY,
                  activeCameraZ + directionZ,
                  0.0f, 1.0f, 0.0f);
    }
}

// ============================================================
// HUMAN / METRO BOARDING HELPERS
// ============================================================

float getTrainWorldYaw()
{
    Vec2 tangent = sampleTrackTangent(trainDistance);
    float yaw = atan2f(tangent.x, tangent.z) * 180.0f / PI;
    if (trainDirection < 0) yaw += 180.0f;
    return yaw;
}

void syncHumanToTrain()
{
    Vec2 trainPosition = sampleTrackPosition(trainDistance);
    float angle = getTrainWorldYaw() * PI / 180.0f;
    humanX = trainPosition.x + cosf(angle) * humanLocalX + sinf(angle) * humanLocalZ;
    humanZ = trainPosition.z - sinf(angle) * humanLocalX + cosf(angle) * humanLocalZ;
    humanY = 0.0f;
}

void updateOnboardHuman()
{
    if (!humanOnboard) return;

    float currentYaw = getTrainWorldYaw();
    float deltaYaw = currentYaw - lastOnboardTrainYaw;
    while (deltaYaw > 180.0f) deltaYaw -= 360.0f;
    while (deltaYaw < -180.0f) deltaYaw += 360.0f;
    humanCameraYaw += deltaYaw;
    if (humanCameraYaw >= 360.0f) humanCameraYaw -= 360.0f;
    if (humanCameraYaw < 0.0f) humanCameraYaw += 360.0f;
    humanYaw = humanCameraYaw;
    lastOnboardTrainYaw = currentYaw;
    syncHumanToTrain();
}

void moveHumanWorld(float dx, float dz)
{
    if (humanOnboard) {
        float angle = getTrainWorldYaw() * PI / 180.0f;
        float localDX = cosf(angle) * dx - sinf(angle) * dz;
        float localDZ = sinf(angle) * dx + cosf(angle) * dz;
        humanLocalX = clampValue(humanLocalX + localDX, -0.48f, 0.48f);
        humanLocalZ = clampValue(humanLocalZ + localDZ, -1.72f, 1.72f);
        syncHumanToTrain();
    } else {
        humanX += dx;
        humanZ += dz;
        humanYaw = humanCameraYaw;
    }
}

void interactWithMetro()
{
    if (trainSpeed > 0.05f || !metroDoorsOpen) return;

    if (humanOnboard) {
        // Step out through the right-hand doorway while the train is stopped.
        humanLocalX = 1.62f;
        humanLocalZ = 0.0f;
        syncHumanToTrain();
        humanOnboard = false;
        return;
    }

    Vec2 trainPosition = sampleTrackPosition(trainDistance);
    float dx = humanX - trainPosition.x;
    float dz = humanZ - trainPosition.z;
    if (sqrtf(dx * dx + dz * dz) <= 3.2f) {
        humanOnboard = true;
        humanLocalX = 0.0f;
        humanLocalZ = 0.0f;
        humanCameraYaw = getTrainWorldYaw();
        lastOnboardTrainYaw = humanCameraYaw;
        humanYaw = humanCameraYaw;
        syncHumanToTrain();
    }
}

// ============================================================
// 2D METRO CARD UI & MODERN HUD HELPERS (VISUAL UPGRADE)
// ============================================================

void drawTextShadowed(float x, float y, const char* text, void* font, float r, float g, float b)
{
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glRasterPos2f(x + 1.5f, y - 1.5f);
    for (const char* p = text; *p; p++) glutBitmapCharacter(font, *p);
    
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (const char* p = text; *p; p++) glutBitmapCharacter(font, *p);
}

float getBitmapTextWidth(const char* text, void* font)
{
    float width = 0.0f;
    while (*text) { width += (float)glutBitmapWidth(font, *text); text++; }
    return width;
}

// Draw a beautiful vertical gradient rectangle
void drawGradientRect2D(const Rect2D& rect, float r1, float g1, float b1, float a1, float r2, float g2, float b2, float a2)
{
    glBegin(GL_QUADS);
    glColor4f(r1, g1, b1, a1);
    glVertex2f(rect.x, rect.y + rect.height);
    glVertex2f(rect.x + rect.width, rect.y + rect.height);
    glColor4f(r2, g2, b2, a2);
    glVertex2f(rect.x + rect.width, rect.y);
    glVertex2f(rect.x, rect.y);
    glEnd();
}

void drawRectOutline2D(const Rect2D& rect, float r, float g, float b, float lineWidth)
{
    glColor3f(r, g, b);
    glLineWidth(lineWidth);
    glBegin(GL_LINE_LOOP);
    glVertex2f(rect.x, rect.y);
    glVertex2f(rect.x + rect.width, rect.y);
    glVertex2f(rect.x + rect.width, rect.y + rect.height);
    glVertex2f(rect.x, rect.y + rect.height);
    glEnd();
    glLineWidth(1.0f);
}

void drawFilledCircle2D(float centerX, float centerY, float radius, int segments, float r, float g, float b)
{
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(centerX, centerY);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * PI * (float)i / (float)segments;
        glVertex2f(centerX + cosf(angle) * radius, centerY + sinf(angle) * radius);
    }
    glEnd();
}

void drawCardOrFoilBody(const Rect2D& card)
{
    if (selectedFoilOption < 0) {
        drawGradientRect2D(card, 0.18f, 0.22f, 0.28f, 0.95f, 0.1f, 0.12f, 0.15f, 0.95f);
        drawRectOutline2D(card, 0.0f, 0.7f, 1.0f, 3.0f);
        return;
    }

    Rect2D cardTab;
    cardTab.x = card.x + 43.0f;
    cardTab.y = card.y + card.height - 2.0f;
    cardTab.width = card.width - 86.0f;
    cardTab.height = 28.0f;

    drawGradientRect2D(cardTab, 0.18f, 0.22f, 0.28f, 0.95f, 0.1f, 0.12f, 0.15f, 0.95f);
    drawRectOutline2D(cardTab, 0.0f, 0.7f, 1.0f, 2.0f);

    if (selectedFoilOption == 0) drawGradientRect2D(card, 0.75f, 0.77f, 0.8f, 1.0f, 0.6f, 0.62f, 0.65f, 1.0f);
    else if (selectedFoilOption == 1) drawGradientRect2D(card, 0.55f, 0.57f, 0.60f, 1.0f, 0.45f, 0.47f, 0.5f, 1.0f);
    else drawGradientRect2D(card, 0.45f, 0.47f, 0.5f, 1.0f, 0.35f, 0.37f, 0.4f, 1.0f);

    drawRectOutline2D(card, 0.9f, 0.95f, 1.0f, 2.0f);

    if (selectedFoilOption == 1) {
        drawFilledCircle2D(card.x + 24.0f, card.y + 30.0f, 9.0f, 28, 0.15f, 0.18f, 0.22f);
        drawFilledCircle2D(card.x + card.width - 26.0f, card.y + 42.0f, 12.0f, 28, 0.15f, 0.18f, 0.22f);
        drawFilledCircle2D(card.x + 31.0f, card.y + 122.0f, 8.0f, 28, 0.15f, 0.18f, 0.22f);
        drawFilledCircle2D(card.x + card.width - 28.0f, card.y + 142.0f, 10.0f, 28, 0.15f, 0.18f, 0.22f);
        drawFilledCircle2D(card.x + 42.0f, card.y + card.height - 39.0f, 11.0f, 28, 0.15f, 0.18f, 0.22f);
        drawFilledCircle2D(card.x + card.width - 42.0f, card.y + card.height - 27.0f, 7.0f, 28, 0.15f, 0.18f, 0.22f);
    }
}

bool pointInsideRect(float x, float y, const Rect2D& rect) {
    return x >= rect.x && x <= rect.x + rect.width && y >= rect.y && y <= rect.y + rect.height;
}

Rect2D getBuyCardButtonRect(int windowWidth, int windowHeight) {
    Rect2D rect; rect.width = 112.0f; rect.height = 34.0f;
    rect.x = (float)windowWidth - rect.width - 28.0f; rect.y = (float)windowHeight - 55.0f;
    return rect;
}

Rect2D getSellCardButtonRect(int windowWidth, int windowHeight) {
    Rect2D rect = getBuyCardButtonRect(windowWidth, windowHeight); rect.y -= 48.0f;
    return rect;
}

Rect2D getRouteStationButtonRect(int row, int stationIndex, int windowWidth, int windowHeight) {
    Rect2D buyButton = getBuyCardButtonRect(windowWidth, windowHeight);
    Rect2D sellButton = getSellCardButtonRect(windowWidth, windowHeight);
    Rect2D rect; rect.width = 42.0f; rect.height = 30.0f;
    rect.x = buyButton.x - 190.0f + stationIndex * 44.0f;
    rect.y = (row == 0 ? buyButton.y : sellButton.y) + 2.0f;
    return rect;
}

bool hasValidRouteSelection() {
    return selectedFromStation >= 0 && selectedFromStation < 4 && selectedToStation >= 0 && selectedToStation < 4 && selectedFromStation != selectedToStation;
}

Rect2D getMetroCardRect(int windowWidth, int windowHeight) {
    Rect2D rect; rect.width = 235.0f; rect.height = 285.0f;
    rect.x = (float)windowWidth - rect.width - 30.0f; rect.y = 55.0f;
    return rect;
}

Rect2D getFoilButtonRect(int index, int windowWidth, int windowHeight) {
    Rect2D card = getMetroCardRect(windowWidth, windowHeight);
    Rect2D rect; rect.width = 205.0f; rect.height = 34.0f;
    rect.x = card.x - rect.width - 28.0f; rect.y = card.y + 118.0f - index * 45.0f;
    return rect;
}

bool isMetroCardActivityNormal() {
    if (!metroCardOwned || cardFromStation < 0 || cardFromStation > 3 || cardToStation < 0 || cardToStation > 3 || cardFromStation == cardToStation) return false;
    for (int i = 0; i < 4; i++) if (stationPassCount[i] != cardStationCount[i]) return false;
    int firstStation = cardFromStation < cardToStation ? cardFromStation : cardToStation;
    int lastStation = cardFromStation > cardToStation ? cardFromStation : cardToStation;
    for (int i = 0; i < 4; i++) {
        bool stationIsOnPaidRoute = i >= firstStation && i <= lastStation;
        if (!stationIsOnPaidRoute) { if (cardStationCount[i] != 0) return false; }
        else { if (cardStationCount[i] > 1) return false; }
    }
    if (cardJourneyReachedTarget) {
        for (int i = firstStation; i <= lastStation; i++) { if (cardStationCount[i] != 1) return false; }
    }
    return true;
}

void drawCenteredButtonText(const Rect2D& rect, const char* text, float r, float g, float b)
{
    void* font = GLUT_BITMAP_HELVETICA_12;
    float textWidth = getBitmapTextWidth(text, font);
    drawTextShadowed(rect.x + (rect.width - textWidth) * 0.5f, rect.y + rect.height * 0.5f - 4.0f, text, font, r, g, b);
}

void drawMetroCardInterface(int windowWidth, int windowHeight)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const char* stationButtonLabels[4] = { "1st", "2nd", "3rd", "4th" };

    Rect2D fromFirst = getRouteStationButtonRect(0, 0, windowWidth, windowHeight);
    Rect2D toFirst = getRouteStationButtonRect(1, 0, windowWidth, windowHeight);

    drawTextShadowed(fromFirst.x - 52.0f, fromFirst.y + 9.0f, "From :", GLUT_BITMAP_HELVETICA_12, 0.9f, 0.9f, 0.95f);
    drawTextShadowed(toFirst.x - 35.0f, toFirst.y + 9.0f, "To :", GLUT_BITMAP_HELVETICA_12, 0.9f, 0.9f, 0.95f);

    for (int row = 0; row < 2; row++) {
        for (int i = 0; i < 4; i++) {
            Rect2D routeButton = getRouteStationButtonRect(row, i, windowWidth, windowHeight);
            int selectedStation = (row == 0) ? selectedFromStation : selectedToStation;

            if (selectedStation == i) {
                drawGradientRect2D(routeButton, 0.2f, 0.6f, 0.9f, 0.9f, 0.05f, 0.3f, 0.6f, 0.9f);
                drawRectOutline2D(routeButton, 0.5f, 0.8f, 1.0f, 2.0f);
                drawCenteredButtonText(routeButton, stationButtonLabels[i], 1.0f, 1.0f, 1.0f);
            } else {
                drawGradientRect2D(routeButton, 0.2f, 0.22f, 0.25f, 0.85f, 0.1f, 0.12f, 0.15f, 0.85f);
                drawRectOutline2D(routeButton, 0.35f, 0.4f, 0.45f, 1.0f);
                drawCenteredButtonText(routeButton, stationButtonLabels[i], 0.7f, 0.75f, 0.8f);
            }
        }
    }

    Rect2D buyButton = getBuyCardButtonRect(windowWidth, windowHeight);
    if (!metroCardOwned && hasValidRouteSelection()) {
        drawGradientRect2D(buyButton, 0.2f, 0.8f, 0.4f, 0.9f, 0.05f, 0.5f, 0.2f, 0.9f);
        drawRectOutline2D(buyButton, 0.4f, 1.0f, 0.6f, 2.0f);
        drawCenteredButtonText(buyButton, "Buy Card", 1.0f, 1.0f, 1.0f);
    } else {
        drawGradientRect2D(buyButton, 0.2f, 0.22f, 0.25f, 0.85f, 0.1f, 0.12f, 0.15f, 0.85f);
        drawRectOutline2D(buyButton, 0.35f, 0.4f, 0.45f, 1.0f);
        drawCenteredButtonText(buyButton, "Buy Card", 0.5f, 0.5f, 0.55f);
    }

    Rect2D sellButton = getSellCardButtonRect(windowWidth, windowHeight);
    if (metroCardOwned) {
        drawGradientRect2D(sellButton, 0.9f, 0.3f, 0.3f, 0.9f, 0.6f, 0.1f, 0.1f, 0.9f);
        drawRectOutline2D(sellButton, 1.0f, 0.5f, 0.5f, 2.0f);
        drawCenteredButtonText(sellButton, "Sell Card", 1.0f, 1.0f, 1.0f);
    } else {
        drawGradientRect2D(sellButton, 0.2f, 0.22f, 0.25f, 0.85f, 0.1f, 0.12f, 0.15f, 0.85f);
        drawRectOutline2D(sellButton, 0.35f, 0.4f, 0.45f, 1.0f);
        drawCenteredButtonText(sellButton, "Sell Card", 0.5f, 0.5f, 0.55f);
    }

    if (!metroCardOwned || !metroCardVisible) {
        glDisable(GL_BLEND);
        return;
    }

    Rect2D card = getMetroCardRect(windowWidth, windowHeight);
    drawCardOrFoilBody(card);

    drawTextShadowed(card.x + 20.0f, card.y + card.height - 30.0f, "METRO PASS", GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f);
    drawTextShadowed(card.x + 20.0f, card.y + card.height - 68.0f, "Visited Stations:", GLUT_BITMAP_HELVETICA_12, 0.7f, 0.8f, 0.9f);

    char line[128];
    for (int i = 0; i < 4; i++) {
        sprintf(line, "Station %d : %d", i + 1, cardStationCount[i]);
        drawTextShadowed(card.x + 20.0f, card.y + card.height - 91.0f - i * 20.0f, line, GLUT_BITMAP_HELVETICA_12, 0.9f, 0.95f, 1.0f);
    }

    char routeLine[128];
    sprintf(routeLine, "From: %d    To: %d", cardFromStation + 1, cardToStation + 1);
    drawTextShadowed(card.x + 20.0f, card.y + 86.0f, routeLine, GLUT_BITMAP_HELVETICA_12, 0.0f, 0.8f, 1.0f);

    drawTextShadowed(card.x + 20.0f, card.y + 50.0f, "Card Activity :", GLUT_BITMAP_HELVETICA_12, 0.7f, 0.8f, 0.9f);

    if (isMetroCardActivityNormal()) {
        drawTextShadowed(card.x + 118.0f, card.y + 50.0f, "Normal", GLUT_BITMAP_HELVETICA_12, 0.2f, 1.0f, 0.4f);
    } else {
        drawTextShadowed(card.x + 118.0f, card.y + 50.0f, "Unusual", GLUT_BITMAP_HELVETICA_12, 1.0f, 0.3f, 0.3f);
    }

    const char* foilLabels[3] = { "Foil Sleeve (Cheap)", "Foil Sleeve (Torn)", "Foil Sleeve (Expensive)" };
    Rect2D firstFoilButton = getFoilButtonRect(0, windowWidth, windowHeight);
    drawTextShadowed(firstFoilButton.x + 5.0f, firstFoilButton.y + firstFoilButton.height + 20.0f, "Put / remove card in:", GLUT_BITMAP_HELVETICA_12, 0.9f, 0.9f, 0.95f);

    for (int i = 0; i < 3; i++) {
        Rect2D foilButton = getFoilButtonRect(i, windowWidth, windowHeight);
        if (selectedFoilOption == i) {
            drawGradientRect2D(foilButton, 0.4f, 0.45f, 0.5f, 0.9f, 0.2f, 0.25f, 0.3f, 0.9f);
            drawRectOutline2D(foilButton, 0.6f, 0.9f, 1.0f, 2.0f);
            drawCenteredButtonText(foilButton, foilLabels[i], 1.0f, 1.0f, 1.0f);
        } else {
            drawGradientRect2D(foilButton, 0.2f, 0.22f, 0.25f, 0.85f, 0.1f, 0.12f, 0.15f, 0.85f);
            drawRectOutline2D(foilButton, 0.35f, 0.4f, 0.45f, 1.0f);
            drawCenteredButtonText(foilButton, foilLabels[i], 0.7f, 0.75f, 0.8f);
        }
    }
    glDisable(GL_BLEND);
}

// ============================================================
// HUD
// ============================================================

void drawHUD()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Rect2D hudBg;
    hudBg.x = 10.0f; hudBg.y = 10.0f; hudBg.width = 760.0f; hudBg.height = 160.0f;
    drawGradientRect2D(hudBg, 0.15f, 0.18f, 0.22f, 0.85f, 0.05f, 0.08f, 0.12f, 0.85f);
    drawRectOutline2D(hudBg, 0.3f, 0.4f, 0.5f, 1.5f);
    
    glDisable(GL_BLEND);

    char buffer[256];
    void* fontLg = GLUT_BITMAP_HELVETICA_18;
    void* fontSm = GLUT_BITMAP_HELVETICA_12;

    sprintf(buffer, "Speed: %.1f km/h", trainSpeed * 3.6f);
    drawTextShadowed(25, 145, buffer, fontLg, 1.0f, 1.0f, 1.0f);

    sprintf(buffer, "Throttle: %.0f%%", throttle * 100.0f);
    drawTextShadowed(25, 120, buffer, fontLg, 0.3f, 1.0f, 0.5f);

    sprintf(buffer, "Brake: %.0f%%", brake * 100.0f);
    drawTextShadowed(25, 95, buffer, fontLg, 1.0f, 0.4f, 0.4f);

    const char* cameraNames[] = { "Chase", "Driver", "Top", "Drone", "Person (3rd Person)", "Person (1st Person)" };
    sprintf(buffer, "Camera: %s", cameraNames[cameraMode]);
    drawTextShadowed(25, 70, buffer, fontLg, 0.2f, 0.8f, 1.0f);

        drawTextShadowed(25, 45, metroDoorsOpen ? "Doors OPEN | E: Board/Exit when stopped near the car" : "Doors CLOSED | E: Board/Exit when stopped near the car", fontSm, metroDoorsOpen ? 0.2f : 1.0f, metroDoorsOpen ? 1.0f : 0.4f, metroDoorsOpen ? 0.4f : 0.3f);
    drawTextShadowed(25, 25, "W/S: Train speed | B/N: Brake | C: Camera | Person: Arrows + U/J/H/K | R: Reset | ESC: Exit", fontSm, 0.7f, 0.8f, 0.9f);

    drawMetroCardInterface(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// ============================================================
// MANUAL CONTROLS
// ============================================================

void updateControls(float deltaTime)
{
    if (keyW) {
        brake = 0.0f; manualBrakeApplied = false;
        throttle += THROTTLE_RAMP_RATE * deltaTime;
        throttle = clampValue(throttle, 0.0f, 1.0f);
    }
    if (keyS) {
        throttle -= 0.70f * deltaTime;
        throttle = clampValue(throttle, 0.0f, 1.0f);
    }
}

// ============================================================
// TRAIN PHYSICS
// ============================================================

void updatePhysics()
{
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    if (deltaTime > 0.1f) deltaTime = 0.1f;

    if (waitingAtTerminal) { trainSpeed = 0.0f; throttle = 0.0f; brake = 1.0f; metroDoorsOpen = true; updateOnboardHuman(); return; }

    updateControls(deltaTime);
    if (!manualBrakeApplied) brake = 0.0f;

    float rollingResistance = 0.0f;
    if (trainSpeed > 0.01f) rollingResistance = 0.18f;
    rollingResistance += 0.035f * trainSpeed;
    rollingResistance += 0.0035f * trainSpeed * trainSpeed;

    float terminalDistance = (trainDirection > 0) ? totalTrackLength : 0.0f;
    float previousLeadingCabDistance = getLeadingCabDistance();
    float distanceToTerminal;

    if (trainDirection > 0) distanceToTerminal = terminalDistance - previousLeadingCabDistance;
    else distanceToTerminal = previousLeadingCabDistance - terminalDistance;

    if (distanceToTerminal >= 0.0f && distanceToTerminal < TERMINAL_BRAKE_LOOKAHEAD) {
        if (!manualBrakeApplied) {
            throttle = 0.0f;
            if (trainSpeed > 0.05f) {
                float safeDistance = distanceToTerminal - 0.35f;
                if (safeDistance < 0.25f) safeDistance = 0.25f;
                float requiredTotalDeceleration = (trainSpeed * trainSpeed) / (2.0f * safeDistance);
                requiredTotalDeceleration *= 1.08f;
                float requiredBrakeDeceleration = requiredTotalDeceleration - rollingResistance;
                if (requiredBrakeDeceleration < 0.0f) requiredBrakeDeceleration = 0.0f;
                float automaticBrake = requiredBrakeDeceleration / MAX_BRAKE_DECELERATION;
                brake = clampValue(automaticBrake, 0.0f, 1.0f);
            }
            if (trainSpeed < 0.85f && distanceToTerminal > 0.55f) { brake = 0.0f; throttle = 0.20f; }
        }
    }

    float engineAcceleration = throttle * ENGINE_ACCELERATION;
    float brakeAcceleration = brake * MAX_BRAKE_DECELERATION;
    float gentleSlowdown = 0.0f;

    if (keyS && throttle <= 0.01f) gentleSlowdown = 1.1f;

    float acceleration = engineAcceleration - rollingResistance - brakeAcceleration - gentleSlowdown;
    trainSpeed += acceleration * deltaTime;
    trainSpeed = clampValue(trainSpeed, 0.0f, MAX_SPEED);
    if (trainSpeed < 0.02f && throttle <= 0.01f) trainSpeed = 0.0f;

    float previousTrainCenter = trainDistance;
    previousLeadingCabDistance = clampTrackDistance(previousTrainCenter + (float)trainDirection * METRO_CAB_OFFSET);

    trainDistance += (float)trainDirection * trainSpeed * deltaTime;
    trainDistance = clampTrackDistance(trainDistance);

    float currentLeadingCabDistance = getLeadingCabDistance();
    if (trainSpeed > 0.0f) updateStationPassCounters(previousLeadingCabDistance, currentLeadingCabDistance);

    bool reachedTerminal = false;
    float currentDistanceToTerminal;

    if (trainDirection > 0) {
        currentDistanceToTerminal = terminalDistance - currentLeadingCabDistance;
        reachedTerminal = currentLeadingCabDistance >= terminalDistance;
    } else {
        currentDistanceToTerminal = currentLeadingCabDistance - terminalDistance;
        reachedTerminal = currentLeadingCabDistance <= terminalDistance;
    }

    if (!reachedTerminal && currentDistanceToTerminal >= 0.0f && currentDistanceToTerminal <= 0.60f && trainSpeed < 1.0f) reachedTerminal = true;

    if (reachedTerminal) {
        trainDistance = terminalDistance - (float)trainDirection * METRO_CAB_OFFSET;
        trainDistance = clampTrackDistance(trainDistance);
        trainSpeed = 0.0f; throttle = 0.0f; brake = 1.0f; manualBrakeApplied = false;
        trainDirection = -trainDirection; waitingAtTerminal = true;
        metroDoorsOpen = true;
        terminalThrottleArmed = !keyW; keyW = false;
    }

    // Doors close as soon as the train begins moving and reopen when stopped.
    if (trainSpeed > 0.05f) metroDoorsOpen = false;
    else if (!waitingAtTerminal) metroDoorsOpen = true;
    updateOnboardHuman();
}

// ============================================================
// LIGHTING & MATERIALS
// ============================================================

void setupLighting()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Warm, realistic twilight/sunset light direction
    GLfloat lightPosition[] = { -50.0f, 60.0f, -80.0f, 1.0f };
    GLfloat diffuse[] = { 1.0f, 0.90f, 0.85f, 1.0f };
    GLfloat ambient[] = { 0.35f, 0.40f, 0.50f, 1.0f }; 
    GLfloat specular[] = { 1.0f, 0.95f, 0.9f, 1.0f }; 

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat mat_specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat mat_shininess[] = { 40.0f }; 
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

// ============================================================
// SKYBOX (VISUAL UPGRADE)
// ============================================================

void drawSkybox()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Stunning atmospheric twilight sky gradient
    glBegin(GL_QUADS);
    glColor3f(0.10f, 0.20f, 0.40f); // Deep twilight blue
    glVertex2f(0.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glColor3f(0.85f, 0.60f, 0.50f); // Warm peach horizon
    glVertex2f(1.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// ============================================================
// DISPLAY
// ============================================================

void display()
{
    updatePhysics();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawSkybox();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    setCamera();
    setupLighting();

    drawGround();
    drawCity(); // Render procedural skyline
    drawTrack();
    drawStations();
    drawEnvironment();
    drawTrain();
    if (cameraMode == 4) drawHuman(); // First-person mode hides the human body.
    drawHUD();

    glutSwapBuffers();
    glutPostRedisplay();
}

// ============================================================
// WINDOW RESIZE
// ============================================================

void reshape(int width, int height)
{
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)width / (float)height, 0.1f, 320.0f);
    glMatrixMode(GL_MODELVIEW);
}

// ============================================================
// RESET
// ============================================================

void resetSimulation()
{
    trainDistance = INITIAL_TRAIN_DISTANCE; trainSpeed = 0.0f; throttle = 0.0f; brake = 0.0f; manualBrakeApplied = false;
    trainDirection = 1; waitingAtTerminal = false; terminalThrottleArmed = true; cameraMode = 2; keyW = false; keyS = false;
    for (int i = 0; i < 6; i++) { camPanX[i] = 0.0f; camPanZ[i] = 0.0f; camLift[i] = 0.0f; }
    droneCamX = -5.0f; droneCamY = 12.0f; droneCamZ = 48.0f; droneYaw = 180.0f; dronePitch = -12.0f;
    humanX = stations[0].x + 4.0f; humanY = 0.0f; humanZ = stations[0].z; humanYaw = 180.0f; humanCameraYaw = 180.0f; humanCameraPitch = -8.0f;
    metroDoorsOpen = true; humanOnboard = false; humanLocalX = 0.0f; humanLocalZ = 0.0f; lastOnboardTrainYaw = 0.0f;
    for (int i = 0; i < 4; i++) { stationPassCount[i] = 0; cardStationCount[i] = 0; }
    metroCardOwned = false; metroCardVisible = false; selectedFoilOption = -1; selectedFromStation = -1; selectedToStation = -1;
    cardFromStation = -1; cardToStation = -1; cardJourneyReachedTarget = false;
}

// ============================================================
// MOUSE UI
// ============================================================

void mouseClick(int button, int state, int x, int y)
{
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;
    int windowWidth = glutGet(GLUT_WINDOW_WIDTH); int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
    float mouseX = (float)x; float mouseY = (float)(windowHeight - y);

    Rect2D buyButton = getBuyCardButtonRect(windowWidth, windowHeight);
    Rect2D sellButton = getSellCardButtonRect(windowWidth, windowHeight);

    if (!metroCardOwned) {
        for (int i = 0; i < 4; i++) {
            Rect2D fromButton = getRouteStationButtonRect(0, i, windowWidth, windowHeight);
            if (pointInsideRect(mouseX, mouseY, fromButton)) { selectedFromStation = i; glutPostRedisplay(); return; }
            Rect2D toButton = getRouteStationButtonRect(1, i, windowWidth, windowHeight);
            if (pointInsideRect(mouseX, mouseY, toButton)) { selectedToStation = i; glutPostRedisplay(); return; }
        }
    }

    if (pointInsideRect(mouseX, mouseY, buyButton)) {
        if (!metroCardOwned && hasValidRouteSelection()) {
            metroCardOwned = true; metroCardVisible = true; selectedFoilOption = -1;
            cardFromStation = selectedFromStation; cardToStation = selectedToStation; cardJourneyReachedTarget = false;
            for (int i = 0; i < 4; i++) { stationPassCount[i] = 0; cardStationCount[i] = 0; }
        }
        glutPostRedisplay(); return;
    }

    if (pointInsideRect(mouseX, mouseY, sellButton)) {
        if (metroCardOwned) {
            metroCardOwned = false; metroCardVisible = false; selectedFoilOption = -1; selectedFromStation = -1;
            selectedToStation = -1; cardFromStation = -1; cardToStation = -1; cardJourneyReachedTarget = false;
        }
        glutPostRedisplay(); return;
    }

    if (metroCardOwned && metroCardVisible) {
        for (int i = 0; i < 3; i++) {
            Rect2D foilButton = getFoilButtonRect(i, windowWidth, windowHeight);
            if (pointInsideRect(mouseX, mouseY, foilButton)) {
                if (selectedFoilOption == i) selectedFoilOption = -1; else selectedFoilOption = i;
                glutPostRedisplay(); return;
            }
        }
    }
}

// ============================================================
// NORMAL KEY DOWN
// ============================================================

void keyboardDown(unsigned char key, int x, int y)
{
    if (key == 'w' || key == 'W') {
        if (waitingAtTerminal) {
            if (terminalThrottleArmed) { waitingAtTerminal = false; terminalThrottleArmed = false; keyW = true; brake = 0.0f; manualBrakeApplied = false; } else { keyW = false; }
        } else { keyW = true; brake = 0.0f; manualBrakeApplied = false; }
    }
    else if (key == 's' || key == 'S') keyS = true;
    else if (key == 'b' || key == 'B') { throttle = 0.0f; brake = 1.0f; manualBrakeApplied = true; }
    else if (key == 'n' || key == 'N') { brake = 0.0f; manualBrakeApplied = false; }
    else if (key == 'c' || key == 'C') { cameraMode++; if (cameraMode > 5) cameraMode = 0; }
    else if (key == 'e' || key == 'E') interactWithMetro();
    else if (key == '1') { if (cameraMode == 3) droneCamY += personVerticalSpeed; else if (cameraMode < 4) camLift[cameraMode] += 2.0f; }
    else if (key == '2') { if (cameraMode == 3) { droneCamY -= personVerticalSpeed; if (droneCamY < 1.0f) droneCamY = 1.0f; } else if (cameraMode < 4) camLift[cameraMode] -= 2.0f; }
    else if ((key == 'u' || key == 'U') && cameraMode == 3) { float yawRadians = droneYaw * PI / 180.0f; droneCamX += sinf(yawRadians) * personMoveSpeed; droneCamZ += cosf(yawRadians) * personMoveSpeed; }
    else if ((key == 'j' || key == 'J') && cameraMode == 3) { float yawRadians = droneYaw * PI / 180.0f; droneCamX -= sinf(yawRadians) * personMoveSpeed; droneCamZ -= cosf(yawRadians) * personMoveSpeed; }
    else if ((key == 'h' || key == 'H') && cameraMode == 3) { float yawRadians = droneYaw * PI / 180.0f; droneCamX += cosf(yawRadians) * personMoveSpeed; droneCamZ += sinf(yawRadians) * personMoveSpeed; }
    else if ((key == 'k' || key == 'K') && cameraMode == 3) { float yawRadians = droneYaw * PI / 180.0f; droneCamX -= cosf(yawRadians) * personMoveSpeed; droneCamZ -= sinf(yawRadians) * personMoveSpeed; }
    else if ((key == 'u' || key == 'U') && (cameraMode == 4 || cameraMode == 5)) { float a = humanCameraYaw * PI / 180.0f; moveHumanWorld(sinf(a) * humanMoveSpeed, cosf(a) * humanMoveSpeed); }
    else if ((key == 'j' || key == 'J') && (cameraMode == 4 || cameraMode == 5)) { float a = humanCameraYaw * PI / 180.0f; moveHumanWorld(-sinf(a) * humanMoveSpeed, -cosf(a) * humanMoveSpeed); }
    else if ((key == 'h' || key == 'H') && (cameraMode == 4 || cameraMode == 5)) { float a = humanCameraYaw * PI / 180.0f; moveHumanWorld(cosf(a) * humanMoveSpeed, -sinf(a) * humanMoveSpeed); }
    else if ((key == 'k' || key == 'K') && (cameraMode == 4 || cameraMode == 5)) { float a = humanCameraYaw * PI / 180.0f; moveHumanWorld(-cosf(a) * humanMoveSpeed, sinf(a) * humanMoveSpeed); }
    else if (key == 'r' || key == 'R') resetSimulation();
    else if (key == 27) exit(0);
}

// ============================================================
// NORMAL KEY RELEASE
// ============================================================

void keyboardUp(unsigned char key, int x, int y)
{
    if (key == 'w' || key == 'W') { keyW = false; if (waitingAtTerminal) terminalThrottleArmed = true; }
    if (key == 's' || key == 'S') keyS = false;
}

// ============================================================
// ARROW KEYS
// ============================================================

void specialKeys(int key, int x, int y)
{
    float movementSpeed = 2.5f;

    if (cameraMode == 3) {
        if (key == GLUT_KEY_UP) { dronePitch += personRotationStep; if (dronePitch > 89.0f) dronePitch = 89.0f; }
        else if (key == GLUT_KEY_DOWN) { dronePitch -= personRotationStep; if (dronePitch < -89.0f) dronePitch = -89.0f; }
        else if (key == GLUT_KEY_LEFT) { droneYaw += personRotationStep; if (droneYaw >= 360.0f) droneYaw -= 360.0f; }
        else if (key == GLUT_KEY_RIGHT) { droneYaw -= personRotationStep; if (droneYaw < 0.0f) droneYaw += 360.0f; }
        return;
    }

    if (cameraMode == 4 || cameraMode == 5) {
        if (key == GLUT_KEY_UP) { humanCameraPitch += personRotationStep; if (humanCameraPitch > 25.0f) humanCameraPitch = 25.0f; }
        else if (key == GLUT_KEY_DOWN) { humanCameraPitch -= personRotationStep; if (humanCameraPitch < -15.0f) humanCameraPitch = -15.0f; }
        else if (key == GLUT_KEY_LEFT) { humanCameraYaw += personRotationStep; if (humanCameraYaw >= 360.0f) humanCameraYaw -= 360.0f; }
        else if (key == GLUT_KEY_RIGHT) { humanCameraYaw -= personRotationStep; if (humanCameraYaw < 0.0f) humanCameraYaw += 360.0f; }
        return;
    }

    if (cameraMode == 2) {
        if (key == GLUT_KEY_UP) camPanZ[2] -= movementSpeed;
        if (key == GLUT_KEY_DOWN) camPanZ[2] += movementSpeed;
        if (key == GLUT_KEY_LEFT) camPanX[2] -= movementSpeed;
        if (key == GLUT_KEY_RIGHT) camPanX[2] += movementSpeed;
    } else {
        Vec2 tangent = sampleTrackTangent(trainDistance);
        tangent.x *= (float)trainDirection; tangent.z *= (float)trainDirection;
        Vec2 right = { tangent.z, -tangent.x };

        if (key == GLUT_KEY_UP) { camPanX[cameraMode] += tangent.x * movementSpeed; camPanZ[cameraMode] += tangent.z * movementSpeed; }
        if (key == GLUT_KEY_DOWN) { camPanX[cameraMode] -= tangent.x * movementSpeed; camPanZ[cameraMode] -= tangent.z * movementSpeed; }
        if (key == GLUT_KEY_LEFT) { camPanX[cameraMode] -= right.x * movementSpeed; camPanZ[cameraMode] -= right.z * movementSpeed; }
        if (key == GLUT_KEY_RIGHT) { camPanX[cameraMode] += right.x * movementSpeed; camPanZ[cameraMode] += right.z * movementSpeed; }
    }
}

// ============================================================
// INITIALIZATION
// ============================================================

void initialize()
{
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    generateTextures();
    buildTrack();

    for (int i = 0; i < 4; i++) {
        stationTrackDistance[i] = findNearestTrackDistance(stations[i].x, stations[i].z);
    }

    trainDistance = INITIAL_TRAIN_DISTANCE;
    trainDirection = 1; waitingAtTerminal = false; terminalThrottleArmed = true; lastTime = glutGet(GLUT_ELAPSED_TIME);
}

// ============================================================
// MAIN
// ============================================================

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1400, 850);
    glutInitWindowPosition(50, 40);
    glutCreateWindow("3D Train Simulator - Epic Terrain & Cityscape");

    initialize();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseClick);

    glutMainLoop();
    return 0;
}