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
#include <nanogui/checkbox.h>
#include <nanogui/textbox.h>
#include <cmath>
#include <string>
#include "../include/VgeApp.h"
#include "OutputWindow.h"
#include <nanogui/glutil.h>
#include <sstream>
#include <cmath>
#include <tuple>

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

    Screen app{{WINDOW_SIZE_X, WINDOW_SIZE_Y}, "VGE"};

    Window *window = new Window(&app, "Vstupní soubor");
    window->setPosition(Vector2i(0, 0));
    window->setLayout(new GroupLayout());
    window->setFixedHeight(100);
    window->setFixedWidth(250);
    addFileDialog(window);

    window = new Window(&app, "Nearest Neighbor of a Line");
    window->setFixedHeight(200);
    window->setFixedWidth(250);
    window->setPosition(Vector2i(0, 100));
    window->setLayout(new GroupLayout());
    addControls(window);
    arrangement = new Arrangement();

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
        if (nnrIndex != -1) {
            pointShader.setUniform("nnr", inputPoints[nnrIndex]);
            pointShader.setUniform("nnrIntensity", (int) trunc(glfwGetTime() * 10) % 2);

        }
        pointShader.drawArray(GL_POINTS, 0, this->inputPoints.size());
        if(arrangement->valid && drawLevels){
            arrangement->draw();
            if (queryLineIndex >= 0 && lines.size() >= queryLineIndex) {
                lineShader.bind();
                lineShader.drawArray(GL_LINES, queryLineIndex, 2);
            }
        }else if (useLineShader) {
            lineShader.bind();
            lineShader.drawArray(GL_LINES, 0, lines.size()*2);
        } else if (queryLineIndex >= 0 && lines.size() >= queryLineIndex) {
            lineShader.bind();
            lineShader.drawArray(GL_LINES, queryLineIndex, 2);
        }

        app.drawWidgets();
        glfwSwapBuffers(app.glfwWindow());
        glfwPollEvents();
    }

    nanogui::shutdown();
    exit(EXIT_SUCCESS);
}

void VgeApp::addControls(Window *window) {
    auto box = new Widget(window);
    box->setLayout(new BoxLayout(Orientation::Vertical,
                                 Alignment::Middle, 0, 6));
    auto tools = new Widget(box);
    tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                   Alignment::Middle, 0, 6));
    new Label(tools, "Query line: y = ", "sans-bold");
    queryLine[0] = new TextBox(tools);
    queryLine[0]->setEditable(true);
    queryLine[0]->setFixedSize(Vector2i(40, 20));
    queryLine[0]->setValue("0.0");
    queryLine[0]->setDefaultValue("0.0");
    queryLine[0]->setFontSize(16);
    queryLine[0]->setFormat("[-]?[0-9]*\\.?[0-9]+");
    queryLine[0]->setCallback([this](const std::string &value) {
        if (value == "no")
            return false;
        drawQueryLine();
        return true;
    });
    new Label(tools, "* x - ", "sans-bold");

    queryLine[1] = new TextBox(tools);
    queryLine[1]->setEditable(true);
    queryLine[1]->setFixedSize(Vector2i(40, 20));
    queryLine[1]->setValue("0.0");
    queryLine[1]->setDefaultValue("0.0");
    queryLine[1]->setFontSize(16);
    queryLine[1]->setFormat("[-]?[0-9]*\\.?[0-9]+");
    queryLine[1]->setCallback([this](const std::string &value) {
        if (value == "no")
            return false;
        drawQueryLine();
        return true;
    });
    queryLineVertical = new CheckBox(box, "is query line vertical?");
    queryLineVertical->setChecked(false);
    queryLineVertical->setCallback([this](const bool &value) {
        drawQueryLine();
    });
    auto b = new Button(box, "Calculete and Draw lines");
    b->setCallback([&] {
        drawQueryLine();
        calculateLines();
    });

    b = new Button(box, "Solve");
    b->setCallback([&] {
        solveNNL();
    });
}

