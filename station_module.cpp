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
GLuint texNoise, texWood, texBrick;

void generateTextures()
{
    // Generate simple procedural textures in memory to vastly improve visuals
    // without requiring external image files.
    
    // 1. Noise/Grain (Used for Ground, Ballast, Metal, Asphalt)
    glGenTextures(1, &texNoise);
    glBindTexture(GL_TEXTURE_2D, texNoise);
    unsigned char dataN[128][128][3];
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            unsigned char c = 180 + rand() % 75; // Grayscale noise
            dataN[i][j][0] = dataN[i][j][1] = dataN[i][j][2] = c;
        }
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, dataN);

    // 2. Wood Grain (Used for Sleepers, Tree Trunks)
    glGenTextures(1, &texWood);
    glBindTexture(GL_TEXTURE_2D, texWood);
    unsigned char dataW[128][128][3];
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            // Directional sine wave noise for grain effect
            unsigned char c = 130 + (rand() % 30) + (int)(40 * sin(i * 0.15 + sin(j * 0.05) * 3.0));
            dataW[i][j][0] = dataW[i][j][1] = dataW[i][j][2] = c;
        }
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, dataW);

    // 3. Brick Pattern (Used for Station Buildings)
    glGenTextures(1, &texBrick);
    glBindTexture(GL_TEXTURE_2D, texBrick);
    unsigned char dataB[128][128][3];
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            int row = j / 16;
            bool edgeX = (i % 32 < 2) && (row % 2 == 0);
            bool edgeXOff = ((i + 16) % 32 < 2) && (row % 2 == 1);
            bool edgeY = (j % 16 < 2);
            
            if (edgeX || edgeXOff || edgeY) {
                // Mortar
                dataB[i][j][0] = dataB[i][j][1] = dataB[i][j][2] = 90;
            } else {
                // Brick variation
                unsigned char c = 200 + rand() % 40;
                dataB[i][j][0] = dataB[i][j][1] = dataB[i][j][2] = c;
            }
        }
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, dataB);
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

// trainDistance is the CENTER of the three-car metro consist.
// The two cab cars sit on either side of this center position.
float trainDistance = 0.0f;
float trainSpeed = 0.0f;

float throttle = 0.0f;
float brake = 0.0f;

// Tracks whether B/N/W set a real user brake command.  This lets the
// automatic terminal braking vary smoothly without overriding a manual stop.
bool manualBrakeApplied = false;

// 100 km/h converted to metres per second.
const float MAX_SPEED = 100.0f / 3.6f;

// Faster metro acceleration so the train reaches 100 km/h much sooner.
const float ENGINE_ACCELERATION = 7.0f;
const float THROTTLE_RAMP_RATE = 1.25f;
const float MAX_BRAKE_DECELERATION = 8.5f;

// +1 = travelling from Station 1 toward Station 4 / far end of the route
// -1 = travelling from Station 4 toward Station 1 / start end of the route
int trainDirection = 1;

// Distance from the middle car center to either cab-car center.
// Keeping trainDistance at the consist center lets the two-headed metro
// reverse without visually jumping to a different place on the track.
const float METRO_CAB_OFFSET = 5.35f;

// Initial center position near the open top-left end of the route.
// This places the metro before Station 1 exactly as in the sketch.
const float INITIAL_TRAIN_DISTANCE = 7.0f;

// Terminal behavior: the metro passes Station 4 / Station 1 first, then
// continues to the PHYSICAL END of the open track.  At the end it slows,
// stops, changes travel direction, and waits for a fresh W press.
bool waitingAtTerminal = false;
bool terminalThrottleArmed = true;

// Automatic braking starts before the physical end of the rail.  This gives
// enough stopping distance even at the new 100 km/h maximum speed.
const float TERMINAL_BRAKE_LOOKAHEAD = 82.0f;

// ============================================================
// KEY STATES
// ============================================================

bool keyW = false;
bool keyS = false;

// ============================================================
// CAMERA
// ============================================================

// 0 = Chase
// 1 = Driver
// 2 = Top
// 3 = Person
int cameraMode = 2;

// Separate camera movement for each mode
float camPanX[4] = {0.0f, 0.0f, 0.0f, 0.0f};
float camPanZ[4] = {0.0f, 0.0f, 0.0f, 0.0f};
float camLift[4] = {0.0f, 0.0f, 0.0f, 0.0f};

// Independent Person camera. This camera is NOT fixed to the train.
float personCamX = -5.0f;
float personCamY = 12.0f;
float personCamZ = 48.0f;

// Horizontal facing direction. 180 degrees looks toward negative Z.
float personYaw = 180.0f;

// Vertical look angle in degrees.
float personPitch = -12.0f;

// Independent Person camera movement / rotation amounts.
float personMoveSpeed = 3.0f;
float personVerticalSpeed = 2.5f;
float personRotationStep = 10.0f;

// Current camera world position.
// Used by the station counter circles so they always face the camera.
float activeCameraX = 0.0f;
float activeCameraY = 96.0f;
float activeCameraZ = 0.0f;

int lastTime = 0;

// ============================================================
// STATIONS
//
// Placement follows the NEW open serpentine drawing:
//
// Station 1 = beside the first/top straight, shortly after the start
// Station 2 = beside the third straight on the left
// Station 3 = beside the fifth straight on the right
// Station 4 = beside the final/bottom straight near the left terminal
// ============================================================

Station stations[4] =
{
    {
        "Station 1",
        -36.0f, -43.0f,
        6.0f, 3.0f
    },

    {
        "Station 2",
        -36.0f, -8.0f,
        7.0f, 3.5f
    },

    {
        "Station 3",
        32.0f, 16.0f,
        7.0f, 3.5f
    },

    {
        "Station 4",
        -38.0f, 54.0f,
        7.0f, 3.5f
    }
};

// ============================================================
// STATION PASS COUNTERS
// ============================================================

// Number of times the locomotive has crossed each station platform.
int stationPassCount[4] = {0, 0, 0, 0};

// Distance along the railway that corresponds to each station.
// Filled automatically in initialize() by finding the closest
// point on the railway to each platform.
float stationTrackDistance[4] = {0.0f, 0.0f, 0.0f, 0.0f};


// ============================================================
// METRO CARD UI / STATE
// ============================================================

// A card is created when the player presses the Buy Card button.
// Both the live station counters and the card counters start from zero
// for that card session, then normally increase together.
bool metroCardOwned = false;
bool metroCardVisible = false;
int cardStationCount[4] = {0, 0, 0, 0};

// Route selection made BEFORE buying a card.
// -1 means that row has not been selected yet.
int selectedFromStation = -1;
int selectedToStation = -1;

// Once a card is purchased, its route is locked to these values until
// the card is sold or the simulation is reset.
int cardFromStation = -1;
int cardToStation = -1;

// Becomes true the first time the selected destination station is crossed
// during the current card session.  Once the destination has been reached,
// every station on the paid route must have been crossed exactly once.
bool cardJourneyReachedTarget = false;

// Aluminum foil state:
// -1 = card is outside any foil
//  0 = cheap light-gray foil
//  1 = torn gray foil
//  2 = expensive ash-colored foil (freezes the card counters)
// Clicking the currently selected foil button again removes the card
// from that foil.
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
    if (value < minimum)
        return minimum;

    if (value > maximum)
        return maximum;

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
    float length =
        sqrtf(v.x * v.x + v.z * v.z);

    if (length < 0.0001f)
    {
        return {0.0f, 0.0f};
    }

    return
    {
        v.x / length,
        v.z / length
    };
}

// ============================================================
// TEXTURED & DETAILED CUBE (VISUAL UPGRADE)
// Replaces simple glutSolidCube to allow proper UV mapping.
// ============================================================

