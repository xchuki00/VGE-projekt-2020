//
// Created by patrik on 24.04.20.
//

#ifndef VGE_LEVELTREE_H
#define VGE_LEVELTREE_H

#include <vector>
#include <nanogui/glutil.h>
#include <vector>
#include <tuple>
#include <iostream>

using nanogui::Vector3f;
using nanogui::Vector2f;
using nanogui::Matrix4f;
using nanogui::Matrix3f;

using namespace std;

struct LineEquation {
    LineEquation(float d, float d1) {
        A = d;
        B = d1;
    }

    float A;
    float B;
};

struct Edge {
    Vector3f start;
    Vector3f end;
    int lineIndex;
    bool sweeped = false;
    LineEquation *lineEquation;
};

class LevelTree {
public:
    int windowSizeX;
    int windowSizeY;
    int theta;
    vector<Edge> level;
    LevelTree *left = nullptr;
    LevelTree *right = nullptr;
    float maxTheta = 0;
    int edgeCount = 0;
    bool initiciated = false;
    nanogui::GLShader lineShader;

    LevelTree(int windowX, int windowY) {
        lineShader.initFromFiles("level_tree_shader", "../shader/vert.glsl", "../shader/frag.glsl");
        lineShader.bind();
        windowSizeX = windowX;
        windowSizeY = windowY;
        Matrix4f mvp;
        mvp.setIdentity();
        mvp.topLeftCorner<3, 3>() = Matrix3f(Eigen::AngleAxisf((float) 0, Vector3f::UnitZ())) * 0.25f;

        mvp.row(0) *= (float) windowSizeX / (float) windowSizeY;

        lineShader.setUniform("modelViewProj", mvp);
        Vector3f point1, point2;
        Vector2f lineColor(0, 0);
    }

    LevelTree() {
    }

    void print() {
        vector<LevelTree *> stack;
        stack.push_back(this);
        int pos = 1;
        while (!stack.empty()) {

            for (int i = 0; i < pos; i++) {
                if(stack.empty()){
                    break;
                }
                LevelTree *actual = stack[stack.size() - 1];
                stack.pop_back();
                if (actual == nullptr) {
                    cout << "x" << ", ";
                } else {
                    cout << actual->theta << ", ";

                    if (actual->left != nullptr) {
                        stack.insert(stack.begin(), actual->left);
                    } else {
                        stack.insert(stack.begin(), nullptr);
                    }
                    if (actual->right != nullptr) {
                        stack.insert(stack.begin(), actual->right);
                    } else {
                        stack.insert(stack.begin(), nullptr);
                    }
                }

            }
            cout << endl;
            pos *= 2;
        }
    }

    void initTree(int minTheta, int maxTheta) {
        initiciated = true;
        this->maxTheta = maxTheta;
        theta = minTheta + ((maxTheta - minTheta) / 2);
        cout << minTheta << "," << maxTheta << " -> " << theta << endl;
        if (minTheta <= theta - 1) {
            left = new LevelTree();
            left->initTree(minTheta, theta - 1);
        }
        if (maxTheta >= theta + 1) {
            right = new LevelTree();
            right->initTree(theta + 1, maxTheta);
        }
    }

    bool addNode(vector<Edge> newLevel, int newTheta) {
        if (!initiciated) {
            return false;
        }
        if (newTheta == theta) {
            level.insert(level.end(), newLevel.begin(), newLevel.end());
            return true;
        } else if (newTheta > theta) {
            return right->addNode(newLevel, newTheta);
        } else if (newTheta < theta) {
            return left->addNode(newLevel, newTheta);
        }
    }

    bool addNode(Edge newLevel, int newTheta) {
        if (!initiciated) {
            return false;
        }
        if (newTheta == theta) {
            level.emplace_back(newLevel);
            return true;
        } else if (newTheta > theta) {
            return right->addNode(newLevel, newTheta);
        } else if (newTheta < theta) {
            return left->addNode(newLevel, newTheta);
        }
    }