void VgeApp::calculateLines() {
    lineShader.bind();
    Matrix4f mvp;
    mvp.setIdentity();
    mvp.topLeftCorner<3, 3>() = Matrix3f(Eigen::AngleAxisf((float) 0, Vector3f::UnitZ())) * 0.25f;

    mvp.row(0) *= (float) WINDOW_SIZE_X / (float) WINDOW_SIZE_Y;

    lineShader.setUniform("modelViewProj", mvp);
    lines.clear();
    linesColors.clear();
    queryLineIndex = -1;
    drawQueryLine();
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
        Vector3f lineColor(red / sum, green / sum,0);
        lines.push_back(tuple<Vector3f,Vector3f>(point1,point2));
        linesColors.push_back(lineColor);
        linesColors.push_back(lineColor);

//        cout << "[" << point1.x() << "," << point1.y() << "]," << "[" << point2.x() << "," << point2.y() << "]" << endl;
    }
    lineShader.uploadAttrib("position", (uint32_t) lines.size() * 6, 3, sizeof(GL_FLOAT),
                            GL_FLOAT, 0, lines.data(), -1);
    lineShader.uploadAttrib("color", (uint32_t) linesColors.size() * 3, 3, sizeof(GL_FLOAT),
                            GL_FLOAT, 0, linesColors.data(), -1);
    useLineShader = true;
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

void VgeApp::solveNNL() {
    if (queryLineVertical->checked()) {
        float xCoord = stof(queryLine[1]->value());
        nnrIndex=0;
        for (auto &point : inputPoints) {
            cout<< xCoord<<" > "<<point.x()<<endl;
            if(xCoord < point.x()){
                float prev  = abs(xCoord - inputPoints[nnrIndex-1].x());
                float actual  = abs(xCoord - point.x());
                if(prev<actual){
                    nnrIndex--;
                }
                cout<<nnrIndex<<endl;
                break;
            }
            nnrIndex++;
        }
    } else {
        calculateLines();
        calculateArrangement();
    }
}

void VgeApp::drawQueryLine() {
    cout << "A: " << queryLine[0]->value() << "B: " << queryLine[1]->value() << endl;
    lineShader.bind();
    Matrix4f mvp;
    mvp.setIdentity();
    mvp.topLeftCorner<3, 3>() = Matrix3f(Eigen::AngleAxisf((float) 0, Vector3f::UnitZ())) * 0.25f;

    mvp.row(0) *= (float) WINDOW_SIZE_X / (float) WINDOW_SIZE_Y;

    lineShader.setUniform("modelViewProj", mvp);
    Vector3f point1, point2;
    Vector3f lineColor(0, 0,1);

    if (queryLineVertical->checked()) {
        point1 = Vector3f(stof(queryLine[1]->value()), WINDOW_SIZE_Y, 0);
        point2 = Vector3f(stof(queryLine[1]->value()), -WINDOW_SIZE_Y, 0);
    } else {
        Vector2f point(stof(queryLine[0]->value()), stof(queryLine[1]->value()));
        float y = point.x() * WINDOW_SIZE_X - point.y();
        point1 = Vector3f(WINDOW_SIZE_X, y, 0);
        y = point.x() * (-WINDOW_SIZE_X) - point.y();
        point2 = Vector3f(-WINDOW_SIZE_X, y, 0);
    }
    if (queryLineIndex == -1) {
        queryLineIndex = lines.size();
        lines.push_back(tuple<Vector3f,Vector3f>(point1,point2));
        linesColors.push_back(lineColor);
        linesColors.push_back(lineColor);
    } else {
        lines[queryLineIndex] = tuple<Vector3f,Vector3f>(point1,point2);
        linesColors[queryLineIndex] = (lineColor);
        linesColors[queryLineIndex + 1] = (lineColor);
    }
    lineShader.uploadAttrib("position", (uint32_t) lines.size() * 6, 3, sizeof(GL_FLOAT),
                            GL_FLOAT, 0, lines.data(), -1);
    lineShader.uploadAttrib("color", (uint32_t) linesColors.size() * 3, 3, sizeof(GL_FLOAT),
                            GL_FLOAT, 0, linesColors.data(), -1);
}

void VgeApp::openInputCsv() {
    auto file = file_dialog(
            {{"csv", "Csv"},
             {"txt", "Text file"}}, false);
    cout << "File dialog result: " << file << endl;

    std::ifstream infile(file);
    std::string line;
    this->inputPoints.clear();
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
    useLineShader = false;
    nnrIndex = -1;
    arrangement->valid = false;
    pointShader.bind();
    pointShader.uploadAttrib("position", (uint32_t) this->inputPoints.size() * 3, 3, sizeof(GL_FLOAT),
                             GL_FLOAT, 0, this->inputPoints.data(), -1);

}

void VgeApp::sortPointsByX() {
    sort(inputPoints.begin(), inputPoints.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.x() < rhs.x();
    });

}

void VgeApp::calculateArrangement() {
    arrangement->createArragement(&inputPoints,&lines,queryLineIndex);
    pointShader.bind();
    pointShader.uploadAttrib("position", (uint32_t) this->inputPoints.size() * 3, 3, sizeof(GL_FLOAT),
                             GL_FLOAT, 0, this->inputPoints.data(), -1);
}

