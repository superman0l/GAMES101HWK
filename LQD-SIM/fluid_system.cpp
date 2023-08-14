#include "fluid_system.hpp"

#define _USE_MATH_DEFINES
#include <iostream>
#include <math.h>

namespace SPH {
	FluidSystem::FluidSystem() {
		m_viscosity = 1.0f;
		m_restDensity = 1000.f;
		m_pointMass = 0.0004f;
		m_gasConstantK = 1.0f;
		m_smoothRadius = 0.01f;

		m_boundartStiffness = 80000.f;//反弹系数 调试刚体碰撞
		m_boundaryDampening = 256.f;
		m_speedLimiting = 30.f;
		m_accelLimiting = 1000.f;
		m_boundary2 = 0.01f;

		m_kernelPoly6 = 315.f / (64.f * M_PI * pow(m_smoothRadius, 9));
		m_kernelSpiky = -45.f / (M_PI * pow(m_smoothRadius, 6));
		m_kernelViscosity = 45.f / (M_PI * pow(m_smoothRadius, 6));

		//ContainerBox = Box(Eigen::Vector2f(0.f, 0.f), Eigen::Vector2f(50.f, 50.f));
		//FluidBox = Box(Eigen::Vector2f(20.f, 10.f), Eigen::Vector2f(30.f, 40.f));
	}
	FluidSystem::~FluidSystem()
	{
	}

	void FluidSystem::Update() {
		m_gridContainer.fillGrid(m_buffer, ContainerBox);
		computePressure();
		computeForce();
		advance();
	}

	void FluidSystem::computePressure() {
		float h2 = m_smoothRadius * m_smoothRadius;

		for (int i = 0; i < m_buffer.countMolecule(); i++) {
			molecule t = m_buffer.getMolecule(i);
			t.neighbor.clear();
			int x, y;
			m_gridContainer.getIndex(t.pos, ContainerBox, x, y);
			std::set<int> mset = m_gridContainer.getAround(x, y);
			float sum = 0.f;
			for (auto j : mset) {
				if (j == i)
					sum += pow(h2, 3.f);
				else {
					molecule t2 = m_buffer.getMolecule(j);
					Eigen::Vector2f pi_pj = (t.pos - t2.pos);
					float r = pi_pj.norm();
					if (m_smoothRadius > r)
					{
						float h2_r2 = h2 - r * r;
						sum += pow(h2_r2, 3.f);  //(h^2-r^2)^3
						t.neighbor.push_back(j);
					}
				}
			}
			t.density = m_kernelPoly6 * m_pointMass * sum;
			t.pressure = (t.density - m_restDensity) * m_gasConstantK;
			m_buffer.setMolecule(i, t);
		}
	}

	void FluidSystem::computeForce() {
		float h2 = m_smoothRadius * m_smoothRadius;

		for (unsigned int i = 0; i < m_buffer.countMolecule(); i++)
		{
			molecule t = m_buffer.getMolecule(i);

			Eigen::Vector2f accel_sum = { 0.f,0.f };
			std::vector<int> mset = t.neighbor;

			for (int j = 0; j < mset.size(); j++)
			{

				molecule tj = m_buffer.getMolecule(mset[j]);
				//r(i)-r(j)
				Eigen::Vector2f ri_rj = t.pos - tj.pos;
				//h-r
				float r = ri_rj.norm();
				float h_r = m_smoothRadius - r;
				//h^2-r^2
				float h2_r2 = h2 - r * r;

				//F_Pressure
				//m_kernelSpiky = -45.0f/(3.141592f * h^6);			
				float pterm = -m_pointMass * m_kernelSpiky * h_r * h_r * (t.pressure + tj.pressure) / (2.f * t.density * tj.density);
				accel_sum += ri_rj * pterm / r;

				//F_Viscosity
				//m_kernelViscosity = 45.0f/(3.141592f * h^6);
				float vterm = m_kernelViscosity * m_viscosity * h_r * m_pointMass / (t.density * tj.density);
				accel_sum += (tj.velocity_eval - t.velocity_eval) * vterm;

				//external force
				accel_sum += m_externalForce;
			}
			t.accel = accel_sum;
			m_buffer.setMolecule(i, t);
		}
	}

