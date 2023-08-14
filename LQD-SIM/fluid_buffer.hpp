#pragma once

#include <Eigen/Eigen>


namespace SPH {
	struct Box {
		Eigen::Vector2f Min;
		Eigen::Vector2f Max;

		Box(Eigen::Vector2f _Min, Eigen::Vector2f _Max) : Min(_Min), Max(_Max) {}
	};

	struct molecule {
	public:
		Eigen::Vector2f pos = { 0.f,0.f };
		float density = 0.f;
		float pressure = 0.f;
		Eigen::Vector2f accel = { 0.f,0.f };

		Eigen::Vector2f velocity = { 0.f,0.f };
		Eigen::Vector2f last_pos = { 0.f,0.f };
		Eigen::Vector2f velocity_eval = { 0.f,0.f };

		std::vector<int> neighbor;
		molecule(Eigen::Vector2f a) :pos(a),last_pos(a) {}
	};

	class Buffer {
	public:
		void reset() { FluidBuffer.clear(); };
		void addMolecule(molecule a) { FluidBuffer.push_back(a); }
		molecule getMolecule(int index) { return FluidBuffer[index]; }
		int countMolecule() { return FluidBuffer.size(); }
		void setMolecule(int index, molecule a) { if (index >= 0 && index < countMolecule())FluidBuffer[index] = a; }

	private:
		std::vector<molecule> FluidBuffer;
	};
}