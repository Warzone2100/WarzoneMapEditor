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

#include "stats.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "log.hpp"

using json = nlohmann::json;

std::map<std::string, Sstructure> Sstructures;
std::map<std::string, Sfeature> Sfeatures;

bool ParseStats(std::string path) {
	{
		std::ifstream f(path+"structure.json");
		if(!f) {
			log_error("Failed to open structure file: %s", strerror(errno));
			return false;
		}
		json j = json::parse(f);
		int loaded = 0;
		for(auto& [id, jo] : j.items()) {
			log_trace("ID: %s : \n%s", id.c_str(), jo.dump(4).c_str());
			Sstructure s;
			jo.at("armour").get_to(s.armour);
			jo.at("breadth").get_to(s.breadth);
			jo.at("buildPoints").get_to(s.buildPoints);
			if(jo.count("combinesWithWall"))
				jo.at("combinesWithWall").get_to(s.combinesWithWall);
			else
				s.combinesWithWall = false;
			jo.at("ecmID").get_to(s.ecmID);
			jo.at("height").get_to(s.height);
			jo.at("hitpoints").get_to(s.hitpoints);
			jo.at("id").get_to(s.id);
			jo.at("name").get_to(s.name);
			if(jo.count("resistance"))
				jo.at("resistance").get_to(s.resistance);
			else
				s.resistance = -1;
			jo.at("sensorID").get_to(s.sensorID);
			jo.at("structureModel").get_to(s.structureModel);
			jo.at("thermal").get_to(s.thermal);
			jo.at("type").get_to(s.type);
			if(jo.count("weapons"))
				jo.at("weapons").get_to(s.weapons);
			else
				s.weapons = {};
			jo.at("width").get_to(s.width);
			Sstructures.insert_or_assign(id, s);
			loaded++;
		}
		log_info("Loaded %d structures", loaded);
		f.close();
	}
	{
		std::ifstream f(path+"features.json");
		if(!f) {
			log_error("Failed to open features file: %s", strerror(errno));
			return false;
		}
		json j = json::parse(f);
		int loaded = 0;
		for(auto& [id, jo] : j.items()) {
			log_trace("ID: %s : \n%s", id.c_str(), jo.dump(4).c_str());
			Sfeature s;
			if(jo.count("armour")) {
				jo.at("armour").get_to(s.armour);
			} else {
				s.armour = 1;
			}
			jo.at("breadth").get_to(s.breadth);
			jo.at("hitpoints").get_to(s.hitpoints);
			jo.at("id").get_to(s.id);
			if(jo.count("lineOfSight")) {
				jo.at("lineOfSight").get_to(s.lineOfSight);
			} else {
				s.lineOfSight = 0;
			}
			jo.at("model").get_to(s.model);
			jo.at("name").get_to(s.name);
			if(jo.count("startVisible")) {
				jo.at("startVisible").get_to(s.startVisible);
			} else {
				s.startVisible = false;
			}
			jo.at("tileDraw").get_to(s.tileDraw);
			jo.at("type").get_to(s.type);
			jo.at("width").get_to(s.width);
			Sfeatures.insert_or_assign(id, s);
			loaded++;
		}
		log_info("Loaded %d features", loaded);
		f.close();
	}
	return true;
}