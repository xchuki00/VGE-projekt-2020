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
        cout << "SL: " << sweepLine << endl;
        int theta = 0;
        for (auto &edge : edges) {
            if ((edge.end.x() < sweepLine && sweepLine < edge.start.x()) ||
                (edge.end.x() > sweepLine && sweepLine > edge.start.x())) {
                cout << "THETA: " << theta << endl;
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
            cout << "SL: " << sweepLine << endl;
            for (auto &edge : edges) {
                if ((edge.end.x() < sweepLine && sweepLine < edge.start.x()) ||
                    (edge.end.x() > sweepLine && sweepLine > edge.start.x())) {
                    cout << "THETA: " << theta << endl;
                    cout << "[" << edge.start.x() << "," << edge.start.y() << "]" << "[" << edge.end.x() << ","
                         << edge.end.y() << "]" << endl;
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
        cout << "SL: " << sweepLine << endl;
        for (auto &edge : edges) {
            if ((edge.end.x() < sweepLine && sweepLine < edge.start.x()) ||
                (edge.end.x() > sweepLine && sweepLine > edge.start.x())) {
                cout << "THETA: " << theta << endl;
                if (!edge.sweeped) {
                    edge.sweeped = true;
                    levelTree->addNode(edge, theta);
                }
                theta++;
            }
        }
    }

    void createArragement(vector<Vector3f> *inputPoints, vector<tuple<Vector3f, Vector3f>> *lines) {
        levelTree = new LevelTree();
        levelTree->initTree(0, (lines->size()) - 1);
        calculateIntersection(lines);
        for (auto &edge :edges) {
            cout << edge.lineIndex << ": [" << edge.start.x() << "," << edge.start.y() << "]" << "[" << edge.end.x()
                 << "," << edge.end.y() << "]" << endl;
        }
        planeSweep();
        levelTree->resetShaderData();
        valid = true;
    };

    bool intersect(Edge line1, tuple<Vector3f, Vector3f> line2, Vector3f *vertex) {
        Vector3f A, B, C, D;
        A = line1.start;
        B = line1.end;
        C = get<0>(line2);
        D = get<1>(line2);
        float param1[2];
        float param2[2];
        param1[0] = (B.y() - A.y()) / (B.x() - A.x());
        param1[1] = A.y() - A.x() * param1[0];

        param2[0] = (D.y() - C.y()) / (D.x() - C.x());
        param2[1] = C.y() - C.x() * param2[0];

        float x = (param1[1] - param2[1]) / (param2[0] - param1[0]);
        float y = x * param1[0] + param1[1];
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
            edges.push_back(newEdge);
            for (int i = 0; i < vector.size() - 1; i++) {
                newEdge.start = vector[i];
                newEdge.end = vector[i + 1];
                newEdge.lineIndex = edge.lineIndex;
                edges.push_back(newEdge);
            }
            newEdge.start = vector[vector.size() - 1];
            newEdge.start = (edge.start.x() > edge.end.x()) ? edge.start : edge.end;
            newEdge.lineIndex = edge.lineIndex;
            edges.push_back(newEdge);
        } else {
            edges.push_back(edge);
        }
    }

    bool pointOnEdge(vector<Edge>::iterator edge, Vector3f vertex) {
        return !((edge->start.x() >= vertex.x() && edge->end.x() >= vertex.x()) ||
                 (edge->start.x() <= vertex.x() && edge->end.x() <= vertex.x()) ||
                 (edge->start.y() >= vertex.y() && edge->end.y() >= vertex.y()) ||
                 (edge->start.y() <= vertex.y() && edge->end.y() <= vertex.y()));
    }

    void splitEdge(vector<Edge>::iterator edge, Vector3f vertex) {
        auto end = edge->end;
        edge->end = vertex;
        Edge newEdge;
        newEdge.start = vertex;
        newEdge.end = end;
        newEdge.lineIndex = edge->lineIndex;
        edge++;
        edges.insert(edge, newEdge);
    }

    void calculateIntersection(vector<tuple<Vector3f, Vector3f>> *lines) {
        vector<Edge> addedLines;
        int index = 0;
        for (auto &line : *lines) {
            if (index == 0) {
                index++;
                continue;
            }
            vector<Vector3f> newVertices; //vertex of arrangement
            Vector3f vertex(0, 0, 0);
            int addIndex = 1;
            for (auto &addline : addedLines) {
                if (intersect(addline, line, &vertex)) {
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
