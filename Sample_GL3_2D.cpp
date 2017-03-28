#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <time.h>
#include <stdlib.h>
#include <thread>
#include <ao/ao.h>
#include <mpg123.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

#include <GL/gl.h>
#include <GL/glu.h>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;
	GLuint TextureBuffer;
	GLuint TextureID;

	GLenum PrimitiveMode; // GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES, GL_TRIANGLE_STRIP_ADJACENCY and GL_TRIANGLES_ADJACENCY
	GLenum FillMode; // GL_FILL, GL_LINE
	int NumVertices;
};
typedef struct VAO VAO;


struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID; // For use with normal shader
	GLuint TexMatrixID; // For use with texture shader
} Matrices;

struct FTGLFont {
	//FTFont* font;
	GLuint fontMatrixID;
	GLuint fontColorID;
} GL3Font;

typedef struct Point {
	float x;
	float y;
	float z;
} Point;

struct COLOR {
	float r;
	float g;
	float b;
};
typedef struct COLOR COLOR;

typedef struct Triangle {
	int p1;
	int p2;
	int p3; //Indices of the points corresponding to the triangle
} Triangle;

struct Sprite {
	string name;
	float x,y,z;
	VAO* object;
	int status;
	float x_scale,y_scale,z_scale;
	float x_speed,y_speed,z_speed;
	float rotation_x_offset, rotation_y_offset, rotation_z_offset;
	float friction; //Value from 0 to 1
	int health;
	int isRotating;
	int direction_x, direction_y, direction_z;
	float remAngle; //the remaining angle to finish animation
	int isMovingAnim;
	float weight;
	int inAir;
	int fixed;
	float angle_x, angle_y, angle_z;
};
typedef struct Sprite Sprite;

map <string, Sprite> objects;
map <string, Sprite> playerObjects;
int player_score=0;
int player_health=100;
int player_rotating=0;
float camera_fov=1.3;
int currentLevel=0;
int height,width;
int camera_follow=0;
int camera_follow_adjust=0;
int camera_top=0;
int camera_fps=0;
float fps_head_offset=0,fps_head_offset_x=0;
int head_tilting=0;
int isNight=0;
int super_jump_mode=0;
int current_jump=0;
int powerup_timer=0;
bool hortoleft = false, hortoright = false, hortoup = false, hortodown = false;
bool verttoright = false, verttoleft = false, verttoup = false, verttodown = false;
bool standing = false, roll = false, horhor = false;
int vertroll=0;
bool gameOver = false;
bool bridgeclosed = true, triggery = false;
int tryi = 0;
float bricksi = 0;
#define BITS 8
int player_moving_forward=0;
int player_moving_left=0;
int player_moving_backward=0;
int player_moving_right=0;
float x_change, y_change, zoom_camera;
int numMoves = 0;
float timeElapsed = 0, start_time = 0;
float bricksFall = false;
void createRectangle (GLuint textureID, GLfloat vertex_buffer[],string name);
void reshapeWindow (GLFWwindow* window, int width, int height);

//The level specific map and trap map are loaded from files
int gameMap[10][10];

//1 is not present, 2,3,4 are present
int gameMapTrap[10][10];


//Ensure the panning does not go out of the map
void check_pan(){
	if(x_change-400.0f/zoom_camera<-400)
		x_change=-400+400.0f/zoom_camera;
	else if(x_change+400.0f/zoom_camera>400)
		x_change=400-400.0f/zoom_camera;
	if(y_change-300.0f/zoom_camera<-300)
		y_change=-300+300.0f/zoom_camera;
	else if(y_change+300.0f/zoom_camera>300)
		y_change=300-300.0f/zoom_camera;
}

string convertInt(int number)
{
	if (number == 0)
		return "0";
	string temp="";
	string returnvalue="";
	while (number>0)
	{
		temp+=number%10+48;
		number/=10;
	}
	for (int i=0;i<temp.length();i++)
		returnvalue+=temp[temp.length()-i-1];
	return returnvalue;
}

void goToNextLevel(GLFWwindow* window);

GLuint programID, waterProgramID, fontProgramID, textureProgramID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	cout << "Compiling shader : " <<  vertex_file_path << endl;
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage( max(InfoLogLength, int(1)) );
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	cout << VertexShaderErrorMessage.data() << endl;

	// Compile Fragment Shader
	cout << "Compiling shader : " << fragment_file_path << endl;
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage( max(InfoLogLength, int(1)) );
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	cout << FragmentShaderErrorMessage.data() << endl;

	// Link the program
	cout << "Linking program" << endl;
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	cout << ProgramErrorMessage.data() << endl;

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	cout << "Error: " << description << endl;
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

