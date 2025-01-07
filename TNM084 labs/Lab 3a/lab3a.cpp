
// Fractal tree generation

// C++ version 2022.
// Adapted to lastest GLUGG.

#define MAIN
#include <vector>
#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "LoadTGA.h"
#include "glugg.h"

// uses framework OpenGL
// uses framework Cocoa

#define kTerrainSize 64
#define kPolySize 0.5

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))



void MakeCylinderAlt(int aSlices, float height, float topwidth, float bottomwidth)
{
	gluggMode(GLUGG_TRIANGLE_STRIP);
	vec3 top = SetVector(0,height,0);
	vec3 center = SetVector(0,0,0);
	vec3 bn = SetVector(0,-1,0); // Bottom normal
	vec3 tn = SetVector(0,1,0); // Top normal

	for (float a = 0.0; a < 2.0*M_PI+0.0001; a += 2.0*M_PI / aSlices)
	{
		float a1 = a;

		vec3 p1 = SetVector(topwidth * cos(a1), height, topwidth * sin(a1));
		vec3 p2 = SetVector(bottomwidth * cos(a1), 0, bottomwidth * sin(a1));
		vec3 pn = SetVector(cos(a1), 0, sin(a1));

// Done making points and normals. Now create polygons!
		gluggNormalv(pn);
	    gluggTexCoord(height, a1/M_PI);
	    gluggVertexv(p2);
	    gluggTexCoord(0, a1/M_PI);
	    gluggVertexv(p1);
	}

	// Then walk around the top and bottom with fans
	gluggMode(GLUGG_TRIANGLE_FAN);
	gluggNormalv(bn);
	gluggVertexv(center);
	// Walk around edge
	for (float a = 0.0; a <= 2.0*M_PI+0.001; a += 2.0*M_PI / aSlices)
	{
		vec3 p = vec3(bottomwidth * cos(a), 0, bottomwidth * sin(a));
	    gluggVertexv(p);
	}
	// Walk around edge
	gluggMode(GLUGG_TRIANGLE_FAN); // Reset to new fan
	gluggNormalv(tn);
	gluggVertexv(top);
	for (float a = 2.0*M_PI; a >= -0.001; a -= 2.0*M_PI / aSlices)
	{
		vec3 p = vec3(topwidth * cos(a), height, topwidth * sin(a));
	    gluggVertexv(p);
	}
}


mat4 projectionMatrix;

Model *floormodel;
GLuint grasstex, barktex;

// Reference to shader programs
GLuint phongShader, texShader;

// Floor quad
// Floor quad
//Original
/*GLfloat vertices2[] = {	-20.5,0.0,-20.5,
						20.5,0.0,-20.5,
						20.5,0.0,20.5,
						-20.5,0.0,20.5};*/

//New
GLfloat vertices2[] = {	-100,0.0,-100,
						100,0.0,-100,
						100,0.0,100,
						-100,0.0,100};
GLfloat normals2[] = {	0,1.0,0,
						0,1.0,0,
						0,1.0,0,
						0,1.0,0};
//Original
/*GLfloat texcoord2[] = {	50.0f, 50.0f,
						0.0f, 50.0f,
						0.0f, 0.0f,
						50.0f, 0.0f};*/
//New
GLfloat texcoord2[] = {	100.0f, 100.0f,
						0.0f, 100.0f,
						0.0f, 0.0f,
						100.0f, 0.0f};
GLuint indices2[] = {	0,3,2, 0,2,1};

//From lab 3b
vec3 vertices[kTerrainSize*kTerrainSize];
vec2 texCoords[kTerrainSize*kTerrainSize];
vec3 normals[kTerrainSize*kTerrainSize];
GLuint indices[(kTerrainSize-1)*(kTerrainSize-1)*3*2];


