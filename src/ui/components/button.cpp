#include "button.h"
#include <iostream>

Button::Button(const std::string& label) : label(label) {}

void Button::setLabel(const std::string& newLabel) {
    label = newLabel;
}

std::string Button::getLabel() const {
    return label;
}

void Button::onClick(std::function<void()> callback) {
    clickCallback = callback;
}