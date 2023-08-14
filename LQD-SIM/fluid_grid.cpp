#include "fluid_grid.hpp"

#include <set>

namespace SPH {
	void GridContainer::init(Box a, float b) {
		int row, column;
		row = ceil((a.Max[0] - a.Min[0]) / b);
		column = ceil((a.Max[1] - a.Min[1]) / b);
		GridData.resize(row, std::vector<std::set<int>>(column, std::set<int>()));
		GridSize = row * column;
		GridX = row; GridY = column;
	}

	void GridContainer::fillGrid(Buffer b, Box box) {
		GridData.clear();
		GridData.resize(GridX, std::vector<std::set<int>>(GridY, std::set<int>()));

		for (int i = 0; i < b.countMolecule(); i++) {
			int x, y;
			getIndex(b.getMolecule(i).pos, box, x, y);
			GridData[x][y].insert(i);
		}
	}

	void GridContainer::getIndex(Eigen::Vector2f a, Box b, int& x, int& y) {
		x = (int)(a[0] / (b.Max[0] - b.Min[0]));
		y = (int)(a[1] / (b.Max[1] - b.Min[1]));
		if (x < 0)x = 0;
		if (y < 0)y = 0;
		if (x >= GridX)x = GridX - 1;
		if (y >= GridY)y = GridY - 1;
	}

	std::set<int> GridContainer::getAround(int i, int j) {
		int startx = i < 1 ? 0 : i - 1, endx = i + 1 >= GridX ? GridX - 1 : i + 1;
		int starty = j < 1 ? 0 : j - 1, endy = j + 1 >= GridY ? GridY - 1 : j + 1;
		std::set<int> result;
		for (int x = startx; x <= endx; x++) {
			for (int y = starty; y <= endy; y++) {
				std::set_union(result.begin(), result.end(), GridData[x][y].begin(), GridData[x][y].end(), inserter(result, result.begin()));
			}
		}
		return result;
	}
}