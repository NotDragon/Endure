//
// Created by mixal on 12/13/2022.
//
#include "structs.h"
#include "json.hpp"
#include <fstream>
#include <string>
#include <sys/stat.h>

using json = nlohmann::json;

#ifndef ENDURE_SAVE_SYSTEM_H
#define ENDURE_SAVE_SYSTEM_H

bool fileExists(const std::string& filename)
{
	struct stat buf{};
	if (stat(filename.c_str(), &buf) != -1)
	{
		return true;
	}
	return false;
}

void save(){
	int saveFileCounter = 0;
	std::ofstream saveFile;


	while(fileExists("save" + std::to_string(saveFileCounter) + ".json")){
		saveFileCounter++;
	}

	saveFile.open("save" + std::to_string(saveFileCounter) + ".json");

	json fileData;

	fileData["Player"] = {
			{"x", character.x},
			{"y", character.y},
			{"speedX", character.speedX},
			{"speedY", character.speedY},
			{"isIdle", character.isIdle},
			{
					"hotbarItems",
					{
							{
									"0",
									{
											{"durability", character.hotbarItems[0].durability},
											{"maxDurability", character.hotbarItems[0].maxDurability},
											{"attackSpeed", character.hotbarItems[0].attackSpeed},
											{"damage", character.hotbarItems[0].damage},
											{"id", character.hotbarItems[0].id}
									}

							},
							{
									"1",
									{
											{"durability", character.hotbarItems[1].durability},
											{"maxDurability", character.hotbarItems[1].maxDurability},
											{"attackSpeed", character.hotbarItems[1].attackSpeed},
											{"damage", character.hotbarItems[1].damage},
											{"id", character.hotbarItems[1].id}
									}

							},
							{
									"2",
									{
											{"durability", character.hotbarItems[2].durability},
											{"maxDurability", character.hotbarItems[2].maxDurability},
											{"attackSpeed", character.hotbarItems[2].attackSpeed},
											{"damage", character.hotbarItems[2].damage},
											{"id", character.hotbarItems[2].id}
									}

							},
							{
									"3",
									{
											{"durability", character.hotbarItems[3].durability},
											{"maxDurability", character.hotbarItems[3].maxDurability},
											{"attackSpeed", character.hotbarItems[3].attackSpeed},
											{"damage", character.hotbarItems[3].damage},
											{"id", character.hotbarItems[3].id}
									}

							},
							{
									"4",
									{
											{"durability", character.hotbarItems[4].durability},
											{"maxDurability", character.hotbarItems[4].maxDurability},
											{"attackSpeed", character.hotbarItems[4].attackSpeed},
											{"damage", character.hotbarItems[4].damage},
											{"id", character.hotbarItems[4].id}
									}

							},
							{
									"5",
									{
											{"durability", character.hotbarItems[5].durability},
											{"maxDurability", character.hotbarItems[5].maxDurability},
											{"attackSpeed", character.hotbarItems[5].attackSpeed},
											{"damage", character.hotbarItems[5].damage},
											{"id", character.hotbarItems[5].id}
									}

							},
					}
			},
			{"hotbarSelectedSlot", character.selectedItemSlot},
			{"abilities", character.abilities},
			{"maxAbilities", character.maxAbilities},
			{
					"stats",
					{
							{"defence", character.defence},
							{"strength", character.strength},
							{"m_defence", character.m_defence},
							{"agility", character.agility},
							{"luck", character.luck},
							{"stamina", character.stamina},
							{"visability", character.visability},
							{"points", character.statPoints}
					}
			},
			{"timeSinceLastAttack", character.timeSinceLastAttack},
			{"facing", character.facing},
			{"health", character.health},
			{"maxHealth", character.maxHealth},
			{"state", character.state},
			{"stateTexture", character.stateTexture},
	};

	fileData["Enemy"] = {
			{"count", enemies.size()}
	};


	for(int i = 0; i < enemies.size(); i++){
		auto enemy = enemies[i];
		fileData["enemy" + std::to_string(i)] = {
				{"isAware", enemy.isAware},
				{"timeFromLastAttack", enemy.timeFromLastAttack},
				{"health", enemy.health},
				{"maxHealth", enemy.maxHealth},
				{"awarenessRadius", enemy.awarenessRadius},
				{"range", enemy.range},
				{"damage", enemy.damage},
				{"attackSpeed", enemy.attackSpeed},
				{"xDest", enemy.xDest},
				{"yDest", enemy.yDest},
				{"xFinalDest", enemy.xFinalDest},
				{"yFinalDest", enemy.yFinalDest},
				{"facing", enemy.facing},
				{"isBoss", enemy.isBoss},
				{"x", enemy.x},
				{"y", enemy.y},
				{"isEnabled", enemy.isEnabled}
		};
	}

	fileData["Chest"] = {
			{"count", chests.size()}
	};

	for(int i = 0; i < chests.size();i++)
	{
		auto chest = chests[i].second;
		auto hasItem = chests[i].first.second;
		auto state = chests[i].first.first;

		fileData["chest" + std::to_string(i)] = {
				{"hasItem", hasItem},
				{"state", state},
				{"x", chest.x},
				{"y", chest.y},
				{"itemId", chest.item.id},
				{"room", chest.room},
				{"isOpen", chest.isOpen}
		};
	}

	fileData["NHC"] = {
			{"count", NHCs.size()}
	};

	for(int i = 0; i < NHCs.size();i++)
	{
		auto NHC = NHCs[i];
		fileData["NHC" + std::to_string(i)] = {
				{"isAware", NHC.isAware},
				{"timeFromLastAttack", NHC.timeFromLastAttack},
				{"health", NHC.health},
				{"maxHealth", NHC.maxHealth},
				{"awarenessRadius", NHC.awarenessRadius},
				{"facing", NHC.facing},
				{"x", NHC.x},
				{"y", NHC.y},
				{"isEnabled", NHC.isEnabled}
		};
	}

	saveFile << std::setw(4) << fileData << std::endl;

	std::cout << "saved: \n";
	std::cout << std::setw(4) << fileData << std::endl;
	std::cout << " in " << "save" + std::to_string(saveFileCounter) + ".json" << "\n";
}