    void resetShaderData() {
        vector<LevelTree *> stack;
        stack.push_back(this);
        vector<tuple<Vector3f, Vector3f>> data;
        vector<Vector3f> colors;
        int pos = 1;
        float color;
        while (!stack.empty()) {
            if (pos > stack.size()) {
                pos = stack.size();
            };
            for (int i = 0; i < pos; i++) {
                LevelTree *actual = stack[stack.size() - 1];
                stack.pop_back();

                for (auto &edge:actual->level) {
                    data.push_back(tuple<Vector3f, Vector3f>(edge.start, edge.end));
                    color = actual->theta / maxTheta;
                    colors.push_back(Vector3f(color, color, 0.1));
                    colors.push_back(Vector3f(color, color, 0.1));

                }
                if (actual->right != nullptr) {
                    stack.insert(stack.begin(), actual->right);
                }
                if (actual->left != nullptr) {
                    stack.insert(stack.begin(), actual->left);
                }
            }
            pos *= 2;
        }
        edgeCount = data.size();
        cout << "DATA SIZE " << edgeCount << endl;
        lineShader.bind();
        lineShader.uploadAttrib("position", (uint32_t) data.size() * 6, 3, sizeof(GL_FLOAT),
                                GL_FLOAT, 0, data.data(), -1);
        lineShader.uploadAttrib("color", (uint32_t) colors.size() * 3, 3, sizeof(GL_FLOAT),
                                GL_FLOAT, 0, colors.data(), -1);
    }

    bool upOrDown(Vector3f queryPoint, Edge *pEdge) {
        for (auto &edge : level) {
            if ((edge.start.x() >= queryPoint.x() && edge.end.x() <= queryPoint.x()) ||
                (edge.start.x() <= queryPoint.x() && edge.end.x() >= queryPoint.x())) {
                *pEdge = edge;
                cout << edge.lineIndex << ": [" << edge.start.x() << "," << edge.start.y() << "]" << "[" << edge.end.x()
                     << "," << edge.end.y() << "]"<<" eq: y = "<<edge.lineEquation->A<<" * x + "<<edge.lineEquation->B << endl;
                float y = edge.lineEquation->A * queryPoint.x() + edge.lineEquation->B;
                if (y < queryPoint.y()) {
                    return true;
                } else {
                    return false;
                }
            }
        }
        return false;
    }

    int find(Vector3f queryPoint, Edge *up, Edge *down) {
        vector<tuple<int, Edge >> stack;
        auto actual = this;
        bool leaf = false;
        bool upOrDown = true;
        int stheta;
        Edge edge;
        cout << "START FIND ["<<queryPoint.x()<<", "<<queryPoint.y()<<"]" << endl;
        while (!leaf) {
            if (actual->upOrDown(queryPoint, &edge)) {
                cout << "up " << actual->theta << endl;
                if (actual->right != nullptr) {
                    stack.push_back(tuple<int, Edge >(actual->theta, edge));
                    actual = actual->right;
                } else {
                    *down = edge;
                    upOrDown = true;
                    stheta = actual->theta + 1;
                    leaf = true;
                }
            } else {
                cout << "down " << actual->theta << endl;
                if (actual->left != nullptr) {
                    stack.push_back(tuple<int, Edge>(actual->theta, edge));
                    actual = actual->left;
                } else {
                    *up = edge;
                    upOrDown = false;
                    stheta = actual->theta - 1;
                    leaf = true;
                }
            }
        }
        bool finded = false;
        while (!stack.empty()) {
            auto actual = stack[stack.size() - 1];
            stack.pop_back();
            if (get<0>(actual) == stheta) {
                finded = true;
                if (upOrDown) {
                    *up = get<1>(actual);
                } else {
                    *down = get<1>(actual);
                }
                break;
            }
        }
        if (finded) {
            return 0;
        } else if (upOrDown) {
            return 2;
        } else {
            return 1;
        }
    }

    void draw() {
        lineShader.bind();
        lineShader.drawArray(GL_LINES, 0, edgeCount * 2);
    }
};


#endif //VGE_LEVELTREE_H
