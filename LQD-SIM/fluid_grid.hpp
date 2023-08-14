#include<vector>
#include<set>
#include<algorithm>

#include "fluid_buffer.hpp"

namespace SPH {
	class GridContainer {
	public:
		void init(Box a, float size);
		void fillGrid(Buffer b, Box box);
		void addGridData(int i, int j, int element) { GridData[i][j].insert(element); }
		std::set<int> getGridData(int i, int j) { return GridData[i][j]; }
		std::set<int> getAround(int i, int j);
		int getGridSize() { return GridSize; }
		void getIndex(Eigen::Vector2f a, Box b, int& x, int& y);

	private:
		std::vector<std::vector<std::set<int>>> GridData;
		int GridSize;
		int GridX;
		int GridY;
	};
}