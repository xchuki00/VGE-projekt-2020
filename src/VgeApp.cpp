//
// Created by patrik on 05.04.20.
//
#include <iostream>
#include <fstream>
#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/button.h>
#include <nanogui/popupbutton.h>
#include <nanogui/graph.h>
#include <cmath>
#include <string>
#include "../include/VgeApp.h"
#include <nanogui/window.h>
#include "OutputWindow.h"
#include <nanogui/glutil.h>
#include <sstream>

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

string VgeApp::loadShader(string path) {
    std::ifstream ifs(path);
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    return content;
}

VgeApp::VgeApp() {
    nanogui::init();

    /**
     * Create a screen, add a window.
     * To the window add a label and a slider widget.
     */

    Screen app{{650, 600}, "VGE"};

    Window *window = new Window(&app, "Vstupní soubor");
    window->setPosition(Vector2i(0, 0));
    window->setLayout(new GroupLayout());
    window->setFixedHeight(100);
    window->setFixedWidth(250);
    addFileDialog(window);
    window = new Window(&app, "Výstupní soubor");
    window->setPosition(Vector2i(0, 100));
    window->setLayout(new GroupLayout());
    window->setFixedHeight(100);
    window->setFixedWidth(250);
    addFileDialog(window);

    window = new Window(&app, "Ovládání");
    window->setFixedHeight(200);
    window->setFixedWidth(250);
    window->setPosition(Vector2i(0, 200));
    window->setLayout(new GroupLayout());
    addControls(window);

    // Do the layout calculations based on what was added to the GUI
    app.performLayout();

    /**
     * Load GLSL shader code from embedded resources
     * See: https://github.com/cyrilcode/embed-resource
     */
    mShader.initFromFiles("raymarching_shader", "../shader/vert.glsl", "../shader/frag.glsl");


    MatrixXu indices(3, 2); /* Draw 2 triangles */
    indices.col(0) << 0, 1, 2;
    indices.col(1) << 2, 3, 0;

    MatrixXf positions(3, 4);
    positions.col(0) << -1, -1, 0;
    positions.col(1) << 1, -1, 0;
    positions.col(2) << 1, 1, 0;
    positions.col(3) << -1, 1, 0;

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

void VgeApp::addControls(Window *window) {
    new Label(window, "Ovládání", "sans-bold");
    auto tools = new Widget(window);
    tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                   Alignment::Middle, 0, 6));
    auto b = new Button(tools, "Metoda1");
    b->setCallback([&] {
        metoda1(window);
    });
    b = new Button(tools, "Metoda1");
    b->setCallback([&] {
        metoda1(window);
    });

}

void VgeApp::metoda1(Window *window) {
    outputWindow = new OutputWindow();
    cout << "Metoda1" << endl;
}

void VgeApp::addFileDialog(Window *window) {
    new Label(window, "File dialog", "sans-bold");
    auto tools = new Widget(window);
    tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                   Alignment::Middle, 0, 6));
    auto b = new Button(tools, "Open");
    b->setCallback([&] {
        auto file = file_dialog(
                {{"csv", "Csv"},
                 {"txt", "Text file"}}, false);
        cout << "File dialog result: " << file << endl;

        std::ifstream infile(file);
        std::string line;
        while (std::getline(infile, line))
        {

            std::istringstream s(line);
            std::string field;
            Vector3f point;
            int i=0;
            while (getline(s, field,';')){
                point.array()[i]= std::stof (field);
                i++;
            }
            this->inputPoints.push_back(point);
        }
        for (auto &point : this->inputPoints){
            cout<<"x: "<<point.x()<<"y: "<<point.y()<<"z: "<<point.z()<<endl;
        }

    });
    b = new Button(tools, "Save");
    b->setCallback([&] {
        cout << "File dialog result: " << file_dialog(
                {{"png", "Portable Network Graphics"},
                 {"txt", "Text file"}}, true) << endl;
    });

}

