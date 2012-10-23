#include "Terrain.h"

void TW_CALL tw_recreateTerrain(void *clientData)
{ 
	Terrain *in = static_cast<Terrain *>(clientData); // scene pointer is stored in clientData
	in->recreate();                            
}

void Terrain::buildMenu(TwBar* menu)
{
	TwAddVarRW(menu, "Terr cell scale", TW_TYPE_FLOAT, &info.cellScale, "group=Terrain");
	TwAddVarRW(menu, "Terr heightScale", TW_TYPE_FLOAT, &info.heightScale, "group=Terrain");
	TwAddVarRW(menu, "Terr cells per patch", TW_TYPE_UINT32, &info.cellsPerPatch_dim, "group=Terrain");
	TwAddButton(menu, "Recreate terrain", tw_recreateTerrain, this, "group=Display");
	TwDefine("Settings/Terrain opened=false");
};