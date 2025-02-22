
// Fractal tree generation

// C++ version 2022.
// Adapted to lastest GLUGG.

#define MAIN
#include <iostream>
#include <vector>
#include <cstdlib>
#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "LoadTGA.h"
#include "glugg.h"


// uses framework OpenGL
// uses framework Cocoa

#define kTerrainSize 129
#define kPolySize 1.0

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define waterHeight -2.0

GLint TessLevelInner = 10;
GLint TessLevelOuter1 = 10;
GLint TessLevelOuter2 = 10;
GLint TessLevelOuter3 = 10;

std::vector<int> stoneTessInner;
std::vector<int> stoneTessOuter1;
std::vector<int> stoneTessOuter2;
std::vector<int> stoneTessOuter3;

std::vector<int> cloudTessInner;
std::vector<int> cloudTessOuter1;
std::vector<int> cloudTessOuter2;
std::vector<int> cloudTessOuter3;


// ------------ DrawPatchModel: modified utility function DrawModel from LittleOBJLoader ---------------
static void ReportRerror(const char *caller, const char *name)
{
	static unsigned int draw_error_counter = 0;
   if(draw_error_counter < NUM_DRAWMODEL_ERROR)
   {
		    fprintf(stderr, "%s warning: '%s' not found in shader!\n", caller, name);
		    draw_error_counter++;
   }
   else if(draw_error_counter == NUM_DRAWMODEL_ERROR)
   {
		    fprintf(stderr, "%s: Number of error bigger than %i. No more vill be printed.\n", caller, NUM_DRAWMODEL_ERROR);
		    draw_error_counter++;
   }
}

// Same as DrawModel but with GL_PATCHES
void DrawPatchModel(Model *m, GLuint program, const char* vertexVariableName, const char* normalVariableName, const char* texCoordVariableName)
{
    #ifndef GL_PATCHES
        #define GL_PATCHES 0x0000000e
    #endif

	if (m != NULL)
	{
		GLint loc;

		glPatchParameteri(GL_PATCH_VERTICES, 3);
		glBindVertexArray(m->vao);	// Select VAO

		glBindBuffer(GL_ARRAY_BUFFER, m->vb);
		loc = glGetAttribLocation(program, vertexVariableName);
		if (loc >= 0)
		{
			glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(loc);
		}
		else
			ReportRerror("DrawModel", vertexVariableName);

		if (normalVariableName!=NULL)
		{
			loc = glGetAttribLocation(program, normalVariableName);
			if (loc >= 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m->nb);
				glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(loc);
			}
			else
				ReportRerror("DrawModel", normalVariableName);
		}

		// VBO for texture coordinate data NEW for 5b
		if ((m->texCoordArray != NULL)&&(texCoordVariableName != NULL))
		{
			loc = glGetAttribLocation(program, texCoordVariableName);
			if (loc >= 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m->tb);
				glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(loc);
			}
			else
				ReportRerror("DrawModel", texCoordVariableName);
		}

		glDrawElements(GL_PATCHES, m->numIndices, GL_UNSIGNED_INT, 0L);
	}
}



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
mat4 worldToView;
mat4 modelToWorldMatrix;

Model *floormodel, *watermodel;
GLuint grasstex, barktex, leaftex,watertex, stonetex, bark4tex, bjorktex;

// Reference to shader programs
GLuint phongShader, texShader, stoneShader;

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

vec3 verticeswater[kTerrainSize*kTerrainSize];
vec2 texCoordswater[kTerrainSize*kTerrainSize];
vec3 normalswater[kTerrainSize*kTerrainSize];
GLuint indiceswater[(kTerrainSize-1)*(kTerrainSize-1)*3*2];


// THIS IS WHERE YOUR WORK GOES!

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

