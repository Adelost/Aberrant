
#ifndef CAMERA_H
#define CAMERA_H

#include "Util.h"

class Camera
{
private:
	// Camera coordinate system with coordinates relative to world space.
	XMFLOAT3 mPosition;
	XMFLOAT3 mRight;
	XMFLOAT3 mUp;
	XMFLOAT3 mLook;
	bool debug;

	// Cache frustum properties.
	float mNearZ;
	float mFarZ;
	float mAspect;
	float mFovY;
	float mNearWindowHeight;
	float mFarWindowHeight;
	float walkingSpeed;

	// Cache View/Proj matrices.
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	TwBar *tw_menu;

public:
	float height;
	float smoothFactor;

	Camera()
	{
		mPosition = XMFLOAT3(0.0f, 50.0f, 0.0f); 
		mRight = XMFLOAT3(1.0f, 0.0f, 0.0f);
		mUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
		mLook = XMFLOAT3(0.0f, 0.0f, 1.0f);
		walkingSpeed = 8.0f;

		height = 15.0f;
		smoothFactor = 8.0f;

		debug = false;
	};
	~Camera(){};

	// Get/Set world camera position.
	void buildMenu(TwBar* menu)
	{
		TwAddVarRW(menu, "Walking speed", TW_TYPE_FLOAT, &walkingSpeed, "group=Camera");
		TwAddVarRW(menu, "Position", TW_TYPE_DIR3F, &mPosition, "group=Camera");
		TwAddVarRW(menu, "Direction", TW_TYPE_DIR3F, &mLook, "group=Camera");
		TwAddVarRW(menu, "Debug camera", TW_TYPE_BOOLCPP, &debug, "group=Camera");
		TwDefine("Settings/Camera opened=false");
	};

	// Get/Set world camera position.
	XMVECTOR GetPositionXM()
	{
		return XMLoadFloat3(&mPosition);
	};
	XMFLOAT3 GetPosition()
	{
		return mPosition;
	};
	void SetPosition(float x, float y, float z)
	{
		mPosition = XMFLOAT3(x, y, z);
	};
	void SetPosition(const XMFLOAT3& v)
	{
		mPosition = v;
	};
	void ModifyHeight(float h)
	{
		mPosition.y += h;
	};

	// Get camera basis vectors.
	XMVECTOR GetRightXM(){};
	XMFLOAT3 GetRight(){};
	XMVECTOR GetUpXM(){};
	XMFLOAT3 GetUp(){};
	XMVECTOR GetLookXM()
	{
		return XMLoadFloat3(&mLook);
	};
	XMFLOAT3 GetLook()
	{
		return mLook;
	};

	// Set frustum.
	void SetLens(float fovY, float aspect, float zn, float zf)
	{
		// cache properties
		mFovY = fovY;
		mAspect = aspect;
		mNearZ = zn;
		mFarZ = zf;

		mNearWindowHeight = 2.0f * mNearZ * tanf( 0.5f*mFovY );
		mFarWindowHeight  = 2.0f * mFarZ * tanf( 0.5f*mFovY );

		XMMATRIX P = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
		XMStoreFloat4x4(&mProj, P);
	};

	// Get View/Proj matrices.
	XMMATRIX View()
	{
		return XMLoadFloat4x4(&mView);
	};
	XMMATRIX Proj()
	{
		return XMLoadFloat4x4(&mProj);
	};
	XMMATRIX ViewProj()
	{
		if(debug)
		{
			UpdateViewMatrixDebug();
		}
		return XMMatrixMultiply(View(), Proj());
	};
	XMMATRIX ViewProjDebug()
	{
		UpdateViewMatrix();
		return XMMatrixMultiply(View(), Proj());
	};