glm::vec3 getRGBfromHue (int hue)
{
	float intp;
	float fracp = modff(hue/60.0, &intp);
	float x = 1.0 - abs((float)((int)intp%2)+fracp-1.0);

	if (hue < 60)
		return glm::vec3(1,x,0);
	else if (hue < 120)
		return glm::vec3(x,1,0);
	else if (hue < 180)
		return glm::vec3(0,1,x);
	else if (hue < 240)
		return glm::vec3(0,x,1);
	else if (hue < 300)
		return glm::vec3(x,0,1);
	else
		return glm::vec3(1,0,x);
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

struct VAO* create3DTexturedObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* texture_buffer_data, GLuint textureID, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;
	vao->TextureID = textureID;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->TextureBuffer));  // VBO - textures

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->TextureBuffer); // Bind the VBO textures
	glBufferData (GL_ARRAY_BUFFER, 2*numVertices*sizeof(GLfloat), texture_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			2,                  // attribute 2. Textures
			2,                  // size (s,t)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}


/* Create an OpenGL Texture from an image */
GLuint createTexture (const char* filename)
{
	GLuint TextureID;
	// Generate Texture Buffer
	glGenTextures(1, &TextureID);
	// All upcoming GL_TEXTURE_2D operations now have effect on our texture buffer
	glBindTexture(GL_TEXTURE_2D, TextureID);
	// Set our texture parameters
	// Set texture wrapping to GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering (interpolation)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load image and create OpenGL texture
	int twidth, theight;
	unsigned char* image = SOIL_load_image(filename, &twidth, &theight, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D); // Generate MipMaps to use
	SOIL_free_image_data(image); // Free the data read from file after creating opengl texture
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess it up

	return TextureID;
}


/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = -1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
int inAir=0;
//Camera eye, target and up vector components
float eye_x,eye_y,eye_z;
float target_x=-50,target_y,target_z=-50;
float angle=0;
float camera_radius;
int left_mouse_clicked;
int right_mouse_clicked;
int camera_disable_rotation=0;
float jump_height;

int player_sprint=0;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_LEFT:
				hortoleft = true;
				numMoves++;
				break;
			case GLFW_KEY_H:
				camera_follow_adjust=1-camera_follow_adjust;
				break;
			case GLFW_KEY_RIGHT:
				hortoright = true;
				numMoves++;
				break;
			case GLFW_KEY_UP:
				hortoup = true;
				numMoves++;
				break;
			case GLFW_KEY_DOWN:
				hortodown = true;
				numMoves++;
				break;
			case GLFW_KEY_T:
				camera_disable_rotation=1;
				camera_follow=0;
				camera_fps=0;
				camera_radius=1; //Top view
				camera_top=1;
				eye_x = objects["player"].x+camera_radius*cos(angle*M_PI/180);
				eye_z = objects["player"].z+camera_radius*sin(angle*M_PI/180);
				eye_y=1100;
				target_x=objects["player"].x;
				target_y=0;
				target_z=objects["player"].z;
				fps_head_offset=0;
				fps_head_offset_x=0;
				camera_fov=1.3;
				reshapeWindow(window,700,1400);
				break;
			case GLFW_KEY_R:
				camera_disable_rotation=1;
				camera_follow=0;
				camera_fps=0;
				camera_top=0;
				camera_radius=800; //Tower view
				eye_x = -1500+camera_radius;
				eye_z = 500;
				eye_y=1500;
				target_x=-50;
				target_y=0;
				target_z=-50;
				fps_head_offset=0;
				fps_head_offset_x=0;
				camera_fov=1.3;
				reshapeWindow(window,700,1400);
				break;
			case GLFW_KEY_Y:
				camera_disable_rotation=0;
				camera_follow=0;
				camera_fps=0;
				camera_top=0;
				camera_radius=800; //Tower view
				eye_x = -50+camera_radius*cos(angle*M_PI/180);
				eye_z = -50+camera_radius*sin(angle*M_PI/180);
				eye_y=1100;
				target_x=-50;
				target_y=0;
				target_z=-50;
				fps_head_offset=0;
				fps_head_offset_x=0;
				camera_fov=1.3;
				reshapeWindow(window,700,1400);
				break;
			case GLFW_KEY_U:
				camera_disable_rotation=1;
				camera_fps=0;
				camera_top=0;
				camera_follow=1;
				fps_head_offset=0;
				fps_head_offset_x=0;
				camera_fov=1.3;
				reshapeWindow(window,700,1400);
				break;
			case GLFW_KEY_I:
				camera_disable_rotation=1;
				camera_follow=0;
				camera_fps=1;
				camera_top=0;
				fps_head_offset=0;
				fps_head_offset_x=0;
				camera_fov=1.3;
				target_x=objects["player"].x - 500;
				target_y=objects["player"].y;
				target_z=objects["player"].z - 500;
				eye_x=objects["player"].x + 1000;
				eye_y=objects["player"].y;
				eye_z=objects["player"].z - 1000;
				reshapeWindow(window,700,1400);
				break;
			case GLFW_KEY_Z:
				player_sprint=0;
				break;
			case GLFW_KEY_J:
				player_moving_forward=0;
				break;
			case GLFW_KEY_N:
				player_moving_backward=0;
				break;
			case GLFW_KEY_W:
				head_tilting=0;
				break;
			case GLFW_KEY_S:
				head_tilting=0;
				break;
			case GLFW_KEY_A:
				player_moving_left=0;
				break;
			case GLFW_KEY_D:
				player_moving_right=0;
				break;
			case GLFW_KEY_M:
				player_rotating=0;
				player_moving_right=0;
				break;
			case GLFW_KEY_B:
				player_rotating=0;
				player_moving_left=0;
				break;
			case GLFW_KEY_C:
				rectangle_rot_status = !rectangle_rot_status;
				break;
			case GLFW_KEY_P:
				triangle_rot_status = !triangle_rot_status;
				break;
			case GLFW_KEY_X:
				// do something ..
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_M:
				if(camera_fps==1){
					player_moving_right=1;
				}
				else{
					player_rotating=1;
				}
				break;
			case GLFW_KEY_B:
				if(camera_fps==1){
					player_moving_left=1;
				}
				else{
					player_rotating=-1; //The left key has a slight problem when used together with up or down and space.
				}
				break;
			case GLFW_KEY_Z:
				player_sprint=1;
				break;
			case GLFW_KEY_J:
				player_moving_forward=1;
				break;
			case GLFW_KEY_N:
				player_moving_backward=1;
				break;
			case GLFW_KEY_W:
				head_tilting=1;
				break;
			case GLFW_KEY_S:
				head_tilting=-1;
				break;
			case GLFW_KEY_A:
				player_moving_left=1;
				break;
			case GLFW_KEY_D:
				player_moving_right=1;
				break;
			case GLFW_KEY_SPACE:
				if(((super_jump_mode==1 && current_jump==0) || inAir==0)){
					//Dont let the person jump when inside the elevator
					if(inAir!=0)
						current_jump=1;
					else
						current_jump=0;
					if(super_jump_mode)
						objects["player"].y_speed=15;
					else
						objects["player"].y_speed=13;
					objects["player"].y+=5;
					inAir=1;
					jump_height=objects["player"].y;
				}
				break;
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			default:
				break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_PRESS){
				left_mouse_clicked=1;
				break;
			}
			if (action == GLFW_RELEASE){
				triangle_rot_dir *= -1;
				left_mouse_clicked=0;
				break;
			}
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_PRESS){
				right_mouse_clicked=1;
				break;
			}
			if (action == GLFW_RELEASE){
				triangle_rot_dir *= -1;
				right_mouse_clicked=0;
				break;
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = camera_fov; //Use from 1 to 2

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 7000.0f);

	// Ortho projection for 2D views
	//Matrices.projection = glm::ortho(-1000.0f, 1000.0f, -1000.0f, 1000.0f, -1000.0f, 5000.0f);
}