	void FluidSystem::advance() {
		float deltaTime = 0.003f;
		
		for (int i = 0; i < m_buffer.countMolecule(); i++) {
			molecule t = m_buffer.getMolecule(i);
			//限制最大速度 
			if (t.velocity.norm() > m_speedLimiting)
			{
				t.velocity *= m_speedLimiting / t.velocity.norm();
				t.accel *= m_accelLimiting / t.accel.norm();
			}
			//碰撞
			float boundwidth = 0.01f;
			float diff = boundwidth - (t.pos[0] - ContainerBox.Min[0]);
			if (diff > 0.f)
			{
				//t.pos[0] = ContainerBox.Min[0];
				float coe = t.pos[0] - ContainerBox.Min[0];
				Eigen::Vector2f norm(1.f, 0.f);
				float adj;
				if (t.pos[0] + t.velocity[0] * deltaTime < ContainerBox.Min[0] && coe > 0)adj = m_boundary2 / coe - m_boundaryDampening * norm.dot(t.velocity_eval);
				else adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(t.velocity_eval);
				t.accel += adj * norm;
			}
			diff = boundwidth - (ContainerBox.Max[0] - t.pos[0]);
			if (diff > 0.f)
			{
				//t.pos[0] = ContainerBox.Max[0];
				float coe =  ContainerBox.Max[0] - t.pos[0];
				Eigen::Vector2f norm(-1.f, 0.f);
				float adj;
				if (t.pos[0] + t.velocity[0] * deltaTime > ContainerBox.Max[0] && coe>0)adj = m_boundary2 / coe - m_boundaryDampening * norm.dot(t.velocity_eval);
				else adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(t.velocity_eval);
				t.accel += adj * norm;
			}
			diff = boundwidth - (t.pos[1] - ContainerBox.Min[1]);
			if (diff > 0.f)
			{
				//t.pos[1] = ContainerBox.Min[1];
				float coe = t.pos[1] - ContainerBox.Min[1];
				Eigen::Vector2f norm(0.f, 1.f);
				float adj;
				if (t.pos[1] + t.velocity[1] * deltaTime < ContainerBox.Min[1] && coe>0)adj = m_boundary2 / coe - m_boundaryDampening * norm.dot(t.velocity_eval);
				else adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(t.velocity_eval);
				t.accel += adj * norm;
			}
			diff = boundwidth - (ContainerBox.Max[1] - t.pos[1]);
			if (diff > 0.f)
			{
				//t.pos[1] = ContainerBox.Max[1];
				float coe =  ContainerBox.Max[1] - t.pos[1];
				Eigen::Vector2f norm(0.f, -1.f);
				float adj;
				if (t.pos[1] + t.velocity[1] * deltaTime > ContainerBox.Max[1] && coe>0)adj = m_boundary2 / coe - m_boundaryDampening * norm.dot(t.velocity_eval);
				else adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(t.velocity_eval);
				t.accel += adj * norm;
			}

			t.velocity = t.velocity + t.accel * deltaTime;
			//simulateVerlet
			
			Eigen::Vector2f temp_position = t.pos;
			t.pos += (1 - 0.00005) * (t.pos - t.last_pos) + t.accel * powf(deltaTime, 2);
			t.last_pos = temp_position;
			m_buffer.setMolecule(i, t);
			
			Eigen::Vector2f vnext = t.velocity + t.accel * deltaTime;			// v(t+1/2) = v(t-1/2) + a(t) dt			
			t.velocity_eval = (t.velocity + vnext) * 0.5f;				// v(t+1) = [v(t-1/2) + v(t+1/2)] * 0.5		used to compute forces later
			t.velocity = vnext;
			
			//t.pos += t.velocity * deltaTime;
			m_buffer.setMolecule(i, t);
		}

	}

	void FluidSystem::init() {
		//fill fluid box
		m_buffer.reset();
		float distance = pow(m_pointMass / m_restDensity, 1.f / 3.f);
		addFluid(FluidBox, distance/2);

		addForce(Eigen::Vector2f(0.f, -9.8f));//重力

		m_gridContainer.init(ContainerBox, m_smoothRadius * 2);
	}

	void FluidSystem::addFluid(Box fluid, float dis) {
		for (float i = fluid.Min[0] + dis / 2; i < fluid.Max[0] - dis / 2; i += dis) {
			for (float j = fluid.Min[1] + dis / 2; j < fluid.Max[1] - dis / 2; j += dis) {
				molecule a = molecule(Eigen::Vector2f(i, j));
				m_buffer.addMolecule(a);
			}
		}
	}
}