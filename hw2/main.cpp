#pragma optimize("", off)

#include <iostream>
#include <vector>
#include <chrono>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <GL/glew.h>

// Imgui + bindings
#include "imgui.h"
#include "../bindings/imgui_impl_glfw.h"
#include "../bindings/imgui_impl_opengl3.h"

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>


#define TINYOBJLOADER_IMPLEMENTATION

#include <tiny_obj_loader.h>


// STB, load images
#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

// Math constant and routines for OpenGL interop
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl_shader.h"

#include <filesystem>

namespace fs = std::filesystem;

#include <vector>

static void glfw_error_callback(int error, const char *description) {
    throw std::runtime_error(fmt::format("Glfw Error {}: {}\n", error, description));
}

void create_cubemap(GLuint &vbo, GLuint &vao, GLuint &ebo) {
    const float vertices[] = {
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
    };
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
}

int load_object(char const *filename, GLuint &vao, GLuint &vbo) {
    spdlog::info("Loading model: {}", filename);

    tinyobj::attrib_t attrib;
    std::vector <tinyobj::shape_t> shapes;
    std::vector <tinyobj::material_t> materials;

    auto parent = fs::absolute(fs::path(filename)).parent_path();
    std::string err;
    const bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, parent.u8string().c_str(), true);

    if (!ret)
        throw std::runtime_error(fmt::format("Model load error: {}", err));


    std::vector<float> vertices;

// Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                auto vx = attrib.vertices[3 * idx.vertex_index + 0];
                auto vy = attrib.vertices[3 * idx.vertex_index + 1];
                auto vz = attrib.vertices[3 * idx.vertex_index + 2];
                auto nx = attrib.normals[3 * idx.normal_index + 0];
                auto ny = attrib.normals[3 * idx.normal_index + 1];
                auto nz = attrib.normals[3 * idx.normal_index + 2];
                vertices.push_back(vx);
                vertices.push_back(vy);
                vertices.push_back(vz);

                vertices.push_back(nx);
                vertices.push_back(ny);
                vertices.push_back(nz);
            }
            index_offset += fv;
        }
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &(vertices[0]), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return vertices.size() / 6;
}

unsigned int load_cubemap() {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    std::string faces[] = {
            "skybox/right.jpg",
            "skybox/left.jpg",
            "skybox/top.jpg",
            "skybox/bottom.jpg",
            "skybox/front.jpg",
            "skybox/back.jpg",
    };

    int width, height, nrChannels;
    for (unsigned int i = 0; i < 6; i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

float zoom = 100.0f;

float yaw = 90.0f;
float pitch = 0.0f;

int prev_mouse_button = -1;
float last_x;
float last_y;
glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);

void cursor_callback(GLFWwindow *window, double xpos, double ypos) {
    int button_action = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    if (button_action == GLFW_PRESS && prev_mouse_button != GLFW_PRESS) {
        last_x = x;
        last_y = y;
    }
    prev_mouse_button = button_action;
    if (button_action != GLFW_PRESS) {
        return;
    }

    float xoffset = x - last_x;
    float yoffset = y - last_y;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    if (yaw > 360) {
        yaw -= 360;
    }
    if (yaw < 0) {
        yaw += 360;
    }

    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    last_x = x;
    last_y = y;
//    std::cout << direction.x << " " << direction.y << " " << direction.z << " " << yaw << " " << pitch << std::endl;
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    zoom -= (float) yoffset;
    if (zoom < 10.0f)
        zoom = 10.0f;
    if (zoom > 150.0f)
        zoom = 150.0f;
}

int main(int, char **) {
    try {
        // Use GLFW to create a simple window
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            throw std::runtime_error("glfwInit error");

        // GL 3.3 + GLSL 330
        const char *glsl_version = "#version 330";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

        // Create window with graphics context
        GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui - Conan", NULL, NULL);
        if (window == NULL)
            throw std::runtime_error("Can't create glfw window");

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        glfwSetCursorPosCallback(window, cursor_callback);
        glfwSetScrollCallback(window, scroll_callback);

        if (glewInit() != GLEW_OK)
            throw std::runtime_error("Failed to initialize glew");

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        GLuint model_vao, model_vbo;
        int triangles_num = load_object("bunny.obj", model_vao, model_vbo);

        GLuint vbo, vao, ebo;
        create_cubemap(vbo, vao, ebo);

        // init shader
        shader_t cubemap_shader("cubemap-shader.vs", "cubemap-shader.fs");
        shader_t bunny_shader("model.vs", "model.fs");

        // Setup GUI context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
        ImGui::StyleColorsDark();

        unsigned int cubemap = load_cubemap();
        glEnable(GL_DEPTH_TEST);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            // Get windows size
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);

            // Gui start new frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Settings");
            static float refractive_index = 2;
            ImGui::SliderFloat("refractive index", &refractive_index, 1, 5);
            ImGui::End();

            glViewport(0, 0, display_w, display_h);
            glDepthMask(GL_TRUE);
            glClearColor(0.3, 0.3, 0.3, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // cubemap
            cubemap_shader.use();
            glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 0), -direction, glm::vec3(0, 1, 0));
            glm::mat4 projection = glm::perspective(glm::radians(zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT,
                                                    0.1f, 100.0f);

            cubemap_shader.set_uniform("view", glm::value_ptr(view));
            cubemap_shader.set_uniform("projection", glm::value_ptr(projection));

            glBindVertexArray(vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
            glDepthMask(GL_FALSE);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // object
            auto model = glm::scale(glm::mat4(1), glm::vec3(0.30, 0.30, 0.30));
            auto object_view = glm::lookAt(direction, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

            bunny_shader.use();
            bunny_shader.set_uniform("projection", glm::value_ptr(projection));
            bunny_shader.set_uniform("model", glm::value_ptr(model));
            bunny_shader.set_uniform("view", glm::value_ptr(object_view));
            bunny_shader.set_uniform("cubemap", int(0));
            bunny_shader.set_uniform("refractive_index", refractive_index);
            bunny_shader.set_uniform("camera_position", direction.x, direction.y, direction.z);

            glBindVertexArray(model_vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
            glDepthMask(GL_TRUE);
            glDrawArrays(GL_TRIANGLES, 0, triangles_num);
            glBindVertexArray(0);

            // Generate gui render commands
            ImGui::Render();

            // Execute gui render commands using OpenGL backend
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Swap the backbuffer with the frontbuffer that is used for screen display
            glfwSwapBuffers(window);
        }

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    catch (std::exception const &e) {
        spdlog::critical("{}", e.what());
        return 1;
    }

    return 0;
}
