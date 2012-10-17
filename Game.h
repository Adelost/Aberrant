/*
Important note, "x = lenght, y = width, z = height", 
except in "specific" D3DX functions where y and z has to be
converted to "y = height, z = width"
*/

#ifndef PACMAN_H
#define PACMAN_H

#include <d3dx10.h>
#include <vector>
#include "GameEntity.h"

class Game{
public:
	static const int sizeXY=30;
	Maze *maze;
	GameEntity *entity;

	//Constructor
	Game()
	{
		maze = new Maze();
		entity = new GameEntity(maze);
	};

	~Game()
	{
		delete maze;
		delete entity;
	};

	//Functions
	void run(float dt)
	{
		////Resource
		//static float updateSpeed=0.0f;
		//updateSpeed+=dt;
		updateEntities(dt);
	};

	void updateEntities(float dt)
	{
		entity->update(dt);
	};
};
#endif