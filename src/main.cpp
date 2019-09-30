#include <glad/glad.h> // Glad has to be include before glfw
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>
#include <stb_image.h>

#include "UserInterface.h"
#include "Shader.h"
#include "objLoader.h"
#include "Navigation.h"
#include <vector>
#include <string>
using namespace std;

// Window current width
unsigned int windowWidth = 800;
// Window current height
unsigned int windowHeight = 600;

// Window title
const char *windowTitle = "Tarea_3";
// Window pointer
GLFWwindow *window;

// Shader object
Shader *backfaceShader, *rayCastShader;
// Index (GPU) of the geometry buffer
unsigned int VBO;
// Index (GPU) vertex array object
unsigned int VAO;
// Index (GPU) of the texture
unsigned int textureID;

//New data
glm::mat4 MVP, model, view, projection;
COBJ *model01 = new COBJ();
navigate *mov = new navigate();
CUserInterface *userInterface;
void mouseCallback(GLFWwindow *window, int action, int button, int mods);
void scrollCallback(GLFWwindow *window, double x, double y);
void cursorPos(GLFWwindow *window, double x, double y);
void keyInput(GLFWwindow *window, int key, int scancode, int action, int mods);

bool camera = true;


//New Data - Project Volume Rendering
int XDIM=256, YDIM=256, ZDIM=256;
const int Size = XDIM*YDIM*ZDIM;

unsigned int g_texture2D;
unsigned int g_texture3D;
unsigned int g_newFrameBuf;

bool LoadVolumeFromFile(const char* fileName);
bool initFrameBuffer(GLuint texObj, GLuint texWidth, GLuint texHeight);
GLuint initFace2DTex(GLuint bfTexWidth, GLuint bfTexHeight);


void initInterface()
{
    TwInit(TW_OPENGL_CORE, NULL);
    TwWindowSize(windowWidth, windowHeight);

    userInterface = CUserInterface::Instance();
}

float vertices[] = {
	-0.5f,-0.5f,-0.5f,
	-0.5f,-0.5f, 0.5f,
	-0.5f, 0.5f, 0.5f,
	 0.5f, 0.5f,-0.5f,
	-0.5f,-0.5f,-0.5f,
	-0.5f, 0.5f,-0.5f,
	 0.5f,-0.5f, 0.5f,
	-0.5f,-0.5f,-0.5f,
	 0.5f,-0.5f,-0.5f,
	 0.5f, 0.5f,-0.5f,
	 0.5f,-0.5f,-0.5f,
	-0.5f,-0.5f,-0.5f,
	-0.5f,-0.5f,-0.5f,
	-0.5f, 0.5f, 0.5f,
	-0.5f, 0.5f,-0.5f,
	 0.5f,-0.5f, 0.5f,
	-0.5f,-0.5f, 0.5f,
	-0.5f,-0.5f,-0.5f,
	-0.5f, 0.5f, 0.5f,
	-0.5f,-0.5f, 0.5f,
	 0.5f,-0.5f, 0.5f,
	 0.5f, 0.5f, 0.5f,
	 0.5f,-0.5f,-0.5f,
	 0.5f, 0.5f,-0.5f,
	 0.5f,-0.5f,-0.5f,
	 0.5f, 0.5f, 0.5f,
	 0.5f,-0.5f, 0.5f,
	 0.5f, 0.5f, 0.5f,
	 0.5f, 0.5f,-0.5f,
	-0.5f, 0.5f,-0.5f,
	 0.5f, 0.5f, 0.5f,
	-0.5f, 0.5f,-0.5f,
	-0.5f, 0.5f, 0.5f,
	 0.5f, 0.5f, 0.5f,
	-0.5f, 0.5f, 0.5f,
	 0.5f,-0.5f, 0.5f
	};
/**
 * Handles the window resize
 * @param{GLFWwindow} window pointer
 * @param{int} new width of the window
 * @param{int} new height of the window
 * */
void resize(GLFWwindow *window, int width, int height)
{
    windowWidth = width;
    windowHeight = height;
    // Sets the OpenGL viewport size and position
    glViewport(0, 0, windowWidth, windowHeight);
    // Resize interface
    userInterface->reshape();
}
/**
 * Initialize the glfw library
 * @returns{bool} true if everything goes ok
 * */
bool initWindow()
{
    // Initialize glfw
    glfwInit();
    // Sets the Opegl context to Opengl 3.0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Creates the window
    window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);

    // The window couldn't be created
    if (!window)
    {
        std::cout << "Failed to create the glfw window" << std::endl;
        glfwTerminate(); // Stops the glfw program
        return false;
    }

    // Creates the glfwContext, this has to be made before calling initGlad()
    glfwMakeContextCurrent(window);

    // Window resize callback
    glfwSetFramebufferSizeCallback(window, resize);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetCursorPosCallback(window, cursorPos);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyInput);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    return true;
}
/**
 * Initialize the glad library
 * @returns{bool} true if everything goes ok
 * */
