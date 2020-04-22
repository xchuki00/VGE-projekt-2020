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

#ifndef VGE_VGEAPP_H
#define VGE_VGEAPP_H

using nanogui::Window;
using nanogui::Vector3f;
using namespace std;

class VgeApp {
public:
    nanogui::GLShader mShader;
    OutputWindow *outputWindow;
    std::vector<Vector3f> inputPoints;
    VgeApp();

    void addFileDialog(Window *window);

    void metoda1(Window *window);

    void addControls(Window *window);


    string loadShader(string path);
};


#endif //VGE_VGEAPP_H
