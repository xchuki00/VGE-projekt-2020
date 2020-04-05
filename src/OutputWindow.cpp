//
// Created by patrik on 05.04.20.
//
#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/popupbutton.h>
#include <cmath>
#include "../include/OutputWindow.h"
using namespace std;
using namespace nanogui;
using nanogui::Screen;
using nanogui::Window;
using nanogui::GroupLayout;
using nanogui::Button;
using nanogui::Vector2f;
using nanogui::MatrixXu;
using nanogui::MatrixXf;
using nanogui::Label;

OutputWindow::OutputWindow() {
    nanogui::init();

    /**
     * Create a screen, add a window.
     * To the window add a label and a slider widget.
     */

    Screen app{{650, 600}, "Output"};

    // Do the layout calculations based on what was added to the GUI
    app.performLayout();

    /**
     * Load GLSL shader code from embedded resources
     * See: https://github.com/cyrilcode/embed-resource
     */
    mShader.init(
            /* An identifying name */
            "a_simple_shader",

            /* Vertex shader */
            "#version 330\n"
            "uniform mat4 modelViewProj;\n"
            "in vec3 position;\n"
            "void main() {\n"
            "    gl_Position = modelViewProj * vec4(position, 1.0);\n"
            "}",

            /* Fragment shader */
            "#version 330\n"
            "out vec4 color;\n"
            "uniform float intensity;\n"
            "void main() {\n"
            "    color = vec4(vec3(intensity), 1.0);\n"
            "}"
    );

    MatrixXu indices(3, 2); /* Draw 2 triangles */
    indices.col(0) << 0, 1, 2;
    indices.col(1) << 2, 3, 0;

    MatrixXf positions(3, 4);
    positions.col(0) << -1, -1, 0;
    positions.col(1) <<  1, -1, 0;
    positions.col(2) <<  1,  1, 0;
    positions.col(3) << -1,  1, 0;

    mShader.bind();
    mShader.uploadIndices(indices);
    mShader.uploadAttrib("position", positions);
    mShader.setUniform("intensity", 0.5f);

    app.drawAll();
    app.setVisible(true);

    /**
     * 10: clear screen
     * 20: set modulation value
     * 30: draw using shader
     * 40: draw GUI
     * 50: goto 10
     */
    while (!glfwWindowShouldClose(app.glfwWindow())) {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        mShader.bind();
        mShader.drawIndexed(GL_TRIANGLES, 0, 2);

        app.drawWidgets();

        glfwSwapBuffers(app.glfwWindow());
        glfwPollEvents();
    }

    nanogui::shutdown();
    exit(EXIT_SUCCESS);



}