bool initGlad()
{
    // Initialize glad
    int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    // If something went wrong during the glad initialization
    if (!status)
    {
        std::cout << status << std::endl;
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    return true;
}
/**
 * Initialize the opengl context
 * */
void initGL()
{
    // Enables the z-buffer test
    glEnable(GL_DEPTH_TEST);
    // Sets the ViewPort
    glViewport(0, 0, windowWidth, windowHeight);
    // Sets the clear color
    glClearColor(0.f, 0.f, 0.f, 1.0f);
}
/**
 * Builds all the geometry buffers and
 * loads them up into the GPU
 * (Builds a simple triangle)
 * */
void buildGeometry()
{
    model01->load("assets/obj/quad.obj");
    glGenVertexArrays(1, &VAO);
    // Creates on GPU the ertex buffer object
    glGenBuffers(1, &VBO);
    // Binds the vertex array to set all the its properties
    glBindVertexArray(VAO);
        // Sets the buffer geometry data
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
        // Sets the vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    glBindVertexArray(0);

    projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    view = glm::lookAt(glm::vec3(4, 3, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    model = glm::mat4(1.0f);
    MVP = projection * view * model; // Remember, matrix multiplication is the other way around

}
/**
 * Loads a texture into the GPU
 * @param{const char} path of the texture file
 * @returns{unsigned int} GPU texture index
 * */
unsigned int loadTexture(const char *path)
{
    unsigned int id;
    // Creates the texture on GPU
    glGenTextures(1, &id);
    // Loads the texture
    int textureWidth, textureHeight, numberOfChannels;
    // Flips the texture when loads it because in opengl the texture coordinates are flipped
	stbi_set_flip_vertically_on_load(false);
    // Loads the texture file data
    unsigned char *data = stbi_load(path, &textureWidth, &textureHeight, &numberOfChannels, 0);
    if (data)
    {
        // Gets the texture channel format
        GLenum format;
        switch (numberOfChannels)
        {
        case 1:
            format = GL_RED;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        }

        // Binds the texture
        glBindTexture(GL_TEXTURE_2D, id);
        // Creates the texture
        glTexImage2D(GL_TEXTURE_2D, 0, format, textureWidth, textureHeight, 0, format, GL_UNSIGNED_BYTE, data);
        // Creates the texture mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set the filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cout << "ERROR:: Unable to load texture " << path << std::endl;
        glDeleteTextures(1, &id);
    }
    // We dont need the data texture anymore because is loaded on the GPU
    stbi_image_free(data);

    return id;
}
/**
 * Loads a texture into the GPU
 * @param{vector<std::string>} path of the skybox file
 * @returns{unsigned int} GPU skybox index
 * */
/**
 * Initialize everything
 * @returns{bool} true if everything goes ok
 * */
bool init()
{
    // Initialize the window, and the glad components
    if (!initWindow() | !initGlad())
        return false;

    // Initialize the opengl context
    initGL();

    // Init interface
    initInterface();

    // Loads the shader
    backfaceShader = new Shader("assets/shaders/backFaceOut.vert", "assets/shaders/backFaceOut.frag");
    rayCastShader = new Shader("assets/shaders/rayCast.vert", "assets/shaders/rayCast.frag");
    
    //Init framebuffers aux
    g_texture2D= initFace2DTex(windowWidth, windowHeight);
    LoadVolumeFromFile("assets/volumes/bonsai.raw");
    initFrameBuffer(g_texture2D, windowWidth, windowHeight);
    
    // Loads all the geometry into the GPU
    buildGeometry();
    // Loads the texture into the GPU

    return true;
}
/**
 * Process the keyboard input
 * There are ways of implementing this function through callbacks provide by
 * the GLFW API, check the GLFW documentation to find more
 * @param{GLFWwindow} window pointer
 * */
void processKeyboardInput(GLFWwindow *window)
{
    // Checks if the escape key is pressed
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        // Tells glfw to close the window as soon as possible
        glfwSetWindowShouldClose(window, true);

    // Checks if the r key is pressed
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        // Reloads the shader
        delete backfaceShader;
        delete rayCastShader;
        backfaceShader = new Shader("assets/shaders/backFaceOut.vert", "assets/shaders/backFaceOut.frag");
        rayCastShader = new Shader("assets/shaders/rayCast.vert", "assets/shaders/rayCast.frag");
    }
}
/**
 * Render Function
 * */
void render()
{
    // Clears the color and depth buffers from the frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, g_newFrameBuf);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, windowWidth, windowHeight);

    //Keyboard monitor
    if (camera)
        mov->movements(window);

    projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);;
    view = mov->getViewMatrix();
    model = glm::mat4(1.0f);

    MVP = projection * view * model; // Remember, matrix multiplication is the other way around

    //Model Properties
    backfaceShader->use();
    backfaceShader->setMat4("model", model);
    backfaceShader->setMat4("view", view);
    backfaceShader->setMat4("projection", projection);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDisable(GL_CULL_FACE);

    //Now render the cube's front
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, windowWidth, windowHeight);

    //Keyboard monitor
    if (camera)
        mov->movements(window);

    projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);;
    view = mov->getViewMatrix();
    model = glm::mat4(1.0f);

    MVP = projection * view * model; // Remember, matrix multiplication is the other way around

    //Model Properties
    rayCastShader->use();
    rayCastShader->setMat4("MVP", MVP);
    rayCastShader->setVec2("screenSize", glm::vec2(windowWidth, windowHeight));
    rayCastShader->setInt("bfTexture", 1);
    rayCastShader->setInt("volTexture", 2);
    
    glActiveTexture(GL_TEXTURE1); // activate the texture unit first before binding texture
	glBindTexture(GL_TEXTURE_2D, g_texture2D);
    glActiveTexture(GL_TEXTURE2); // activate the texture unit first before binding texture
	glBindTexture(GL_TEXTURE_3D, g_texture3D);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);


}
/**
 * App main loop
 * */