void mousescroll(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset==-1) {
		camera_fov*=1.1;
	}
	else if(yoffset==1){
		camera_fov/=1.1; //make it bigger than current size
	}
	if(camera_fov>=2){
		camera_fov=2;
	}
	if(camera_fov<=0.5){
		camera_fov=0.5;
	}
	if(x_change-400.0f/zoom_camera<-400)
		x_change=-400+400.0f/zoom_camera;
	else if(x_change+400.0f/zoom_camera>400)
		x_change=400-400.0f/zoom_camera;
	if(y_change-300.0f/zoom_camera<-300)
		y_change=-300+300.0f/zoom_camera;
	else if(y_change+300.0f/zoom_camera>300)
		y_change=300-300.0f/zoom_camera;
	reshapeWindow(window,700,1400);
}

VAO *triangle, *skybox, *skybox1, *skybox2, *skybox3, *skybox4, *skybox5;

// Creates the triangle object used in this sample code
void createTriangle ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */
	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	static const GLfloat vertex_buffer_data [] = {
		0, 1,0, // vertex 0
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 0
		0,1,0, // color 1
		0,0,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

void createRectangle (GLuint textureID, GLfloat vertex_buffer[],string name)
{
	// GL3 accepts only Triangles. Quads are not supported
	GLfloat vertex_buffer_data [18];
	int i;
	for(i=0;i<18;i++){
		vertex_buffer_data[i]=vertex_buffer[i];
	}

	GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0,0,1, // color 4
		1,0,0  // color 1
	};

	// Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
	GLfloat texture_buffer_data [] = {
		0,1, // TexCoord 1 - bot left
		1,1, // TexCoord 2 - bot right
		1,0, // TexCoord 3 - top right

		1,0, // TexCoord 3 - top right
		0,0, // TexCoord 4 - top left
		0,1  // TexCoord 1 - bot left
	};
}