void drawCube(
    float x,
    float y,
    float z,

    float sx,
    float sy,
    float sz,

    float r,
    float g,
    float b,
    
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

    // We build the custom scaled cube here rather than using glScalef 
    // so that the UV texture coordinates scale natively with the world size.
    // This prevents texture stretching.
    glBegin(GL_QUADS);
    
    // Front
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-hx, -hy, hz);
    glTexCoord2f(sx, 0.0f);   glVertex3f(hx, -hy, hz);
    glTexCoord2f(sx, sy);     glVertex3f(hx, hy, hz);
    glTexCoord2f(0.0f, sy);   glVertex3f(-hx, hy, hz);
    
    // Back
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(hx, -hy, -hz);
    glTexCoord2f(sx, 0.0f);   glVertex3f(-hx, -hy, -hz);
    glTexCoord2f(sx, sy);     glVertex3f(-hx, hy, -hz);
    glTexCoord2f(0.0f, sy);   glVertex3f(hx, hy, -hz);
    
    // Top
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-hx, hy, hz);
    glTexCoord2f(sx, 0.0f);   glVertex3f(hx, hy, hz);
    glTexCoord2f(sx, sz);     glVertex3f(hx, hy, -hz);
    glTexCoord2f(0.0f, sz);   glVertex3f(-hx, hy, -hz);
    
    // Bottom
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-hx, -hy, -hz);
    glTexCoord2f(sx, 0.0f);   glVertex3f(hx, -hy, -hz);
    glTexCoord2f(sx, sz);     glVertex3f(hx, -hy, hz);
    glTexCoord2f(0.0f, sz);   glVertex3f(-hx, -hy, hz);
    
    // Right
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(hx, -hy, hz);
    glTexCoord2f(sz, 0.0f);   glVertex3f(hx, -hy, -hz);
    glTexCoord2f(sz, sy);     glVertex3f(hx, hy, -hz);
    glTexCoord2f(0.0f, sy);   glVertex3f(hx, hy, hz);
    
    // Left
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
    if (!trackPoints.empty())
    {
        Vec2 last =
            trackPoints.back();

        if (
            distance2D(
                last,
                {x, z}
            ) < 0.01f
        )
        {
            return;
        }
    }

    trackPoints.push_back(
        {x, z}
    );
}

// ------------------------------------------------------------
// STRAIGHT TRACK
// ------------------------------------------------------------

void addLine(
    float x1,
    float z1,

    float x2,
    float z2,

    int samples
)
{
    for (int i = 0; i <= samples; i++)
    {
        float t =
            (float)i /
            (float)samples;

        float x =
            x1 +
            (x2 - x1) * t;

        float z =
            z1 +
            (z2 - z1) * t;

        pushTrackPoint(
            x,
            z
        );
    }
}

// ------------------------------------------------------------
// CIRCULAR TURN
// ------------------------------------------------------------

void addArc(
    float centerX,
    float centerZ,

    float radius,

    float startDegrees,
    float endDegrees,

    int samples
)
{
    for (int i = 0; i <= samples; i++)
    {
        float t =
            (float)i /
            (float)samples;

        float degrees =
            startDegrees +
            (endDegrees - startDegrees) * t;

        float radians =
            degrees * PI / 180.0f;

        float x =
            centerX +
            radius * cosf(radians);

        float z =
            centerZ +
            radius * sinf(radians);

        pushTrackPoint(
            x,
            z
        );
    }
}

// ------------------------------------------------------------
// LARGE SMOOTH OUTER CURVE
// ------------------------------------------------------------

void addBezier(
    Vec2 p0,
    Vec2 p1,
    Vec2 p2,
    Vec2 p3,
    int samples
)
{
    for (int i = 0; i <= samples; i++)
    {
        float t =
            (float)i /
            (float)samples;

        float u =
            1.0f - t;

        float x =
            u * u * u * p0.x +
            3.0f * u * u * t * p1.x +
            3.0f * u * t * t * p2.x +
            t * t * t * p3.x;

        float z =
            u * u * u * p0.z +
            3.0f * u * u * t * p1.z +
            3.0f * u * t * t * p2.z +
            t * t * t * p3.z;

        pushTrackPoint(
            x,
            z
        );
    }
}

// ============================================================
// BUILD TRACK
// ============================================================

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

    for (size_t i = 1; i < trackPoints.size(); i++)
    {
        float segment = distance2D(trackPoints[i - 1], trackPoints[i]);
        cumulativeLength.push_back(cumulativeLength.back() + segment);
    }

    totalTrackLength = cumulativeLength.back();
}

// ============================================================
// TRACK SAMPLING
// ============================================================

float clampTrackDistance(float distance)
{
    if (distance < 0.0f)
    {
        return 0.0f;
    }

    if (distance > totalTrackLength)
    {
        return totalTrackLength;
    }

    return distance;
}

Vec2 sampleTrackPosition(float distance)
{
    distance = clampTrackDistance(distance);

    for (size_t i = 0; i < cumulativeLength.size() - 1; i++)
    {
        if (distance >= cumulativeLength[i] && distance <= cumulativeLength[i + 1])
        {
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

// ============================================================
// STATION PASS DETECTION
// ============================================================

float findNearestTrackDistance(float x, float z)
{
    float bestDistance = 0.0f;
    float bestError = 1.0e30f;

    for (size_t i = 0; i < trackPoints.size(); i++)
    {
        float dx = trackPoints[i].x - x;
        float dz = trackPoints[i].z - z;
        float error = dx * dx + dz * dz;

        if (error < bestError)
        {
            bestError = error;
            bestDistance = cumulativeLength[i];
        }
    }

    return bestDistance;
}

bool crossedTrackMarker(float previousDistance, float currentDistance, float markerDistance)
{
    if (fabsf(currentDistance - previousDistance) < 0.00001f)
    {
        return false;
    }

    if (currentDistance > previousDistance)
    {
        return markerDistance > previousDistance && markerDistance <= currentDistance;
    }

    return markerDistance < previousDistance && markerDistance >= currentDistance;
}

void updateStationPassCounters(float previousDistance, float currentDistance)
{
    if (!metroCardOwned)
    {
        return;
    }

    bool cardCounterFrozen = selectedFoilOption == 2;

    for (int i = 0; i < 4; i++)
    {
        if (crossedTrackMarker(previousDistance, currentDistance, stationTrackDistance[i]))
        {
            stationPassCount[i]++;

            if (i == cardToStation)
            {
                cardJourneyReachedTarget = true;
            }

            if (!cardCounterFrozen)
            {
                cardStationCount[i]++;
            }
        }
    }
}

// ============================================================
// GROUND
// ============================================================

void drawGround()
{
    glDisable(GL_LIGHTING);

    // Ground Base
    glColor3f(0.70f, 0.90f, 0.58f);
    
    // VISUAL UPGRADE: Map procedural noise texture to the vast ground plain
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texNoise);
    
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);     glVertex3f(-85.0f, -0.15f, -55.0f);
    glTexCoord2f(170.0f, 0.0f);   glVertex3f(85.0f, -0.15f, -55.0f);
    glTexCoord2f(170.0f, 110.0f); glVertex3f(85.0f, -0.15f, 55.0f);
    glTexCoord2f(0.0f, 110.0f);   glVertex3f(-85.0f, -0.15f, 55.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    // Grid
    glColor3f(0.43f, 0.73f, 0.33f);
    glBegin(GL_LINES);
    for (int x = -80; x <= 80; x += 8)
    {
        glVertex3f((float)x, -0.14f, -55.0f);
        glVertex3f((float)x, -0.14f, 55.0f);
    }
    for (int z = -48; z <= 48; z += 8)
    {
        glVertex3f(-85.0f, -0.14f, (float)z);
        glVertex3f(85.0f, -0.14f, (float)z);
    }
    glEnd();

    glEnable(GL_LIGHTING);
}

// ============================================================
// TRACK NORMAL / TANGENT
// ============================================================

Vec2 pointTangent(size_t index)
{
    size_t count = trackPoints.size();
    size_t previous = (index == 0) ? 0 : index - 1;
    size_t next = (index >= count - 1) ? count - 1 : index + 1;

    return normalize2D({
        trackPoints[next].x - trackPoints[previous].x,
        trackPoints[next].z - trackPoints[previous].z
    });
}

// ============================================================
// DRAW TRACK
// ============================================================

void drawTrack()
{
    // --------------------------------------------------------
    // Ballast (VISUAL UPGRADE: Textured gravel)
    // --------------------------------------------------------
    glDisable(GL_LIGHTING);
    glColor3f(0.82f, 0.82f, 0.80f);
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texNoise);
    
    glBegin(GL_QUAD_STRIP);
    for (size_t i = 0; i < trackPoints.size(); i++)
    {
        Vec2 tangent = pointTangent(i);
        Vec2 normal = { -tangent.z, tangent.x };
        float width = 1.75f;
        float dist = cumulativeLength[i];

        glTexCoord2f(0.0f, dist * 0.5f);
        glVertex3f(trackPoints[i].x + normal.x * width, 0.02f, trackPoints[i].z + normal.z * width);

        glTexCoord2f(2.0f, dist * 0.5f);
        glVertex3f(trackPoints[i].x - normal.x * width, 0.02f, trackPoints[i].z - normal.z * width);
    }
    glEnd();
    
    glDisable(GL_TEXTURE_2D);

    // --------------------------------------------------------
    // Rails
    // --------------------------------------------------------
    glLineWidth(3.0f);
    glColor3f(0.15f, 0.15f, 0.17f);

    for (int side = -1; side <= 1; side += 2)
    {
        glBegin(GL_LINE_STRIP);
        for (size_t i = 0; i < trackPoints.size(); i++)
        {
            Vec2 tangent = pointTangent(i);
            Vec2 normal = { -tangent.z, tangent.x };
            float offset = 0.60f * side;
            glVertex3f(trackPoints[i].x + normal.x * offset, 0.14f, trackPoints[i].z + normal.z * offset);
        }
        glEnd();
    }
    glEnable(GL_LIGHTING);

    // --------------------------------------------------------
    // Sleepers (VISUAL UPGRADE: Wood Texture)
    // --------------------------------------------------------
    for (float d = 0.0f; d < totalTrackLength; d += 1.8f)
    {
        Vec2 position = sampleTrackPosition(d);
        Vec2 tangent = sampleTrackTangent(d);
        float angle = atan2f(tangent.x, tangent.z) * 180.0f / PI;

        glPushMatrix();
        glTranslatef(position.x, 0.05f, position.z);
        glRotatef(angle, 0.0f, 1.0f, 0.0f);
        
        // Pass texWood into our custom drawCube
        drawCube(0.0f, 0.0f, 0.0f, 2.1f, 0.12f, 0.28f, 0.45f, 0.30f, 0.15f, texWood);

        glPopMatrix();
    }
}

// ============================================================
// PROFESSIONAL METRO TRAIN MODEL
// ============================================================

void drawMetroWheel(float x, float y, float z)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(1.0f, 1.0f, 0.42f);
    glColor3f(0.055f, 0.060f, 0.070f);
    glutSolidSphere(0.37f, 18, 14);
    glPopMatrix();
}

