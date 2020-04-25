//
// Created by patrik on 24.04.20.
//

#ifndef VGE_LEVELTREE_H
#define VGE_LEVELTREE_H
#define WINDOW_SIZE_X 1000
#define WINDOW_SIZE_Y 1000

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
struct Edge {
    Vector3f start;
    Vector3f end;
    int lineIndex;
    bool sweeped = false;
};
class LevelTree {
public:
    int theta;
    vector<Edge> level;
    LevelTree *left = nullptr;
    LevelTree *right = nullptr;
    float maxTheta = 0;
    int edgeCount = 0;
    bool initiciated = false;
    nanogui::GLShader lineShader;

    LevelTree(){
        lineShader.initFromFiles("level_tree_shader", "../shader/vert.glsl", "../shader/frag.glsl");
        lineShader.bind();
        Matrix4f mvp;
        mvp.setIdentity();
        mvp.topLeftCorner<3, 3>() = Matrix3f(Eigen::AngleAxisf((float) 0, Vector3f::UnitZ())) * 0.25f;

        mvp.row(0) *= (float) WINDOW_SIZE_X / (float) WINDOW_SIZE_Y;

        lineShader.setUniform("modelViewProj", mvp);
        Vector3f point1, point2;
        Vector2f lineColor(0, 0);
    }
    void print() {
        vector<LevelTree *> stack;
        stack.push_back(this);
        int pos = 1;
        while (!stack.empty()) {
            if(pos>stack.size()){
                pos = stack.size();
            };
            for (int i = 0; i < pos; i++) {
                LevelTree *actual = stack[stack.size() - 1];
                stack.pop_back();
                cout << actual->theta<<"("<<stack.size()<<")"<<", ";
                if(actual->right != nullptr){
                    stack.insert(stack.begin(), actual->right);
                }
                if(actual->left != nullptr){
                    stack.insert(stack.begin(), actual->left);
                }
            }
            cout<<endl;
            pos *= 2;
        }
    }

    void initTree(int minTheta, int maxTheta) {
        initiciated = true;
        this->maxTheta = maxTheta;
        theta = minTheta + ((maxTheta - minTheta) / 2);
        cout<<minTheta<<","<<maxTheta<<" -> "<<theta<<endl;
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
        } else if (newTheta < theta){
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
        } else if (newTheta < theta){
            return left->addNode(newLevel, newTheta);
        }
    }
    void resetShaderData(){
        vector<LevelTree *> stack;
        stack.push_back(this);
        vector<tuple<Vector3f,Vector3f>> data;
        vector<Vector3f> colors;
        int pos = 1;
        float color;
        while (!stack.empty()) {
            if(pos>stack.size()){
                pos = stack.size();
            };
            for (int i = 0; i < pos; i++) {
                LevelTree *actual = stack[stack.size() - 1];
                stack.pop_back();

                for(auto & edge:actual->level){
                    data.push_back(tuple<Vector3f,Vector3f>(edge.start,edge.end));
                    color = actual->theta / maxTheta;
                    colors.push_back(Vector3f(color, color, 0.1));
                    colors.push_back(Vector3f(color, color, 0.1));

                }
                if(actual->right != nullptr){
                    stack.insert(stack.begin(), actual->right);
                }
                if(actual->left != nullptr){
                    stack.insert(stack.begin(), actual->left);
                }
            }
            pos *= 2;
        }
        cout<<"DATA SIZE "<<edgeCount<<endl;
        edgeCount = data.size();
        lineShader.bind();
        lineShader.uploadAttrib("position", (uint32_t) data.size() * 6, 3, sizeof(GL_FLOAT),
                                GL_FLOAT, 0, data.data(), -1);
        lineShader.uploadAttrib("color", (uint32_t) colors.size() * 3, 3, sizeof(GL_FLOAT),
                                GL_FLOAT, 0, colors.data(), -1);
    }

    void draw() {
        lineShader.bind();
        lineShader.drawArray(GL_LINES, 0, edgeCount*2);
    }
};


#endif //VGE_LEVELTREE_H
