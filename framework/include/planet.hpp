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

struct Planet {
    Planet(std::string name, float distance, float speed, float size) :
            name{name},
            distance{(distance>0.0f)?distance:-1.0f},
            speed{(speed>0.0f)?speed:-1.0f},
            size{(size>0.0f)?size:-1.0f},
            type{"root"}
    { };
    Planet(std::string name, float distance, float speed, float size, std::string const& type) :
    /* types are: sun as root Object, planet and moon (moons are around a planet)
     * a moon need a pointer (name) to the planet.
     */
            name{name},
            distance{(distance>0.0f)?distance:-1.0f},
            speed{(speed>0.0f)?speed:-1.0f},
            size{(size>0.0f)?size:-1.0f},
            type{(type=="root"||type=="sun"||type=="moon"||type=="planet")? type : "error_type"}
    { };

    bool is_moon() const {
        return type == "moon";
    }

    bool is_root() const {
        return (type == "root" || type == "sun");
    }

    bool is_planet() const {
        return type == "planet";
    }


    std::string name;
    float distance; // to the sun? [0,0,0] --> z
    // Winkel, Höhenwinkel, Azimut? https://de.wikipedia.org/wiki/Azimut
    float speed;
    float size;
    float mass;
    std::string type; // type actually don't have a real meaning... i think we could also add an planet or the sun as a moon to a planet... and so on... so it doesn't matter.
    std::vector<Planet*> moon;
//    Planet* child_; // there could be more than one moon... hmm. maybe better parent?!!
//    Planet* parent_;
    //position --> time, rotation
    //transformationMatrix
    //invertTransposedMatrix
    //Texture
    //etc
};

/*
 Shape => Planet as list or tree or
    -> Composite
    -> Planet
    -> Moon?
    -> Star(Sun)?
    -> Meteor?
    -> SpaceShip
    -> ...
 */

#endif /* planet_h */