void drawMetroBogie(float z)
{
    drawCube(0.0f, 0.48f, z, 1.70f, 0.26f, 1.15f, 0.15f, 0.16f, 0.18f, texNoise);
    
    drawMetroWheel(-1.02f, 0.43f, z + 0.34f);
    drawMetroWheel( 1.02f, 0.43f, z + 0.34f);
    drawMetroWheel(-1.02f, 0.43f, z - 0.34f);
    drawMetroWheel( 1.02f, 0.43f, z - 0.34f);
}

void drawMetroUnderbody(float carLength)
{
    drawCube(0.0f, 0.68f, 0.0f, 2.12f, 0.28f, carLength, 0.15f, 0.16f, 0.18f, texNoise);
    drawCube(-0.48f, 0.56f, 0.0f, 0.70f, 0.30f, 1.45f, 0.18f, 0.20f, 0.23f, texNoise);
    drawCube(0.48f, 0.56f, 0.0f, 0.70f, 0.30f, 1.45f, 0.18f, 0.20f, 0.23f, texNoise);
}

void drawMetroRoofEquipment(float carLength)
{
    drawCube(0.0f, 2.68f, 0.0f, 2.06f, 0.18f, carLength - 0.18f, 0.74f, 0.77f, 0.80f, texNoise);
    drawCube(0.0f, 2.82f, -0.72f, 1.12f, 0.18f, 0.72f, 0.50f, 0.54f, 0.58f, texNoise);
    drawCube(0.0f, 2.82f, 0.72f, 1.12f, 0.18f, 0.72f, 0.50f, 0.54f, 0.58f, texNoise);
}

void drawMetroSideDoor(float x, float z)
{
    drawCube(x, 1.67f, z, 0.055f, 1.62f, 0.78f, 0.13f, 0.16f, 0.19f, texNoise);
    drawCube(x + (x < 0.0f ? -0.012f : 0.012f), 1.55f, z, 0.035f, 1.24f, 0.67f, 0.77f, 0.80f, 0.82f, texNoise);
    drawCube(x + (x < 0.0f ? -0.030f : 0.030f), 1.98f, z, 0.025f, 0.48f, 0.46f, 0.10f, 0.20f, 0.27f);
}

void drawMetroSideWindow(float x, float z, float width)
{
    drawCube(x, 2.02f, z, 0.052f, 0.64f, width, 0.075f, 0.095f, 0.115f);
    drawCube(x + (x < 0.0f ? -0.018f : 0.018f), 2.02f, z, 0.022f, 0.53f, width - 0.10f, 0.16f, 0.34f, 0.43f);
}

void drawMetroGangway(float z)
{
    drawCube(0.0f, 1.58f, z, 1.46f, 1.56f, 0.18f, 0.075f, 0.080f, 0.090f);
    drawCube(0.0f, 0.62f, z, 0.62f, 0.22f, 0.30f, 0.08f, 0.09f, 0.10f);
}

// ------------------------------------------------------------
// MODERN METRO CAB CAR
// ------------------------------------------------------------

