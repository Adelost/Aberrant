#ifndef MAZE_H
#define MAZE_H

#include <d3dx10.h>
#include <fstream>
#include "Util.h"
using namespace std;

class Maze{
private:
	D3DXMATRIX position;
	static const int sizeX=28;
	static const int sizeY=31;
	int grid[sizeX][sizeY];
	D3DXQUATERNION qua_rot_tween;

public:
	Maze()
	{
		// World cordinates
		D3DXMatrixIdentity(&position);

		// Init grid
		createMaze();
	};

	void createMaze()
	{
		// Reset grid
		for(int x=0; x<sizeX; x++)
			for(int y=0; y<sizeY; y++)
				grid[x][y]=0;

		//// Create labyrinth
		//for(int x=0; x<sizeX; x++)
		//{
		//	for(int y=0; y<sizeY; y++)
		//	{
		//		if((y+x) % 4 == 0)
		//			grid[x][y]=1;
		//	}	
		//}

		loadFromTextfile();
	};

	void loadFromTextfile()
	{
		//Load High Score
		string line;
		string fileName = "labyrinth.txt";
		ifstream f(fileName);
		if(f.is_open())
		{
			for (int y=sizeY-1; y>=0; y--)
			{
				getline(f,line);
				for(int x=0; x<sizeX; x++)
				{
					if(line[x]=='#')
						grid[x][y]=1;
					else
						grid[x][y]=0;
				}
			}
			f.close();
		}
		else
		{
			string message =  "Unable to find: "+fileName;
			QMessageBox::information(0, "Error", message.c_str());
		}
	};

	D3DXMATRIX getPosition(int x, int y)
	{
		D3DXMATRIX translation;
		D3DXMatrixIdentity(&translation);
		float middleX = (float)sizeX*0.5f-0.5f; // middle of grid
		float middleY = (float)sizeY*0.5f-0.5f; // middle of grid
		D3DXMatrixTranslation(&translation, x-sizeX-10.0f, 20.0f, y-sizeY);
		
		return translation*position;
	};

	int getSizeX()
	{
		return sizeX;
	};

	int getSizeY()
	{
		return sizeY;
	};

	int getTile(int x, int y)
	{
		// treat coordinates outside of maze as empty tiles
		if(x<0 || y<0)
			return 0;
		if(x>=sizeX || y>=sizeY)
			return 0;

		// get maze tile
		return grid[x][y];
	};

	void buildMenu(TwBar* menu)
	{
		TwAddVarRW(menu, "Maze rotation", TW_TYPE_QUAT4F, &qua_rot_tween, "opened=false axisz=-z group=Maze");
		TwDefine("Settings/Maze group='Game'");
	};
};
#endif