void createTrapezium (string name, float degreeRotation, COLOR colorA, COLOR colorB, COLOR colorC, COLOR colorD, float x, float y, float height, float width, string component)
{
	// GL3 accepts only Triangles. Quads are not supported
	float w=width/2,h=height/2;
	GLfloat vertex_buffer_data [] = {
		-w+10,-h,0, // vertex 1
		-w,h,0, // vertex 2
		w,h,0, // vertex 3

		w,h,0, // vertex 3
		w-10,-h,0, // vertex 4
		-w+10,-h,0  // vertex 1
	};

	GLfloat color_buffer_data [] = {
		0.5, 0.7, 0.65, // color 1
		0.5, 0.7, 0.65,  // color 2
		0.5, 0.7, 0.65,  // color 3

		0.5, 0.7, 0.65,  // color 4
		0.5, 0.7, 0.65,  // color 5
		0.5, 0.7, 0.65,  // color 6
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	VAO *rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createModel (string name, float x_pos, float y_pos, float z_pos, float x_scale, float y_scale, float z_scale, string filename, string layer, int status, int weight) //Create object from blender
{
	GLfloat vertex_buffer_data [100000] = {
	};
	GLfloat color_buffer_data [100000] = {
	};
	vector<Point> points;
	int len=0;
	string line;
	float a,b,c;
	int start=0;
	ifstream myfile;
	myfile.open(filename.c_str());
	if (myfile.is_open()){
		while (myfile >> line){
			if(line.length()==1 && line[0]=='v'){
				myfile >> a >> b >> c;
				Point cur_point = {};
				cur_point.x=a;
				cur_point.y=b;
				cur_point.z=c;
				points.push_back(cur_point);
			}
		}
		myfile.close();
	}
	int t[3],temp;
	int bcount=0,ccount=0;
	myfile.open(filename.c_str());
	if (myfile.is_open()){
		while (myfile >> line){
			if(line.length()==1 && line[0]=='f'){
				string linemod;
				getline(myfile, linemod);
				int j,ans=0,tt=0,state=0;
				for(j=0;j<linemod.length();j++){
					if(linemod[j]==' '){
						ans=0;
						state=1;
					}
					else if(linemod[j]=='/' && ans!=0 && state==1){
						t[tt]=ans;
						tt++;
						state=0;
					}
					else if(linemod[j]!='/'){
						ans=ans*10+linemod[j]-'0';
					}
				}
				t[tt]=ans;
				Triangle my_triangle = {};
				my_triangle.p1=t[0]-1;
				my_triangle.p2=t[1]-1;
				my_triangle.p3=t[2]-1;
				vertex_buffer_data[bcount]=points[my_triangle.p1].x*x_scale;
				vertex_buffer_data[bcount+1]=points[my_triangle.p1].y*y_scale;
				vertex_buffer_data[bcount+2]=points[my_triangle.p1].z*z_scale;
				vertex_buffer_data[bcount+3]=points[my_triangle.p2].x*x_scale;
				vertex_buffer_data[bcount+4]=points[my_triangle.p2].y*y_scale;
				vertex_buffer_data[bcount+5]=points[my_triangle.p2].z*z_scale;
				vertex_buffer_data[bcount+6]=points[my_triangle.p3].x*x_scale;
				vertex_buffer_data[bcount+7]=points[my_triangle.p3].y*y_scale;
				vertex_buffer_data[bcount+8]=points[my_triangle.p3].z*z_scale;
				bcount+=9;
			}
			if(line.length()==1 && line[0]=='c'){
				float r1,g1,b1,r2,g2,b2,r3,g3,b3;
				myfile >> r1 >> g1 >> b1 >> r2 >> g2 >> b2 >> r3 >> g3 >> b3;
				color_buffer_data[ccount]=r1/255.0;
				color_buffer_data[ccount+1]=g1/255.0;
				color_buffer_data[ccount+2]=b1/255.0;
				color_buffer_data[ccount+3]=r2/255.0;
				color_buffer_data[ccount+4]=g2/255.0;
				color_buffer_data[ccount+5]=b2/255.0;
				color_buffer_data[ccount+6]=r3/255.0;
				color_buffer_data[ccount+7]=g3/255.0;
				color_buffer_data[ccount+8]=b3/255.0;
				ccount+=9;
			}
		}
		myfile.close();
	}
	VAO* myobject = create3DObject(GL_TRIANGLES, bcount/3, vertex_buffer_data, color_buffer_data, GL_FILL);
	Sprite vishsprite = {};
	vishsprite.name = name;
	vishsprite.object = myobject;
	vishsprite.x=x_pos;
	vishsprite.y=y_pos;
	vishsprite.z=z_pos;
	vishsprite.status=status;
	vishsprite.fixed=0;
	vishsprite.friction=0.4;
	vishsprite.health=100;
	vishsprite.weight=weight;
	vishsprite.x_scale=x_scale;
	vishsprite.y_scale=y_scale;
	vishsprite.z_scale=z_scale;
	if(layer=="player")
		playerObjects[name]=vishsprite;
	else
		objects[name]=vishsprite;
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
double prev_mouse_x;
double prev_mouse_y;
float gravity=0.5;
float trapTimer=0;
int justInAir=0;
float player_speed=1.5;

float previous_mouse_y,previous_mouse_x;
float previous_mouse_y2,previous_mouse_x2;
float previous_mouse_y3,previous_mouse_x3;
int previous=0;


/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window)
{
	printf("Number of moves: %d\n", numMoves);
	printf("Time taken: %d seconds\n", (int)(glfwGetTime() - start_time));
	if(tryi < -1200 || bricksi < -2000){
	//	gameOver = true;
		if(bricksFall) {
			printf("YOU LOST\n");
			currentLevel--;
		}
		else {
			printf("YOU WON!\n");
		}
		goToNextLevel(window);
	}
	if(camera_top){
		eye_x = objects["player"].x+camera_radius*cos(angle*M_PI/180);
		eye_z = objects["player"].z+camera_radius*sin(angle*M_PI/180) + 200;
		eye_y=1300;
		target_x=objects["player"].x + 100;
		target_y=0;
		target_z=objects["player"].z + 100;
	}
	if(camera_follow==1){
		target_x=objects["player"].x;
		target_y=objects["player"].y;
		target_z=objects["player"].z;
		eye_x=objects["player"].x-200;
		eye_y=objects["player"].y + 200;
		eye_z=objects["player"].z - 200;
	}
	if(camera_fps==1){
		double new_mouse_x,new_mouse_y;
		glfwGetCursorPos(window,&new_mouse_x,&new_mouse_y);
		if(abs(new_mouse_y-previous_mouse_y2)>=1){
			fps_head_offset-=(new_mouse_y-previous_mouse_y2)/13;
			previous_mouse_y2=new_mouse_y;
		}
		else if(new_mouse_y<=10 || new_mouse_y>=655){
			if(new_mouse_y<=10){
				fps_head_offset-=-0.3;
			}
			else{
				fps_head_offset-=0.3;
			}
		}
		if(abs(new_mouse_x-previous_mouse_x2)>=1){
			objects["player"].angle_y-=(new_mouse_x-previous_mouse_x2)/8;
			previous_mouse_x2=new_mouse_x;
		}
		else if(new_mouse_x<=10 || new_mouse_x>=1355){
			if(new_mouse_x<=10){
				objects["player"].angle_y+=1.5;
			}
			else{
				objects["player"].angle_y-=1.5;
			}
		}
		if(fps_head_offset>=30){
			fps_head_offset=30;
		}
		if(fps_head_offset<=-30){
			fps_head_offset=-30;
		}
		target_x=objects["player"].x+42*sin((objects["player"].angle_y)*M_PI/180);
		target_y=objects["player"].y+60+fps_head_offset;
		target_z=objects["player"].z+42*cos((objects["player"].angle_y)*M_PI/180);
		eye_x=objects["player"].x+42*sin(objects["player"].angle_y*M_PI/180)-10*sin(objects["player"].angle_y*M_PI/180);
		eye_y=objects["player"].y+60;
		eye_z=objects["player"].z+42*cos(objects["player"].angle_y*M_PI/180)-10*cos(objects["player"].angle_y*M_PI/180);
	}

	if(inAir==1){
		objects["player"].y_speed-=gravity;
		if(objects["player"].y_speed<=-12.0){
			objects["player"].y_speed=-12.0;
		}
		//Check collision in the y-axis to detect if the player is in air or not
	}

	if(player_rotating!=0){
		objects["player"].angle_y-=player_rotating*2;
	}
	if(player_moving_forward!=0){
		objects["player"].z+=(1+player_sprint)*player_speed*cos(objects["player"].angle_y*M_PI/180)*2;

		objects["player"].x+=(1+player_sprint)*player_speed*sin(objects["player"].angle_y*M_PI/180)*2;

	}
	else if(player_moving_backward!=0){
		objects["player"].z-=(1+player_sprint)*player_speed*cos(objects["player"].angle_y*M_PI/180)*2;

		objects["player"].x-=(1+player_sprint)*player_speed*sin(objects["player"].angle_y*M_PI/180)*2;

	}
	if(player_moving_left!=0){
		objects["player"].z+=(1+player_sprint)*player_speed*cos((objects["player"].angle_y+90)*M_PI/180)*2;

		objects["player"].x+=(1+player_sprint)*player_speed*sin((objects["player"].angle_y+90)*M_PI/180)*2;

	}
	else if(player_moving_right!=0){
		objects["player"].z-=(1+player_sprint)*player_speed*cos((objects["player"].angle_y+90)*M_PI/180)*2;

		objects["player"].x-=(1+player_sprint)*player_speed*sin((objects["player"].angle_y+90)*M_PI/180)*2;

	}

	double new_mouse_x,new_mouse_y;
	glfwGetCursorPos(window,&new_mouse_x,&new_mouse_y);
	if(left_mouse_clicked==1 && camera_follow==0 && camera_disable_rotation==0){
		angle+=(previous_mouse_x3-new_mouse_x)/10;
		previous_mouse_x=new_mouse_x;
		eye_x = -50+camera_radius*cos(angle*M_PI/180);
		eye_z = -50+camera_radius*sin(angle*M_PI/180);
	}
	previous_mouse_x3=new_mouse_x;
	if(right_mouse_clicked && camera_follow==0 && camera_disable_rotation==0){
		if (abs(previous_mouse_y-new_mouse_y)>=35)
			previous_mouse_y = new_mouse_y;
		else
			eye_y += new_mouse_y - previous_mouse_y;
		previous_mouse_y=new_mouse_y;
	}
	prev_mouse_x=new_mouse_x;
	prev_mouse_y=new_mouse_y;
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye (eye_x, eye_y, eye_z);
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (target_x, target_y, target_z);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	static float c = 0;
	c++;
	//Matrices.view = glm::lookAt(glm::vec3(0,0,10), glm::vec3(0,0,0), glm::vec3(sinf(c*M_PI/180.0),3*cosf(c*M_PI/180.0),0)); // Fixed camera for 2D (ortho) in XY plane
	Matrices.view = glm::lookAt(eye, target, up); // Fixed camera for 2D (ortho) in XY plane
	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model
	static int fontScale = 0;

	//Draw the objects
	for(map<string,Sprite>::iterator it=objects.begin();it!=objects.end();it++){
		string current = it->first; //The name of the current object
		glUseProgram (programID);
		if(objects[current].status==0)
			continue;
		glm::mat4 MVP;  // MVP = Projection * View * Model

		Matrices.model = glm::mat4(1.0f);

		glm::mat4 ObjectTransform;
		ObjectTransform = glm::translate(glm::vec3(-0.5,-0.5,-0.5));
		if(objects[current].name == "player") {
			if(hortoleft) {
				objects[current].angle_x += 90;
				hortoleft = false;
				if(standing) {
					standing = !standing;
					objects[current].x -= 225;
					roll = true;
					horhor = true;
				}
				else {
					horhor = false;
					vertroll++;
					if(roll) {
						objects[current].x -= 225;
						standing = !standing;
					}
					else {
						objects[current].x -= 150;
					}
					roll = false;
				}
			}
			else if(hortoright) {
				objects[current].angle_x -= 90;
				hortoright = false;
				if(standing) {
					horhor = true;
					standing = !standing;
					objects[current].x += 225;
					roll = true;
				}
				else {
					horhor = false;
					vertroll++;
					if(roll) {
						objects[current].x += 225;
						standing = !standing;
					}
					else {
						objects[current].x += 150;
					}
					roll = false;
				}
			}
			else if(hortoup) {
				if(!standing && horhor) {
					objects[current].z -= 150;
					hortoup = false;
					horhor = true;
				}
				else if(!standing && (vertroll%4==0 || vertroll%4 == 2)) {
					objects[current].z -= 225;
					objects[current].angle_z += 90;
					hortoup = false;
					standing = !standing;
					horhor = false;
				}
				else 	if(!standing && (vertroll%4!=0 && vertroll%4!=2)) {
					objects[current].z -= 225;
					objects[current].angle_z += 90;
					objects[current].angle_x -= 90;
					hortoup = false;
					standing = !standing;
					horhor = false;
				}
				else if(standing && vertroll % 2 == 0) {
					objects[current].z -= 225;
					objects[current].angle_z += 90;
					hortoup = false;
					horhor = false;
					standing = !standing;
				}
				else if(standing && vertroll % 2 != 0) {
					objects[current].z -= 225;
					objects[current].angle_z += 90;
					objects[current].angle_x -= 90;
					hortoup = false;
					horhor = false;
					standing = !standing;
				}
			}
			else if(hortodown) {
				if(!standing && horhor) {
					objects[current].z += 150;
					hortodown = false;
					horhor = true;
				}
				else if(!standing && !horhor && (vertroll%4==0 || vertroll%4 == 2)) {
					objects[current].z += 225;
					objects[current].angle_z -= 90;
					hortodown = false;
					standing = !standing;
				}
				else 	if(!standing && (vertroll%4!=0 && vertroll%4!=2)) {
					objects[current].z += 225;
					objects[current].angle_z -= 90;
					objects[current].angle_x += 90;
					hortodown = false;
					standing = !standing;
				}
				else if(standing && vertroll % 2 == 0) {
					objects[current].z += 225;
					objects[current].angle_z -= 90;
					hortodown = false;
					standing = !standing;
				}
				else if(standing && vertroll % 2 != 0) {
					objects[current].z += 225;
					objects[current].angle_z -= 90;
					objects[current].angle_x += 90;
					hortodown = false;
					standing = !standing;
				}
			}
			if(standing) {
				objects[current].y = 325+75;
			}
			else {
				objects[current].y = 325;
			}
		}

		float ox = objects["player"].x;
		float oz = objects["player"].z;
		float bx1, bz1, bx2, bz2;
		if(ox <= -750-10 || oz <= -750-10 || ox >= 600+10 || oz >= 600+10) {
			bricksFall = true;
		}
		for(int i=0; i<=9; i++) {
			for(int j=0; j<=9; j++) {
				if(!standing) {
					if(!horhor) {
						bz1 = oz - 75;
						bz2 = oz + 75;
						bx1 = ox;
						bx2 = ox;
					}
					else {
						bz1 = oz;
						bz2 = oz;
						bx1 = ox - 75;
						bx2 = ox + 75;
					}

					bool t1 = (((bz1 <= (i-5)*150 + 75) && (bz1 >= (i-5)*150 - 75)) && ((bx1 <= (j-5)*150 + 75) && ( bx1 >= (j-5)*150 - 75)));
					bool t2 = (((bz2 <= (i-5)*150 + 75) && (bz2 >= (i-5)*150 - 75)) && ((bx2 <= (j-5)*150 + 75) && ( bx2 >= (j-5)*150 - 75)));
					if((t1 || t2) && ((gameMapTrap[i][j] == -1) || (gameMapTrap[i][j] == 5 && bridgeclosed))) {
						bricksFall = true;
					}
				}
				else {
					if(((oz <= (i-5)*150 + 75) && (oz >= (i-5)*150 - 75)) && ((ox <= (j-5)*150 + 75) && (ox >= (j-5)*150 - 75))) {
						if(gameMapTrap[i][j] == -1 || (gameMapTrap[i][j] == 5 && bridgeclosed)) {  // no brick
							bricksFall = true;
						}
						else if(gameMapTrap[i][j] == 9) {  // winning hole
							triggery = true;
						}
						else if(gameMapTrap[i][j] == 8) {  // weak bricks
							bricksFall = true;
						}
						else if(gameMapTrap[i][j] == 4) { // draw bridge
							for(map<string,Sprite>::iterator it=objects.begin();it!=objects.end();it++){
								string current = it->first;
								if(objects[current].weight == 10) {
									objects[current].status = 1;
								}
							}
							bridgeclosed = false;
						}
					}
				}
			}
		}

		ObjectTransform = glm::translate(glm::vec3(-0.5,-0.5,-0.5)) * ObjectTransform;
		glm::mat4 rotateObject = glm::rotate((float)((objects[current].angle_y)*M_PI/180.0f), glm::vec3(0,1,0));  // rotate about vector (1,0,0)
		rotateObject*=glm::rotate((float)((objects[current].angle_x)*M_PI/180.0f), glm::vec3(1,0,0));
		rotateObject*=glm::rotate((float)((objects[current].angle_z)*M_PI/180.0f), glm::vec3(0,0,1));
		glm::mat4 scaleObject = glm::scale (glm::vec3(1.0f,1.0f,1.0f));

		//		rotateObject*=glm::rotate( (float)(90 * M_PI/180.0f), glm::vec3(objects[current].x, objects[current].y, objects[current].z) );
		if(triggery && (current == "player"))
			tryi-= 10;

		if(bricksFall)
			bricksi -= 0.2;

		glm::mat4 translateObject;
		if(!bricksFall) {
			if(current != "player")
				translateObject = glm::translate (glm::vec3(objects[current].x, objects[current].y, objects[current].z)); // glTranslatef
			else
				translateObject = glm::translate (glm::vec3(objects[current].x, objects[current].y + tryi, objects[current].z)); // glTranslatef
		}
		else{
			translateObject = glm::translate (glm::vec3(objects[current].x, objects[current].y + bricksi, objects[current].z)); // glTranslatef
		}

		ObjectTransform = rotateObject*scaleObject * ObjectTransform;
		ObjectTransform=translateObject*ObjectTransform;
		Matrices.model *= ObjectTransform;
		MVP = VP * Matrices.model; // MVP = p * V * M

		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		GLint myUniformLocation = glGetUniformLocation(programID, "objectPosition");
		glUniform3f(myUniformLocation,objects[current].x,objects[current].y,objects[current].z);

		if(current!="player"){
			myUniformLocation = glGetUniformLocation(programID, "isPlayer");
			glUniform1f(myUniformLocation,0.0);
		}
		else{
			myUniformLocation = glGetUniformLocation(programID, "isPlayer");
			glUniform1f(myUniformLocation,1.0);
		}

		myUniformLocation = glGetUniformLocation(programID, "playerAngleY");
		glUniform1f(myUniformLocation,objects["player"].angle_y);

		myUniformLocation = glGetUniformLocation(programID, "playerAngleXZ");
		glUniform1f(myUniformLocation,-fps_head_offset);

		myUniformLocation = glGetUniformLocation(programID, "isNight");
		glUniform1f(myUniformLocation,isNight);

		myUniformLocation = glGetUniformLocation(programID, "playerPosition");
		glUniform3f(myUniformLocation,objects["player"].x,objects["player"].y+60,objects["player"].z);

		draw3DObject(objects[current].object);
		//glPopMatrix ();
	}

	glUseProgram (programID);
	//Draw the player
	for(map<string,Sprite>::iterator it=playerObjects.begin();it!=playerObjects.end();it++){
		string current = it->first; //The name of the current object
		if(playerObjects[current].status==0)
			continue;
		glm::mat4 MVP;  // MVP = Projection * View * Model

		Matrices.model = glm::mat4(1.0f);

		glm::mat4 ObjectTransform;
		glm::mat4 rotateObject = glm::rotate((float)((objects["player"].angle_y)*M_PI/180.0f), glm::vec3(0,1,0));
		rotateObject*=glm::rotate((float)((objects[current].angle_x)*M_PI/180.0f), glm::vec3(1,0,0));
		rotateObject*=glm::rotate((float)((objects[current].angle_z)*M_PI/180.0f), glm::vec3(0,0,1));
		glm::mat4 selfRotate = glm::rotate((float)((playerObjects[current].angle_y)*M_PI/180.0f), glm::vec3(0,1,0));
		selfRotate*=glm::rotate((float)((playerObjects[current].angle_x)*M_PI/180.0f), glm::vec3(1,0,0));
		selfRotate*=glm::rotate((float)((playerObjects[current].angle_z)*M_PI/180.0f), glm::vec3(0,0,1));
		glm::mat4 translateSelfOffset = glm::translate (glm::vec3(playerObjects[current].rotation_x_offset,playerObjects[current].rotation_y_offset,playerObjects[current].rotation_z_offset));
		glm::mat4 translateSelfOffsetBack = glm::translate (glm::vec3(-playerObjects[current].rotation_x_offset,-playerObjects[current].rotation_y_offset,-playerObjects[current].rotation_z_offset));
		glm::mat4 translateRelative = glm::translate (glm::vec3(playerObjects[current].x,playerObjects[current].y,playerObjects[current].z));
		glm::mat4 translateRelativeBack = glm::translate (glm::vec3(-playerObjects[current].x,-playerObjects[current].y,-playerObjects[current].z));
		glm::mat4 translateObject = glm::translate (glm::vec3(playerObjects[current].x+objects["player"].x, playerObjects[current].y+objects["player"].y, playerObjects[current].z+objects["player"].z)); // glTranslatef
		ObjectTransform=translateObject*translateRelativeBack*rotateObject*translateRelative*translateSelfOffsetBack*selfRotate*translateSelfOffset;

		Matrices.model *= ObjectTransform;
		MVP = VP * Matrices.model; // MVP = p * V * M

		GLint myUniformLocation = glGetUniformLocation(programID, "objectPosition");
		glUniform3f(myUniformLocation,objects[current].x,objects[current].y,objects[current].z);

		myUniformLocation = glGetUniformLocation(programID, "isPlayer");
		glUniform1f(myUniformLocation,1.0);

		myUniformLocation = glGetUniformLocation(programID, "playerAngleY");
		glUniform1f(myUniformLocation,objects["player"].angle_y);

		myUniformLocation = glGetUniformLocation(programID, "playerAngleXZ");
		glUniform1f(myUniformLocation,-fps_head_offset);

		myUniformLocation = glGetUniformLocation(programID, "isNight");
		glUniform1f(myUniformLocation,isNight);

		myUniformLocation = glGetUniformLocation(programID, "playerPosition");
		glUniform3f(myUniformLocation,objects["player"].x,objects["player"].y+60,objects["player"].z);

		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		draw3DObject(playerObjects[current].object);
		//glPopMatrix ();
	}

	// Render with texture shaders now
	glUseProgram(textureProgramID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
	glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle * rotateRectangle);
	MVP = VP * Matrices.model;

	// Copy MVP to texture shaders
	glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);

	// Set the texture sampler to access Texture0 memory
	glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);

	// Increment angles
	float increments = 0;

}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
	glfwSetScrollCallback(window, mousescroll); // mouse scroll

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	string levelmap = "Levels/gameMap"+convertInt(currentLevel+1);
	string trapmap = "Levels/gameMapTrap"+convertInt(currentLevel+1);
	levelmap+=".txt";
	trapmap+=".txt";

	cout << levelmap << endl;
	fstream myfile(levelmap.c_str());
	int a,i=0,j=0;
	if (myfile.is_open()){
		while(myfile >> a){
			if(i==10){
				i=0;
				j++;
			}
			gameMap[i][j]=a;
			i++;
		}
		myfile.close();
	}

	fstream myfile2(trapmap.c_str());
	i=0;
	j=0;
	if (myfile2.is_open()){
		while(myfile2 >> a){
			if(i==10){
				i=0;
				j++;
			}
			gameMapTrap[i][j]=a;
			i++;
		}
		myfile2.close();
	}

	// Load Textures
	// Enable Texture0 as current texture memory
	glActiveTexture(GL_TEXTURE0);
	// load an image file directly as a new OpenGL texture
	// GLuint texID = SOIL_load_OGL_texture ("Images/beach.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_TEXTURE_REPEATS); // Buggy for OpenGL3
	// check for an error during the load process
	//if(textureID == 0 )
	//	cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

	// Create and compile our GLSL program from the texture shaders
	textureProgramID = LoadShaders( "TextureRender.vert", "TextureRender.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.TexMatrixID = glGetUniformLocation(textureProgramID, "MVP");

	float scale=1;

	int k,stus;
	for(i=0;i<10;i++){
		for(j=0;j<10;j++){
			if(gameMap[i][j]!=0){
				if(gameMapTrap[i][j] != -1){
					int p;
					for(p=0;p<gameMap[i][j];p++){
						string name = "floorcube";
						name.append(convertInt(i)+convertInt(j)+convertInt(p));
						if(gameMapTrap[i][j] == 5) {
							createModel (name,(j-5)*150,1*150+150/2,(i-5)*150,150,30,150,"Models/greencube.data","",0, 10);
						}
						else if(gameMapTrap[i][j] == 4) {
							createModel (name,(j-5)*150,1*150+150/2,(i-5)*150,150,30,150,"Models/bluecube.data","",1, 5);
						}
						else if(gameMapTrap[i][j] == 3) {
							createModel (name,(j-5)*1500,1*1500+150/2,(i-5)*1500,150,30,150,"Models/floortrap.data","",1, 5);
						}
						else if(gameMapTrap[i][j] == 8) {
							createModel (name,(j-5)*150,1*150+150/2,(i-5)*150,150,30,150,"Models/redcube.data","",1, 5);
						}
						else if(gameMapTrap[i][j]==6){
							createModel("trapit",(j-5)*150,gameMap[i][j]*150-75,(i-5)*150,150,120,150,"Models/greeencude.data","",1,5);
							int random=rand()%7;
							if(random<=1){
								name.append(convertInt(i)+convertInt(j));
								createModel("danger",(j-5)*150,gameMap[i][j]*150,(i-5)*150,30,30,30,"Models/bluecude.data","",1,5);
								objects[name].angle_y=(rand()%360);
							}
							int p;
							for(p=0;p<gameMap[i][j]-1;p++){
								string name = "floorcube";
								name.append(convertInt(i)+convertInt(j)+convertInt(p));
								createModel (name,(j-5)*150,p*150+150/2,(i-5)*150,150,150,150,"Models/cube.data","",1,5);
							}
							string name = "floorcube";
							name.append(convertInt(i)+convertInt(j)+convertInt(p));
							createModel (name,(j-5)*150,p*150+150/2-75/2,(i-5)*150,150,75,150,"Models/cube.data","",1,5);
						}
						else if(gameMapTrap[i][j] == 9) {
							continue;
						}
						else {
							if((i+j)%2==0)
								createModel (name,(j-5)*150,1*150+150/2,(i-5)*150,150,30,150,"Models/cube.data","",1, 5);
							else
								createModel (name,(j-5)*150,1*150+150/2,(i-5)*150,150,30,150,"Models/oddtile.data","",1, 5);
						}
					}
				}
			}
		}
	}

	createModel("player",-750,325,-750+75,300*scale,150*scale,150*scale,"Models/cuboid.data","",1, 5); //The player's body

	playerObjects["playerhat"].angle_y=-90;
	if(!isNight)
		playerObjects["playerhat"].status=0;
	else
		playerObjects["playerhat"].status=1;
	objects["player"].angle_y=-90;

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "shader.vert", "shader.frag" );
	waterProgramID = LoadShaders ( "watershader.vert", "watershader.frag");
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void goToNextLevel(GLFWwindow* window){
	currentLevel++;
	//If next level does not exist, then the arrays will not be updated and you will be shown the last level again
	objects.clear();
	playerObjects.clear();
	player_score = 0;
	camera_fov = 1.3;
	//camera_follow=0;
	camera_follow_adjust=0;
	camera_fps=0;
	fps_head_offset=0;
	fps_head_offset_x=0;
	hortoleft = false;
	hortoright = false;
	hortoup = false;
	hortodown = false;
	verttoright = false;
	verttoleft = false;
	verttoup = false;
	verttodown = false;
	standing = false;
	roll = false;
	horhor = false;
	vertroll=0;
	gameOver = false;
	bridgeclosed = true;
	triggery = false;
	tryi = 0;
	bricksi = 0;
	player_moving_forward=0;
	player_moving_left=0;
	player_moving_backward=0;
	player_moving_right=0;
	numMoves = 0;
	timeElapsed = 0;
	start_time = 0;
	bricksFall = false;
	start_time = glfwGetTime();
	initGL(window,width,height);
}

