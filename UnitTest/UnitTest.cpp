#include "pch.h"
#include "CppUnitTest.h"
#include"../ProjectDirectX12/utility.hpp"
#include<DirectXMath.h>


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	using namespace DirectX;

	TEST_CLASS(UnitTest)
	{
	public:	
		TEST_METHOD(TestMethod1)
		{
			XMFLOAT3 eye{ 0.f,0.f,0.f };
			float asspect = 0.5f;
			float nearZ = 0.f;
			float farZ = 100.f;
			float viewAngle = XM_PIDIV2;
			XMFLOAT3 forward{ 0.f,0.f,1.f };
			XMFLOAT3 right{ 1.f,0.f,0.f };

			std::array<XMFLOAT3, 8> vertex1{};

			pdx12::get_frustum_vertex(eye, asspect, nearZ, farZ, viewAngle, forward, right, vertex1);

			float nearY = std::tanf(viewAngle * 0.5f) * nearZ;
			float nearX = nearY * asspect;

			float farY = std::tanf(viewAngle * 0.5f) * farZ;
			float farX = farY * asspect;

			std::array<XMFLOAT3, 8> vertex2{ {
				{nearX,nearY,0.f},
				{-nearX,nearY,0.f},
				{-nearX,-nearY,0.f},
				{nearX,-nearY,0.f},
				{farX,farY,farZ},
				{-farX,farY,farZ},
				{-farX,-farY,farZ},
				{farX,-farY,farZ},
			} };

			Assert::AreEqual(vertex2[0].x, vertex1[0].x);
			Assert::AreEqual(vertex2[1].x, vertex1[1].x);
			Assert::AreEqual(vertex2[2].x, vertex1[2].x);
			Assert::AreEqual(vertex2[3].x, vertex1[3].x);
			Assert::AreEqual(vertex2[4].x, vertex1[4].x);
			Assert::AreEqual(vertex2[5].x, vertex1[5].x);
			Assert::AreEqual(vertex2[6].x, vertex1[6].x);
			Assert::AreEqual(vertex2[7].x, vertex1[7].x);

			Assert::AreEqual(vertex2[0].y, vertex1[0].y);
			Assert::AreEqual(vertex2[1].y, vertex1[1].y);
			Assert::AreEqual(vertex2[2].y, vertex1[2].y);
			Assert::AreEqual(vertex2[3].y, vertex1[3].y);
			Assert::AreEqual(vertex2[4].y, vertex1[4].y);
			Assert::AreEqual(vertex2[5].y, vertex1[5].y);
			Assert::AreEqual(vertex2[6].y, vertex1[6].y);
			Assert::AreEqual(vertex2[7].y, vertex1[7].y);

			Assert::AreEqual(vertex2[0].z, vertex1[0].z);
			Assert::AreEqual(vertex2[1].z, vertex1[1].z);
			Assert::AreEqual(vertex2[2].z, vertex1[2].z);
			Assert::AreEqual(vertex2[3].z, vertex1[3].z);
			Assert::AreEqual(vertex2[4].z, vertex1[4].z);
			Assert::AreEqual(vertex2[5].z, vertex1[5].z);
			Assert::AreEqual(vertex2[6].z, vertex1[6].z);
			Assert::AreEqual(vertex2[7].z, vertex1[7].z);


			XMMATRIX matrix{};
			pdx12::get_clop_matrix(vertex1, matrix);

			for (std::size_t i = 0; i < 8; i++)
				pdx12::apply(vertex1[i], matrix);

			Assert::IsTrue(-1.f <= vertex1[0].x && vertex1[0].x <= 1.f);
			Assert::IsTrue(-1.f <= vertex1[1].x && vertex1[1].x <= 1.f);
			Assert::IsTrue(-1.f <= vertex1[2].x && vertex1[2].x <= 1.f);
			Assert::IsTrue(-1.f <= vertex1[3].x && vertex1[3].x <= 1.f);
			Assert::IsTrue(-1.f <= vertex1[4].x && vertex1[4].x <= 1.f);
			Assert::IsTrue(-1.f <= vertex1[5].x && vertex1[5].x <= 1.f);
			Assert::IsTrue(-1.f <= vertex1[6].x && vertex1[6].x <= 1.f);
			Assert::IsTrue(-1.f <= vertex1[7].x && vertex1[7].x <= 1.f);

			Assert::IsTrue(-1.f <= vertex1[0].y && vertex1[0].y <= 1.f);
			Assert::IsTrue(-1.f <= vertex1[1].y && vertex1[1].y <= 1.f);
			Assert::IsTrue(-1.f <= vertex1[2].y && vertex1[2].y <= 1.f);
			Assert::IsTrue(-1.f <= vertex1[3].y && vertex1[3].y <= 1.f);
			Assert::IsTrue(-1.f <= vertex1[4].y && vertex1[4].y <= 1.f);
			Assert::IsTrue(-1.f <= vertex1[5].y && vertex1[5].y <= 1.f);
			Assert::IsTrue(-1.f <= vertex1[6].y && vertex1[6].y <= 1.f);
			Assert::IsTrue(-1.f <= vertex1[7].y && vertex1[7].y <= 1.f);

		}
	};
}