void load(int num){
	if(!fileExists("save" + std::to_string(num) + ".json")){
		std::cout << "failed to load save" + std::to_string(num) + ".json";
		return;
	}

	std::ifstream saveFile("save" + std::to_string(num) + ".json");

	std::string tmp;
	std::string saveFileData;

	while(!saveFile.eof())
	{
		saveFile >> tmp;
		saveFileData += tmp;
	}
	saveFileData.resize(saveFileData.size() - 1);

	json fileData = json::parse(saveFileData);

	json player = fileData["Player"];

	character.x = player["x"];

	character.y = player["y"];
	character.speedX = player["speedX"];
	character.speedY = player["speedY"];
	character.isIdle = player["isIdle"];

	for(auto& i: character.hotbarItems)
	{
		i = air;
	}

	json hotbar = player["hotbarItems"];

	for(int i = 0; i < 6; i++){
		character.hotbarItems[i].durability = hotbar[std::to_string(i)]["durability"];
		character.hotbarItems[i].maxDurability = hotbar[std::to_string(i)]["maxDurability"];
		character.hotbarItems[i].attackSpeed = hotbar[std::to_string(i)]["attackSpeed"];
		character.hotbarItems[i].damage = hotbar[std::to_string(i)]["damage"];
		character.hotbarItems[i].id = hotbar[std::to_string(i)]["id"];
	}

	character.selectedItemSlot = player["hotbarSelectedSlot"];
	character.abilities = player["abilities"];
	character.maxAbilities = player["maxAbilities"];

	json stats = player["stats"];

	character.defence = stats["defence"];
	character.strength = stats["strength"];
	character.m_defence = stats["m_defence"];
	character.agility = stats["agility"];
	character.luck = stats["luck"];
	character.stamina = stats["stamina"];
	character.visability = stats["visability"];
	character.statPoints = stats["points"];

	character.timeSinceLastAttack = player["timeSinceLastAttack"];
	character.facing = player["facing"];
	character.health = player["health"];
	character.maxHealth = player["maxHealth"];
	character.state = player["state"];
	character.stateTexture = player["stateTexture"];

	for(int i = 0; i < fileData["Enemy"]["count"]; i++)
	{
		auto enemy = fileData["enemy" + std::to_string(i)];
		enemies[i].attackSpeed = enemy["attackSpeed"];
		enemies[i].awarenessRadius = enemy["awarenessRadius"];
		enemies[i].damage = enemy["damage"];
		enemies[i].facing = enemy["facing"];
		enemies[i].health = enemy["health"];
		enemies[i].isAware = enemy["isAware"];
		enemies[i].isBoss = enemy["isBoss"];
		enemies[i].isEnabled = enemy["isEnabled"];
		enemies[i].maxHealth = enemy["maxHealth"];
		enemies[i].range = enemy["range"];
		enemies[i].timeFromLastAttack = enemy["timeFromLastAttack"];
		enemies[i].x = enemy["x"];
		enemies[i].xDest = enemy["xDest"];
		enemies[i].xFinalDest = enemy["xFinalDest"];
		enemies[i].y = enemy["y"];
		enemies[i].yDest = enemy["yDest"];
		enemies[i].yFinalDest = enemy["yFinalDest"];
	}

	for(int i = 0; i < fileData["NHC"]["count"]; i++)
	{
		auto nch = fileData["NHC" + std::to_string(i)];
		enemies[i].isAware = nch["isAware"];
		enemies[i].timeFromLastAttack = nch["timeFromLastAttack"];
		enemies[i].health = nch["health"];
		enemies[i].maxHealth = nch["maxHealth"];
		enemies[i].awarenessRadius = nch["awarenessRadius"];
		enemies[i].facing = nch["facing"];
		enemies[i].x = nch["x"];
		enemies[i].y = nch["y"];
		enemies[i].isEnabled = nch["isEnabled"];
	}

	for(int i = 0; i < fileData["Chest"]["count"];i++)
	{
		auto chestData = fileData["chest" + std::to_string(i)];
		auto& chest = chests[i].second;
		auto& hasItem = chests[i].first.second;
		auto& state = chests[i].first.first;

		hasItem = chestData["hasItem"];
		state = chestData["state"];
		chest.x = chestData["x"];
		chest.y = chestData["y"];
		chest.item.id = chestData["itemId"];
		chest.room = chestData["room"];
		chest.isOpen = chestData["isOpen"];
	}

	std::cout << "loaded: \n";
	std::cout << std::setw(4) << fileData << std::endl;
	std::cout << " from " << "save" + std::to_string(num) + ".json" << "\n";
}

void load(){
	int saveFileCounter = 0;

	while(fileExists("save" + std::to_string(saveFileCounter) + ".json")){
		saveFileCounter++;
	}


	try {
		load(saveFileCounter - 1);
	}catch (std::exception& e)
	{
		std::cout << "File save" + std::to_string(saveFileCounter - 1) + ".json" << " was damaged\n Failed to load save\n";
		return;
	}
}


#endif// ENDURE_SAVE_SYSTEM_H