void drawMetroCabCarModel()
{
    const float carLength = 4.80f;

    drawMetroUnderbody(carLength);

    // VISUAL UPGRADE: Applying texNoise to the metal shells gives a metallic speckle
    drawCube(0.0f, 1.28f, -0.10f, 2.06f, 1.05f, 4.35f, 0.91f, 0.93f, 0.94f, texNoise);
    drawCube(0.0f, 2.07f, -0.18f, 2.00f, 0.70f, 4.08f, 0.84f, 0.87f, 0.89f, texNoise);

    drawCube(-1.035f, 1.50f, -0.12f, 0.045f, 0.22f, 4.28f, 0.03f, 0.38f, 0.74f, texNoise);
    drawCube( 1.035f, 1.50f, -0.12f, 0.045f, 0.22f, 4.28f, 0.03f, 0.38f, 0.74f, texNoise);

    glPushMatrix();
    glTranslatef(0.0f, 1.66f, 2.10f);
    glScalef(1.04f, 1.18f, 0.70f);
    glColor3f(0.87f, 0.90f, 0.92f);
    glutSolidSphere(1.0f, 28, 20);
    glPopMatrix();

    drawCube(0.0f, 2.05f, 2.69f, 1.62f, 0.72f, 0.07f, 0.055f, 0.085f, 0.11f);
    drawCube(0.0f, 2.12f, 2.735f, 1.44f, 0.50f, 0.035f, 0.10f, 0.27f, 0.35f);
    drawCube(0.0f, 2.12f, 2.765f, 0.065f, 0.52f, 0.025f, 0.055f, 0.065f, 0.075f);
    drawCube(0.0f, 2.48f, 2.75f, 0.75f, 0.17f, 0.035f, 0.035f, 0.055f, 0.065f);
    
    drawCube(0.0f, 1.37f, 2.72f, 1.48f, 0.19f, 0.045f, 0.03f, 0.38f, 0.74f, texNoise);

    drawCube(-0.62f, 1.15f, 2.745f, 0.24f, 0.16f, 0.04f, 1.0f, 0.94f, 0.62f);
    drawCube( 0.62f, 1.15f, 2.745f, 0.24f, 0.16f, 0.04f, 1.0f, 0.94f, 0.62f);

    drawCube(-0.82f, 1.10f, 2.73f, 0.10f, 0.12f, 0.04f, 0.78f, 0.04f, 0.035f);
    drawCube( 0.82f, 1.10f, 2.73f, 0.10f, 0.12f, 0.04f, 0.78f, 0.04f, 0.035f);

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

// ------------------------------------------------------------
// MODERN METRO MIDDLE CAR
// ------------------------------------------------------------

void drawMetroMiddleCarModel()
{
    const float carLength = 4.95f;
    drawMetroUnderbody(carLength);

    drawCube(0.0f, 1.30f, 0.0f, 2.06f, 1.08f, 4.68f, 0.91f, 0.93f, 0.94f, texNoise);
    drawCube(0.0f, 2.08f, 0.0f, 2.00f, 0.70f, 4.55f, 0.84f, 0.87f, 0.89f, texNoise);

    drawCube(-1.035f, 1.50f, 0.0f, 0.045f, 0.22f, 4.62f, 0.03f, 0.38f, 0.74f, texNoise);
    drawCube( 1.035f, 1.50f, 0.0f, 0.045f, 0.22f, 4.62f, 0.03f, 0.38f, 0.74f, texNoise);

    drawMetroSideDoor(-1.030f,  1.28f);
    drawMetroSideDoor( 1.030f,  1.28f);
    drawMetroSideDoor(-1.030f, -1.28f);
    drawMetroSideDoor( 1.030f, -1.28f);

    drawMetroSideWindow(-1.025f,  2.00f, 0.43f);
    drawMetroSideWindow( 1.025f,  2.00f, 0.43f);
    drawMetroSideWindow(-1.025f,  0.56f, 0.47f);
    drawMetroSideWindow( 1.025f,  0.56f, 0.47f);
    drawMetroSideWindow(-1.025f, -0.56f, 0.47f);
    drawMetroSideWindow( 1.025f, -0.56f, 0.47f);
    drawMetroSideWindow(-1.025f, -2.00f, 0.43f);
    drawMetroSideWindow( 1.025f, -2.00f, 0.43f);

    drawMetroGangway( 2.39f);
    drawMetroGangway(-2.39f);

    drawMetroBogie( 1.38f);
    drawMetroBogie(-1.38f);
    drawMetroRoofEquipment(carLength);
}

// ============================================================
// TRAIN ON TRACK
// ============================================================

void drawTrainUnit(float distance, int carType, int facingDirection)
{
    Vec2 position = sampleTrackPosition(distance);
    Vec2 tangent = sampleTrackTangent(distance);

    float angle = atan2f(tangent.x, tangent.z) * 180.0f / PI;
    if (facingDirection < 0) {
        angle += 180.0f;
    }

    glPushMatrix();
    glTranslatef(position.x, 0.18f, position.z);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    if (carType == 0) {
        drawMetroCabCarModel();
    } else {
        drawMetroMiddleCarModel();
    }

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
// STATION TEXT
// ============================================================

void drawText3D(float x, float y, float z, const char* text)
{
    glDisable(GL_LIGHTING);
    glColor3f(0.05f, 0.05f, 0.05f);
    glRasterPos3f(x, y, z);

    while (*text)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
    glEnable(GL_LIGHTING);
}

// ============================================================
// STATION COUNTER SIGN
// ============================================================

void drawCircle2D(float radius, int segments)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * PI * (float)i / (float)segments;
        glVertex3f(cosf(angle) * radius, sinf(angle) * radius, 0.0f);
    }
    glEnd();
}

