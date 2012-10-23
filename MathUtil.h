//***************************************************************************************
// MathUtil.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Helper math class.
//***************************************************************************************

#ifndef MathUtil_H
#define MathUtil_H

#include <Windows.h>
#include <xnamath.h>
#include <Vector>

class MathUtil
{
public:
	// Returns random float in [0, 1).
	static float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// Returns random float in [a, b).
	static float RandF(float a, float b)
	{
		return a + RandF()*(b-a);
	}

	template<typename T>
	static T Min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<typename T>
	static T Max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}
	 
	template<typename T>
	static T Lerp(const T& a, const T& b, float t)
	{
		return a + (b-a)*t;
	}

	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x); 
	}

	// Returns the polar angle of the point (x,y) in [0, 2*PI).
	static float AngleFromXY(float x, float y);

	static XMMATRIX InverseTranspose(CXMMATRIX M)
	{
		// Inverse-transpose is just applied to normals.  So zero out 
		// translation row so that it doesn't get into our inverse-transpose
		// calculation--we don't want the inverse-transpose of the translation.
		XMMATRIX A = M;
		A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		XMVECTOR det = XMMatrixDeterminant(A);
		return XMMatrixTranspose(XMMatrixInverse(&det, A));
	}

	static XMVECTOR RandUnitVec3();
	static XMVECTOR RandHemisphereUnitVec3(XMVECTOR n);

	static int gcd(int a, int b)
	{
		int c = a % b;
		while(c != 0)
		{
			a = b;
			b = c;
			c = a % b;
		}
		return b;
	}
};

class DynamicArray2D
{
private:
	std::vector<float> data;
public:
	int size_x;
	int size_y;
	int size_total;

	DynamicArray2D()
	{
	}
	void copy(DynamicArray2D in)
	{
		for(int i=0; i<in.size_total; i++)
			data[i] = in.get(i); 
	}
	void resize(int x, int y)
	{
		size_x = x;
		size_y = y;
		size_total = size_x*size_y;
		data.resize(size_total);
	}
	void resize(DynamicArray2D *in)
	{
		size_x = in->size_x;
		size_y = in->size_y;
		size_total = size_x*size_y;
		data.resize(size_total);
	}
	void set(int i, float value)
	{
		data[i] = value;
	}
	void set(int x, int y,float value)
	{
		data[x+size_y*y] = value;
	}
	float get(int i)
	{
		return data[i];
	}
	float get(int x, int y)
	{
		float ret = data[x+size_y*y];
		return ret;
	}
	float safe_get(int x, int y)
	{
		float ret = 0.0f;
		if(isValidIndex(x,y))
			ret = data[x+size_y*y];
		return ret;
	}
	bool isValidIndex(int x, int y)
	{
		return 
			x>=0 && x<size_x && 
			y>=0 && y<size_y;
	}
	float average(int x_source, int y_source)
	{
		float average = 0.0f;
		int nrOfsamples = 0;

		for(int y=y_source-1; y<=y_source+1; y++)
		{
			for(int x = x_source-1; x<=x_source+1; x++)
			{
				if(isValidIndex(x,y))
				{
					average += get(x,y);
					nrOfsamples++;
				}
			}
		}

		float ret =  average/nrOfsamples;
		return ret;
	}
	void smooth()
	{
		// Temp array to store filtered array
		DynamicArray2D smooth_heightMap;
		smooth_heightMap.resize(this);

		// Filter array
		for(int y=0; y<size_y; y++)
		{
			for(int x=0; x<size_x; x++)
			{
				smooth_heightMap.set(x, y, average(x,y));
			}
			int test = y;
		}
			

		// Replace old array with filtered one
		copy(smooth_heightMap);
	}
};

#endif // MathUtil_H