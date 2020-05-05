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
    Matrix4f mvp;

    int windowSizeX;
    int windowSizeY;
    TextBox *input[1];

    vector<Vector3f> *inputPoints;
    vector<tuple<Vector3f, Vector3f>> *lines;

    //Hold resulting intersections in model space
    vector<Vector3f> intersections;
    //Hold computed lines from intersection points, also in model space
    vector<tuple<Vector3f, Vector3f>> alignedLines;


    PointAlignment(vector<Vector3f> *inputPoints, vector<tuple<Vector3f, Vector3f>> *lines, int windowX, int windowY) {
        windowSizeX = (int)windowX;
        windowSizeY = (int)windowY;
        this->inputPoints = inputPoints;
        this->lines = lines;
        //this->intersections = new vector<Vector3f>;
        intersections.push_back(Vector3f(0.5,0.5,0));

        //Draw lines + withd blending to add up the color and act as accumulator for transformation
        intersectionsShader.initFromFiles("accumulation_shader", "../shader/alignmentVertex.glsl",
                "../shader/alignmentFragment.glsl");
        //Draw original lines at which points should be aligned
        shader.initFromFiles("intersection_shader", "../shader/pointsVertex.glsl",
                "../shader/pointsFragment.glsl","../shader/pointsGeometry.glsl");

        mvp.setIdentity();

        shader.bind();
        shader.setUniform("modelViewProj", mvp);
        shader.uploadAttrib("position", (uint32_t) intersections.size() * 3, 3, sizeof(GL_FLOAT),
                            GL_FLOAT, 0, intersections.data(), -1);

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
            //drawIntersections();
            return true;
        });

        auto b = new Button(box, "Solve");
        b->setCallback([&] { this->solvePointAlignment(); }
        );
    }

    void solvePointAlignment() {

        uint8_t* pixel_data;

        int threshold = stoi(input[0]->value());

        //PBO INIT
        GLuint pbo;
        glGenBuffers(1,&pbo);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
        glBufferData(GL_PIXEL_PACK_BUFFER, int(windowSizeY)*int(windowSizeX)*4, NULL, GL_STREAM_READ);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        //glViewPort(0,0, widowsizeX, windowSizeY);

        glDisable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);

        //Magic happens here, offscreen render to pixel buffer
        glReadBuffer(GL_BACK);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        intersectionsShader.bind();
        intersectionsShader.setUniform("modelViewProj", mvp);
        intersectionsShader.uploadAttrib("position", (uint32_t)lines->size() * 6, 3, sizeof(GL_FLOAT),
                                         GL_FLOAT, 0, this->lines->data(), -1);
        intersectionsShader.drawArray(GL_LINES, 0, (uint32_t)lines->size() * 2); //Draw into other context

        glReadPixels(0, 0, windowSizeY, windowSizeX, GL_BGRA, GL_UNSIGNED_BYTE, 0);

        //glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
        pixel_data = (uint8_t*) glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if (pixel_data) {
            Matrix4f invmvp = mvp.inverse();

            intersections.clear();
            alignedLines.clear();
            int max = 0;

            float xm = 0;
            float ym = 0;
            float xgl = 0;
            float ygl = 0;

            for (int y = 0; y < windowSizeY; y++) {
                for (int x = 0; x < windowSizeX; x++) {

                    if (*pixel_data >= (0.1 * threshold * 255)) {
                        // Extract points from bitmap, in view space
                        xgl = (2*x / float(windowSizeX)) - 1;
                        ygl = (2*y / float(windowSizeY)) - 1;

                        // Back transform to original space
                        intersections.push_back((invmvp * Vector4f(xgl, ygl, 0, 1)).head(3));
                        alignedLines.push_back(to_dual((invmvp * Vector4f(xgl, ygl, 0, 1)).head(3)));
                    }

                    if (max < *pixel_data) {
                        max = *pixel_data;
                        xm = xgl;
                        ym = xgl;
                    }

                    pixel_data += 4;
                }
            }
            std::cout << "Maximum is:" << max << std::endl << "at x,y:[" << xm << ',' << ym << "]" << std::endl;

            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        }

        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0); //Release to normal pixel operation

        glDeleteBuffers(1,&pbo);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }

    //Make point from line
    Vector3f to_dual(tuple<Vector3f,Vector3f> line) {
        float k = (get<1>(line).y() - get<0>(line).y()) / (get<1>(line).x() - get<0>(line).x());
        float q = (get<1>(line).y() - get<1>(line).x() * k);

        return Vector3f(k, -q, 0);
    }

    //Make line from point
    tuple<Vector3f,Vector3f> to_dual(Vector3f point) {
        return make_tuple(Vector3f((windowSizeY+point.y())/point.x(), windowSizeY, 0), Vector3f(windowSizeX, point.x()*windowSizeX - point.y(), 0));
    }

    void draw() {
        // Draw detected alignments
        // Bug bad back transform - nevykresli sa priamka iba body
        //shader.bind();
        //shader.setUniform("modelViewProj", mvp);
        //shader.uploadAttrib("position", (uint32_t) alignedLines.size() * 6, 3, sizeof(GL_FLOAT),
        //      GL_FLOAT, 0, alignedLines.data(), -1);
        //shader.drawArray(GL_LINES, 0, alignedLines.size() * 2);

        shader.bind();
        shader.setUniform("modelViewProj", mvp);
        shader.uploadAttrib("position", (uint32_t) intersections.size() * 3, 3, sizeof(GL_FLOAT),
                         GL_FLOAT, 0, intersections.data(), -1);
        shader.drawArray(GL_POINTS, 0, intersections.size());
    }
};
#endif //VGE_ALIGNMENT_H
