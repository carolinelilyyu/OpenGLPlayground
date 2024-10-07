#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <iostream>
#include <vector>
//Globals
int gScreenWidth = 640;
int gScreenHeight = 480;
SDL_Window* gGraphicsApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

//VAO
GLuint gVertexArrayObject = 0;
//VBO
GLuint gVertexBufferObject = 0;

//bring in. position the vertex
const std::string gVertexShaderSource =
    "#version 410 core\n"
    "layout(location = 0) in vec4 position;\n"  // Explicitly bind to location 0
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(position.x, position.y, position.z, position.w);\n"
    "}\n";

//output final color for fragment/ pixel. one out
const std::string gFragmentShaderSource =
    "#version 410 core\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "   color = vec4(1.0f, 0.5f, 0.0f, 1.0f);\n"  
    "}\n";


bool gQuit = false; //if true then quit
GLuint CompileShader(GLuint type, 
                        const std::string& source){
    GLuint shaderObject;
    if(type == GL_VERTEX_SHADER){
        shaderObject = glCreateShader(GL_VERTEX_SHADER);
    }else if(type == GL_FRAGMENT_SHADER){
        shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    }
    const char* src = source.c_str();
    glShaderSource(shaderObject, 1, &src, nullptr);
    glCompileShader(shaderObject);
    return shaderObject;
}

//it's job is to take whatever the result of compiling vs and fs and assembling them
GLuint CreateShaderProgram(const std::string& vertexshadersource, 
                        const std::string&  fragmentshadersource){
    GLuint programObject = glCreateProgram();

    GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexshadersource);
    GLuint myFragmentShader =  CompileShader(GL_FRAGMENT_SHADER, fragmentshadersource);

    glAttachShader(programObject, myVertexShader);
    glAttachShader(programObject, myFragmentShader);
    glLinkProgram(programObject);

    //validate our program
    glValidateProgram(programObject);
    //gldetchshader, gldeleteshader
    return programObject;
}

// Need somewhere to hold graphics pipeline; currently above is unsigned integers
//Program Object (for our shaders)
GLuint gGraphicsPipelineShaderProgram = 0;
void CreateGraphicsPipeline(){
    gGraphicsPipelineShaderProgram = CreateShaderProgram(gVertexShaderSource, gFragmentShaderSource);
}

void GetOpenGLVersionInfo(){
    //you must download GLAD in order for these to work
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void VertexSpecification(){
    //lives on the CPU
    const std::vector<GLfloat> vertexPosition 
    {
        //x y z
        -0.8f, -0.8f, 0.0f, //vertex 1
        0.8f, -0.8, 0.0f, //vertex 2
        0.0f, 0.8f, 0.0f //vertex 3
    };

    // Start settings things up on the GPU

    //generate vao
    glGenVertexArrays(1, &gVertexArrayObject);
    //bind to it
    glBindVertexArray(gVertexArrayObject);

    //generate our vbo
    glGenBuffers(1, &gVertexBufferObject);
    //bind our buffer
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
    //load data. must match the type from above (GL_ARRAY_BUFFER)
    glBufferData(GL_ARRAY_BUFFER, 
                vertexPosition.size() * sizeof(GLfloat),
                vertexPosition.data(),
                GL_STATIC_DRAW //just draw a triangle
    );
    //how do we get to that attribute?
    glEnableVertexAttribArray(0);
    //layout. index must match above
    glVertexAttribPointer(0,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        0,
                        (void*) 0
    );
    //this is how we close things. we're done, don't want to bind to anything
    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
}

void InitializeProgram(){
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cout << "SDL could not initialize video subsystem" << std::endl;
        exit(1);
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    gGraphicsApplicationWindow = SDL_CreateWindow("OpenGL Window", 0, 0, gScreenWidth, gScreenHeight, SDL_WINDOW_OPENGL);
    if(gGraphicsApplicationWindow == nullptr){
        std::cout << "SDL_Window was not able to be created" << std::endl;
        exit(1);
    }
    gOpenGLContext = SDL_GL_CreateContext(gGraphicsApplicationWindow);
    if(gOpenGLContext==nullptr){
        std::cout << "OpenGL context not available" << std::endl;
        exit(1);
    }

    //initialize the GLAD library
    if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
        std::cout << "glad was not initialized" << std::endl;
        exit(1);
    }
    GetOpenGLVersionInfo();
}

void Input(){
    SDL_Event e;
    while(SDL_PollEvent(&e) != 0){
        if(e.type == SDL_QUIT){
            std::cout << "Goodbye" << std::endl;
            gQuit = true;
        }
    }
}

void Predraw(){
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glViewport(0,0, gScreenWidth, gScreenHeight);
    //makes bg yellow
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    //makes sure to have the above work
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glUseProgram(gGraphicsPipelineShaderProgram);
}

void Draw(){
    //which vao to use
    glBindVertexArray(gVertexArrayObject);
    //which buffer
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
    glDrawArrays(GL_TRIANGLES, 0,3);
}

void MainLoop(){
    while(!gQuit){
        Input();
        Predraw();
        Draw();

        // Update the screen
        SDL_GL_SwapWindow(gGraphicsApplicationWindow);
    }
}

void Clean(){
    SDL_DestroyWindow(gGraphicsApplicationWindow);
    SDL_Quit();
}

int main(){

    InitializeProgram();

    VertexSpecification();

    CreateGraphicsPipeline();

    MainLoop();

    Clean();

    return 0;
}