#include "GeometryFactory.h"

void readMtlFile(stringstream& stream)
{
	// Read .mtl-file;
	string mtlFileName;
	stream >> mtlFileName;

	string line;
	string prefix;
	string resourceFolder = "root/Meshes/";
	fstream mtlFile(resourceFolder + mtlFileName);


	if(!stream)
	{
		// Show error message
		MessageBoxA(0, "Couldn't find '.mtl'-file!", 0, 0);
	}
	else
	{
		while(mtlFile.eof() == false)
		{
			stringstream lineStreamMtl;
			getline(mtlFile, line);
			lineStreamMtl << line;
			prefix=" ";
			lineStreamMtl >> prefix;

			if(prefix == "map_Kd")
			{
				string temp;
				lineStreamMtl >> temp;
				string texFileName = resourceFolder + "Textures/" + temp;
			}
		}
	}
}




void GeometryFactory::readObjFile(MeshData* meshData)
{
	// Read obj-file;
	string resourceFolder = "root/Meshes/";
	fstream objFile(resourceFolder + "cube.obj");

	if(!objFile)
	{
		// Show error message
		MessageBoxA(0, "Couldn't find specified '.obj'-file!", 0, 0);
	}
	else
	{
		string line;
		string prefix;
		string texFileName = resourceFolder + "Textures/BBRONZE01.dds";

		int count = 0;
		while(objFile.eof() == false)
		{
			count++;

			//prefix = "NULL"; //leave nothing from the previous iteration
			stringstream lineStream;

			getline(objFile, line);
			lineStream << line;
			prefix=" ";
			lineStream >> prefix;

			/*if(prefix == "mtllib")
			readMtlFile(lineStream);*/
			if(prefix == "v")
				readVertices(lineStream);

			else if(prefix == "vt")
				readTextureUV(lineStream);

			else if (prefix == "vn")
				readNormals(lineStream);

			else if(prefix == "f")
				readFaces(lineStream, meshData);

			/*	else if(prefix == "g")
			createObject();*/
		}
	}
}

void GeometryFactory::readFaces(stringstream& f, MeshData* meshData)
{
	// Does normal exists?
	bool normalExists=false;
	if(vertex_norm.size()>0)
		normalExists=true;

	// Does texture UV exists?
	bool texExists=false;
	if(vertex_uv.size()>0)
		texExists=true;

	// Read Face
	int vertPos[3], normPos[3], texPos[3];
	for(int i=0; i<3; i++)
	{
		// Load Vertex
		f >> vertPos[i]; 
		normPos[i]=0;
		texPos[i]=0;

		// If normal, load normal
		if(f.peek() == '/' && normalExists)
		{
			f.ignore();
			f >> normPos[i];  // Load normal
		}

		// If UV coordinates, load UV coordinates
		if(f.peek() == '/' && texExists)
		{
			f.ignore();
			f >> texPos[i];  // Load texture UV
		}

		// Map value to vector (vector index starts on 0)
		vertPos[i]--;
		normPos[i]--;
		texPos[i]--;
	}


	// If normal didn't exist, create own
	XMFLOAT3 normals[3];
	if(!normalExists)
	{
		XMVECTOR v1 = XMLoadFloat3(&vertex_pos[vertPos[1]]) -XMLoadFloat3(&vertex_pos[vertPos[0]]);
		XMVECTOR v2 = XMLoadFloat3(&vertex_pos[vertPos[2]]) -XMLoadFloat3(&vertex_pos[vertPos[0]]);
		XMVECTOR normal = XMVector3Cross(v1, v2);
		normal = XMVector3Normalize(normal);

		for(int i=0; i<3; i++)
			XMStoreFloat3(&normals[i], normal);
	}
	else
	{
		// Read as normal
		for(int i=0; i<3; i++)
		{
			normals[i].x=vertex_norm[normPos[i]].x;
			normals[i].y=vertex_norm[normPos[i]].y;
			normals[i].z=vertex_norm[normPos[i]].z;
		}
	}

	// If texture didn't exist, create own
	XMFLOAT2 textureUVs[3];
	if(!texExists)
	{
		for(int i=0; i<3; i++)
		{
			// Placeholder value
			textureUVs[i].x=0.75f;
			textureUVs[i].y=0.75f;
		}
	}
	else
	{
		for(int i=0; i<3; i++)
		{
			textureUVs[i].x=vertex_uv[texPos[i]].x;
			textureUVs[i].y=vertex_uv[texPos[i]].y;
		}
	}

	// Create vertex from face
	XMFLOAT3 tmp_position[3];
	XMFLOAT3 tmp_normal[3];
	XMFLOAT2 tmp_texCord[3];
	for(int i=0; i<3; i++)
	{
		tmp_position[i]	=	vertex_pos[vertPos[i]];
		tmp_normal[i]	=	normals[i];
		tmp_texCord[i]	=	textureUVs[i];
	}

	// Calculate tangent
	XMFLOAT3 tangent = createTangent(tmp_position,tmp_texCord);
	Vertex tmp_vertices[3];
	for(int i=0; i<3; i++)
	{
		tmp_vertices[i].Position	= tmp_position[i];
		tmp_vertices[i].Normal		= tmp_normal[i];
		tmp_vertices[i].TexC		= tmp_texCord[i];
		tmp_vertices[i].TangentU	= tangent;
	}

	for(int i=0; i<3; i++)
	{
		// Feed vertex buffer
		meshData->Vertices.push_back(tmp_vertices[i]);
		meshData->Indices.push_back(meshData->Indices.size());
	}
}

XMFLOAT3 GeometryFactory::createTangent( XMFLOAT3 v[3], XMFLOAT2 t[3] )
{
	XMFLOAT3 v1=v[0];
	XMFLOAT3 v2=v[1];
	XMFLOAT3 v3=v[2];

	XMFLOAT2 w1=t[0];
	XMFLOAT2 w2=t[1];
	XMFLOAT2 w3=t[2];

	//Vertex
	float x1 = v2.x - v1.x;
	float x2 = v3.x - v1.x;
	float y1 = v2.y - v1.y;
	float y2 = v3.y - v1.y;
	float z1 = v2.z - v1.z;
	float z2 = v3.z - v1.z;

	//Delta UV
	float s1 = w2.x - w1.x;
	float s2 = w3.x - w1.x;
	float t1 = w2.y - w1.y;
	float t2 = w3.y - w1.y;

	//Q=UV*B
	//B=UV(inverse)*Q

	//UV(inverse) = r*UV(transp)
	float r = 1.0f/(s1*t2-s2*t1);

	//Tangent
	XMFLOAT3 tangent((t2*x1-t1*x2)*r, (t2*y1-t1*y2)*r, (t2*z1-t1*z2)*r);

	////Bitangent
	//D3DXVECTOR3 bitangent((s1*x2-s2*x1)*r, (s1*y2-s2*y1)*r, (s1*z2-s2*z1)*r);

	//return tangent;
	return tangent;
}