void drawCircleOutline(float radius, int segments)
{
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++)
    {
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

    for (const char* p = text; *p; p++)
    {
        textWidth += (float)glutStrokeWidth(GLUT_STROKE_ROMAN, *p);
    }

    const float textHeight = 119.05f;
    float scaleByWidth = 2.15f / (textWidth > 1.0f ? textWidth : 1.0f);
    float scaleByHeight = 1.65f / textHeight;
    float textScale = scaleByWidth < scaleByHeight ? scaleByWidth : scaleByHeight;

    glPushMatrix();
    glTranslatef(-textWidth * textScale * 0.5f, -textHeight * textScale * 0.42f, 0.10f);
    glScalef(textScale, textScale, textScale);

    glColor3f(1.0f, 1.0f, 0.0f);
    glLineWidth(4.0f);

    for (const char* p = text; *p; p++)
    {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
    }

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
    drawCube(postX, postHeight * 0.5f, postZ, 0.24f, postHeight, 0.24f, 0.02f, 0.32f, 1.0f);

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

    glColor4f(1.0f, 0.05f, 0.02f, 0.25f);
    drawCircle2D(1.75f, 48);

    glTranslatef(0.0f, 0.0f, 0.02f);
    glColor4f(1.0f, 0.02f, 0.02f, 0.95f);
    drawCircle2D(1.42f, 48);

    glTranslatef(0.0f, 0.0f, 0.02f);
    glColor4f(1.0f, 0.22f, 0.06f, 1.0f);
    glLineWidth(5.0f);
    drawCircleOutline(1.48f, 64);
    glLineWidth(1.0f);

    drawCounterNumber(stationPassCount[stationIndex]);

    glDisable(GL_BLEND);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

// ============================================================
// STATION
// ============================================================

void drawStation(const Station& station)
{
    // Platform
    drawCube(station.x, 0.16f, station.z, station.sizeX, 0.35f, station.sizeZ, 0.93f, 0.93f, 0.93f);
    
    // VISUAL UPGRADE: Platform Caution Edge
    drawCube(station.x, 0.35f, station.z + station.sizeZ*0.5f - 0.25f, station.sizeX, 0.05f, 0.5f, 0.95f, 0.85f, 0.15f);

    // Building (VISUAL UPGRADE: Brick Texture)
    drawCube(station.x, 1.15f, station.z, 2.8f, 1.8f, 2.0f, 0.80f, 0.35f, 0.20f, texBrick);

    // VISUAL UPGRADE: Decorative Windows & Door
    drawCube(station.x + 1.45f, 1.30f, station.z - 0.45f, 0.1f, 0.7f, 0.7f, 0.1f, 0.3f, 0.5f); // Front Window 1
    drawCube(station.x + 1.45f, 1.30f, station.z + 0.45f, 0.1f, 0.7f, 0.7f, 0.1f, 0.3f, 0.5f); // Front Window 2
    drawCube(station.x, 0.9f, station.z + 1.05f, 1.0f, 1.4f, 0.1f, 0.2f, 0.2f, 0.2f); // Side Door

    // Roof (VISUAL UPGRADE: Extended overhang, metal texture)
    drawCube(station.x + 0.2f, 2.18f, station.z, 3.4f, 0.25f, 2.4f, 0.35f, 0.35f, 0.35f, texNoise);

    drawText3D(station.x - 2.2f, 3.0f, station.z, station.name);
}

void drawStations()
{
    for (int i = 0; i < 4; i++)
    {
        drawStation(stations[i]);
        drawStationCounter(stations[i], i);
    }
}

// ============================================================
// TREES
// ============================================================

void drawTree(float x, float z)
{
    // VISUAL UPGRADE: Detailed Wood Trunk
    drawCube(x, 0.9f, z, 0.6f, 1.8f, 0.6f, 0.45f, 0.28f, 0.15f, texWood);

    // VISUAL UPGRADE: 3-Tier Pine Tree
    glPushMatrix();
    glTranslatef(x, 1.6f, z);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Rotate to point UP
    
    glColor3f(0.05f, 0.45f, 0.10f);
    glutSolidCone(1.6f, 2.2f, 16, 8); // Bottom layer

    glTranslatef(0.0f, 0.0f, 1.2f);
    glColor3f(0.08f, 0.50f, 0.13f);
    glutSolidCone(1.3f, 1.8f, 16, 8); // Middle layer

    glTranslatef(0.0f, 0.0f, 1.0f);
    glColor3f(0.12f, 0.55f, 0.16f);
    glutSolidCone(0.9f, 1.4f, 16, 8); // Top layer
    
    glPopMatrix();
}

void drawEnvironment()
{
    drawTree(-72.0f, 42.0f);
    drawTree(70.0f, 42.0f);

    drawTree(-70.0f, 12.0f);
    drawTree(72.0f, 8.0f);

    drawTree(-70.0f, -22.0f);
    drawTree(72.0f, -26.0f);

    drawTree(-66.0f, -48.0f);
    drawTree(70.0f, -47.0f);
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

    if (cameraMode == 0)
    {
        activeCameraX = position.x - tangent.x * 11.0f + panX;
        activeCameraY = 6.0f + lift;
        activeCameraZ = position.z - tangent.z * 11.0f + panZ;

        gluLookAt(
            activeCameraX, activeCameraY, activeCameraZ,
            position.x + tangent.x * 3.0f + panX, 1.8f + lift, position.z + tangent.z * 3.0f + panZ,
            0.0f, 1.0f, 0.0f
        );
    }
    else if (cameraMode == 1)
    {
        activeCameraX = position.x + tangent.x * 0.8f + panX;
        activeCameraY = 2.55f + lift;
        activeCameraZ = position.z + tangent.z * 0.8f + panZ;

        gluLookAt(
            activeCameraX, activeCameraY, activeCameraZ,
            position.x + tangent.x * 20.0f + panX, 2.2f + lift, position.z + tangent.z * 20.0f + panZ,
            0.0f, 1.0f, 0.0f
        );
    }
    else if (cameraMode == 2)
    {
        float topHeight = 96.0f + lift;
        if (topHeight < 25.0f) topHeight = 25.0f;

        activeCameraX = panX;
        activeCameraY = topHeight;
        activeCameraZ = panZ;

        gluLookAt(
            activeCameraX, activeCameraY, activeCameraZ,
            panX, 0.0f, panZ,
            0.0f, 0.0f, -1.0f
        );
    }
    else if (cameraMode == 3)
    {
        float yawRadians = personYaw * PI / 180.0f;
        float pitchRadians = personPitch * PI / 180.0f;

        float directionX = cosf(pitchRadians) * sinf(yawRadians);
        float directionY = sinf(pitchRadians);
        float directionZ = cosf(pitchRadians) * cosf(yawRadians);

        activeCameraX = personCamX;
        activeCameraY = personCamY;
        activeCameraZ = personCamZ;

        gluLookAt(
            activeCameraX, activeCameraY, activeCameraZ,
            personCamX + directionX, personCamY + directionY, personCamZ + directionZ,
            0.0f, 1.0f, 0.0f
        );
    }
}

// ============================================================
// HUD TEXT
// ============================================================

void drawText2D(float x, float y, const char* text)
{
    glRasterPos2f(x, y);
    while (*text)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

// ============================================================
// 2D METRO CARD UI HELPERS
// ============================================================

void drawText2DFont(float x, float y, const char* text, void* font)
{
    glRasterPos2f(x, y);
    while (*text)
    {
        glutBitmapCharacter(font, *text);
        text++;
    }
}

float getBitmapTextWidth(const char* text, void* font)
{
    float width = 0.0f;
    while (*text)
    {
        width += (float)glutBitmapWidth(font, *text);
        text++;
    }
    return width;
}

void drawFilledRect2D(const Rect2D& rect, float r, float g, float b)
{
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(rect.x, rect.y);
    glVertex2f(rect.x + rect.width, rect.y);
    glVertex2f(rect.x + rect.width, rect.y + rect.height);
    glVertex2f(rect.x, rect.y + rect.height);
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
    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * PI * (float)i / (float)segments;
        glVertex2f(centerX + cosf(angle) * radius, centerY + sinf(angle) * radius);
    }
    glEnd();
}

void drawCardOrFoilBody(const Rect2D& card)
{
    if (selectedFoilOption < 0)
    {
        drawFilledRect2D(card, 1.0f, 1.0f, 1.0f);
        drawRectOutline2D(card, 0.90f, 0.10f, 0.10f, 4.0f);
        return;
    }

    Rect2D cardTab;
    cardTab.x = card.x + 43.0f;
    cardTab.y = card.y + card.height - 2.0f;
    cardTab.width = card.width - 86.0f;
    cardTab.height = 28.0f;

    drawFilledRect2D(cardTab, 1.0f, 1.0f, 1.0f);
    drawRectOutline2D(cardTab, 0.25f, 0.25f, 0.25f, 2.0f);

    if (selectedFoilOption == 0) drawFilledRect2D(card, 0.84f, 0.86f, 0.88f);
    else if (selectedFoilOption == 1) drawFilledRect2D(card, 0.62f, 0.64f, 0.66f);
    else drawFilledRect2D(card, 0.50f, 0.52f, 0.54f);

    drawRectOutline2D(card, 0.24f, 0.24f, 0.26f, 4.0f);

    if (selectedFoilOption == 1)
    {
        drawFilledCircle2D(card.x + 24.0f, card.y + 30.0f, 9.0f, 28, 1.0f, 1.0f, 1.0f);
        drawFilledCircle2D(card.x + card.width - 26.0f, card.y + 42.0f, 12.0f, 28, 1.0f, 1.0f, 1.0f);
        drawFilledCircle2D(card.x + 31.0f, card.y + 122.0f, 8.0f, 28, 1.0f, 1.0f, 1.0f);
        drawFilledCircle2D(card.x + card.width - 28.0f, card.y + 142.0f, 10.0f, 28, 1.0f, 1.0f, 1.0f);
        drawFilledCircle2D(card.x + 42.0f, card.y + card.height - 39.0f, 11.0f, 28, 1.0f, 1.0f, 1.0f);
        drawFilledCircle2D(card.x + card.width - 42.0f, card.y + card.height - 27.0f, 7.0f, 28, 1.0f, 1.0f, 1.0f);
    }
}

bool pointInsideRect(float x, float y, const Rect2D& rect)
{
    return x >= rect.x && x <= rect.x + rect.width && y >= rect.y && y <= rect.y + rect.height;
}

Rect2D getBuyCardButtonRect(int windowWidth, int windowHeight)
{
    Rect2D rect;
    rect.width = 112.0f;
    rect.height = 34.0f;
    rect.x = (float)windowWidth - rect.width - 28.0f;
    rect.y = (float)windowHeight - 55.0f;
    return rect;
}

Rect2D getSellCardButtonRect(int windowWidth, int windowHeight)
{
    Rect2D rect = getBuyCardButtonRect(windowWidth, windowHeight);
    rect.y -= 48.0f;
    return rect;
}

Rect2D getRouteStationButtonRect(int row, int stationIndex, int windowWidth, int windowHeight)
{
    Rect2D buyButton = getBuyCardButtonRect(windowWidth, windowHeight);
    Rect2D sellButton = getSellCardButtonRect(windowWidth, windowHeight);
    Rect2D rect;
    rect.width = 42.0f;
    rect.height = 30.0f;
    float firstX = buyButton.x - 190.0f;
    rect.x = firstX + stationIndex * 44.0f;
    rect.y = (row == 0 ? buyButton.y : sellButton.y) + 2.0f;
    return rect;
}

bool hasValidRouteSelection()
{
    return selectedFromStation >= 0 && selectedFromStation < 4 &&
           selectedToStation >= 0 && selectedToStation < 4 &&
           selectedFromStation != selectedToStation;
}

Rect2D getMetroCardRect(int windowWidth, int windowHeight)
{
    Rect2D rect;
    rect.width = 235.0f;
    rect.height = 285.0f;
    rect.x = (float)windowWidth - rect.width - 30.0f;
    rect.y = 55.0f;
    return rect;
}

Rect2D getFoilButtonRect(int index, int windowWidth, int windowHeight)
{
    Rect2D card = getMetroCardRect(windowWidth, windowHeight);
    Rect2D rect;
    rect.width = 205.0f;
    rect.height = 34.0f;
    rect.x = card.x - rect.width - 28.0f;
    rect.y = card.y + 118.0f - index * 45.0f;
    return rect;
}

bool isMetroCardActivityNormal()
{
    if (!metroCardOwned || cardFromStation < 0 || cardFromStation > 3 ||
        cardToStation < 0 || cardToStation > 3 || cardFromStation == cardToStation)
    {
        return false;
    }

    for (int i = 0; i < 4; i++)
    {
        if (stationPassCount[i] != cardStationCount[i])
        {
            return false;
        }
    }

    int firstStation = cardFromStation < cardToStation ? cardFromStation : cardToStation;
    int lastStation = cardFromStation > cardToStation ? cardFromStation : cardToStation;

    for (int i = 0; i < 4; i++)
    {
        bool stationIsOnPaidRoute = i >= firstStation && i <= lastStation;

        if (!stationIsOnPaidRoute)
        {
            if (cardStationCount[i] != 0) return false;
        }
        else
        {
            if (cardStationCount[i] > 1) return false;
        }
    }

    if (cardJourneyReachedTarget)
    {
        for (int i = firstStation; i <= lastStation; i++)
        {
            if (cardStationCount[i] != 1) return false;
        }
    }

    return true;
}

void drawCenteredButtonText(const Rect2D& rect, const char* text, float r, float g, float b)
{
    void* font = GLUT_BITMAP_HELVETICA_12;
    float textWidth = getBitmapTextWidth(text, font);
    glColor3f(r, g, b);
    drawText2DFont(rect.x + (rect.width - textWidth) * 0.5f, rect.y + rect.height * 0.5f - 4.0f, text, font);
}

void drawMetroCardInterface(int windowWidth, int windowHeight)
{
    // --------------------------------------------------------
    // FROM / TO ROUTE SELECTORS
    // --------------------------------------------------------
    const char* stationButtonLabels[4] = { "1st", "2nd", "3rd", "4th" };

    Rect2D fromFirst = getRouteStationButtonRect(0, 0, windowWidth, windowHeight);
    Rect2D toFirst = getRouteStationButtonRect(1, 0, windowWidth, windowHeight);

    glColor3f(0.72f, 0.16f, 0.12f);
    drawText2DFont(fromFirst.x - 52.0f, fromFirst.y + 9.0f, "From :", GLUT_BITMAP_HELVETICA_12);
    drawText2DFont(toFirst.x - 35.0f, toFirst.y + 9.0f, "To :", GLUT_BITMAP_HELVETICA_12);

    for (int row = 0; row < 2; row++)
    {
        for (int i = 0; i < 4; i++)
        {
            Rect2D routeButton = getRouteStationButtonRect(row, i, windowWidth, windowHeight);
            int selectedStation = (row == 0) ? selectedFromStation : selectedToStation;

            if (selectedStation == i)
            {
                drawFilledRect2D(routeButton, 0.78f, 0.92f, 0.68f);
                drawRectOutline2D(routeButton, 0.05f, 0.42f, 0.08f, 4.0f);
            }
            else
            {
                drawFilledRect2D(routeButton, metroCardOwned ? 0.91f : 1.0f, metroCardOwned ? 0.91f : 1.0f, metroCardOwned ? 0.91f : 1.0f);
                drawRectOutline2D(routeButton, 0.05f, 0.05f, 0.05f, 3.0f);
            }
            drawCenteredButtonText(routeButton, stationButtonLabels[i], 0.55f, 0.18f, 0.08f);
        }
    }

    // --------------------------------------------------------
    // BUY CARD BUTTON
    // --------------------------------------------------------
    Rect2D buyButton = getBuyCardButtonRect(windowWidth, windowHeight);
    drawFilledRect2D(buyButton, 1.0f, 1.0f, 1.0f);
    bool buyCardEnabled = !metroCardOwned && hasValidRouteSelection();

    if (buyCardEnabled)
    {
        drawRectOutline2D(buyButton, 0.05f, 0.65f, 0.18f, 4.0f);
        drawCenteredButtonText(buyButton, "Buy Card", 0.05f, 0.55f, 0.16f);
    }
    else
    {
        drawRectOutline2D(buyButton, 0.55f, 0.55f, 0.55f, 3.0f);
        drawCenteredButtonText(buyButton, "Buy Card", 0.48f, 0.48f, 0.48f);
    }

    // --------------------------------------------------------
    // SELL CARD BUTTON
    // --------------------------------------------------------
    Rect2D sellButton = getSellCardButtonRect(windowWidth, windowHeight);
    drawFilledRect2D(sellButton, 1.0f, 1.0f, 1.0f);

    if (metroCardOwned)
    {
        drawRectOutline2D(sellButton, 0.90f, 0.08f, 0.08f, 4.0f);
        drawCenteredButtonText(sellButton, "Sell Card", 0.82f, 0.05f, 0.05f);
    }
    else
    {
        drawRectOutline2D(sellButton, 0.55f, 0.55f, 0.55f, 3.0f);
        drawCenteredButtonText(sellButton, "Sell Card", 0.50f, 0.50f, 0.50f);
    }

    if (!metroCardOwned || !metroCardVisible) return;

    // --------------------------------------------------------
    // METRO PASS / ALUMINUM FOIL SLEEVE
    // --------------------------------------------------------
    Rect2D card = getMetroCardRect(windowWidth, windowHeight);
    drawCardOrFoilBody(card);

    glColor3f(0.05f, 0.05f, 0.05f);
    drawText2DFont(card.x + 20.0f, card.y + card.height - 30.0f, "Metro Pass", GLUT_BITMAP_HELVETICA_18);
    drawText2DFont(card.x + 20.0f, card.y + card.height - 68.0f, "Visited Stations:", GLUT_BITMAP_HELVETICA_12);

    char line[128];
    for (int i = 0; i < 4; i++)
    {
        sprintf(line, "Station %d : %d", i + 1, cardStationCount[i]);
        drawText2DFont(card.x + 20.0f, card.y + card.height - 91.0f - i * 20.0f, line, GLUT_BITMAP_HELVETICA_12);
    }

    char routeLine[128];
    sprintf(routeLine, "From: %d    To: %d", cardFromStation + 1, cardToStation + 1);
    glColor3f(0.72f, 0.12f, 0.12f);
    drawText2DFont(card.x + 20.0f, card.y + 86.0f, routeLine, GLUT_BITMAP_HELVETICA_12);

    glColor3f(0.05f, 0.05f, 0.05f);
    drawText2DFont(card.x + 20.0f, card.y + 50.0f, "Card Activity :", GLUT_BITMAP_HELVETICA_12);

    if (isMetroCardActivityNormal())
    {
        glColor3f(0.05f, 0.65f, 0.18f);
        drawText2DFont(card.x + 118.0f, card.y + 50.0f, "Normal", GLUT_BITMAP_HELVETICA_12);
    }
    else
    {
        glColor3f(0.92f, 0.05f, 0.05f);
        drawText2DFont(card.x + 118.0f, card.y + 50.0f, "Unusual", GLUT_BITMAP_HELVETICA_12);
    }

    const char* foilLabels[3] = { "Aluminum Foil (Cheap)", "Aluminum Foil (torned)", "Aluminum Foil (Expensive)" };
    Rect2D firstFoilButton = getFoilButtonRect(0, windowWidth, windowHeight);

    glColor3f(0.75f, 0.12f, 0.12f);
    drawText2DFont(firstFoilButton.x + 5.0f, firstFoilButton.y + firstFoilButton.height + 20.0f, "Put / remove card in:", GLUT_BITMAP_HELVETICA_12);

    for (int i = 0; i < 3; i++)
    {
        Rect2D foilButton = getFoilButtonRect(i, windowWidth, windowHeight);

        if (selectedFoilOption == i)
        {
            if (i == 0) drawFilledRect2D(foilButton, 0.84f, 0.86f, 0.88f);
            else if (i == 1) drawFilledRect2D(foilButton, 0.68f, 0.70f, 0.72f);
            else drawFilledRect2D(foilButton, 0.56f, 0.58f, 0.60f);
        }
        else
        {
            drawFilledRect2D(foilButton, 1.0f, 1.0f, 1.0f);
        }

        drawRectOutline2D(foilButton, 0.15f, 0.28f, 0.90f, 3.0f);
        drawCenteredButtonText(foilButton, foilLabels[i], 0.15f, 0.28f, 0.82f);
    }
}

// ============================================================
// HUD
// ============================================================

void drawHUD()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D); // Ensure textures don't bleed onto HUD text

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);
    char buffer[256];

    sprintf(buffer, "Speed: %.1f km/h", trainSpeed * 3.6f);
    drawText2D(18, 145, buffer);

    sprintf(buffer, "Throttle: %.0f%%", throttle * 100.0f);
    drawText2D(18, 120, buffer);

    sprintf(buffer, "Brake: %.0f%%", brake * 100.0f);
    drawText2D(18, 95, buffer);

    const char* cameraNames[] = { "Chase", "Driver", "Top", "Person" };
    sprintf(buffer, "Camera: %s", cameraNames[cameraMode]);
    drawText2D(18, 70, buffer);

    drawText2D(18, 45, "W hold: Speed Up | S hold: Slow Down | B: Brake | N: Release Brake");
    drawText2D(18, 20, "C Camera | Person: Arrows Rotate | U/J Move F/B | H/K Move L/R | 1 Up | 2 Down | R Reset | ESC Exit");

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
    if (keyW)
    {
        brake = 0.0f;
        manualBrakeApplied = false;
        throttle += THROTTLE_RAMP_RATE * deltaTime;
        throttle = clampValue(throttle, 0.0f, 1.0f);
    }

    if (keyS)
    {
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

    if (waitingAtTerminal)
    {
        trainSpeed = 0.0f;
        throttle = 0.0f;
        brake = 1.0f;
        return;
    }

    updateControls(deltaTime);

    if (!manualBrakeApplied)
    {
        brake = 0.0f;
    }

    float rollingResistance = 0.0f;
    if (trainSpeed > 0.01f) rollingResistance = 0.18f;
    rollingResistance += 0.035f * trainSpeed;
    rollingResistance += 0.0035f * trainSpeed * trainSpeed;

    float terminalDistance = (trainDirection > 0) ? totalTrackLength : 0.0f;
    float previousLeadingCabDistance = getLeadingCabDistance();
    float distanceToTerminal;

    if (trainDirection > 0)
    {
        distanceToTerminal = terminalDistance - previousLeadingCabDistance;
    }
    else
    {
        distanceToTerminal = previousLeadingCabDistance - terminalDistance;
    }

    if (distanceToTerminal >= 0.0f && distanceToTerminal < TERMINAL_BRAKE_LOOKAHEAD)
    {
        if (!manualBrakeApplied)
        {
            throttle = 0.0f;

            if (trainSpeed > 0.05f)
            {
                float safeDistance = distanceToTerminal - 0.35f;
                if (safeDistance < 0.25f) safeDistance = 0.25f;

                float requiredTotalDeceleration = (trainSpeed * trainSpeed) / (2.0f * safeDistance);
                requiredTotalDeceleration *= 1.08f;

                float requiredBrakeDeceleration = requiredTotalDeceleration - rollingResistance;
                if (requiredBrakeDeceleration < 0.0f) requiredBrakeDeceleration = 0.0f;

                float automaticBrake = requiredBrakeDeceleration / MAX_BRAKE_DECELERATION;
                brake = clampValue(automaticBrake, 0.0f, 1.0f);
            }

            if (trainSpeed < 0.85f && distanceToTerminal > 0.55f)
            {
                brake = 0.0f;
                throttle = 0.20f;
            }
        }
    }

    float engineAcceleration = throttle * ENGINE_ACCELERATION;
    float brakeAcceleration = brake * MAX_BRAKE_DECELERATION;
    float gentleSlowdown = 0.0f;

    if (keyS && throttle <= 0.01f)
    {
        gentleSlowdown = 1.1f;
    }

    float acceleration = engineAcceleration - rollingResistance - brakeAcceleration - gentleSlowdown;
    trainSpeed += acceleration * deltaTime;
    trainSpeed = clampValue(trainSpeed, 0.0f, MAX_SPEED);

    if (trainSpeed < 0.02f && throttle <= 0.01f)
    {
        trainSpeed = 0.0f;
    }

    float previousTrainCenter = trainDistance;
    previousLeadingCabDistance = clampTrackDistance(previousTrainCenter + (float)trainDirection * METRO_CAB_OFFSET);

    trainDistance += (float)trainDirection * trainSpeed * deltaTime;
    trainDistance = clampTrackDistance(trainDistance);

    float currentLeadingCabDistance = getLeadingCabDistance();

    if (trainSpeed > 0.0f)
    {
        updateStationPassCounters(previousLeadingCabDistance, currentLeadingCabDistance);
    }

    bool reachedTerminal = false;
    float currentDistanceToTerminal;

    if (trainDirection > 0)
    {
        currentDistanceToTerminal = terminalDistance - currentLeadingCabDistance;
        reachedTerminal = currentLeadingCabDistance >= terminalDistance;
    }
    else
    {
        currentDistanceToTerminal = currentLeadingCabDistance - terminalDistance;
        reachedTerminal = currentLeadingCabDistance <= terminalDistance;
    }

    if (!reachedTerminal && currentDistanceToTerminal >= 0.0f && currentDistanceToTerminal <= 0.60f && trainSpeed < 1.0f)
    {
        reachedTerminal = true;
    }

    if (reachedTerminal)
    {
        trainDistance = terminalDistance - (float)trainDirection * METRO_CAB_OFFSET;
        trainDistance = clampTrackDistance(trainDistance);

        trainSpeed = 0.0f;
        throttle = 0.0f;
        brake = 1.0f;
        manualBrakeApplied = false;

        trainDirection = -trainDirection;
        waitingAtTerminal = true;

        terminalThrottleArmed = !keyW;
        keyW = false;
    }
}

