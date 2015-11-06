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
            type_{"sun"}
    { };
    Planet(std::string name, float distance, float speed, float size, std::string const& type) :
    /* types are: sun as root Object, planet and moon (moons are around a planet)
     * a moon need a pointer (name) to the planet.
     */
            name_{name},
            distance_{(distance>0.0f)?distance:-1.0f},
            speed_{(speed>0.0f)?speed:-1.0f},
            size_{(size>0.0f)?size:-1.0f},
            type_{(type=="sun"||type=="moon"||type=="planet")?type:"error_type"}
    { };


    std::string name() const {
        return name_;
    }

    float distance() const {
        return distance_;
    }

    void distance(float value) {
        distance_=value;
    }

    float speed() const {
        return speed_;
    }

    float size() const {
        return size_;
    }

    std::string type() const {
        return type_;
    }

    bool is_moon() const {
        return type_ == "moon";
    }

    bool is_root() const {
        return type_=="root";
    }

    bool is_planet() const {
        return type_=="planet";
    }

//    Planet* child() const { //whhaaa
//        return child_;
//    }

//    Planet* parent() const { //whhaaa
//        return parent_;
//    }

private:
    std::string name_;
    float distance_; // to the sun? [0,0,0] --> z
    // Winkel, Höhenwinkel, Azimut? https://de.wikipedia.org/wiki/Azimut
    float speed_;
    float size_;
    float mass_;
    std::string type_;
//    Planet* child_; // there could be more than one moon... hmm. maybe better perant?!!
//    Planet* parent_;
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
