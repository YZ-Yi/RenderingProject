//Alejandro Garcia CPSC 591/691 A#1 F2020 Boilerplate

//Code heavily influenced by https://learnopengl.com/ tutorials

#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include <iostream>
#include <vector>
#include <list>

#include "imGui/imgui.h"
#include "imGui_impl_glfw.h"
#include "imGui_impl_opengl3.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);



// window settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera settings
Camera camera(glm::vec3(0.0f, 2.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Object rotation settings
glm::mat4 rotation;
float rotSpeed = 2.5f;

//outline stuff
bool outlineFlag = false;
float creaseAngle = 90.f;
bool creaseFlag = true;
bool silhouetteFlag = true;
bool resetFlag = false;

//texture stuff
float rVal = 1.f;
glm::vec3 objColor;
ImVec4 imgui_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
float opacity = 0.75f;
float hVal = 0.01f;
float sVal = 0.01f;
float vVal = 0.01f;

//light stuff
glm::vec3 lightPositions = glm::vec3(0.f, 0.f, 10.f);

//rendering stuff
int shaderNum = 0;
int materialNum = 0;
int activeModel = 0;

//adjacent list
struct Node {
    unsigned int v;                     //vertex id(index)
    unsigned int f = 0;                 //front face bit
    unsigned int b = 0;                 //back face bit
    std::vector<glm::vec3> norms;       //normals of 2 faces
};

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    const char* glsl_version = "#version 130";

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CPSC 591 Project", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("../shaders/texture.vs.glsl", "../shaders/texture.fs.glsl");
    Shader ourShader2("../shaders/outline.vs.glsl", "../shaders/outline.fs.glsl");


    // load model(s), default model is vase.obj, can load multiple at a time
    // -----------
    Model ourModel("../models/teapot.obj");
    Model ourModel0("../models/teapot.obj");
    Model ourModel1("../models/bunny.obj");
    //Model ourModel("../models/head.obj");
    //Model ourModel("../models/boss.obj");
    Model ourModel2("../models/engine.obj");
    
    std::cout << "Select active model:" << std::endl;
    std::cout << "1.teapot 2.bunny 3.engine" << std::endl;
    std::cin >> activeModel;

    switch (activeModel) {
    case 1:
        ourModel = ourModel0;
        break;
    case 2:
        ourModel = ourModel1;
        break;
    case 3:
        ourModel = ourModel2;
    }

    //edge buffer stuff
    //-------------------------------------------------------------------------------
    std::vector<std::list<Node>> edgeBuffer;
    std::vector<glm::vec3> vertices;

    for (unsigned int j = 0; j < ourModel.meshes[0].indices.size(); j++) {
        list<Node> l;
        edgeBuffer.push_back(l);
    }

    for (unsigned int j = 0; j < ourModel.meshes[0].indices.size() / 3; j++)
    {
        size_t numTriangles = ourModel.meshes[0].indices.size() / 3;
        size_t numIndices = ourModel.meshes[0].indices.size();
        size_t numVertices = ourModel.meshes[0].vertices.size();

        //std::cout << "Vertices " << numVertices << std::endl;
       // std::cout << "Indices " << numIndices << std::endl;

        unsigned int i0, i1, i2;
        Vertex v0, v1, v2;
        //Get indices of this triangle
        i0 = ourModel.meshes[0].indices[j * 3 + 0];
        i1 = ourModel.meshes[0].indices[j * 3 + 1];
        i2 = ourModel.meshes[0].indices[j * 3 + 2];

        
        //sort indices
        if (i0 > i1) {
            unsigned int temp = i0;
            i0 = i1;
            i1 = temp;
        }
        if (i0 > i2) {
            unsigned int temp = i0;
            i0 = i2;
            i2 = temp;
        }
        if (i1 > i2) {
            unsigned int temp = i1;
            i1 = i2;
            i2 = temp;
        }
        //cout << i0 << " " << i1 << " " << i2 << endl;


        v0 = ourModel.meshes[0].vertices[i0];
        v1 = ourModel.meshes[0].vertices[i1];
        v2 = ourModel.meshes[0].vertices[i2];

        //construct edge buffer
        int isVIn = false;
        for (auto it = edgeBuffer[i0].begin(); it != edgeBuffer[i0].end(); ++it) {
            if ((*it).v == i1) {
                isVIn = true;
                break;
            }
        }
        if (isVIn == false) {
            Node n;
            n.v = i1;
            edgeBuffer[i0].push_back(n);
        }

        isVIn = false;
        for (auto it = edgeBuffer[i0].begin(); it != edgeBuffer[i0].end(); ++it) {
            if ((*it).v == i2) {
                isVIn = true;
                break;
            }
        }
        if (isVIn == false) {
            Node n;
            n.v = i2;
            edgeBuffer[i0].push_back(n);
        }

        isVIn = false;
        for (auto it = edgeBuffer[i1].begin(); it != edgeBuffer[i1].end(); ++it) {
            if ((*it).v == i2) {
                isVIn = true;
                break;
            }
        }
        if (isVIn == false) {
            Node n;
            n.v = i2;
            edgeBuffer[i1].push_back(n);

        }

    }

    for (unsigned int j = 0; j < ourModel.meshes[0].indices.size() / 3; j++) {
        unsigned int i0, i1, i2;
        Vertex v0, v1, v2;
        //Get indices of this triangle
        i0 = ourModel.meshes[0].indices[j * 3 + 0];
        i1 = ourModel.meshes[0].indices[j * 3 + 1];
        i2 = ourModel.meshes[0].indices[j * 3 + 2];
        //Get vertices of this triangle using indices
        v0 = ourModel.meshes[0].vertices[i0];
        v1 = ourModel.meshes[0].vertices[i1];
        v2 = ourModel.meshes[0].vertices[i2];

        glm::vec3 a = v1.Position - v0.Position;
        glm::vec3 b = v2.Position - v1.Position;
        glm::vec3 triangleNormal = glm::normalize(glm::cross(a, b));

        //sort indices
        if (i0 > i1) {
            unsigned int temp = i0;
            i0 = i1;
            i1 = temp;
        }
        if (i0 > i2) {
            unsigned int temp = i0;
            i0 = i2;
            i2 = temp;
        }
        if (i1 > i2) {
            unsigned int temp = i1;
            i1 = i2;
            i2 = temp;
        }
        //cout << i0 << " " << i1 << " " << i2 << endl;


        v0 = ourModel.meshes[0].vertices[i0];
        v1 = ourModel.meshes[0].vertices[i1];
        v2 = ourModel.meshes[0].vertices[i2];

        //normals for calculating creases
        for (auto it = edgeBuffer[i0].begin(); it != edgeBuffer[i0].end(); ++it) {
            if ((*it).v == i1) {
                it->norms.push_back(triangleNormal);
                break;
            }
        }

        for (auto it = edgeBuffer[i0].begin(); it != edgeBuffer[i0].end(); ++it) {
            if ((*it).v == i2) {
                it->norms.push_back(triangleNormal);
                break;
            }
        }

        for (auto it = edgeBuffer[i1].begin(); it != edgeBuffer[i1].end(); ++it) {
            if ((*it).v == i2) {
                it->norms.push_back(triangleNormal);
                break;
            }
        }
    }


  
    //-------------------------------------------------------------------------------
    //edge buffer stuff ends here


    //texture stuff
    std::string texPaths[] = { "../textures/material1.png", "../textures/material2.png" };
    unsigned int textures[2];

    //The following block-of-code was adapted from code
    //I found at the following URL:
    //https://learnopengl.com/Getting-started/Textures
    //load and create texture
    //-------------------------------------------------------------------------------
    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).

    for (int i = 0; i < 2; ++i) {
        // ---------
        glGenTextures(1, &textures[i]);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // load image, create texture and generate mipmaps
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        unsigned char* data = stbi_load(texPaths[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture " << i << std::endl;
        }
        stbi_image_free(data);

    }


    //-------------------------------------------------------------------------------------
    //end of texture stuff



    //enable this to draw in wireframe
    // -----------
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //imGui stuff
    //---------------------------------------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Imgui setting END
    //----------------------------------------------------------------------------------

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


    // Infinite render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {

        

        // per-frame time logic
       // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // Clear screen
        // ------
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[materialNum]);

        // don't forget to enable shader before setting uniforms
        ourShader.use();
        // This shader will have filled-in triangles
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        //LIGHTS
        glm::vec3 lightIntensities = glm::vec3(1.f, 1.f, 1.f);
        ourShader.setVec3("lightPositions", lightPositions);
        ourShader.setVec3("lightIntensities", lightIntensities);


        //color
        objColor = glm::vec3(imgui_color.x, imgui_color.y, imgui_color.z);

        if (resetFlag) {
            camera = Camera(glm::vec3(0.0f, 2.0f, 3.0f));
            rotation = glm::mat4(1);
            resetFlag = false;
        }

        if (shaderNum)
            materialNum = 1;
        else
            materialNum = 0;


        //CAMERA
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::vec3 viewPos = camera.Position;
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setVec3("viewPos", viewPos);
        ourShader.setFloat("rVal", rVal);

        //ACTION
        glm::mat4 model = rotation;// The model transformation of the mesh (controlled through arrows)
        //model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	// Head
        //if(activeModel == 2)
          //  model = glm::scale(model, glm::vec3(0.001f, 0.001f, 0.001f));	

        ourShader.setMat4("model", model);
        ourShader.setVec3("objectColor", objColor);
        ourShader.setFloat("opacity", opacity);
        ourShader.setFloat("hVal", hVal);
        ourShader.setFloat("sVal", sVal);
        ourShader.setFloat("vVal", vVal);
        ourShader.setInt("shaderNum", shaderNum);

        ourModel.Draw(ourShader);


        //outline stuff
        //------------------------------------------------------------------------------------------------------------------------------
        if (outlineFlag) {
            // Set the current active shader to shader #2
            ourShader2.use();
            // This shader will not have filled-in triangles
            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            //Send vertex shader data
            ourShader2.setMat4("projection", projection);
            ourShader2.setMat4("view", view);
            ourShader2.setMat4("model", model);

            //reset edge buffer and other
            vertices.clear();
            for (int j = 0; j < ourModel.meshes[0].indices.size(); ++j) {
                for (auto it = edgeBuffer[j].begin(); it != edgeBuffer[j].end(); ++it) {
                    (*it).b = 0;
                    (*it).f = 0;
                }
            }

            //For each triangle, calculate edge buffer
            size_t numTriangles = ourModel.meshes[0].indices.size() / 3;
            for (int j = 0; j < numTriangles; j++)
            {
                unsigned int i0, i1, i2;
                Vertex v0, v1, v2;
                //Get indices of this triangle
                i0 = ourModel.meshes[0].indices[j * 3 + 0];
                i1 = ourModel.meshes[0].indices[j * 3 + 1];
                i2 = ourModel.meshes[0].indices[j * 3 + 2];

                //Get vertices of this triangle using indices
                v0 = ourModel.meshes[0].vertices[i0];
                v1 = ourModel.meshes[0].vertices[i1];
                v2 = ourModel.meshes[0].vertices[i2];

                //Get two edges of the triangle to compute triangle normal
                glm::vec3 a = v1.Position - v0.Position;
                glm::vec3 b = v2.Position - v1.Position;
                glm::vec3 triangleNormal = glm::normalize(glm::cross(a, b));
                //triangleNormal = glm::mat3(rotation) * triangleNormal;
                triangleNormal = glm::normalize(glm::mat3(model) * triangleNormal);

                //sort indices
                if (i0 > i1) {
                    unsigned int temp = i0;
                    i0 = i1;
                    i1 = temp;
                }
                if (i0 > i2) {
                    unsigned int temp = i0;
                    i0 = i2;
                    i2 = temp;
                }
                if (i1 > i2) {
                    unsigned int temp = i1;
                    i1 = i2;
                    i2 = temp;
                }


                //Get vertices of this triangle using indices
                v0 = ourModel.meshes[0].vertices[i0];
                v1 = ourModel.meshes[0].vertices[i1];
                v2 = ourModel.meshes[0].vertices[i2];


                //Compute centroid of the triangle
                glm::vec3 triangleCentroid = (v0.Position + v1.Position + v2.Position) / 3.f;
                // triangleCentroid = glm::mat3(rotation) * triangleCentroid;
                triangleCentroid = glm::mat3(model) * triangleCentroid;

                glm::vec3 viewDirection = glm::normalize(triangleCentroid - viewPos);
                
                //std::cout << i0 << " " << i1 << " " << i2 << " ";
                //std::cout << "(" << triangleNormal.x << ", " << triangleNormal.y << ", " << triangleNormal.z << ") " << std::endl;


                //using algorithm from lec
                //If the dotproduct between the centroid and the viewDirection is , this triangle is front facing
                if (glm::dot(triangleNormal, viewDirection) <= 0.0f)
                {

                    //If this triangle is front facing, we add its 3 vertices to the vertices array
                    //Note that for your assignment, you need to reset the vertices array each frame, and compute all of this inside the infinite loop below


                    for (auto it = edgeBuffer[i0].begin(); it != edgeBuffer[i0].end(); ++it) {
                        if ((*it).v == i1) {
                            (*it).f = (*it).f ^ 1;
                            break;
                        }
                    }
                    
                    for (auto it = edgeBuffer[i0].begin(); it != edgeBuffer[i0].end(); ++it) {
                        if ((*it).v == i2) {
                            (*it).f = (*it).f ^ 1;
                            break;
                        }
                    }

                   

                    for (auto it = edgeBuffer[i1].begin(); it != edgeBuffer[i1].end(); ++it) {
                        if ((*it).v == i2) {
                            (*it).f = (*it).f ^ 1;
                            break;
                        }
                    }

                }
                //else, this triangle is back facing
                else {
                    for (auto it = edgeBuffer[i0].begin(); it != edgeBuffer[i0].end(); ++it) {
                        if ((*it).v == i1) {
                            (*it).b = (*it).b ^ 1;
                            break;
                        }
                    }

                    for (auto it = edgeBuffer[i0].begin(); it != edgeBuffer[i0].end(); ++it) {
                        if ((*it).v == i2) {
                            (*it).b = (*it).b ^ 1;
                            break;
                        }
                    }

                    for (auto it = edgeBuffer[i1].begin(); it != edgeBuffer[i1].end(); ++it) {
                        if ((*it).v == i2) {
                            (*it).b = (*it).b ^ 1;
                            break;
                        }

                    }

                }


                
            }

            //debug
            /*
            std::cout << "EDGEBUFFER" << std::endl;
            for (int j = 0; j < 10; ++j) {
                for (auto& it : edgeBuffer[j]) {
                    cout << j << " " << it.v << " " << it.f << " " << it.b << endl;
                    std::cout << j << " " << it.v;
                    for (auto norm : it.norms)
                        std::cout << "(" << norm.x << ", " << norm.y << ", " << norm.z << ") ";
                    std::cout << std::endl;
                }
            }
            */

            if (silhouetteFlag) {
                //if the line is silhouette, add it to the vertices
                for (int j = 0; j < edgeBuffer.size(); ++j) {
                    Vertex v0, v1;
                    for (auto it = edgeBuffer[j].begin(); it != edgeBuffer[j].end(); ++it) {
                        //if front bit and back bit are both 1, which means it is a silhooute
                        if ((*it).b && (*it).f) {
                            v0 = ourModel.meshes[0].vertices[j];
                            v1 = ourModel.meshes[0].vertices[(*it).v];
                            //cout << j << " "  << (*it).v << " " << (*it).b << " " << (*it).f << endl;
                            vertices.push_back(v0.Position);
                            vertices.push_back(v1.Position);
                        }
                    }
                }

               
            }
           
            if (creaseFlag) {
                //if the line is crease, add it to the creaseVertices
                for (int j = 0; j < edgeBuffer.size(); ++j) {
                    Vertex v0, v1;
                    for (auto it = edgeBuffer[j].begin(); it != edgeBuffer[j].end(); ++it) {
                        //if it's front face
                        if (!(*it).f && !(*it).b) {
                            v0 = ourModel.meshes[0].vertices[j];
                            v1 = ourModel.meshes[0].vertices[(*it).v];
                            if (it->norms.size() >= 2) {
                                glm::vec3 norm1 = it->norms[0];
                                glm::vec3 norm2 = it->norms[1];
                                if (glm::dot(norm1, norm2) <= glm::cos(glm::radians(180.f - creaseAngle))) {
                                    vertices.push_back(v0.Position);
                                    vertices.push_back(v1.Position);
                                }
                            }
                        }
                    }
                }

                
            }
           

            //std::cout << vertices.size() << std::endl;
            //std::cout << edgeBuffer.size() << std::endl;

            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);

            glBindVertexArray(0);


            //draw triangles with line thickness 3.0
            glBindVertexArray(VAO);
            //glPointSize(lineWidth);
            glEnable(GL_LINE_SMOOTH);
            glLineWidth(3.f);
            //glDrawArrays(GL_POINTS, 0, vertices.size());

            glDrawArrays(GL_LINES, 0, vertices.size());
            glBindVertexArray(0);

        }


        //---------------------------------------------------------------------------------------------------------
        //end of outline stuff

        if (!outlineFlag) {
            silhouetteFlag = false;
            creaseFlag = false;
        }

        ImGui_ImplOpenGL3_NewFrame();

        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {

            ImGui::Begin("Interface");                          // Create a window called "Hello, world!" and append into it.

            if (ImGui::Button("Reset Camera And Model"))
                resetFlag = true;

            ImGui::Checkbox("Outline", &outlineFlag);
            ImGui::Checkbox("Silhouette", &silhouetteFlag);
            ImGui::Checkbox("Crease", &creaseFlag);

            ImGui::SliderFloat("Dihedral Angle", &creaseAngle, 0.0f, 150.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

            ImGui::RadioButton("near-silhouette", &shaderNum, 0); ImGui::SameLine();
            ImGui::RadioButton("metal", &shaderNum, 1);

            //texture stuff
            ImGui::ColorEdit3("object color", (float*)&imgui_color); // Edit 3 floats representing a color
            ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f);
            ImGui::SliderFloat("r", &rVal, 0.0f, 10.0f);
            ImGui::SliderFloat("h", &hVal, 0.0f, 0.1f);

            ImGui::End();

        }


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //uncomment this to pause per frame
        /*
        int m;
        std::cin >> m;
        while (m != 1) {
            std:: cin >> m;
        }
        */
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void ProcessKeyboard(Camera_Movement dir, float deltaTime)
{
    float rotVelocity = rotSpeed * deltaTime;
    if (dir == FORWARD)
    {
        rotation = glm::rotate(rotation, -rotVelocity, glm::vec3(1.0, 0.0, 0.0));
    }
    if (dir == BACKWARD)
    {
        rotation = glm::rotate(rotation, rotVelocity, glm::vec3(1.0, 0.0, 0.0));
    }
    if (dir == LEFT)
    {
        rotation = glm::rotate(rotation, -rotVelocity, glm::vec3(0.0, 1.0, 0.0));
    }
    if (dir == RIGHT)
    {
        rotation = glm::rotate(rotation, rotVelocity, glm::vec3(0.0, 1.0, 0.0));
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //Camera controls
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    //Model controls
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        ProcessKeyboard(RIGHT, deltaTime);

    //Light position controls
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        lightPositions += glm::vec3(0.f, 0.1f, 0.f);
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        lightPositions += glm::vec3(0.f, -0.1f, 0.f);
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        lightPositions += glm::vec3(-0.1f, 0.f, 0.f);
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        lightPositions += glm::vec3(0.1f, 0.f, 0.f);
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        lightPositions += glm::vec3(0.f, 0.f, 0.1f);
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        lightPositions += glm::vec3(0.f, 0.f, -0.1f);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}