// ============================================================
// LIGHTING & MATERIALS
// ============================================================

void setupLighting()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPosition[] = { 25.0f, 60.0f, 20.0f, 1.0f };
    GLfloat diffuse[] = { 0.95f, 0.95f, 0.92f, 1.0f };
    GLfloat ambient[] = { 0.35f, 0.35f, 0.38f, 1.0f };
    GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // VISUAL UPGRADE: Add specular reflections

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Give everything a slight natural shine
    GLfloat mat_specular[] = { 0.25f, 0.25f, 0.25f, 1.0f };
    GLfloat mat_shininess[] = { 25.0f };
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

    // Smooth gradient sky background
    glBegin(GL_QUADS);
    // Top (darker blue)
    glColor3f(0.25f, 0.50f, 0.85f);
    glVertex2f(0.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    // Bottom horizon (lighter, slight sunset tint)
    glColor3f(0.70f, 0.85f, 0.95f);
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

    // VISUAL UPGRADE: Replaced solid clear color with gradient skybox
    drawSkybox();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    setCamera();
    setupLighting();

    drawGround();
    drawTrack();
    drawStations();
    drawEnvironment();
    drawTrain();
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
    gluPerspective(60.0f, (float)width / (float)height, 0.1f, 260.0f);
    glMatrixMode(GL_MODELVIEW);
}