void update()
{
    // Loop until something tells the window, that it has to be closed
    while (!glfwWindowShouldClose(window))
    {
        // Checks for keyboard inputs
        processKeyboardInput(window);

        // Renders everything
        render();
		
        if (!camera)
			TwDraw();

		// Swap the buffer
		glfwSwapBuffers(window);

        // Check and call events
        glfwPollEvents();

    }
}
/**
 * App starting point
 * @param{int} number of arguments
 * @param{char const *[]} running arguments
 * @returns{int} app exit code
 * */
int main(int argc, char const *argv[])
{
    // Initialize all the app components
    if (!init())
    {
        // Something went wrong
        std::cin.ignore();
        return -1;
    }

    std::cout << "=====================================================" << std::endl
              << "        Press Escape to close the program            " << std::endl
              << "=====================================================" << std::endl;

    // Starts the app main loop
    update();

    // Deletes the texture from the gpu
    for (int i = 0; i < 1; i++)
    {
        glDeleteTextures(1, &textureID);
        // Deletes the vertex array from the GPU
        glDeleteVertexArrays(1, &VAO);
        // Deletes the vertex object from the GPU
        glDeleteBuffers(1, &VBO);
    }
    // Destroy the shader
    delete backfaceShader;
    delete rayCastShader;


    //Stops the Tw window
    TwTerminate();
    // Stops the glfw program
    glfwTerminate();

    return 0;
}

void mouseCallback(GLFWwindow *window, int action, int button, int mods)
{

    TwMouseButtonID button1 = (TwMouseButtonID)button;
    TwMouseAction action1 = (TwMouseAction)action;
    if (TwMouseButton(action1, button1))
        return;
}
void scrollCallback(GLFWwindow *window, double x, double y)
{
    if (camera)
        mov->scroll(x, y);
}
void cursorPos(GLFWwindow *window, double x, double y)
{
    if (TwMouseMotion(int(x), int(y)))
        return;

    if (camera)
        mov->mouse(window, x, y);
}
void keyInput(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (TwKeyPressed(key, action))
        return;

}

bool LoadVolumeFromFile(const char* fileName) {
    FILE *pFile = fopen(fileName,"rb");
    if(NULL == pFile) {
        return false;
    }
    GLubyte* pVolume=new GLubyte[Size];
    fread(pVolume,sizeof(GLubyte),Size,pFile);
    fclose(pFile);

    //load data into a 3D texture
    glGenTextures(1, &g_texture3D);
    glBindTexture(GL_TEXTURE_3D, g_texture3D);

    // set the texture parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, XDIM, YDIM, ZDIM, 0, GL_RED, GL_UNSIGNED_BYTE, pVolume);
    delete [] pVolume;
    return true;
}

bool initFrameBuffer(GLuint texObj, GLuint texWidth, GLuint texHeight)
{
    // create a depth buffer for our framebuffer
    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texWidth, texHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    // attach the texture and the depth buffer to the framebuffer
    glGenFramebuffers(1, &g_newFrameBuf);
    glBindFramebuffer(GL_FRAMEBUFFER, g_newFrameBuf);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texObj, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::Framebuffer configuration went wrong" << std::endl;
        glfwTerminate();
		return false;
	}

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

GLuint initFace2DTex(GLuint bfTexWidth, GLuint bfTexHeight)
{
    GLuint backFace2DTex;
    glGenTextures(1, &backFace2DTex);
    glBindTexture(GL_TEXTURE_2D, backFace2DTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bfTexWidth, bfTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    return backFace2DTex;
}