#pragma once

#include "fluid_buffer.hpp"
#include "fluid_grid.hpp"

#include <unordered_map>

namespace SPH
{	

	class FluidSystem
	{
	public:
		void init();

		//计算相邻关系
		void computePressure(void);
		//计算加速度
		void computeForce(void);
		//移动
		void advance(void);
		//创建液体
		void addFluid(Box fluid, float dis);
		//每帧更新
		void Update();
		//单独添加力
		void addForce(Eigen::Vector2f a) { m_externalForce += a; };
		void setForce(Eigen::Vector2f a) { m_externalForce = a; };

		//utils
		Buffer getBuffer() { return m_buffer; }
		Box getBound() { return ContainerBox; }

	private:
		GridContainer m_gridContainer;
		Buffer m_buffer;

		//Kernel参数
		float m_kernelPoly6;
		float m_kernelSpiky;
		float m_kernelViscosity;

		//水分子参数
		float m_viscosity; //粘度
		float m_restDensity; //常规密度
		float m_pointMass; //质量
		float m_smoothRadius; //光滑核半径
		float m_gasConstantK; //气体常量K

		float m_speedLimiting; //速度限制 
		float m_accelLimiting; //加速度限制
		//以下用于边界计算
		float m_boundartStiffness;//反弹刚度
		float m_boundaryDampening;//边界阻尼
		float m_boundary2;//接近表面斥力计算

		Box FluidBox = Box(Eigen::Vector2f(0.08, 0.04), Eigen::Vector2f(0.12, 0.16)); //液体
		Box ContainerBox = Box(Eigen::Vector2f(0.f, 0.f), Eigen::Vector2f(0.2, 0.2)); //总面积

		Eigen::Vector2f m_externalForce; //外力

	public:
		FluidSystem();
		~FluidSystem();
	};

}