// THIS IS WHERE YOUR WORK GOES!
void MakeLeaves(float sizeLeaf){
    float randomSize = ((rand() / (float)RAND_MAX))/ 10.0;
    float xOffset = ((rand() / (float)RAND_MAX) - 0.5) * 0.3; // Smaller range
    float yOffset = ((rand() / (float)RAND_MAX) - 0.5) * 0.3;
    float zOffset = ((rand() / (float)RAND_MAX) - 0.5) * 0.3;
    gluggTranslate(xOffset, yOffset, zOffset); // Position leaf

    float rotation = (rand() / (float)RAND_MAX) * 45.0f;  // Rotation between 0 and 45 degrees
    gluggRotate(0, rotation, 0, 1);  // Rotate around Y-axis

    vec3 v1 = SetVector(-sizeLeaf / 2 - randomSize, 0, 0);
    vec3 v2 = SetVector(sizeLeaf / 2 + randomSize, 0, 0);
    vec3 v3 = SetVector(0, sizeLeaf/ 2, 0);

    vec3 normal = cross(v2 - v1, v3 - v1);
    normal = normalize(normal);

    // Draw leaf as a triangle
    gluggMode(GLUGG_TRIANGLES);
    gluggNormalv(normal);
    gluggVertexv(v1);
    gluggVertexv(v2);
    gluggVertexv(v3);
    //gluggBuildBezier(leafControlPoints, leafIndices, 0.1);

}


void Recursion(int depth, float height, float topWidth, float bottomWidth, float angle)
{
    if (depth <= 0){
        // Base case: Generate leaves
        gluggPushMatrix();

        // Move to the tip of the current branch
        gluggTranslate(0, height, 0);

        // Color leaves green
        gluggColor(0.0, 0.8, 0.0);

        // Set leaf properties
        int numLeaves = 10;  // Number of leaves
        float sizeLeaf = 0.5;  // Size of each leaf

        // Place leaves at the tip of the branch
        for (int i = 0; i < numLeaves; ++i)
        {
            gluggPushMatrix();

            MakeLeaves(sizeLeaf);

            gluggPopMatrix();  // Restore leaf transformation
        }

        gluggPopMatrix();

        return; // Basfall, "avsluta/gå tillbaka"
    }
    // Skapa första grenen
    MakeCylinderAlt(20, height, topWidth, bottomWidth);

    // Flytta till toppen av den aktuella grenen
    gluggTranslate(0, height, 0);

    // Vänster gren
    gluggPushMatrix(); // Spara nuvarande transformation (undvika att kommade grenar påverkar denna grens transformationer)
    gluggRotate((rand()%2) - 0.5, (rand()%2) - 0.5, (rand()%2) - 0.5, 1);
    Recursion(depth - 1 - rand()%2, (height * 0.7), topWidth * 0.7, topWidth, angle); // Rekursivt call för nästa djup av grenar
    gluggPopMatrix(); // Återställ transformationen (undvika att nästa gren inte påverkas av denna grens transformationer)

    // Höger gren
    gluggPushMatrix(); // Spara nuvarande transformation
    gluggRotate((rand()%2) - 0.5, (rand()%2) - 0.5, (rand()%2) - 0.5, 1); // Rotera i motsatt riktning
    Recursion(depth - 1 - rand()%2, (height * 0.7), topWidth * 0.7, topWidth, angle); // Rekursivt call för nästa djup av grenar
    gluggPopMatrix(); // Återställ transformationen

    // gren
    /*gluggPushMatrix(); // Spara nuvarande transformation (undvika att kommade grenar påverkar denna grens transformationer)
    gluggRotate((rand()%2) - 0.25, (rand()%2) - 0.25, (rand()%2) - 0.25, 1);
    Recursion(depth - 1 - 10*rand()%2, (height * 0.7), topWidth * 0.7, topWidth, angle); // Rekursivt call för nästa djup av grenar
    gluggPopMatrix(); // Återställ transformationen (undvika att nästa gren inte påverkas av denna grens transformationer)*/
}

