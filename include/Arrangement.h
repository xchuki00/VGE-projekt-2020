//
// Created by patrik on 24.04.20.
//

#ifndef VGE_ARRANGEMENT_H
#define VGE_ARRANGEMENT_H

#include <nanogui/glutil.h>
#include <vector>
#include <tuple>
#include "LevelTree.h"

using nanogui::Vector3f;
using namespace std;


class Arrangement {
public:
    LevelTree *levelTree;
    vector<Vector3f> vertices; //vertex of arrangement
    vector<Edge> edges; //vertex of arrangement
    vector<LineEquation *> lineEquations;
    bool valid = false;

    void planeSweep() {
        sort(vertices.begin(), vertices.end(), [](const auto &lhs, const auto &rhs) {
            return lhs.x() < rhs.x();
        });
        sort(edges.begin(), edges.end(), [](const auto &lhs, const auto &rhs) {
            float l = (lhs.start.y() + lhs.end.y()) / 2;
            float r = (rhs.start.y() + rhs.end.y()) / 2;
            return l < r;
        });
        float sweepLine = vertices[0].x() - 0.1;
        int theta = 0;
        for (auto &edge : edges) {
            if ((edge.end.x() < sweepLine && sweepLine < edge.start.x()) ||
                (edge.end.x() > sweepLine && sweepLine > edge.start.x())) {
                if (!edge.sweeped) {
                    edge.sweeped = true;
                    levelTree->addNode(edge, theta);
                }
                theta++;
            }
        }
        for (int i = 0; i < vertices.size() - 1; i++) {
            sweepLine = vertices[i].x() + (vertices[i + 1].x() - vertices[i].x()) / 2;
            theta = 0;
            for (auto &edge : edges) {
                if ((edge.end.x() < sweepLine && sweepLine < edge.start.x()) ||
                    (edge.end.x() > sweepLine && sweepLine > edge.start.x())) {
                    if (!edge.sweeped) {
                        edge.sweeped = true;
                        levelTree->addNode(edge, theta);
                    }
                    theta++;
                }
            }
        }
        theta = 0;
        sweepLine = vertices[vertices.size() - 1].x() + 0.1;
        for (auto &edge : edges) {
            if ((edge.end.x() < sweepLine && sweepLine < edge.start.x()) ||
                (edge.end.x() > sweepLine && sweepLine > edge.start.x())) {
                if (!edge.sweeped) {
                    edge.sweeped = true;
                    levelTree->addNode(edge, theta);
                }
                theta++;
            }
        }
    }

    void createArragement(vector<Vector3f> *inputPoints, vector<tuple<Vector3f, Vector3f>> *lines, int windowSizeX,
                          int windowSizeY) {
        levelTree = new LevelTree(windowSizeX, windowSizeY);
        levelTree->initTree(0, (lines->size()) - 1);
        calculateIntersection(lines);
        for(auto &edge : edges){
            cout << edge.lineIndex << ": [" << edge.start.x() << "," << edge.start.y() << "]" << "[" << edge.end.x()
                 << "," << edge.end.y() << "]"<<" eq: y = "<<edge.lineEquation->A<<" * x + "<<edge.lineEquation->B << endl;
        }
        planeSweep();
        levelTree->resetShaderData();
        valid = true;
    };

    bool intersect(Edge line1, LineEquation line2, Vector3f *vertex) {
        float x = (line1.lineEquation->B - line2.B) / (line2.A - line1.lineEquation->A);
        float y = x * line1.lineEquation->A + line1.lineEquation->B;
        vertex->x() = x;
        vertex->y() = y;
        return true;
    }


    void addNewEdges(Edge edge, vector<Vector3f> vector) {
        if (vector.size() > 0) {
            sort(vector.begin(), vector.end(), [](const auto &lhs, const auto &rhs) {
                return lhs.x() < rhs.x();
            });
            Edge newEdge;
            newEdge.start = (edge.start.x() < edge.end.x()) ? edge.start : edge.end;
            newEdge.end = vector[0];
            newEdge.lineIndex = edge.lineIndex;
            newEdge.lineEquation = edge.lineEquation;
            edges.push_back(newEdge);
            for (int i = 0; i < vector.size() - 1; i++) {
                newEdge.start = vector[i];
                newEdge.end = vector[i + 1];
                newEdge.lineIndex = edge.lineIndex;
                newEdge.lineEquation = edge.lineEquation;
                edges.push_back(newEdge);
            }
            newEdge.start = vector[vector.size() - 1];
            newEdge.start = (edge.start.x() > edge.end.x()) ? edge.start : edge.end;
            newEdge.lineIndex = edge.lineIndex;
            newEdge.lineEquation = edge.lineEquation;
            edges.push_back(newEdge);
        } else {
            edges.push_back(edge);
        }
    }

    bool pointOnEdge(vector<Edge>::iterator edge, Vector3f vertex) {
        return !((edge->start.x() > vertex.x() && edge->end.x() > vertex.x()) ||
                 (edge->start.x() < vertex.x() && edge->end.x() < vertex.x()));
    }

    void splitEdge(vector<Edge>::iterator edge, Vector3f vertex) {
        auto end = edge->end;
        edge->end = vertex;
        Edge newEdge;
        newEdge.start = vertex;
        newEdge.end = end;
        newEdge.lineIndex = edge->lineIndex;
        newEdge.lineEquation = edge->lineEquation;
        edge++;
        edges.insert(edge, newEdge);
    }

    void calculateIntersection(vector<tuple<Vector3f, Vector3f>> *lines) {
        vector<Edge> addedLines;
        int index = 0;
        for (auto &line : *lines) {
            vector<Vector3f> newVertices; //vertex of arrangement
            Vector3f vertex(0, 0, 0);
            int addIndex = 0;
            auto C = get<0>(line);
            auto D = get<1>(line);
            float A = (D.y() - C.y()) / (D.x() - C.x());
            float B = C.y() - C.x() * A;
            LineEquation *le = new LineEquation(A, B);
            lineEquations.push_back(le);
            for (auto &addline : addedLines) {
                if (intersect(addline, *le, &vertex)) {
                    vertices.push_back(vertex);
                    newVertices.push_back(vertex);
                    for (auto edge = edges.begin(); edge != edges.end(); edge++) {
                        if (edge->lineIndex == addIndex) {
                            if (pointOnEdge(edge, vertex)) {
                                splitEdge(edge, vertex);
                                break;
                            }
                        }
                    }
                }
                addIndex++;
            }
            Edge newEdge;
            newEdge.start = get<0>(line);
            newEdge.end = get<1>(line);
            newEdge.lineIndex = index;
            newEdge.lineEquation = le;
            addedLines.push_back(newEdge);
            addNewEdges(newEdge, newVertices);
            index++;
            newVertices.clear();
        }
    };

    void draw() {
        levelTree->draw();
    }
};


#endif //VGE_ARRANGEMENT_H
