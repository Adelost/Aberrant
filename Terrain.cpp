#include "Terrain.h"

void TW_CALL tw_recreateTerrain(void *clientData)
{ 
	Terrain *in = static_cast<Terrain *>(clientData); // scene pointer is stored in clientData
	in->recreate();                            
}

void Terrain::buildMenu(TwBar* menu)
{
	TwAddVarRW(menu, "Terr max tess (2^x)", TW_TYPE_FLOAT, &cellsPerPatch_dim, "group=Terrain min=0 step=0.01  max=64");
	TwAddVarRW(menu, "Terr min tess (2^x)", TW_TYPE_FLOAT, &tess_min, "group=Terrain min=0 step=0.01  max=64");
	TwAddVarRW(menu, "Terr cell scale", TW_TYPE_FLOAT, &info.cellScale, "group=Terrain");
	TwAddVarRW(menu, "Terr heightScale", TW_TYPE_FLOAT, &info.heightScale, "group=Terrain");
	TwAddVarRW(menu, "Terr cells per patch", TW_TYPE_UINT32, &info.cellsPerPatch_dim, "group=Terrain");
	TwAddButton(menu, "Recreate terrain", tw_recreateTerrain, this, "group=Terrain");
	TwDefine("Settings/Terrain opened=false");
};