void MakeLeaves(float sizeLeaf){
    float randomSize = ((rand() / (float)RAND_MAX)) * 0.2;
    float xOffset = ((rand() / (float)RAND_MAX) - 0.5) * 0.3; // Smaller range
    float yOffset = ((rand() / (float)RAND_MAX) - 0.5) * 0.3;
    float zOffset = ((rand() / (float)RAND_MAX) - 0.5) * 0.3;
    gluggTranslate(xOffset, yOffset, zOffset); // Position leaf

    float rotation = (rand() / (float)RAND_MAX) * 360.0f;  // Rotation between 0 and 360 degrees
    gluggRotate(0, rotation, 0, 1);  // Rotate around Y-axis

    vec3 v1 = SetVector(-sizeLeaf / 2 - randomSize, 0, 0);
    vec3 v2 = SetVector(sizeLeaf / 2 + randomSize, 0, 0);
    vec3 v3 = SetVector(0, sizeLeaf / 2 + randomSize, 0);

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


void Recursion(int depth, float height, float topWidth, float bottomWidth, float angle, int treeType)
{
    if (depth <= 0){
        // Base case: Generate leaves
        gluggPushMatrix();
        MakeCylinderAlt(20, height, topWidth, bottomWidth);
        // Move to the tip of the current branch
        gluggTranslate(0, height, 0);

        // Color leaves green
        gluggColor(0.0, 255.0, 0.0);

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

    if(treeType == 1){
        // Skapa första grenen
    MakeCylinderAlt(20, height, topWidth, bottomWidth);

    // Flytta till toppen av den aktuella grenen
    gluggTranslate(0, height, 0);

    // Vänster gren
    gluggPushMatrix(); // Spara nuvarande transformation (undvika att kommade grenar påverkar denna grens transformationer)
    gluggRotate((rand()%2) - 0.5, (rand()%2) - 0.5, (rand()%2) - 0.5, 1);
    Recursion(depth - 1 - rand()%2, (height * 0.7), topWidth * 0.7, topWidth, angle,1); // Rekursivt call för nästa djup av grenar
    gluggPopMatrix(); // Återställ transformationen (undvika att nästa gren inte påverkas av denna grens transformationer)

    // Höger gren
    gluggPushMatrix(); // Spara nuvarande transformation
    gluggRotate((rand()%2) - 0.5, (rand()%2) - 0.5, (rand()%2) - 0.5, 1); // Rotera i motsatt riktning
    Recursion(depth - 1 - rand()%2, (height * 0.7), topWidth * 0.7, topWidth, angle,1); // Rekursivt call för nästa djup av grenar
    gluggPopMatrix(); // Återställ transformationen
    }

    else if(treeType == 2){
            // Skapa första grenen
    MakeCylinderAlt(20, height, topWidth, bottomWidth);

    // Flytta till toppen av den aktuella grenen
    gluggTranslate(0, height, 0);

    // Vänster gren
    gluggPushMatrix(); // Spara nuvarande transformation (undvika att kommade grenar påverkar denna grens transformationer)
    gluggRotate((rand()%2) - 0.5, (rand()%2) - 0.5, (rand()%2) - 0.5, 1);
    Recursion(depth - 1 - rand()%2, (height * 0.7) * ((rand()%100)/100.0), topWidth * 0.7, topWidth, angle,1); // Rekursivt call för nästa djup av grenar
    gluggPopMatrix(); // Återställ transformationen (undvika att nästa gren inte påverkas av denna grens transformationer)

    // Höger gren
    gluggPushMatrix(); // Spara nuvarande transformation
    gluggRotate((rand()%2) - 0.5, (rand()%2) - 0.5, (rand()%2) - 0.5, 1); // Rotera i motsatt riktning
    Recursion(depth - 1 - rand()%2, (height * 0.7) * ((rand()%100)/100.0), topWidth * 0.7, topWidth, angle,1); // Rekursivt call för nästa djup av grenar
    gluggPopMatrix(); // Återställ transformationen

    }
    else {
           // Skapa första grenen
    MakeCylinderAlt(20, height, topWidth, bottomWidth);

    // Flytta till toppen av den aktuella grenen
    gluggTranslate(0, height, 0);

    // Vänster gren
    gluggPushMatrix(); // Spara nuvarande transformation (undvika att kommade grenar påverkar denna grens transformationer)
    gluggRotate((rand()%2) - 0.5, (rand()%2) - 0.5, (rand()%2) - 0.5, 1);
    Recursion(depth - 2 - rand()%2, (height * 0.7), topWidth * 0.7, topWidth, angle,3); // Rekursivt call för nästa djup av grenar
    gluggPopMatrix(); // Återställ transformationen (undvika att nästa gren inte påverkas av denna grens transformationer)

    // Höger gren
    gluggPushMatrix(); // Spara nuvarande transformation
    gluggRotate((rand()%2) - 0.5, (rand()%2) - 0.5, (rand()%2) - 0.5, 1); // Rotera i motsatt riktning
    Recursion(depth - 2 - rand()%2, (height * 0.7), topWidth * 0.7, topWidth, angle,3); // Rekursivt call för nästa djup av grenar
    gluggPopMatrix(); // Återställ transformationen

        // Vänster gren
    gluggPushMatrix(); // Spara nuvarande transformation (undvika att kommade grenar påverkar denna grens transformationer)
    gluggRotate((rand()%2) - 0.5, (rand()%2) - 0.5, (rand()%2) - 0.5, 1);
    Recursion(depth - 2 - rand()%2, (height * 0.7), topWidth * 0.7, topWidth, angle,3); // Rekursivt call för nästa djup av grenar
    gluggPopMatrix(); // Återställ transformationen (undvika att nästa gren inte påverkas av denna grens transformationer)

    // Höger gren
    gluggPushMatrix(); // Spara nuvarande transformation
    gluggRotate((rand()%2) - 0.5, (rand()%2) - 0.5, (rand()%2) - 0.5, 1); // Rotera i motsatt riktning
    Recursion(depth - 2 - rand()%2, (height * 0.7), topWidth * 0.7, topWidth, angle,3); // Rekursivt call för nästa djup av grenar
    gluggPopMatrix(); // Återställ transformationen

    }


    // gren
    /*gluggPushMatrix(); // Spara nuvarande transformation (undvika att kommade grenar påverkar denna grens transformationer)
    gluggRotate((rand()%2) - 0.25, (rand()%2) - 0.25, (rand()%2) - 0.25, 1);
    Recursion(depth - 1 - 10*rand()%2, (height * 0.7), topWidth * 0.7, topWidth, angle); // Rekursivt call för nästa djup av grenar
    gluggPopMatrix(); // Återställ transformationen (undvika att nästa gren inte påverkas av denna grens transformationer)*/

    /*if (depth <= 0){
        // Base case: Generate leaves
        gluggPushMatrix();

        // Move to the tip of the current branch
        gluggTranslate(0, height, 0);

        // Color leaves green
        gluggColor(0.0, 255.0, 0.0);

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

        return; // Basfall, "avsluta/g� tillbaka"
    }
    // Skapa f�rsta grenen
    MakeCylinderAlt(20, height, topWidth, bottomWidth);

    // Flytta till toppen av den aktuella grenen
    gluggTranslate(0, height, 0);

    // V�nster gren
    gluggPushMatrix(); // Spara nuvarande transformation (undvika att kommade grenar p�verkar denna grens transformationer)
    gluggRotate((rand()%2) - 0.5, (rand()%2) - 0.5, (rand()%2) - 0.5, 1);
    Recursion(depth - 1 - rand()%2, (height * 0.7), topWidth * 0.7, topWidth, angle); // Rekursivt call f�r n�sta djup av grenar
    gluggPopMatrix(); // �terst�ll transformationen (undvika att n�sta gren inte p�verkas av denna grens transformationer)

    // H�ger gren
    gluggPushMatrix(); // Spara nuvarande transformation
    gluggRotate((rand()%2) - 0.5, (rand()%2) - 0.5, (rand()%2) - 0.5, 1); // Rotera i motsatt riktning
    Recursion(depth - 1 - rand()%2, (height * 0.7), topWidth * 0.7, topWidth, angle); // Rekursivt call f�r n�sta djup av grenar
    gluggPopMatrix(); // �terst�ll transformationen

    // gren
    /*gluggPushMatrix(); // Spara nuvarande transformation (undvika att kommade grenar p�verkar denna grens transformationer)
    gluggRotate((rand()%2) - 0.25, (rand()%2) - 0.25, (rand()%2) - 0.25, 1);
    Recursion(depth - 1 - 10*rand()%2, (height * 0.7), topWidth * 0.7, topWidth, angle); // Rekursivt call f�r n�sta djup av grenar
    gluggPopMatrix(); // �terst�ll transformationen (undvika att n�sta gren inte p�verkas av denna grens transformationer)*/

    // Generate child branches with small rotational differences
    /*int branchCount = 2 + rand() % 2; // 2 to 3 branches per level
    float baseAngleX = 10.0f; // Base upward tilt
    float baseAngleY = angle; // Maintain current branch's Y-axis direction

    for (int i = 0; i < branchCount; ++i) {
        gluggPushMatrix();

        // Small deviations in branch rotations
        float rotationX = baseAngleX + ((rand() % 20 - 10) / 10.0f); // Small tilt upwards or downwards
        float rotationY = baseAngleY + ((rand() % 20 - 10) / 10.0f); // Small horizontal deviation
        float rotationZ = ((rand() % 10 - 5) / 10.0f); // Minimal twist

        gluggRotate(rotationX, rotationY, rotationZ, 1);

        // Reduce size for child branches
        float newHeight = height * 0.7f;
        float newTopWidth = topWidth * 0.7f;
        float newBottomWidth = bottomWidth * 0.7f;

        // Recursive call for the next level of branches
        Recursion(depth - 1, newHeight, newTopWidth, newBottomWidth, angle);

        gluggPopMatrix();
    }*/


}

gluggModel MakeTree(std::vector<int>& treeType)
{
	gluggSetPositionName("inPosition");
	gluggSetNormalName("inNormal");
	gluggSetTexCoordName("inTexCoord");

	gluggBegin(GLUGG_TRIANGLES);

	// Between gluggBegin and gluggEnd, call MakeCylinderAlt plus glugg transformations
	// to create a tree.
    int typeTree = rand() % 3;

    Recursion(10, 2.0, 0.15, 0.2, 0.15, typeTree);
    treeType.push_back(typeTree);

	return gluggBuildModel(0);
}


gluggModel MakeBush(std::vector<int>& bushType)
{
	gluggSetPositionName("inPosition");
	gluggSetNormalName("inNormal");
	gluggSetTexCoordName("inTexCoord");

	gluggBegin(GLUGG_TRIANGLES);

	// Between gluggBegin and gluggEnd, call MakeCylinderAlt plus glugg transformations
	// to create a tree.
    int typeBush = rand() % 3;
    Recursion(10 , 2.0/10 + 1.0/10 * (rand() % 3) , 0.15/5, 0.2/5 + 0.1/5 * (rand() % 3), 0.15, typeBush);
    bushType.push_back(typeBush);

	return gluggBuildModel(0);
}

void generateTrees(std::vector<gluggModel>& tree, std::vector<vec3>& treePos, std::vector<int>& treeType, int amount){
    int attempts = 0;
    int maxAttempts = amount * 100;
    while(tree.size() < amount && attempts < maxAttempts){

        attempts++;

        int x = rand() % kTerrainSize;
        int z = rand() % kTerrainSize;

        vec2 treeCoordinates = vec2(x, z);
        int treeVertice = treeCoordinates.y * kTerrainSize + treeCoordinates.x;
        if (treeVertice < 0 || treeVertice >= kTerrainSize * kTerrainSize){
            continue;
        }
        float treeHeight = vertices[treeVertice].y;
        if(treeHeight < waterHeight){
            continue;
        }

        bool collision = false;
        vec3 currentTreePos = vec3(treeCoordinates.x,treeHeight,treeCoordinates.y);
        for(const vec3 treePosition : treePos){
            if(length(VectorSub(vec2(currentTreePos.x, currentTreePos.z),vec2(treePosition.x, treePosition.z))) < 2.0f){
                collision = true;
                break;
            }
        }

        if (!collision) {
            tree.push_back(MakeTree(treeType));
            treePos.push_back(currentTreePos);
        }
    }

    std::cout << "Amount of trees: " << tree.size();
}

void generateBush(std::vector<gluggModel>& bush, std::vector<vec3>& bushPos, const std::vector<vec3> treePos, std::vector<int>& bushType, int amount){
    int attempts = 0;
    int maxAttempts = amount * 100;
    while(bush.size() < amount && attempts < maxAttempts){

        attempts++;

        int x = rand() % kTerrainSize;
        int z = rand() % kTerrainSize;

        vec2 coord = vec2(x, z);
        int ix = z * kTerrainSize + x;
        if (ix < 0 || ix >= kTerrainSize * kTerrainSize){
            continue;
        }

        float bushHeight = vertices[ix].y;
        if(bushHeight < waterHeight){
            continue;
        }

        bool collision = false;
        vec3 currentPos = vec3(x,bushHeight,z);
        for(const vec3 treePosition : treePos){
            if(length(VectorSub(vec2(currentPos.x, currentPos.z),vec2(treePosition.x, treePosition.z))) < 2.0f){
                collision = true;
                break;
            }
        }

        if(!collision){
            for(const vec3 bushPosition : bushPos){
                if(length(VectorSub(vec2(currentPos.x, currentPos.z),vec2(bushPosition.x, bushPosition.z))) < 2.0f){
                    collision = true;
                    break;
                }
            }
        }

        if (!collision) {
            bush.push_back(MakeBush(bushType));
            bushPos.push_back(currentPos);
        }
    }

    std::cout << "Amount of bushes: " << bush.size();
}

void buildTrees(mat4 worldToView, GLuint texShader, std::vector<gluggModel> tree, std::vector<vec3> treePos, std::vector<int>& treeType)
{
    mat4 m;
    for(int i = 0; i < tree.size(); i++){
        m = worldToView * T(treePos[i].x, treePos[i].y, treePos[i].z);
        glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
         if(treeType[i] == 0){
            glBindTexture(GL_TEXTURE_2D, barktex);
        }
        else if(treeType[i] == 1){
            glBindTexture(GL_TEXTURE_2D, bark4tex);
        }
        else{
            glBindTexture(GL_TEXTURE_2D, bjorktex);
        }
        gluggDrawModel(tree[i], texShader);
    }
}

void buildBush(mat4 worldToView, GLuint texShader, std::vector<gluggModel> bush, std::vector<vec3> bushPos, std::vector<int>& bushType)
{
    mat4 m;
    for(int i = 0; i < bush.size(); i++){
        m = worldToView * T(bushPos[i].x, bushPos[i].y, bushPos[i].z);
        glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);

        if(bushType[i] == 0){
            glBindTexture(GL_TEXTURE_2D, barktex);
        }
        else if(bushType[i] == 1){
            glBindTexture(GL_TEXTURE_2D, bark4tex);
        }
        else{
            glBindTexture(GL_TEXTURE_2D, bjorktex);
        }
        gluggDrawModel(bush[i], texShader);
    }
}

Model *MakeStone() {
    Model *stone = LoadModelPlus("cube.obj");
    return stone;
}

void generateStones(std::vector<Model*>& stone, std::vector<vec3>& stonePos, std::vector<vec3>& stoneSize, std::vector<vec3> treePos, std::vector<vec3> bushPos, std::vector<float>& randStone, int amount) {
    int attempts = 0;
    int maxAttempts = amount * 100;
    while(stone.size() < amount && attempts < maxAttempts){

        attempts++;

        int x = rand() % kTerrainSize;
        int z = rand() % kTerrainSize;
        float randomSizeX = ((rand() % 5) * 0.5 + 1)*0.6;
        float randomSizeY = ((rand() % 3) * 0.5 + 1)*0.4;
        float randomSizeZ = ((rand() % 5) * 0.5 + 1)*0.6;

        vec2 coord = vec2(x, z);
        int ix = z * kTerrainSize + x;
        if (ix < 0 || ix >= kTerrainSize * kTerrainSize){
            continue;
        }

        float stoneHeight = vertices[ix].y;
        if(stoneHeight < waterHeight){
            continue;
        }

        bool collision = false;
        vec3 currentPos = vec3(x,stoneHeight,z);
        for(const vec3 treePosition : treePos){
            if(length(VectorSub(vec2(currentPos.x, currentPos.z),vec2(treePosition.x, treePosition.z))) < 3.0f){
                collision = true;
                break;
            }
        }

        if(!collision){
            for(const vec3 bushPosition : bushPos){
                if(length(VectorSub(vec2(currentPos.x, currentPos.z),vec2(bushPosition.x, bushPosition.z))) < 3.0f){
                    collision = true;
                    break;
                }
            }
        }
        if(!collision){
            for(const vec3 stonePosition : stonePos){
                if(length(VectorSub(vec2(currentPos.x, currentPos.z),vec2(stonePosition.x, stonePosition.z))) < 1.0f){
                    collision = true;
                    break;
                }
            }
        }

        if (!collision) {
                int randomTess = rand() % 10;
                float randomShader = rand() / RAND_MAX;
                stone.push_back(MakeStone());
                stonePos.push_back(currentPos);
                stoneSize.push_back(vec3(randomSizeX, randomSizeY, randomSizeZ));
                stoneTessInner.push_back(10 + randomTess);
                stoneTessOuter1.push_back(10 + randomTess);
                stoneTessOuter2.push_back(10 + randomTess);
                stoneTessOuter3.push_back(10 + randomTess);
                randStone.push_back(randomShader);
        }
    }
    std::cout << "Amount of stones: " << stone.size();
}

void buildStone(mat4 worldToViewMatrix, std::vector<Model*>& stone, std::vector<vec3>& stonePos, std::vector<vec3>& stoneSize, std::vector<float>& randStone, GLuint shader) {
    mat4 m;
    for (size_t i = 0; i < stone.size(); ++i) {
        int random = rand() % 5;
        glUniform1i(glGetUniformLocation(stoneShader, "TessLevelInner"), stoneTessInner[i]);
        glUniform1i(glGetUniformLocation(stoneShader, "TessLevelOuter1"), stoneTessInner[i]);
        glUniform1i(glGetUniformLocation(stoneShader, "TessLevelOuter2"), stoneTessInner[i]);
        glUniform1i(glGetUniformLocation(stoneShader, "TessLevelOuter3"), stoneTessInner[i]);
        glUniform1i(glGetUniformLocation(stoneShader, "SphereMaterial"), 0);
        glUniform1f(glGetUniformLocation(stoneShader, "stoneID"), randStone[i]);

        m = worldToViewMatrix * T(stonePos[i].x, stonePos[i].y, stonePos[i].z) * S(stoneSize[i].x, stoneSize[i].y, stoneSize[i].z);

        glUniformMatrix4fv(glGetUniformLocation(shader, "camMatrix"), 1, GL_TRUE, m.m);


        glBindTexture(GL_TEXTURE_2D, stonetex);

         DrawPatchModel(stone[i], shader, "in_Position", "in_Normal", "in_TexCoord");

    }
}

void generateClouds(std::vector<Model*>& cloud, std::vector<vec3>& cloudPos, std::vector<vec3>& cloudSize,std::vector<float>& randCloud, int amount) {
    int attempts = 0;
    int maxAttempts = amount * 100;
    while(cloud.size() < amount && attempts < maxAttempts){

        attempts++;

        int x = rand() % kTerrainSize;
        int z = rand() % kTerrainSize;
        float randomSizeX = ((rand() % 5) * 0.5 + 4);
        float randomSizeY = ((rand() % 2) * 0.5 + 3);
        float randomSizeZ = ((rand() % 7) * 0.5 + 4);

        vec2 coord = vec2(x, z);
        int ix = z * kTerrainSize + x;
        if (ix < 0 || ix >= kTerrainSize * kTerrainSize){
            continue;
        }

        float cloudHeight = 15 + 2 * rand() % 5;

        bool collision = false;
        vec3 currentPos = vec3(x,cloudHeight,z);
        for(const vec3 cloudPosition : cloudPos){
            if(length(VectorSub(vec2(currentPos.x, currentPos.z),vec2(cloudPosition.x, cloudPosition.z))) < 3.0f){
                collision = true;
                break;
            }
        }

        if (!collision) {
                int randomTess = rand() % 10;
                float randomShader = rand() / RAND_MAX;
                cloud.push_back(MakeStone());
                cloudPos.push_back(currentPos);
                cloudSize.push_back(vec3(randomSizeX, randomSizeY, randomSizeZ));
                cloudTessInner.push_back(10 + randomTess);
                cloudTessOuter1.push_back(10 + randomTess);
                cloudTessOuter2.push_back(10 + randomTess);
                cloudTessOuter3.push_back(10 + randomTess);
                randCloud.push_back(randomShader);
        }
    }
    std::cout << "Amount of clouds: " << cloud.size();
}

void buildCloud(mat4 worldToViewMatrix, std::vector<Model*>& cloud, std::vector<vec3>& cloudPos, std::vector<vec3>& cloudSize, std::vector<float>& randCloud, GLuint shader) {
    mat4 m;
    for (size_t i = 0; i < cloud.size(); ++i) {

        glUniform1i(glGetUniformLocation(stoneShader, "TessLevelInner"), cloudTessInner[i]);
        glUniform1i(glGetUniformLocation(stoneShader, "TessLevelOuter1"), cloudTessInner[i]);
        glUniform1i(glGetUniformLocation(stoneShader, "TessLevelOuter2"), cloudTessInner[i]);
        glUniform1i(glGetUniformLocation(stoneShader, "TessLevelOuter3"), cloudTessInner[i]);
        glUniform1i(glGetUniformLocation(stoneShader, "SphereMaterial"), 1);
        glUniform1f(glGetUniformLocation(stoneShader, "cloudID"), randCloud[i]);

        //m = worldToView * T(stonePos[i].x, stonePos[i].y, stonePos[i].z) * S(stoneSize[i], stoneSize[i], stoneSize[i]);;
        m = worldToViewMatrix * T(cloudPos[i].x, cloudPos[i].y, cloudPos[i].z) * S(cloudSize[i].x, cloudSize[i].y, cloudSize[i].z);

        // Upload the transformation matrix
        glUniformMatrix4fv(glGetUniformLocation(shader, "camMatrix"), 1, GL_TRUE, m.m);
        glBindTexture(GL_TEXTURE_2D, stonetex);
        // Render the stone
        //DrawModel(stones[i], shader, "in_Position", "in_Normal", "in_TexCoord");
        DrawPatchModel(cloud[i], shader, "in_Position", "in_Normal", "in_TexCoord");

    }
}

std::vector<gluggModel> tree;
std::vector<vec3> treePos;
std::vector<int> treeType;
std::vector<gluggModel> bush;
std::vector<vec3> bushPos;
std::vector<int> bushType;
//gluggModel tree1;
gluggModel roadModel;
std::vector<vec3> roadVertices;
std::vector<Model *> stone;
std::vector<vec3> stonePos;
std::vector<vec3> stoneSize;
std::vector<float> randStone;
std::vector<Model *> cloud;
std::vector<vec3> cloudPos;
std::vector<vec3> cloudSize;
std::vector<float> randCloud;



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
    for (int x = 0; x < kTerrainSize; x++) { // Increment along the x-axis

        float z = sin(x * 0.03) * 10.0 + 15;  // Sine wave for the road path
        roadVertices.push_back(vec3(x, 0, z - width * 0.5)); // Left side
        roadVertices.push_back(vec3(x, 0, z + width * 0.5)); // Right side
    }
}


