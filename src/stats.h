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
#ifndef STATS_H_INCLUDED
#define STATS_H_INCLUDED

#include <map>
#include <string>
#include <vector>

struct Sstructure {
	int armour;
	int breadth;
	int buildPoints;
	bool combinesWithWall;
	std::string ecmID;
	int height;
	int hitpoints;
	std::string id;
	std::string name;
	int resistance;
	std::string sensorID;
	std::vector<std::string> structureModel;
	int thermal;
	std::string type;
	std::vector<std::string> weapons;
	int width;
};

struct Sfeature {
	int armour;
	int breadth;
	int hitpoints;
	std::string id;
	int lineOfSight;
	std::string model;
	std::string name;
	int startVisible;
	int tileDraw;
	std::string type;
	int width;
};

extern std::map<std::string, Sstructure> Sstructures;
extern std::map<std::string, Sfeature> Sfeatures;

bool ParseStats(std::string path);

#endif