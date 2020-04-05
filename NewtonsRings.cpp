#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/glcanvas.h>
#include <nanogui/graph.h>
#include <cmath>
#include <string>


#if defined(_WIN32)
#include <windows.h>
#endif

#include <nanogui/glutil.h>



using namespace std;
using namespace nanogui;
using nanogui::Screen;
using nanogui::Window;
using nanogui::GroupLayout;
using nanogui::Button;
using nanogui::Vector2f;
using nanogui::MatrixXu;
using nanogui::MatrixXf;
using nanogui::Label;

class NewtonsRingsApp {
public:
    float diameter = 6;
    float radius = 5000;
    Vector3f LS[3];
    Vector3f l = Vector3f(440, 1, 1);
    Vector3i enable = Vector3i(1, 0, 0);
    TextBox *textBoxs[5];
    Slider *sliders[5];
    nanogui::GLShader mShader;

    NewtonsRingsApp() {
        nanogui::init();

        /**
         * Create a screen, add a window.
         * To the window add a label and a slider widget.
         */

        Screen app{{650, 600}, "Newton's circle"};

        Window *window = new Window(&app, "Light sources");
        window->setPosition(Vector2i(0, 0));
        window->setLayout(new GroupLayout());
        window->setFixedHeight(400);
        window->setFixedWidth(250);

        addLightSource(window, "First light source", 0, true);
        addLightSource(window, "Second light source", 1, false);
        addLightSource(window, "Third light source", 2, false);
        window = new Window(&app, "Lens");
        window->setFixedHeight(200);
        window->setFixedWidth(300);
        window->setPosition(Vector2i(0, 400));
        window->setLayout(new GroupLayout());
        addLens(window);


        // Do the layout calculations based on what was added to the GUI
        app.performLayout();


        /**
         * Load GLSL shader code from embedded resources
         * See: https://github.com/cyrilcode/embed-resource
         */
        mShader.initFromFiles("raymarching_shader","../shader/vert.glsl","../shader/frag.glsl");


        /**
         * Fill the screen with a rectangle (2 triangles)
         */
        MatrixXu indices(3, 2);
        indices.col(0) << 0, 1, 2;
        indices.col(1) << 2, 1, 3;
        MatrixXf positions(3, 4);
        positions.col(0) << -1, -1, 0;
        positions.col(1) << 1, -1, 0;
        positions.col(2) << -1, 1, 0;
        positions.col(3) << 1, 1, 0;

        // bind the shader and upload vertex positions and indices
        mShader.bind();

        LS[0] = waveToColor(l[0]);
        LS[1] = waveToColor(l[1]);
        LS[2] = waveToColor(l[2]);
        mShader.uploadIndices(indices);
        mShader.uploadAttrib("a_position", positions);
        mShader.setUniform("LS0", LS[0]);
        mShader.setUniform("LS1", LS[1]);
        mShader.setUniform("LS2", LS[2]);
//        mShader.setUniform("count", 1.0f);
        mShader.setUniform("lambdas0", l[0]);
        mShader.setUniform("lambdas1", l[1]);
        mShader.setUniform("lambdas2", l[2]);
        mShader.setUniform("radius", radius * pow(10, 6));
        mShader.setUniform("diameter", diameter * pow(10, 6));
        double alfa = 2 * asin(diameter / (2 * radius));
        double thick = radius * (1 - cos(alfa / 2));
        float piDivAlfa = 3.141592653589793   / alfa;
        mShader.setUniform("thick", thick * pow(10, 6));
        mShader.setUniform("piDivAlfa", piDivAlfa);


//     Set resolution and screenRatio uniforms
        int fboWidth, fboHeight;
        glfwGetFramebufferSize(app.glfwWindow(), &fboWidth, &fboHeight);
        Matrix4f mvp,sc,trans;
        mvp.setIdentity();
        Vector3f t = Vector3f(250.0f / fboWidth, 200.0f / fboHeight, 0);
        trans = translate(t);
        Vector3f s = Vector3f(400.0f / fboWidth, 400.0f / fboHeight, 0);
        sc = scale(s);
        mvp = trans*sc;
        cout << fboWidth << 'x' << fboHeight << std::endl;

        mShader.setUniform("mvp", mvp);
        app.drawAll();
        app.setVisible(true);

        /**
         * 10: clear screen
         * 20: set modulation value
         * 30: draw using shader
         * 40: draw GUI
         * 50: goto 10
         */
        while (!glfwWindowShouldClose(app.glfwWindow())) {
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            mShader.bind();
            mShader.drawIndexed(GL_TRIANGLES, 0, 2);

            app.drawWidgets();

            glfwSwapBuffers(app.glfwWindow());
            glfwPollEvents();
        }

        nanogui::shutdown();
        exit(EXIT_SUCCESS);
    }