mpg123_handle *mh;
unsigned char *buffer;
size_t buffer_size;
size_t done;
int err;

int driver;
ao_device *dev;

ao_sample_format format;
int channels, encoding;
long rate;

void audio_init() {
	/* initializations */
	ao_initialize();
	driver = ao_default_driver_id();
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size = 3000;
	buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

	/* open the file and get the decoding format */
	mpg123_open(mh, "Sounds/ebit.mp3");
	mpg123_getformat(mh, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * BITS;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev = ao_open_live(driver, &format, NULL);
}

void audio_play() {
	/* decode and play */
	if (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
		ao_play(dev, (char*) buffer, done);
	else mpg123_seek(mh, 0, SEEK_SET);
}


void audio_close() {
	/* clean up */
	//free(buffer);
	ao_close(dev);
	mpg123_close(mh);
	mpg123_delete(mh);
	mpg123_exit();
	ao_shutdown();
}

time_t old_time;

int main (int argc, char** argv)
{
	srand(time(NULL));

	old_time = time(NULL);
	audio_init();

	for(int i=0; i<10; i++) {
		for(int j=0; j<10; j++) {
			gameMap[i][j] = 0;
			gameMapTrap[i][j] = 0;
		}
	}
	width = 1400;
	height = 700;
	camera_radius=800;
	angle=135;
	eye_x = -50+camera_radius*cos(angle*M_PI/180);
	eye_y = 1100;
	eye_z = -50+camera_radius*sin(angle*M_PI/180);

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	currentLevel=-1;
	goToNextLevel(window);

	start_time = glfwGetTime();
	/* Draw in loop */
	while (!glfwWindowShouldClose(window) && !gameOver) {
		audio_play();
		// OpenGL Draw commands
		draw(window);

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();
	}

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