gluggModel MakeTree()
{
	gluggSetPositionName("inPosition");
	gluggSetNormalName("inNormal");
	gluggSetTexCoordName("inTexCoord");

	gluggBegin(GLUGG_TRIANGLES);

	// Between gluggBegin and gluggEnd, call MakeCylinderAlt plus glugg transformations
	// to create a tree.

    Recursion(10 , 2.0, 0.15, 0.2, 0.15);

	//MakeCylinderAlt(20, 2, 0.1, 0.15);

	return gluggBuildModel(0);
}


gluggModel MakeBush()
{
	gluggSetPositionName("inPosition");
	gluggSetNormalName("inNormal");
	gluggSetTexCoordName("inTexCoord");

	gluggBegin(GLUGG_TRIANGLES);

	// Between gluggBegin and gluggEnd, call MakeCylinderAlt plus glugg transformations
	// to create a tree.

    Recursion(10 , 2.0/10, 0.15/5, 0.2/5, 0.15);

	//MakeCylinderAlt(20, 2, 0.1, 0.15);

	return gluggBuildModel(0);
}

void generateTrees(std::vector<gluggModel>& tree, std::vector<vec3>& treePos, int amount){
    while(tree.size() < amount){
    gluggModel treeInstance = MakeTree();
    tree.push_back(treeInstance);
    vec2 treecoordinates = vec2(((rand()%32)),((rand()%32)));
    int treevertice = treecoordinates.y*2 * kTerrainSize + treecoordinates.x*2;
    float treeheight = vertices[treevertice].y;
    if(treeheight > 2){treePos.push_back(vec3(treecoordinates.x,treeheight,treecoordinates.y));}

    }

}

void generateBush(std::vector<gluggModel>& Bush, std::vector<vec3>& BushPos, int amount){
    while(Bush.size() < amount){
    gluggModel bushInstance = MakeBush();
    Bush.push_back(bushInstance);
    vec2 bushcoordinates = vec2(((rand()%32)),((rand()%32)));
    int bushvertice = bushcoordinates.y*2 * kTerrainSize + bushcoordinates.x*2;
    float bushheight = vertices[bushvertice].y;
    if(bushheight > 2){BushPos.push_back(vec3(bushcoordinates.x,bushheight,bushcoordinates.y));}

    }

}

void buildTrees(mat4 worldToView, GLuint texShader, std::vector<gluggModel> tree, std::vector<vec3> treePos)
{
    mat4 m;
    for(int i = 0; i < tree.size(); i++){
        m = worldToView * T(treePos[i].x, treePos[i].y, treePos[i].z);
        glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
        gluggDrawModel(tree[i], texShader);
    }
}

void buildBush(mat4 worldToView, GLuint texShader, std::vector<gluggModel> Bush, std::vector<vec3> bushPos)
{
    mat4 m;
    for(int i = 0; i < Bush.size(); i++){
        m = worldToView * T(bushPos[i].x, bushPos[i].y, bushPos[i].z);
        glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
        gluggDrawModel(Bush[i], texShader);
    }
}


std::vector<gluggModel> tree;
std::vector<vec3> treePos;
std::vector<gluggModel> Bush;
std::vector<vec3> bushPos;
//gluggModel tree1;
gluggModel roadModel;
std::vector<vec3> roadVertices;

float fract(float x){
    return x - floor(x);
}

//From Stefan
int hash(int x) {
    return (34 * x * x + 10 * x) % 289;
}

vec2 hash2f(const vec2& p) {

    // Convert the floating-point components to integers (implicitly truncating)
    int ix = (int)p.x;
    int iy = (int)p.y;

    // First hash for x component
    int hx = hash(ix);
    // First hash for y component
    int hy = hash(iy);

    // Now apply double hashing: hash(hash(x) + y)
    float fx = (float)(hash(hx + iy) % 289) / 289.0f;
    float fy = (float)(hash(hy + ix) % 289) / 289.0f;

    // Return the final hashed result as a 2D vector
    return vec2(fx, fy);
}

