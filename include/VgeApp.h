//
// Created by patrik on 05.04.20.
//


#if defined(_WIN32)
#include <windows.h>
#endif
#include <nanogui/window.h>
#include "OutputWindow.h"
#include <nanogui/glutil.h>


#ifndef VGE_VGEAPP_H
#define VGE_VGEAPP_H

using nanogui::Window;
using namespace std;

class VgeApp {
public:
    nanogui::GLShader mShader;
    OutputWindow *outputWindow;

    VgeApp();

    void addFileDialog(Window *window);

    void metoda1(Window *window);

    void addControls(Window *window);


    string loadShader(string path);
};


#endif //VGE_VGEAPP_H
