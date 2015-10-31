//
// Created by Martin Heinrich on 31.10.15.
//

#include "composit.h"


bool Composit::add_child(const std::shared_ptr<::Composit::Planet> & planet) {
    if(planet!= nullptr) {
        Planet_.push_back(planet);
        return true;
    } else {
        return false;
    }
}