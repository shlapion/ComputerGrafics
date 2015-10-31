//
// Created by Martin Heinrich on 31.10.15.
//

#ifndef OPENGL_FRAMEWORK_SHAPE_H
#define OPENGL_FRAMEWORK_SHAPE_H

#include "planet.hpp"
#include <vector>

class Composit : public Planet {

public:
    Composit(std::string const& name, float distance, float speed, float size) : Planet(name, distance, speed, size) {};

    bool add_child(std::shared_ptr<Planet> const&);

private:
    std::vector<std::shared_ptr<Planet>> Planet_;
};


#endif //OPENGL_FRAMEWORK_SHAPE_H