// ============================================================
// RESET
// ============================================================

void resetSimulation()
{
    trainDistance = INITIAL_TRAIN_DISTANCE;
    trainSpeed = 0.0f;
    throttle = 0.0f;
    brake = 0.0f;
    manualBrakeApplied = false;
    trainDirection = 1;
    waitingAtTerminal = false;
    terminalThrottleArmed = true;
    cameraMode = 2;
    keyW = false;
    keyS = false;

    for (int i = 0; i < 4; i++)
    {
        camPanX[i] = 0.0f;
        camPanZ[i] = 0.0f;
        camLift[i] = 0.0f;
    }

    personCamX = -5.0f;
    personCamY = 12.0f;
    personCamZ = 48.0f;
    personYaw = 180.0f;
    personPitch = -12.0f;

    for (int i = 0; i < 4; i++)
    {
        stationPassCount[i] = 0;
        cardStationCount[i] = 0;
    }

    metroCardOwned = false;
    metroCardVisible = false;
    selectedFoilOption = -1;
    selectedFromStation = -1;
    selectedToStation = -1;
    cardFromStation = -1;
    cardToStation = -1;
    cardJourneyReachedTarget = false;
}

// ============================================================
// MOUSE UI
// ============================================================

void mouseClick(int button, int state, int x, int y)
{
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

    int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

    float mouseX = (float)x;
    float mouseY = (float)(windowHeight - y);

    Rect2D buyButton = getBuyCardButtonRect(windowWidth, windowHeight);
    Rect2D sellButton = getSellCardButtonRect(windowWidth, windowHeight);

    if (!metroCardOwned)
    {
        for (int i = 0; i < 4; i++)
        {
            Rect2D fromButton = getRouteStationButtonRect(0, i, windowWidth, windowHeight);
            if (pointInsideRect(mouseX, mouseY, fromButton))
            {
                selectedFromStation = i;
                glutPostRedisplay();
                return;
            }

            Rect2D toButton = getRouteStationButtonRect(1, i, windowWidth, windowHeight);
            if (pointInsideRect(mouseX, mouseY, toButton))
            {
                selectedToStation = i;
                glutPostRedisplay();
                return;
            }
        }
    }

    if (pointInsideRect(mouseX, mouseY, buyButton))
    {
        if (!metroCardOwned && hasValidRouteSelection())
        {
            metroCardOwned = true;
            metroCardVisible = true;
            selectedFoilOption = -1;
            cardFromStation = selectedFromStation;
            cardToStation = selectedToStation;
            cardJourneyReachedTarget = false;

            for (int i = 0; i < 4; i++)
            {
                stationPassCount[i] = 0;
                cardStationCount[i] = 0;
            }
        }
        glutPostRedisplay();
        return;
    }

    if (pointInsideRect(mouseX, mouseY, sellButton))
    {
        if (metroCardOwned)
        {
            metroCardOwned = false;
            metroCardVisible = false;
            selectedFoilOption = -1;
            selectedFromStation = -1;
            selectedToStation = -1;
            cardFromStation = -1;
            cardToStation = -1;
            cardJourneyReachedTarget = false;
        }
        glutPostRedisplay();
        return;
    }

    if (metroCardOwned && metroCardVisible)
    {
        for (int i = 0; i < 3; i++)
        {
            Rect2D foilButton = getFoilButtonRect(i, windowWidth, windowHeight);
            if (pointInsideRect(mouseX, mouseY, foilButton))
            {
                if (selectedFoilOption == i) selectedFoilOption = -1;
                else selectedFoilOption = i;
                glutPostRedisplay();
                return;
            }
        }
    }
}

