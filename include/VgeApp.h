//
// Created by patrik on 05.04.20.
//

#ifndef VGE_VGEAPP_H
#define VGE_VGEAPP_H

#if defined(_WIN32)
#include <windows.h>
#endif
#include <nanogui/window.h>
#include "OutputWindow.h"
#include <nanogui/glutil.h>
#include <vector>
#include <tuple>
#include <nanogui/textbox.h>
#include <nanogui/checkbox.h>
#include "Arrangement.h"
#include "Nnr.h"

#define WINDOW_SIZE_X 1000
#define WINDOW_SIZE_Y 1000

using nanogui::Window;
using nanogui::Vector3f;
using nanogui::Vector2f;
using nanogui::TextBox;
using nanogui::CheckBox;

using namespace std;

class VgeApp {
public:
    nanogui::GLShader pointShader;
    nanogui::GLShader lineShader;
    int version = 0;
    bool lineCalculated = false;
    vector<Vector3f> inputPoints;
    vector<tuple<Vector3f,Vector3f>> lines;
    vector<int> linesXpoints;
    vector<Vector3f> linesColors;
    Nnr *nnr;
    VgeApp();

    void addFileDialog(Window *window);

    void calculateLines();

    void addControls(Widget *app);


    string loadShader(string path);

    void solveNNL();

    void drawQueryLine();

    void openInputCsv();

    void sortPointsByX();

    void calculateArrangement();
};


#endif //VGE_VGEAPP_H