    void addLightSource(Window *window, string s, int index, bool use) {
        Widget *tools = new Widget(window);

        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));

        new Label(tools, s, "sans-bold");
        CheckBox *c = new CheckBox(tools, "use");
        c->setChecked(use);
        c->setCallback([this, index](const bool &value) {
            this->enable[index] = value;
            float v;
//            float c = 0.0f;
//            for (int i = 0; i < 3; i++) {
//                c += this->enable[i];
//            }
            if (value) {
                v = stof(textBoxs[index]->value());
            } else {
                v = 1;
            }
            this->LS[index] = this->waveToColor(v);
            this->l[index] = v;
            mShader.bind();
//            mShader.setUniform("count", c);
            string i = std::to_string(index);
            mShader.setUniform("lambdas" + i, this->l[index]);
            mShader.setUniform("LS" + i, this->LS[index]);

        });
        tools = new Widget(window);
        //window->setPosition(Vector2i(425, 288));
        GridLayout *layout =
                new GridLayout(Orientation::Horizontal, 2,
                               Alignment::Middle, 15, 5);
        layout->setColAlignment(
                {Alignment::Maximum, Alignment::Fill});
        layout->setSpacing(0, 10);
        tools->setLayout(layout);
        new Label(tools, "Wave Lenght:", "sans-bold");
        textBoxs[index] = new TextBox(tools);
        textBoxs[index]->setEditable(true);
        textBoxs[index]->setFixedSize(Vector2i(100, 20));
        textBoxs[index]->setValue("440");
        textBoxs[index]->setUnits("nm");
        textBoxs[index]->setDefaultValue("0.0");
        textBoxs[index]->setFontSize(16);
        textBoxs[index]->setFormat("[-]?[0-9]*\\.?[0-9]+");
        textBoxs[index]->setCallback([this, index](const std::string &value) {
            if (value == "no" || this->enable[index] == 0)
                return false;
            float v = stof(value);
            this->LS[index] = this->waveToColor(v);
            this->l[index] = v;
            mShader.bind();
            string i = std::to_string(index);
            mShader.setUniform("lambdas" + i, this->l[index]);
            mShader.setUniform("LS" + i, this->LS[index]);
            sliders[index]->setValue((v-300)/600);
            return true;
        });
        sliders[index] = new Slider(window);
        sliders[index]->setValue((440.0f-300.0f)/600.0f);
        sliders[index]->setFixedWidth(160);

        sliders[index]->setCallback([this,index](float value) {
            if (this->enable[index] == 0)
                return false;
            ostringstream ostr;
            int tens = 100;
            value = round((value*600+300)*tens)/tens;
            ostr << value;
            textBoxs[index]->setValue(ostr.str());
            this->LS[index] = this->waveToColor(value);
            this->l[index] = value;
            mShader.bind();
            string i = std::to_string(index);
            mShader.setUniform("lambdas" + i, this->l[index]);
            mShader.setUniform("LS" + i, this->LS[index]);
        });
    }

    void addLens(Window *window) {
        Widget *tools = new Widget(window);

        //window->setPosition(Vector2i(425, 288));
        GridLayout *layout =
                new GridLayout(Orientation::Horizontal, 2,
                               Alignment::Middle, 15, 5);
        layout->setColAlignment(
                {Alignment::Maximum, Alignment::Fill});
        layout->setSpacing(0, 10);
        tools->setLayout(layout);
        new Label(tools, "Diameter of lens:", "sans-bold");
        textBoxs[3] = new TextBox(tools);
        textBoxs[3]->setEditable(true);
        textBoxs[3]->setFixedSize(Vector2i(100, 20));
        textBoxs[3]->setValue("6");
        textBoxs[3]->setUnits("mm");
        textBoxs[3]->setDefaultValue("0.0");
        textBoxs[3]->setFontSize(16);
        textBoxs[3]->setFormat("[-]?[0-9]*\\.?[0-9]+");
        textBoxs[3]->setCallback([this](const std::string &value) {
            if (value == "no")
                return false;

            this->diameter = stof(value);
            double alfa = 2 * asin(diameter / (2 * radius));
            double thick = radius * (1 - cos(alfa / 2));
            float piDivAlfa = 3.141592653589793 / alfa;
            mShader.bind();

            mShader.setUniform("diameter", diameter * pow(10, 6));
            mShader.setUniform("thick", thick * pow(10, 6));
            mShader.setUniform("piDivAlfa", piDivAlfa);
            sliders[3]->setValue((this->diameter)/100);

            return true;
        });
        sliders[3] = new Slider(window);
        sliders[3]->setValue(6.0f/100.0f);
        sliders[3]->setFixedWidth(160);

        sliders[3]->setCallback([this](float value) {
            ostringstream ostr;
//            int tens = 100;
//            value = round((value*100.0f)*tens)/tens;
            ostr << value*100;
            textBoxs[3]->setValue(ostr.str());
            this->diameter = value*100;
            double alfa = 2 * asin(diameter / (2 * radius));
            double thick = radius * (1 - cos(alfa / 2));
            float piDivAlfa = 3.141592653589793 / alfa;
            mShader.bind();

            mShader.setUniform("diameter", diameter * pow(10, 6));
            mShader.setUniform("thick", thick * pow(10, 6));
            mShader.setUniform("piDivAlfa", piDivAlfa);
        });
        tools = new Widget(window);

        //window->setPosition(Vector2i(425, 288));
        layout =
                new GridLayout(Orientation::Horizontal, 2,
                               Alignment::Middle, 15, 5);
        layout->setColAlignment(
                {Alignment::Maximum, Alignment::Fill});
        layout->setSpacing(0, 10);
        tools->setLayout(layout);
        new Label(tools, "Radius of curvature of lens:", "sans-bold");
        textBoxs[4] = new TextBox(tools);
        textBoxs[4]->setEditable(true);
        textBoxs[4]->setFixedSize(Vector2i(100, 20));
        textBoxs[4]->setValue("5000");
        textBoxs[4]->setUnits("mm");
        textBoxs[4]->setDefaultValue("0.0");
        textBoxs[4]->setFontSize(16);
        textBoxs[4]->setFormat("[-]?[0-9]*\\.?[0-9]+");
        textBoxs[4]->setCallback([this](const std::string &value) {
            if (value == "no")
                return false;

            this->radius = stof(value);
            double alfa = 2 * asin(diameter / (2 * radius));
            double thick = radius * (1 - cos(alfa / 2));
            float piDivAlfa = 3.141592653589793 / alfa;
            mShader.bind();

            mShader.setUniform("radius", radius * pow(10, 6));
            mShader.setUniform("thick", thick * pow(10, 6));
            mShader.setUniform("piDivAlfa", piDivAlfa);
            sliders[4]->setValue((this->radius)/15000);
            return true;
        });
        sliders[4] = new Slider(window);
        sliders[4]->setValue(5000.0f/15000.0f);
        sliders[4]->setFixedWidth(160);

        sliders[4]->setCallback([this](float value) {
            ostringstream ostr;
            int tens = 100;
            value = round((value*15000.0f)*tens)/tens;
            ostr << value;
            textBoxs[4]->setValue(ostr.str());
            this->radius = value;
            double alfa = 2 * asin(diameter / (2 * radius));
            double thick = radius * (1 - cos(alfa / 2));
            float piDivAlfa = 3.141592653589793 / alfa;
            mShader.bind();

            mShader.setUniform("radius", radius * pow(10, 6));
            mShader.setUniform("thick", thick * pow(10, 6));
            mShader.setUniform("piDivAlfa", piDivAlfa);
        });
    }

    Vector3f waveToColor(float wave) {
        float R, G, B;
        if (wave >= 300 && wave < 440) {
            R = -(wave - 440.) / (440. - 350.);
            G = 0.0;
            B = 1.0;

        } else if (wave >= 440 && wave < 490) {
            R = 0.0;
            G = (wave - 440.) / (490. - 440.);
            B = 1.0;
        } else if (wave >= 490 && wave < 510) {
            R = 0.0;
            G = 1.0;
            B = -(wave - 510.) / (510. - 490.);
        } else if (wave >= 510 && wave < 580) {
            R = (wave - 510.) / (580. - 510.);
            G = 1.0;
            B = 0.0;
        } else if (wave >= 580 && wave < 645) {
            R = 1.0;
            G = -(wave - 645.) / (645. - 580.);
            B = 0.0;
        } else if (wave >= 645 && wave <= 800) {
            R = 1.0;
            G = 0.0;
            B = 0.0;
        } else {
            R = 0.0;
            G = 0.0;
            B = 0.0;
        }
        float SSS;
        if (wave >= 300 && wave < 420) {
            SSS = 0.3 + 0.7 * (wave - 350) / (420 - 350);
        } else if (wave >= 420 && wave <= 700) {
            SSS = 1.0;
        } else if (wave > 700 && wave <= 800) {
            SSS = 0.3 + 0.7 * (780 - wave) / (780 - 700);
        } else {
            SSS = 0.0;
            SSS *= 255;
        }
        return Vector3f((SSS * R), (SSS * G), (SSS * B));
    }

    string loadShader(string path){
        std::ifstream ifs(path);
        std::string content( (std::istreambuf_iterator<char>(ifs) ),
                             (std::istreambuf_iterator<char>()    ) );
        return content;
    }
};

int main() {
    NewtonsRingsApp *app = new NewtonsRingsApp();
}
