//
// Created by lamar on 19.12.18.
//

#include "Queue.h"

std::ostream& operator<<(std::ostream& s, const Tmp& arg) {
    return std::cout << arg.get();
}
