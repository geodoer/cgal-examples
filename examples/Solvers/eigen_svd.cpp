/*
 *\brief Eigen库的SVD（Singular Value Decomposition，奇异值分解）
 *\desc:
	CGAL中的SVD是封装Eigen的，但是接口不够灵活，因此可直接基于Eigen实现。
	CGAL的SVD可参考：cgal_svd.cpp 
 *\references:
 *	1. https://blog.csdn.net/qq_39400324/article/details/123604126
 */
#include<iostream>
#include<Eigen/SVD>

int main()
{
	Eigen::MatrixXf m = Eigen::MatrixXf::Zero(3, 2);
	m << 0, 1, 1, 1, 1, 0;
	std::cout << "Here is the matrix m:" << std::endl << m << std::endl;
	Eigen::JacobiSVD<Eigen::MatrixXf> svd(m, Eigen::ComputeFullU | Eigen::ComputeFullV);
	std::cout << "Its singular values are:" << std::endl << svd.singularValues() << std::endl;
	std::cout << "Its left singular vectors are the columns of the thin U matrix:" << std::endl << std::endl << svd.matrixU() << std::endl;
	std::cout << "Its right singular vectors are the columns of the thin V matrix:" << std::endl << std::endl << svd.matrixV() << std::endl;
	return 0;
}
