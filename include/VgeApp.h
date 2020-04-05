//
// Created by patrik on 05.04.20.
//
#include <iostream>
#include <fstream>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/button.h>
#include <nanogui/popupbutton.h>
#include <nanogui/graph.h>
#include <cmath>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#endif

#include <nanogui/glutil.h>
#include <python3.6m/asdl.h>

#ifndef VGE_VGEAPP_H
#define VGE_VGEAPP_H
#include "OutputWindow.h"
using nanogui::Window;
using namespace std;

class VgeApp {
public:
    nanogui::GLShader mShader;
    OutputWindow *outputWindow;
    void addFileDialog(Window *window);

    void metoda1(Window *window);

    void addControls(Window *window);

    VgeApp();

    string loadShader(string path);
};


#endif //VGE_VGEAPP_H
