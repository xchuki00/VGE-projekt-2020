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
    nanogui::GLShader pointShader;
    nanogui::GLShader lineShader;
    Arrangement *arrangement = nullptr;
    int lastVersion = -1;
    int *actualVersion;
    int windowSizeX;
    int windowSizeY;
    TextBox *queryLineInput[2];
    CheckBox *queryLineVertical;
    vector<Vector3f> *inputPoints;
    vector<tuple<Vector3f, Vector3f>> *lines;
    vector<int> *linesXpoints;
    bool drawArragements = false;
    int nnrIndex = -1;
    bool querySet = false;
    vector<Vector3f> queryLine;
    vector<Vector3f> queryPoint;

    Nnr(vector<Vector3f> *inputPoints, vector<tuple<Vector3f, Vector3f>> *lines, vector<int> *linesXpoint, int *version,
        int windowX,
        int windowY) {
        windowSizeX = windowX;
        windowSizeY = windowY;
        arrangement = new Arrangement();
        actualVersion = version;
        this->inputPoints = inputPoints;
        this->lines = lines;
        this->linesXpoints = linesXpoint;
        pointShader.initFromFiles("query_point_shader", "../shader/queryVertex.glsl", "../shader/pointsFragment.glsl",
                                  "../shader/pointsGeometry.glsl");
        lineShader.initFromFiles("query_line_shader", "../shader/queryLineVertex.glsl", "../shader/frag.glsl");
    }

    void draw() {
        if (lastVersion == *actualVersion && drawArragements) {
            arrangement->draw();
        }
        if (querySet) {
            pointShader.bind();
            pointShader.setUniform("nnrIntensity", (int) trunc(glfwGetTime() * 10) % 2);

            pointShader.drawArray(GL_POINTS, 0, queryPoint.size());
            lineShader.bind();
            lineShader.drawArray(GL_LINES, 0, queryLine.size());
        }
    }

    void addControls(Widget *app) {
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
        queryLineInput[0] = new TextBox(tools);
        queryLineInput[0]->setEditable(true);
        queryLineInput[0]->setFixedSize(Vector2i(40, 20));
        queryLineInput[0]->setValue("0.0");
        queryLineInput[0]->setDefaultValue("0.0");
        queryLineInput[0]->setFontSize(16);
        queryLineInput[0]->setFormat("[-]?[0-9]*\\.?[0-9]+");
        queryLineInput[0]->setCallback([this](const std::string &value) {
            if (value == "no")
                return false;
            drawQueryLine();
            return true;
        });
        new Label(tools, "* x - ", "sans-bold");

        queryLineInput[1] = new TextBox(tools);
        queryLineInput[1]->setEditable(true);
        queryLineInput[1]->setFixedSize(Vector2i(40, 20));
        queryLineInput[1]->setValue("0.0");
        queryLineInput[1]->setDefaultValue("0.0");
        queryLineInput[1]->setFontSize(16);
        queryLineInput[1]->setFormat("[-]?[0-9]*\\.?[0-9]+");
        queryLineInput[1]->setCallback([this](const std::string &value) {
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
        drawQueryLine();
        if (queryLineVertical->checked()) {
            float xCoord = stof(queryLineInput[1]->value());
            nnrIndex = 0;
            for (auto &point : (*inputPoints)) {
                cout << xCoord << " > " << point.x() << endl;
                if (xCoord < point.x()) {
                    float prev = abs(xCoord - (*inputPoints)[nnrIndex - 1].x());
                    float actual = abs(xCoord - point.x());
                    if (prev < actual) {
                        nnrIndex--;
                    }
                    break;
                }
                nnrIndex++;
            }
            queryPoint.push_back((*inputPoints)[nnrIndex]);
            pointShader.bind();
            pointShader.uploadAttrib("position", (uint32_t) queryPoint.size() * 3, 3, sizeof(GL_FLOAT),
                                     GL_FLOAT, 0, queryPoint.data(), -1);
        } else {
            if (lastVersion != *actualVersion) {
                lastVersion = *actualVersion;
                arrangement->createArragement(inputPoints, lines, windowSizeX, windowSizeY);
            }
            cout << "A: " << queryLineInput[0]->value() << "B: " << queryLineInput[1]->value() << endl;
            Vector3f point(stof(queryLineInput[0]->value()), stof(queryLineInput[1]->value()), 0);
            Edge up, down;
            auto bothOrUpOrDown = arrangement->levelTree->find(point, &up, &down);
            pointShader.bind();
            if (bothOrUpOrDown == 0) {
                float diffUp, diffDown;
                diffUp = abs(point.y() - (up.lineEquation->A * point.x() + up.lineEquation->B));
                diffDown = abs(point.y() - (down.lineEquation->A * point.x() + down.lineEquation->B));

                if (diffDown > diffUp) {
                    queryPoint.push_back((*inputPoints)[(*linesXpoints)[up.lineIndex]]);
                    pointShader.setUniform("nnr", (*inputPoints)[(*linesXpoints)[up.lineIndex]]);
                } else {
                    queryPoint.push_back((*inputPoints)[(*linesXpoints)[down.lineIndex]]);
                    pointShader.setUniform("nnr", (*inputPoints)[(*linesXpoints)[down.lineIndex]]);
                }
            } else if (bothOrUpOrDown == 1) {
                queryPoint.push_back((*inputPoints)[(*linesXpoints)[up.lineIndex]]);
                pointShader.setUniform("nnr", (*inputPoints)[(*linesXpoints)[up.lineIndex]]);
            } else if (bothOrUpOrDown == 2) {
                queryPoint.push_back((*inputPoints)[(*linesXpoints)[down.lineIndex]]);
                pointShader.setUniform("nnr", (*inputPoints)[(*linesXpoints)[down.lineIndex]]);
            }
            pointShader.uploadAttrib("position", (uint32_t) queryPoint.size() * 3, 3, sizeof(GL_FLOAT),
                                     GL_FLOAT, 0, queryPoint.data(), -1);
        }
    }

    void drawQueryLine() {
        Vector3f point1, point2;
        Vector3f lineColor(0, 0, 1);
        queryLine.clear();
        queryPoint.clear();
        if (queryLineVertical->checked()) {
            point1 = Vector3f(stof(queryLineInput[1]->value()), windowSizeY, 0);
            point2 = Vector3f(stof(queryLineInput[1]->value()), -windowSizeY, 0);
            queryPoint.push_back(Vector3f(stof(queryLineInput[1]->value()), 0, 0));
            queryLine.push_back(point1);
            queryLine.push_back(point2);
        } else {
            Vector2f point(stof(queryLineInput[0]->value()), stof(queryLineInput[1]->value()));
            float y = point.x() * windowSizeX - point.y();
            point1 = Vector3f(windowSizeX, y, 0);
            y = point.x() * (-windowSizeX) - point.y();
            point2 = Vector3f(-windowSizeX, y, 0);
            queryLine.push_back(point1);
            queryLine.push_back(point2);
            queryPoint.push_back(Vector3f(stof(queryLineInput[0]->value()), stof(queryLineInput[1]->value()), 0));
        }
        Matrix4f mvp;
        mvp.setIdentity();
        mvp.topLeftCorner<3, 3>() = Matrix3f(Eigen::AngleAxisf((float) 0, Vector3f::UnitZ())) * 0.25f;

        mvp.row(0) *= (float) windowSizeX / (float) windowSizeY;
        lineShader.bind();
        lineShader.setUniform("modelViewProj", mvp);
        lineShader.uploadAttrib("position", (uint32_t) queryLine.size() * 3, 3, sizeof(GL_FLOAT),
                                GL_FLOAT, 0, queryLine.data(), -1);
        pointShader.bind();
        pointShader.setUniform("modelViewProj", mvp);
        pointShader.setUniform("nnr", Vector3f(windowSizeX,windowSizeY,0));
        pointShader.uploadAttrib("position", (uint32_t) queryPoint.size() * 3, 3, sizeof(GL_FLOAT),
                                 GL_FLOAT, 0, queryPoint.data(), -1);
        querySet = true;
    }
};


#endif //VGE_NNR_H
