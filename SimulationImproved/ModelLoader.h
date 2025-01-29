#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "ObjModel.h"

class ModelLoader final {
public:
    const bool LoadOBJ(const std::string& pathname, ObjModel& model) const;
};
