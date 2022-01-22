/*
    This file is part of WZ2100 Map Editor.
    Copyright (C) 2020-2021  maxsupermanhd
    Copyright (C) 2020-2021  bjorn-ali-goransson

    WZ2100 Map Editor is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    WZ2100 Map Editor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with WZ2100 Map Editor; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "Models.h"

#include <map>
#include <memory>

#include "log.hpp"
#include "Pie.h"

std::map<std::string, PIEmodel*> loaded_models;

PIEmodel* GetModel(std::string filename) {
	if(loaded_models.count(filename)) {
		return loaded_models[filename];
	}
	PIEmodel* l = new PIEmodel();
	if(!l->ReadPIE(filename)) {
		log_error("Failed to load model [%s]", filename.c_str());
		return nullptr;
	}
	loaded_models[filename] = l;
	return l;
}

void FreeModels() {
	for(auto s : loaded_models) {
		delete loaded_models[s.first];
		loaded_models.erase(s.first);
	}
}