// ============================================================
// NORMAL KEY DOWN
// ============================================================

void keyboardDown(unsigned char key, int x, int y)
{
    if (key == 'w' || key == 'W')
    {
        if (waitingAtTerminal)
        {
            if (terminalThrottleArmed)
            {
                waitingAtTerminal = false;
                terminalThrottleArmed = false;
                keyW = true;
                brake = 0.0f;
                manualBrakeApplied = false;
            }
            else
            {
                keyW = false;
            }
        }
        else
        {
            keyW = true;
            brake = 0.0f;
            manualBrakeApplied = false;
        }
    }
    else if (key == 's' || key == 'S')
    {
        keyS = true;
    }
    else if (key == 'b' || key == 'B')
    {
        throttle = 0.0f;
        brake = 1.0f;
        manualBrakeApplied = true;
    }
    else if (key == 'n' || key == 'N')
    {
        brake = 0.0f;
        manualBrakeApplied = false;
    }
    else if (key == 'c' || key == 'C')
    {
        cameraMode++;
        if (cameraMode > 3) cameraMode = 0;
    }
    else if (key == '1')
    {
        if (cameraMode == 3) personCamY += personVerticalSpeed;
        else camLift[cameraMode] += 2.0f;
    }
    else if (key == '2')
    {
        if (cameraMode == 3)
        {
            personCamY -= personVerticalSpeed;
            if (personCamY < 1.0f) personCamY = 1.0f;
        }
        else
        {
            camLift[cameraMode] -= 2.0f;
        }
    }
    else if ((key == 'u' || key == 'U') && cameraMode == 3)
    {
        float yawRadians = personYaw * PI / 180.0f;
        personCamX += sinf(yawRadians) * personMoveSpeed;
        personCamZ += cosf(yawRadians) * personMoveSpeed;
    }
    else if ((key == 'j' || key == 'J') && cameraMode == 3)
    {
        float yawRadians = personYaw * PI / 180.0f;
        personCamX -= sinf(yawRadians) * personMoveSpeed;
        personCamZ -= cosf(yawRadians) * personMoveSpeed;
    }
    else if ((key == 'h' || key == 'H') && cameraMode == 3)
    {
        float yawRadians = personYaw * PI / 180.0f;
        personCamX += cosf(yawRadians) * personMoveSpeed;
        personCamZ += sinf(yawRadians) * personMoveSpeed;
    }
    else if ((key == 'k' || key == 'K') && cameraMode == 3)
    {
        float yawRadians = personYaw * PI / 180.0f;
        personCamX -= cosf(yawRadians) * personMoveSpeed;
        personCamZ -= sinf(yawRadians) * personMoveSpeed;
    }
    else if (key == 'r' || key == 'R')
    {
        resetSimulation();
    }
    else if (key == 27)
    {
        exit(0);
    }
}

// ============================================================
// NORMAL KEY RELEASE
// ============================================================

void keyboardUp(unsigned char key, int x, int y)
{
    if (key == 'w' || key == 'W')
    {
        keyW = false;
        if (waitingAtTerminal)
        {
            terminalThrottleArmed = true;
        }
    }

    if (key == 's' || key == 'S')
    {
        keyS = false;
    }
}

// ============================================================
// ARROW KEYS
// ============================================================

void specialKeys(int key, int x, int y)
{
    float movementSpeed = 2.5f;

    if (cameraMode == 3)
    {
        if (key == GLUT_KEY_UP)
        {
            personPitch += personRotationStep;
            if (personPitch > 89.0f) personPitch = 89.0f;
        }
        else if (key == GLUT_KEY_DOWN)
        {
            personPitch -= personRotationStep;
            if (personPitch < -89.0f) personPitch = -89.0f;
        }
        else if (key == GLUT_KEY_LEFT)
        {
            personYaw += personRotationStep;
            if (personYaw >= 360.0f) personYaw -= 360.0f;
        }
        else if (key == GLUT_KEY_RIGHT)
        {
            personYaw -= personRotationStep;
            if (personYaw < 0.0f) personYaw += 360.0f;
        }
        return;
    }

    if (cameraMode == 2)
    {
        if (key == GLUT_KEY_UP) camPanZ[2] -= movementSpeed;
        if (key == GLUT_KEY_DOWN) camPanZ[2] += movementSpeed;
        if (key == GLUT_KEY_LEFT) camPanX[2] -= movementSpeed;
        if (key == GLUT_KEY_RIGHT) camPanX[2] += movementSpeed;
    }
    else
    {
        Vec2 tangent = sampleTrackTangent(trainDistance);
        tangent.x *= (float)trainDirection;
        tangent.z *= (float)trainDirection;
        Vec2 right = { tangent.z, -tangent.x };

        if (key == GLUT_KEY_UP)
        {
            camPanX[cameraMode] += tangent.x * movementSpeed;
            camPanZ[cameraMode] += tangent.z * movementSpeed;
        }
        if (key == GLUT_KEY_DOWN)
        {
            camPanX[cameraMode] -= tangent.x * movementSpeed;
            camPanZ[cameraMode] -= tangent.z * movementSpeed;
        }
        if (key == GLUT_KEY_LEFT)
        {
            camPanX[cameraMode] -= right.x * movementSpeed;
            camPanZ[cameraMode] -= right.z * movementSpeed;
        }
        if (key == GLUT_KEY_RIGHT)
        {
            camPanX[cameraMode] += right.x * movementSpeed;
            camPanZ[cameraMode] += right.z * movementSpeed;
        }
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

    // Call custom texture generator to build procedural visuals in memory
    generateTextures();

    buildTrack();

    for (int i = 0; i < 4; i++)
    {
        stationTrackDistance[i] = findNearestTrackDistance(stations[i].x, stations[i].z);
    }

    trainDistance = INITIAL_TRAIN_DISTANCE;
    trainDirection = 1;
    waitingAtTerminal = false;
    terminalThrottleArmed = true;
    lastTime = glutGet(GLUT_ELAPSED_TIME);
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
    glutCreateWindow("3D Train Simulator - Visual Upgrade (Textured)");

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