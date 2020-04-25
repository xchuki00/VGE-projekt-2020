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
#include <nanogui/window.h>
#include <cmath>
#include <string>
#include "../include/VgeApp.h"
#include <nanogui/glutil.h>
#include <tuple>

using namespace std;
using namespace nanogui;
using nanogui::Screen;
using nanogui::Window;
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

    Screen app{{WINDOW_SIZE_X, WINDOW_SIZE_Y}, "VGE"};

    Window *window = new Window(&app, "Vstupní soubor");
    window->setPosition(Vector2i(0, 0));
    window->setLayout(new GroupLayout());
    window->setFixedHeight(100);
    window->setFixedWidth(250);
    addFileDialog(window);

    nnr = new Nnr(&inputPoints,&lines,WINDOW_SIZE_X,WINDOW_SIZE_Y);

    addControls(&app);

    // Do the layout calculations based on what was added to the GUI
    app.performLayout();

    /**
     * Load GLSL shader code from embedded resources
     * See: https://github.com/cyrilcode/embed-resource
     */
    pointShader.initFromFiles("input_shader", "../shader/pointsVertex.glsl", "../shader/pointsFragment.glsl",
                              "../shader/pointsGeometry.glsl");
    lineShader.initFromFiles("arragement_shader", "../shader/vert.glsl", "../shader/frag.glsl");

    pointShader.bind();
    pointShader.uploadAttrib("position", (uint32_t) this->inputPoints.size() * 3, 3, sizeof(GL_FLOAT),
                             GL_FLOAT, 0, this->inputPoints.data(), -1);

    Matrix4f mvp;
    mvp.setIdentity();
    mvp.topLeftCorner<3, 3>() = Matrix3f(Eigen::AngleAxisf((float) 0, Vector3f::UnitZ())) * 0.25f;

    mvp.row(0) *= (float) WINDOW_SIZE_X / (float) WINDOW_SIZE_Y;

    pointShader.setUniform("modelViewProj", mvp);

    app.drawAll();
    app.setVisible(true);

    while (!glfwWindowShouldClose(app.glfwWindow())) {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        pointShader.bind();
        pointShader.drawArray(GL_POINTS, 0, this->inputPoints.size());
        if (lineCalculated) {
            lineShader.bind();
            lineShader.drawArray(GL_LINES, 0, lines.size() * 2);
        } else {
            lineShader.bind();
            lineShader.drawArray(GL_LINES, 0, 2);
        }

        app.drawWidgets();
        glfwSwapBuffers(app.glfwWindow());
        glfwPollEvents();
    }

    nanogui::shutdown();
    exit(EXIT_SUCCESS);
}

void VgeApp::addControls(Widget *app) {
    nnr->addControls(app);
}

void VgeApp::calculateLines() {
    if (!lineCalculated) {
        lineShader.bind();
        Matrix4f mvp;
        mvp.setIdentity();
        mvp.topLeftCorner<3, 3>() = Matrix3f(Eigen::AngleAxisf((float) 0, Vector3f::UnitZ())) * 0.25f;

        mvp.row(0) *= (float) WINDOW_SIZE_X / (float) WINDOW_SIZE_Y;

        lineShader.setUniform("modelViewProj", mvp);
        lines.clear();
        linesColors.clear();
        linesXpoints.clear();
        int pointIndex = 0;
        for (auto &point : inputPoints) {
            //P(a,b) -> line y = ax-b
            float y;
            y = point.x() * WINDOW_SIZE_X - point.y();
            Vector3f point1(WINDOW_SIZE_X, y, 0);
            y = point.x() * (-WINDOW_SIZE_X) - point.y();
            Vector3f point2(-WINDOW_SIZE_X, y, 0);
            float red = abs(point.x());
            float green = abs(point.y());
            float sum = red + green;
            Vector3f lineColor(red / sum, green / sum, 0);
            linesXpoints.push_back(pointIndex);
            lines.push_back(tuple<Vector3f, Vector3f>(point1, point2));
            linesColors.push_back(lineColor);
            linesColors.push_back(lineColor);
            pointIndex++;

        }
        lineShader.uploadAttrib("position", (uint32_t) lines.size() * 6, 3, sizeof(GL_FLOAT),
                                GL_FLOAT, 0, lines.data(), -1);
        lineShader.uploadAttrib("color", (uint32_t) linesColors.size() * 3, 3, sizeof(GL_FLOAT),
                                GL_FLOAT, 0, linesColors.data(), -1);
        lineCalculated = true;
    }
}

void VgeApp::addFileDialog(Window *window) {
    auto tools = new Widget(window);
    tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                   Alignment::Middle, 0, 6));
    new Label(tools, "Input: ", "sans-bold");
    auto b = new Button(tools, "Open file");
    b->setCallback([&] {
        openInputCsv();
    });
}



void VgeApp::openInputCsv() {
    auto file = file_dialog(
            {{"csv", "Csv"},
             {"txt", "Text file"}}, false);
    cout << "File dialog result: " << file << endl;

    std::ifstream infile(file);
    std::string line;
    this->inputPoints.clear();
    lines.clear();
    linesColors.clear();
    linesXpoints.clear();
    while (std::getline(infile, line)) {

        std::istringstream s(line);
        std::string field;
        Vector3f point;
        int i = 0;
        while (getline(s, field, ';')) {
            point.array()[i] = std::stof(field);
            i++;
        }
        this->inputPoints.push_back(point);
    }

    sortPointsByX();
    lineCalculated = false;
    calculateLines();
    pointShader.bind();
    pointShader.uploadAttrib("position", (uint32_t) this->inputPoints.size() * 3, 3, sizeof(GL_FLOAT),
                             GL_FLOAT, 0, this->inputPoints.data(), -1);

}

void VgeApp::sortPointsByX() {
    sort(inputPoints.begin(), inputPoints.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.x() < rhs.x();
    });

}


