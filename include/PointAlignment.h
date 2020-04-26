//
// Created by Imrich on 25. 4. 2020.
//
#ifndef VGE_ALIGNMENT_H
#define VGE_ALIGNMENT_H

#include <nanogui/window.h>
#include "OutputWindow.h"
#include <nanogui/glutil.h>
#include <vector>
#include <tuple>
#include <nanogui/textbox.h>
#include <nanogui/label.h>
#include <nanogui/button.h>
#include <nanogui/checkbox.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>

using namespace nanogui;
using namespace std;

class PointAlignment {
public:
    int threshold = 1;
    GLShader shader;
    GLShader intersectionsShader;

    float windowSizeX;
    float windowSizeY;
    TextBox *input[0];

    vector<Vector3f> *inputPoints;
    vector<tuple<Vector3f, Vector3f>> *lines;
    vector<Vector3f> *intersections;

    PointAlignment(vector<Vector3f> *inputPoints, vector<tuple<Vector3f, Vector3f>> *lines, int windowX, int windowY) {
        windowSizeX = (float)windowX;
        windowSizeY = (float)windowY;
        this->inputPoints = inputPoints;
        this->lines = lines;
    }

    void addControls(Widget * app) {
        Window *window = new Window(app, "Points on one line in dual plane");
        window->setFixedHeight(200);
        window->setFixedWidth(250);
        window->setPosition(Vector2i(0, 300));
        window->setLayout(new GroupLayout());
        auto box = new Widget(window);
        box->setLayout(new BoxLayout(Orientation::Vertical,
                                     Alignment::Middle, 0, 6));
        auto tools = new Widget(box);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));
        new Label(tools, "threshold: t = ", "sans-bold");
        input[0] = new TextBox(tools);
        input[0]->setEditable(true);
        input[0]->setFixedSize(Vector2i(40, 20));
        input[0]->setValue("2");
        input[0]->setDefaultValue("2");
        input[0]->setFontSize(16);
        input[0]->setFormat("[1-9][0-9]*");
        input[0]->setCallback([this](const std::string &value) {
            if (value == "no")
                return false;
            drawIntersections();
            return true;
        });

        auto b = new Button(box, "Solve");
        b->setCallback([&] {
            //solvePointAlignment();
        });
    }

    void solvePointAlignment() {
        intersectionsShader.bind();
        intersectionsShader.drawArray(GL_LINES, 0, inputPoints->size());
    }

    void drawIntersections() {
        shader.bind();
        shader.drawArray(GL_POINTS, 0, intersections->size());
    }

    void draw() {
//        shader.bind();
//        shader.setUniform("nnrIntensity", (int) trunc(glfwGetTime() * 10) % 2);
//
//        shader.drawArray(GL_LINES, 0, edgeCount * 2);

        drawIntersections();
    }
};
#endif //VGE_ALIGNMENT_H