void FlattenTerrainForRoad(std::vector<vec3>& roadVertices) {
    const float tolerance = 1e-5f;
    for (auto& roadVertex : roadVertices) {
        for (int x = 0; x < kTerrainSize; x++) {
            for (int z = 0; z < kTerrainSize; z++) {
                int ix = z * kTerrainSize + x;
                if (std::abs(vertices[ix].x - roadVertex.x) < tolerance &&
                    std::abs(vertices[ix].z - roadVertex.z) < tolerance) {
                    // Adjust terrain height to match road
                    roadVertex.y = vertices[ix].y;
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

void diamondStep(int step_size, int half_size, float roughness) {
    for (int x = half_size; x < kTerrainSize; x += step_size) {
        for (int z = half_size; z < kTerrainSize; z += step_size) {
            int center = z * kTerrainSize + x;

            float offset = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * roughness;
            float avg = (vertices[(z - half_size) * kTerrainSize + (x - half_size)].y +
                         vertices[(z - half_size) * kTerrainSize + (x + half_size)].y +
                         vertices[(z + half_size) * kTerrainSize + (x - half_size)].y +
                         vertices[(z + half_size) * kTerrainSize + (x + half_size)].y) /4.0f;
            vertices[center].y = avg + offset;
        }
    }
}

void squareStep(int step_size, int half_size, float roughness) {

    for (int x = 0; x < kTerrainSize; x += half_size) {
        for (int z = (x + half_size) % step_size; z < kTerrainSize; z += step_size) {
            int count = 0;
            float sum = 0.0f;

            if (z - half_size >= 0) { // Top
                sum += vertices[(z - half_size) * kTerrainSize + x].y;
                count++;
            }
            if (z + half_size < kTerrainSize) { // Bottom
                sum += vertices[(z + half_size) * kTerrainSize + x].y;
                count++;
            }
            if (x - half_size >= 0) { // Left
                sum += vertices[z * kTerrainSize + (x - half_size)].y;
                count++;
            }
            if (x + half_size < kTerrainSize) { // Right
                sum += vertices[z * kTerrainSize + (x + half_size)].y;
                count++;
            }

            int ix = z * kTerrainSize + x;
            float avg = sum / count;
            float offset = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * roughness * 2.0f;
            vertices[ix].y = avg + offset;

        }
    }
}

//https://medium.com/@f.scaramelli0/heightmap-generation-using-the-diamond-square-algorithm-part-1-7c558aff7525
void diamondSquare(float roughness) {
    int step_size = kTerrainSize - 1;

    while (step_size > 1) {
        int half_step = step_size / 2;
        diamondStep(step_size, half_step, roughness);
        squareStep(step_size, half_step, roughness);
        step_size /= 2;
        roughness *= 0.6f; // Reduce roughness for finer details
    }
}

void smoothTerrain() {
    for (int x = 1; x < kTerrainSize - 1; ++x) {
        for (int z = 1; z < kTerrainSize - 1; ++z) {
            int ix = z * kTerrainSize + x;
            float avg = (vertices[ix - 1].y + vertices[ix + 1].y +
                         vertices[ix - kTerrainSize].y + vertices[ix + kTerrainSize].y) / 4.0f;
            vertices[ix].y = avg;
        }
    }
}
void addCraters(int numCraters, float radius, float depth) {
    for (int i = 0; i < numCraters; ++i) {
        float centerX = rand() % kTerrainSize;
        float centerZ = rand() % kTerrainSize;

        for (int x = 0; x < kTerrainSize; ++x) {
            for (int z = 0; z < kTerrainSize; ++z) {
                float dx = centerX - x;
                float dz = centerZ - z;
                float distance = sqrt(dx * dx + dz * dz);

                if (distance < radius) {
                    int ix = z * kTerrainSize + x;
                    vertices[ix].y -= depth * (1.0f - distance / radius);
                }
            }
        }
    }
}


//Taken from lab 3b
void MakeTerrain()
{
	// TO DO: This is where your terrain generation goes if on CPU.
	for (int x = 0; x < kTerrainSize; x++)
	for (int z = 0; z < kTerrainSize; z++)
	{
		int ix = z * kTerrainSize + x;

        vertices[ix] = vec3(x, 0, z);
		texCoords[ix] = vec2(x, z);
		normals[ix] = vec3(0,1,0);
	}
	vertices[0].y = -2 + static_cast<float>(rand()) / RAND_MAX;
    vertices[(kTerrainSize - 1) * kTerrainSize].y = 3 - static_cast<float>(rand()) / RAND_MAX;
    vertices[kTerrainSize * (kTerrainSize - 1)].y = 2 + static_cast<float>(rand()) / RAND_MAX;
    vertices[kTerrainSize * kTerrainSize - 1].y = static_cast<float>(rand()) / RAND_MAX - 4;
	diamondSquare(3.5f);
	addCraters(1, 20, 10);
	addCraters(1, 30 , -10);
	smoothTerrain();



	// Make indices
	// You don't need to change this.
	for (int x = 0; x < kTerrainSize-1; x++)
	for (int z = 0; z < kTerrainSize-1; z++)
	{
		// Quad count
		int q = (z*(kTerrainSize-1)) + (x);
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
	    vec3 v2 = vertices[(z) * kTerrainSize + MIN(x+1,kTerrainSize - 1)];
	    vec3 v3 = vertices[MAX(z-1,0) * kTerrainSize + (x)];
	    vec3 v4 = vertices[MIN(z+1,kTerrainSize - 1) * kTerrainSize + (x)];
	    vec3 Vector1 = v1 - v2;
	    vec3 Vector2 = v3 - v4;
	    vec3 normal = cross(Vector1,Vector2);

		normals[z * kTerrainSize + x] = normal;
	}
}

void MakeTerrainwater()
{
	// TO DO: This is where your terrain generation goes if on CPU.
	for (int x = 0; x < kTerrainSize; x++)
	for (int z = 0; z < kTerrainSize; z++)
	{
		int ix = z * kTerrainSize + x;

		verticeswater[ix] = vec3(x * kPolySize, waterHeight, z * kPolySize);
		texCoordswater[ix] = vec2(x, z);
		normalswater[ix] = vec3(0,1,0);
	}

	// Make indices
	// You don't need to change this.
	for (int x = 0; x < kTerrainSize-1; x++)
	for (int z = 0; z < kTerrainSize-1; z++)
	{
		// Quad count
		int q = (z*(kTerrainSize-1)) + (x);
		// Indices
		indiceswater[q*2*3] = x + z * kTerrainSize; // top left
		indiceswater[q*2*3+1] = x+1 + z * kTerrainSize;
		indiceswater[q*2*3+2] = x + (z+1) * kTerrainSize;
		indiceswater[q*2*3+3] = x+1 + z * kTerrainSize;
		indiceswater[q*2*3+4] = x+1 + (z+1) * kTerrainSize;
		indiceswater[q*2*3+5] = x + (z+1) * kTerrainSize;
	}

	// Make normal vectors
	// TO DO: This is where you calculate normal vectors
	for (int x = 0; x < kTerrainSize; x++)
	for (int z = 0; z < kTerrainSize; z++)
	{
	    vec3 v1 = verticeswater[(z) * kTerrainSize + MAX(x-1,0)];
	    vec3 v2 = verticeswater[(z) * kTerrainSize + MIN(x+1,63)];
	    vec3 v3 = verticeswater[MAX(z-1,0) * kTerrainSize + (x)];
	    vec3 v4 = verticeswater[MIN(z+1,63) * kTerrainSize + (x)];
	    vec3 Vector1 = v1 - v2;
	    vec3 Vector2 = v3 - v4;
	    vec3 normal = cross(Vector1,Vector2);

		normalswater[z * kTerrainSize + x] = normal;

	}

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
	glUseProgram(stoneShader);
	glUniformMatrix4fv(glGetUniformLocation(stoneShader, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
}

gluggModel stoneModel;

void init(void)
{
	// GL inits
	glClearColor(0.4,0.6,0.8,0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glCullFace(GL_TRUE);
    glCullFace(GL_BACK);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	printError("GL inits");


	projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 300.0);
	mat4 worldToViewMatrix = lookAt(0, 0, 3, 0,0,0, 0,1,0);
	modelToWorldMatrix = IdentityMatrix();

	// Load and compile shader
	phongShader = loadShaders("phong.vert", "phong.frag");
	texShader = loadShaders("textured.vert", "textured.frag");
	stoneShader = loadShadersGT("lab4.vs", "lab4.fs", "lab4.gs",
                                "lab4.tcs", "lab4.tes");

    printError("init shader");

	// Upload geometry to the GPU:
	//OLD
	//floormodel = LoadDataToModel((vec3 *)vertices2, (vec3 *)normals2, (vec2 *)texcoord2, NULL,
			//indices2, 4, 6);

    //FROM LABB 3B
    MakeTerrain();

    floormodel = LoadDataToModel(vertices, normals, texCoords, NULL,
			indices, kTerrainSize*kTerrainSize, (kTerrainSize-1)*(kTerrainSize-1)*2*3);

    MakeTerrainwater();
    watermodel = LoadDataToModel(verticeswater, normalswater, texCoordswater, NULL,
			indiceswater, kTerrainSize*kTerrainSize, (kTerrainSize-1)*(kTerrainSize-1)*2*3);

// Important! The shader we upload to must be active!
	glUseProgram(phongShader);
	glUniformMatrix4fv(glGetUniformLocation(phongShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUseProgram(texShader);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);

	glUniform1i(glGetUniformLocation(texShader, "tex"), 0); // Texture unit 0


	LoadTGATextureSimple("grassNew.tga", &grasstex);
	glBindTexture(GL_TEXTURE_2D, grasstex);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	LoadTGATextureSimple("bark2.tga", &barktex);

	LoadTGATextureSimple("water.tga", &watertex);

	LoadTGATextureSimple("ivyleaf.tga", &leaftex);

	LoadTGATextureSimple("granite.tga", &stonetex);

	LoadTGATextureSimple("bjorkbjork.tga", &bjorktex);

	LoadTGATextureSimple("bark4.tga", &bark4tex);

    generateTrees(tree, treePos, treeType, 300);
    generateBush(bush, bushPos, treePos, bushType, 300);

    glUseProgram(stoneShader);
    generateStones(stone, stonePos, stoneSize ,treePos, bushPos, randStone, 100);
    generateClouds(cloud, cloudPos, cloudSize, randCloud, 30);
	glUniformMatrix4fv(glGetUniformLocation(stoneShader, "camMatrix"), 1, GL_TRUE, worldToViewMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(stoneShader, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(stoneShader, "mdlMatrix"), 1, GL_TRUE, IdentityMatrix().m);

	printError("init arrays");
}

GLfloat a = 0.0;
//vec3 campos = vec3(0, 1.5, 10);
//FROM LAB 3B
vec3 campos = vec3(kTerrainSize*kPolySize/4, 1.5, kTerrainSize*kPolySize/4);
vec3 forward = vec3(0, 0, -4);
vec3 up = vec3(0, 1, 0);


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

    glBindTexture(GL_TEXTURE_2D, watertex);
	glUseProgram(texShader);

    DrawModel(watermodel, texShader, "inPosition", "inNormal", "inTexCoord");

	glUseProgram(texShader);
    buildTrees(worldToView, texShader, tree, treePos, treeType);
    buildBush(worldToView, texShader, bush, bushPos, bushType);

    glUseProgram(stoneShader);

    buildStone(worldToView, stone, stonePos, stoneSize, randStone, stoneShader);
    buildCloud(worldToView, cloud, cloudPos, cloudSize, randCloud, stoneShader);

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
	glutPostRedisplay();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

    // Request (for example) an OpenGL 4.1 Core profile context
    glutInitContextVersion(4, 1);

	//glutInitContextVersion(3, 2);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(640,360);
	glutCreateWindow ("Procedurell Landsbygd");
	glutRepeatingTimer(20);
	glutDisplayFunc(display);
	glutKeyboardFunc(keys);
	glutReshapeFunc(reshape);
	init ();
	glutMainLoop();
	exit(0);
}
