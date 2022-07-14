#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include <vector>

namespace Utils
{
	double CombinationCount(double n, double k);
	double CombinationCount(unsigned int n, unsigned int k);
	void EulerEstimation(std::vector<double>& mu,
			unsigned int b,
			double rho,
			int d,
			double h,
			double max_diff,
			int itr_max);
	
	class Helper_Functions
	{
	public:
	};
}

#endif // !HELPER_FUNCTIONS_H