vec2 VectorAdd(vec2 a,vec2 b){
    vec2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

vec2 VectorSub(vec2 a,vec2 b){
    vec2 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

float length(vec2 a){
 float result;
 result = a.x*a.x + a.y*a.y;
 return result;
}

//Taken from https://iquilezles.org/articles/smoothvoronoi/
float smoothVoronoi(vec2 x)
{
    vec2 p = vec2(floor(x.x), floor(x.y));
    vec2 f = VectorSub(x,p);

    float res = 0.0;
    for(int j=-1; j<=1; j++)
    for(int i=-1; i<=1; i++)
    {
        vec2 b = vec2(i, j);
        vec2  r = VectorAdd(VectorSub(b,f), hash2f(VectorAdd(p,b)));
        float d = length(r);

        res += exp2(-32.0*d);
    }
    return -(1.0/32.0)*log2(res);
}

void GenerateRoadPath(std::vector<vec3>& roadVertices, int length, float width) {
    for (int i = 0; i < kTerrainSize; i++) {
        float x = i * kPolySize;  // Increment along the x-axis
        float z = sin(i * 0.05) * 10.0;  // Sine wave for the road path
        roadVertices.push_back(vec3(x, 0.5, z - width * 0.5)); // Left side
        roadVertices.push_back(vec3(x, 0.5, z + width * 0.5)); // Right side
    }
}

void FlattenTerrainForRoad(const std::vector<vec3>& roadVertices) {
    for (auto& roadVertex : roadVertices) {
        for (int x = 0; x < kTerrainSize; x++) {
            for (int z = 0; z < kTerrainSize; z++) {
                int ix = z * kTerrainSize + x;
                float difference = Norm(VectorSub(vertices[ix].y,roadVertex.y));
                if (difference < kPolySize) {
                    //vertices[ix].y = -difference; // Flatten terrain to match road height
                    vertices[ix].y = roadVertex.y;
                }
            }
        }
    }
}

gluggModel MakeRoad(const std::vector<vec3>& roadVertices) {
    gluggBegin(GLUGG_TRIANGLE_STRIP);

    for (size_t i = 0; i < roadVertices.size(); i += 2) {
        vec3 left = roadVertices[i];
        vec3 right = roadVertices[i + 1];
        vec3 normal = vec3(0, 1, 0);

        gluggNormalv(normal);
        gluggVertexv(left);
        gluggVertexv(right);
    }

    return gluggBuildModel(0);
}

void buildRoad(gluggModel roadModel, mat4 worldToView) {
    mat4 m = worldToView;
    glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
    gluggDrawModel(roadModel, texShader);
}

float fbm(vec2 pos, float scale){
    float t = 0.0;
    float G = exp(-scale);
    float f = 1.0;
    float a = 1.0;

    int numOctaves = 4;
    for(int i = 0; i < numOctaves; i++){
        t += a * smoothVoronoi(vec2(pos.x * f, pos.y * f));
        f *= 2.0;
        a *= G;
    }
    return t;
}

//Taken from lab 3b
void MakeTerrain()
{
	// TO DO: This is where your terrain generation goes if on CPU.
	for (int x = 0; x < kTerrainSize; x++)
	for (int z = 0; z < kTerrainSize; z++)
	{
		int ix = z * kTerrainSize + x;

		#define bumpHeight 0.5
		#define bumpWidth 2.0

		vec2 vecSmooth = vec2(x,z);

		//// squared distance to center
		//float h = ( (x - kTerrainSize/2)/bumpWidth * (x - kTerrainSize/2)/bumpWidth +  (z - kTerrainSize/2)/bumpWidth * (z - kTerrainSize/2)/bumpWidth );
		//float y = MAX(0, 3-h) * bumpHeight + smoothVoronoi(vecSmooth) * 0.1;

        float scale = 0.05;
        vec2 vecScale = vec2(vecSmooth.x * scale,vecSmooth.y * scale);

        float y = fbm(vecScale, 1) * 3;

		vertices[ix] = vec3(x * kPolySize, y * 3, z * kPolySize);
		texCoords[ix] = vec2(x, z);
		normals[ix] = vec3(0,1,0);
	}

	// Make indices
	// You don't need to change this.
	for (int x = 0; x < kTerrainSize-1; x++)
	for (int z = 0; z < kTerrainSize-1; z++)
	{
		// Quad count
		int q = (z*(kTerrainSize-1)) + (x);
		// Polyon count = q * 2
		// Indices
		indices[q*2*3] = x + z * kTerrainSize; // top left
		indices[q*2*3+1] = x+1 + z * kTerrainSize;
		indices[q*2*3+2] = x + (z+1) * kTerrainSize;
		indices[q*2*3+3] = x+1 + z * kTerrainSize;
		indices[q*2*3+4] = x+1 + (z+1) * kTerrainSize;
		indices[q*2*3+5] = x + (z+1) * kTerrainSize;
	}

	// Make normal vectors
	// TO DO: This is where you calculate normal vectors
	for (int x = 0; x < kTerrainSize; x++)
	for (int z = 0; z < kTerrainSize; z++)
	{
	    vec3 v1 = vertices[(z) * kTerrainSize + MAX(x-1,0)];
	    vec3 v2 = vertices[(z) * kTerrainSize + MIN(x+1,63)];
	    vec3 v3 = vertices[MAX(z-1,0) * kTerrainSize + (x)];
	    vec3 v4 = vertices[MIN(z+1,63) * kTerrainSize + (x)];
	    vec3 Vector1 = v1 - v2;
	    vec3 Vector2 = v3 - v4;
	    vec3 normal = cross(Vector1,Vector2);

		normals[z * kTerrainSize + x] = normal;

	}

	GenerateRoadPath(roadVertices, 50, 2.0);
    FlattenTerrainForRoad(roadVertices);
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);

	// Set the clipping volume
	projectionMatrix = perspective(45,1.0f*w/h,1,1000);
	glUseProgram(phongShader);
	glUniformMatrix4fv(glGetUniformLocation(phongShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUseProgram(texShader);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
}


void init(void)
{
	// GL inits
	glClearColor(0.4,0.6,0.8,0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	printError("GL inits");

	projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 300.0);

	// Load and compile shader
	phongShader = loadShaders("phong.vert", "phong.frag");
	texShader = loadShaders("textured.vert", "textured.frag");
	printError("init shader");

	// Upload geometry to the GPU:
	//OLD
	//floormodel = LoadDataToModel((vec3 *)vertices2, (vec3 *)normals2, (vec2 *)texcoord2, NULL,
			//indices2, 4, 6);

    //FROM LABB 3B
    MakeTerrain();
    floormodel = LoadDataToModel(vertices, normals, texCoords, NULL,
			indices, kTerrainSize*kTerrainSize, (kTerrainSize-1)*(kTerrainSize-1)*2*3);
    roadModel = MakeRoad(roadVertices);

// Important! The shader we upload to must be active!
	glUseProgram(phongShader);
	glUniformMatrix4fv(glGetUniformLocation(phongShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUseProgram(texShader);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);

	glUniform1i(glGetUniformLocation(texShader, "tex"), 0); // Texture unit 0

	LoadTGATextureSimple("grass.tga", &grasstex);
	glBindTexture(GL_TEXTURE_2D, grasstex);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	LoadTGATextureSimple("bark2.tga", &barktex);

    /*for(int i = 0; i < 50; i++){
        tree.push_back(MakeTree());
        treePos.push_back(vec3((rand()%40) - 20,0, (rand()%40) - 20 ));
    }*/
    generateTrees(tree, treePos, 50);
    generateBush(Bush, bushPos, 50);

	//tree = MakeTree();
	//tree1 = MakeTree();

	printError("init arrays");
}

GLfloat a = 0.0;
//vec3 campos = vec3(0, 1.5, 10);
//FROM LAB 3B
vec3 campos = vec3(kTerrainSize*kPolySize/4, 1.5, kTerrainSize*kPolySize/4);
vec3 forward = vec3(0, 0, -4);
vec3 up = vec3(0, 1, 0);

/*mat4 buildTree(vec3 position, mat4& worldToView, mat4& m , int treenum){

    m = worldToView * T(position.x,position.y,position.z);
    glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	gluggDrawModel(tree[treenum], texShader);
	return m;
}*/


void display(void)
{
	printError("pre display");

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 worldToView, m; // m1, m2, m, tr, scale;

	if (glutKeyIsDown('a'))
		forward = mat3(Ry(0.05))* forward;
	if (glutKeyIsDown('d'))
		forward = mat3(Ry(-0.05)) * forward;
	if (glutKeyIsDown('w'))
		campos = campos + forward * 0.1;
	if (glutKeyIsDown('s'))
		campos = campos - forward * 0.1;
	if (glutKeyIsDown('q'))
	{
		vec3 side = cross(forward, vec3(0,1,0));
		campos = campos - side * 0.05;
	}
	if (glutKeyIsDown('e'))
	{
		vec3 side = cross(forward, vec3(0,1,0));
		campos = campos + side * 0.05;
	}

	// Move up/down
	if (glutKeyIsDown('z'))
		campos = campos + vec3(0,1,0) * 0.01;
	if (glutKeyIsDown('c'))
		campos = campos - vec3(0,1,0) * 0.01;

	// NOTE: Looking up and down is done by making a side vector and rotation around arbitrary axis!
	if (glutKeyIsDown('+'))
	{
		vec3 side = cross(forward, vec3(0,1,0));
		mat4 m = ArbRotate(side, 0.05);
		forward = mat3(m) * forward;
	}
	if (glutKeyIsDown('-'))
	{
		vec3 side = cross(forward, vec3(0,1,0));
		mat4 m = ArbRotate(side, -0.05);
		forward = m * forward;
	}

	worldToView = lookAtv(campos, campos + forward, up);

	a += 0.1;

	glBindTexture(GL_TEXTURE_2D, grasstex);
	// Floor
	glUseProgram(texShader);
	m = worldToView;
	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	DrawModel(floormodel, texShader, "inPosition", "inNormal", "inTexCoord");

	// Draw the tree, as defined on MakeTree
	glBindTexture(GL_TEXTURE_2D, barktex);
	glUseProgram(texShader);

    /*
    m = worldToView * T(0, 0, 0);
    glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
    gluggDrawModel(tree[0], texShader);


    m = worldToView * T(1, 0, 0);
    glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	gluggDrawModel(tree1, texShader);
    */

    /*for(int i = 0; i < tree.size(); i++){
        m = worldToView * T(treePos[i].x, treePos[i].y, treePos[i].z);
        glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
        gluggDrawModel(tree[i], texShader);
    }*/

    buildTrees(worldToView, texShader, tree, treePos);
    buildBush(worldToView, texShader, Bush, bushPos);

    buildRoad(roadModel, worldToView);

    /*
	m = buildTree((0,4,0),worldToView,m,0);
	m = buildTree((0,4,0),worldToView,m,1);
    */

	printError("display");

	glutSwapBuffers();
}

void keys(unsigned char key, int x, int y)
{
	switch (key)
	{
		case ' ':
			forward.y = 0;
			forward = normalize(forward) * 4.0;
			break;
	}
}


int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(640,360);
	glutCreateWindow ("Fractal tree lab");
	glutRepeatingTimer(20);
	glutDisplayFunc(display);
	glutKeyboardFunc(keys);
	glutReshapeFunc(reshape);
	init ();
	glutMainLoop();
}
