//
//  planet.h
//  OpenGL_Framework
//
//  Created by Martin Heinrich on 28.10.15.
//  Changed from name shape to planet
//

#ifndef planet_h
#define planet_h
#include <iostream>

class Planet {
public:
    Planet(std::string name, float distance, float speed, float size) :
            name_{name},
            distance_{(distance>0.0f)?distance:-1.0f},
            speed_{(speed>0.0f)?speed:-1.0f},
            size_{(size>0.0f)?size:-1.0f}
    { };

    float distance() const {
        return distance_;
    }

    float speed() const {
        return speed_;
    }

    float size() const {
        return size_;
    }

private:
    std::string name_;
    float distance_;
    float speed_;
    float size_;
    //position
    //transformationMatrix
    //inverTransposedMatrix
    //Texture
    //etc
};

/*
 Shape 
    -> Composit
    -> Planet
    -> Moon?
    -> Star(Sun)?
    -> Meteor?
    -> SpaceShip
    -> ...
 */

#endif /* planet_h */
