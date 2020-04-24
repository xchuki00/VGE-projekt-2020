//
// Created by patrik on 05.04.20.
//


#if defined(_WIN32)
#include <windows.h>
#endif
#include <nanogui/window.h>
#include "OutputWindow.h"
#include <nanogui/glutil.h>
#include <vector>
#include <nanogui/textbox.h>
#include <nanogui/checkbox.h>

#ifndef VGE_VGEAPP_H
#define VGE_VGEAPP_H
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
    bool useLineShader = false;
    OutputWindow *outputWindow;
    std::vector<Vector3f> inputPoints;
    std::vector<Vector3f> lines;
    std::vector<Vector2f> linesColors;
    int queryLineIndex = -1;
    TextBox *queryLine[2];
    CheckBox *queryLineVertical;
    int nnrIndex = -1;
    VgeApp();

    void addFileDialog(Window *window);

    void calculateLines();

    void addControls(Window *window);


    string loadShader(string path);

    void solveNNL();

    void drawQueryLine();

    void openInputCsv();

    void sortPointsByX();

    void calculateArrangement();
};


#endif //VGE_VGEAPP_H
