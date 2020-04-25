//
// Created by patrik on 25.04.20.
//

#ifndef VGE_NNR_H
#define VGE_NNR_H

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
#include "Arrangement.h"


using namespace nanogui;
using namespace std;

class Nnr {
public:
    nanogui::GLShader shader;
    Arrangement *arrangement = nullptr;
    bool drawLevels = true;
    float windowSizeX;
    float windowSizeY;
    TextBox *queryLine[2];
    CheckBox *queryLineVertical;
    vector<Vector3f> *inputPoints;
    vector<tuple<Vector3f, Vector3f>> *lines;
    int nnrIndex = -1;

    Nnr(vector<Vector3f> *inputPoints, vector<tuple<Vector3f, Vector3f>> *lines, int windowX, int windowY) {
        windowSizeX = (float)windowX;
        windowSizeY = (float)windowY;
        arrangement = new Arrangement();
        this->inputPoints = inputPoints;
        this->lines = lines;
    }

    void draw() {
        if (arrangement->valid && drawLevels) {
            arrangement->draw();
        }
        if (nnrIndex != -1) {
            shader.setUniform("nnrIntensity", (int) trunc(glfwGetTime() * 10) % 2);
        }
    }

    void addControls(Widget * app) {
        Window *window = new Window(app, "Nearest Neighbor of a Line");
        window->setFixedHeight(200);
        window->setFixedWidth(250);
        window->setPosition(Vector2i(0, 100));
        window->setLayout(new GroupLayout());
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
        auto b = new Button(box, "Solve");
        b->setCallback([&] {
            solveNNL();
        });
    }

    void solveNNL() {
        if (queryLineVertical->checked()) {
            float xCoord = stof(queryLine[1]->value());
            nnrIndex = 0;
            for (auto &point : *inputPoints) {
                cout << xCoord << " > " << point.x() << endl;
                if (xCoord < point.x()) {
                    float prev = abs(xCoord - (*inputPoints)[nnrIndex - 1].x());
                    float actual = abs(xCoord - point.x());
                    if (prev < actual) {
                        nnrIndex--;
                    }
                    cout << nnrIndex << endl;
                    break;
                }
                nnrIndex++;
            }
        } else {
            if (!arrangement->valid) {
                calculateArrangement();
            }
            cout << "A: " << queryLine[0]->value() << "B: " << queryLine[1]->value() << endl;
            Vector3f point(stof(queryLine[0]->value()), stof(queryLine[1]->value()), 0);
            Edge *up, *down;
            arrangement->levelTree->find(point, up, down);
//            nnrIndex = (*linesXpoints)[up->lineIndex];
        }
    }

    void drawQueryLine() {
        cout << "A: " << queryLine[0]->value() << "B: " << queryLine[1]->value() << endl;
        shader.bind();
        Matrix4f mvp;
        mvp.setIdentity();
        mvp.topLeftCorner<3, 3>() = Matrix3f(Eigen::AngleAxisf((float) 0, Vector3f::UnitZ())) * 0.25f;

        mvp.row(0) *= (float) windowSizeX / (float) windowSizeY;

        shader.setUniform("modelViewProj", mvp);
        Vector3f point1, point2, query;
        Vector3f lineColor(0, 0, 1);

        if (queryLineVertical->checked()) {
            point1 = Vector3f(stof(queryLine[1]->value()), windowSizeY, 0);
            point2 = Vector3f(stof(queryLine[1]->value()), -windowSizeY, 0);
            query = Vector3f(stof(queryLine[1]->value()), 0, 0);

        } else {
            Vector2f point(stof(queryLine[0]->value()), stof(queryLine[1]->value()));
            float y = point.x() * windowSizeX - point.y();
            point1 = Vector3f(windowSizeX, y, 0);
            y = point.x() * (-windowSizeX) - point.y();
            point2 = Vector3f(-windowSizeX, y, 0);
            query = Vector3f(stof(queryLine[0]->value()), stof(queryLine[1]->value()), 0);
        }
    }

    void calculateArrangement() {
        arrangement->createArragement(inputPoints, lines);
    }
};


#endif //VGE_NNR_H
