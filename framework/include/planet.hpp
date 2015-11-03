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
            size_{(size>0.0f)?size:-1.0f},
            child_{nullptr},
            parent_{nullptr}
    { };
    Planet(std::string name, float distance, float speed, float size, Planet* child) :
            name_{name},
            distance_{(distance>0.0f)?distance:-1.0f},
            speed_{(speed>0.0f)?speed:-1.0f},
            size_{(size>0.0f)?size:-1.0f},
            child_{child},
            parent_{nullptr}
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

    bool hasMoon() const {
        return child_ != nullptr;
    }

private:
    std::string name_;
    float distance_; // to the sun? [0,0,0] --> z
    // Winkel, Höhenwinkel, Azimut? https://de.wikipedia.org/wiki/Azimut
    float speed_;
    float size_;
    Planet* child_; // there could be more than one moon... hmm. maybe better perant?!!
    Planet* parent_;
    //position --> time, rotation
    //transformationMatrix
    //inverTransposedMatrix
    //Texture
    //etc
};

/*
 Shape => Planet as list or tree or
    -> Composit
    -> Planet
    -> Moon?
    -> Star(Sun)?
    -> Meteor?
    -> SpaceShip
    -> ...
 */

#endif /* planet_h */