	// Strafe/Walk the camera a distance d.
	void Strafe(float d)
	{
		// mPosition += d*mRight
		XMVECTOR s = XMVectorReplicate(d*walkingSpeed);
		XMVECTOR r = XMLoadFloat3(&mRight);
		XMVECTOR p = XMLoadFloat3(&mPosition);
		XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, r, p));
	};
	void Walk(float d)
	{
		// mPosition += d*mLook
		XMVECTOR s = XMVectorReplicate(d*walkingSpeed);
		XMVECTOR l = XMLoadFloat3(&mLook);
		XMVECTOR p = XMLoadFloat3(&mPosition);
		XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, l, p));
	};
	void Pitch(float angle){
		// Rotate up and look vector about the right vector.
		XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mRight), angle);

		XMStoreFloat3(&mUp,   XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
		XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
	};
	void RotateY(float angle)
	{
		// Rotate the basis vectors about the world y-axis.
		XMMATRIX R = XMMatrixRotationY(angle);

		XMStoreFloat3(&mRight,   XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
		XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
		XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
	};
	// After modifying camera position/orientation, call to rebuild the view matrix.
	void UpdateViewMatrix()
	{
		XMVECTOR R = XMLoadFloat3(&mRight);
		XMVECTOR U = XMLoadFloat3(&mUp);
		XMVECTOR L = XMLoadFloat3(&mLook);
		XMVECTOR P = XMLoadFloat3(&mPosition);

		// Keep camera's axes orthogonal to each other and of unit length.
		L = XMVector3Normalize(L);
		U = XMVector3Normalize(XMVector3Cross(L, R));

		// U, L already ortho-normal, so no need to normalize cross product.
		R = XMVector3Cross(U, L); 

		// Fill in the view matrix entries.
		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, L));

		XMStoreFloat3(&mRight, R);
		XMStoreFloat3(&mUp, U);
		XMStoreFloat3(&mLook, L);

		mView(0,0) = mRight.x; 
		mView(1,0) = mRight.y; 
		mView(2,0) = mRight.z; 
		mView(3,0) = x;   

		mView(0,1) = mUp.x;
		mView(1,1) = mUp.y;
		mView(2,1) = mUp.z;
		mView(3,1) = y;  

		mView(0,2) = mLook.x; 
		mView(1,2) = mLook.y; 
		mView(2,2) = mLook.z; 
		mView(3,2) = z;   

		mView(0,3) = 0.0f;
		mView(1,3) = 0.0f;
		mView(2,3) = 0.0f;
		mView(3,3) = 1.0f;
	};
	void UpdateViewMatrixDebug()
	{
		XMVECTOR R = XMLoadFloat3(&mRight);
		XMVECTOR U = XMLoadFloat3(&mUp);
		XMVECTOR L = XMLoadFloat3(&XMFLOAT3(0.0f,-1.0f,0.0f));
		XMVECTOR P = XMLoadFloat3(&XMFLOAT3(0.0f, 500.0f,0.0f));

		// Keep camera's axes orthogonal to each other and of unit length.
		L = XMVector3Normalize(L);
		U = XMVector3Normalize(XMVector3Cross(L, R));

		// U, L already ortho-normal, so no need to normalize cross product.
		R = XMVector3Cross(U, L); 

		// Fill in the view matrix entries.
		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, L));

		XMFLOAT3 tmpLook;
		XMStoreFloat3(&mRight, R);
		XMStoreFloat3(&mUp, U);
		XMStoreFloat3(&tmpLook, L);

		mView(0,0) = mRight.x; 
		mView(1,0) = mRight.y; 
		mView(2,0) = mRight.z; 
		mView(3,0) = x;   

		mView(0,1) = mUp.x;
		mView(1,1) = mUp.y;
		mView(2,1) = mUp.z;
		mView(3,1) = y;  

		mView(0,2) = tmpLook.x; 
		mView(1,2) = tmpLook.y; 
		mView(2,2) = tmpLook.z; 
		mView(3,2) = z;   

		mView(0,3) = 0.0f;
		mView(1,3) = 0.0f;
		mView(2,3) = 0.0f;
		mView(3,3) = 1.0f;
	};
};

#endif // CAMERA_H