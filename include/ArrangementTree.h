//
// Created by patrik on 24.04.20.
//

#ifndef NEWTONS_RINGS_ARRANGEMENTTREE_H
#define NEWTONS_RINGS_ARRANGEMENTTREE_H

using nanogui::Vector3f;

class ArrangementTree {
public:
    Vector3f start;
    Vector3f end;

    ArrangementTree* leftChild;
    ArrangementTree* rightChild;
};


#endif //NEWTONS_RINGS_ARRANGEMENTTREE_H
