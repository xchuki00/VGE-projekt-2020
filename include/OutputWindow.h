//
// Created by patrik on 05.04.20.
//

#ifndef VGE_OUTPUTWINDOW_H
#define VGE_OUTPUTWINDOW_H
#if defined(_WIN32)
#include <windows.h>
#endif
#include <nanogui/window.h>
#include "OutputWindow.h"
#include <nanogui/glutil.h>



class OutputWindow {
public:
    OutputWindow();
    nanogui::GLShader mShader;

};


#endif //VGE_OUTPUTWINDOW_H
