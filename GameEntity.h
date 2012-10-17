#ifndef GROUP_H
#define GROUP_H

#include "Maze.h"
#include "Util.h"

typedef struct Int2{
	Int2(){x=0; y=0;}
	Int2(int x, int y){this->x = x; this->y=y;}

	int x;
	int y;
} Int2;

typedef struct Float2{
	Float2(){x=0.0f; y=0.0f;}
	Float2(float x, float y){this->x = x; this->y=y;}
	float x;
	float y;
} Float2;

class GameEntity{
private:
	Int2 pos;
	float pos_offset;
	Int2 dir_queue;
	Int2 dir;
	bool isMoving;

	D3DXQUATERNION qua_rot_tween;
	float speed;
	float turningSpeed; // turning speed (smaller is faster)
	
	Maze *maze;

public:
	GameEntity(Maze *maze)
	{
		this->maze = maze;

		// Settings
		speed = 4.3f;
		turningSpeed = 8.0f;
		
		// Starting values
		pos = Int2(1,1);
		pos_offset = 0.0f;
		dir.x=1;
		dir.y=0;
		isMoving = false;
		D3DXQuaternionIdentity(&qua_rot_tween);
	}

	void move(int x, int y)
	{
		isMoving = true;

		// Prevent diagonal movements
		if(x!=0 && y!=0)
			x = 0; 

		// Update movement queue
		dir_queue.x=x;
		dir_queue.y=y;
	};

	void stop()
	{
		isMoving = false;
	};

	void update(float dt)
	{
		// True: entity is in move state
		if(isMoving)
		{
			// True: Player is moving between two grid points 
			if(pos_offset>0.0f)
			{
				// interpolate "between" positions
				pos_offset-=dt*speed;

				// allow U-turn while moving between two grid points
				if(dir.x == -dir_queue.x && dir.y == -dir_queue.y)
				{
					dir = dir_queue;
					pos_offset = 1.0f - pos_offset;
					pos = Int2(pos.x+dir.x,  pos.y+dir.y);
				}
			}
			// False: player is at intersection 
			else
			{
				Int2 newPos = Int2(pos.x+dir_queue.x,  pos.y+dir_queue.y);
				// True: queued direction would cause wall collision 
				if(maze->getTile(newPos.x,newPos.y) == 1)
				{
					newPos.x = pos.x+dir.x;
					newPos.y = pos.y+dir.y;
					// True: old direction would cause collision 
					if(maze->getTile(newPos.x,newPos.y) == 1)
					{
						// stop pacman and abort further testing
						isMoving = false;
						return; // abort, we're done
					}
				}
				// Else: queued direction is valid for use
				else {
					dir = dir_queue;
				}

				//
				// Perform movement
				//

				pos=newPos;
				pos_offset +=1.0f;
				// Bugfix: it should only be possible for an entity to move a maximum of one square at every update
				if(pos_offset<=0.0f)
					pos_offset = 1.0f; // reset movement
			}
		}

		// Interpolate rotation on cube
		interpolateRotation(dt);
	};

	void interpolateRotation(float dt)
	{
		// Create quaternion facing in rotation we want to interpolate to
		D3DXMATRIX mat_rot;
		D3DXVECTOR3 vec_eye(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 vec_at((float)dir.x, 0.0f, (float)dir.y);
		D3DXVECTOR3 vec_up(0.0f, -1.0f, 0.0f);
		D3DXMatrixLookAtLH(&mat_rot, &vec_eye, &vec_at, &vec_up);
		D3DXQUATERNION qua_rot;
		D3DXQuaternionRotationMatrix(&qua_rot,&mat_rot);

		// Interpolate old rotation with new rotation using quaternions
		D3DXQUATERNION qua_out;
		D3DXQuaternionSlerp(&qua_out,&qua_rot_tween,&qua_rot, turningSpeed*dt);
		qua_rot_tween = qua_out;
	};

	D3DXMATRIX getPos()
	{
		// Hides transition between grid
		D3DXMATRIX translation;
		D3DXMatrixIdentity(&translation);
		float xTween = -dir.x * pos_offset;
		float yTween = -dir.y * pos_offset;
		D3DXMatrixTranslation(&translation, xTween, 0, yTween);

		// Return
		D3DXMATRIX mat_rot_tween; D3DXMatrixRotationQuaternion(&mat_rot_tween,&qua_rot_tween);
		return mat_rot_tween*translation*maze->getPosition(pos.x,pos.y);
	};

	D3DXMATRIX debug_getPos()
	{
		// Return
		return maze->getPosition(pos.x,pos.y);
	};

	void buildMenu(TwBar* menu)
	{
		TwAddVarRW(menu, "Pacman speed", TW_TYPE_FLOAT, &speed, "group=Entities");
		TwAddVarRW(menu, "Turning speed", TW_TYPE_FLOAT, &turningSpeed, "group=Entities");
		TwAddVarRW(menu, "Pacman Rotation", TW_TYPE_QUAT4F, &qua_rot_tween, "opened=false axisz=-z group=Entities");
		TwDefine("Settings/Entities group='Game'");
	};
};
#endif