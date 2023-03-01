/*
 *\brief 解算N个点对的变换矩阵
 */
#include<CGAL/Simple_cartesian.h>
using Kernel = CGAL::Simple_cartesian<double>;
using Point3 = Kernel::Point_3;

#include<Eigen/Core>
#include<Eigen/Dense> //Matrix.determinant
#include<Eigen/SVD>

Eigen::Matrix4d solveTransformMatrix(const std::vector<Point3>& A, const std::vector<Point3>& B)
{
	int numOfPnts = A.size();

	//# 至少3个点对
	if (numOfPnts < 3 || numOfPnts != B.size())
	{
		return {};
	}

	//# 计算质心
	double centroidA[3] = {0}, centroidB[3] = {0};
	{
		for (int i{ 0 }; i < numOfPnts; ++i)
		{
			for (int j{ 0 }; j < 3; ++j)
			{
				centroidA[j] += A[i][j];
				centroidB[j] += A[i][j];
			}
		}

		for (int j{ 0 }; j < 3; ++j)
		{
			centroidA[j] /= numOfPnts;
			centroidB[j] /= numOfPnts;
		}
	}

	//# 去质心坐标
	Eigen::MatrixXd ASubCentroid(3, numOfPnts);
	Eigen::MatrixXd BSubCentroid(3, numOfPnts);

	for (int i = 0; i < numOfPnts; ++i)//N组点
	{
		//三行
		ASubCentroid(0, i) = A[i].x() - centroidA[0];
		ASubCentroid(1, i) = A[i].y() - centroidA[1];
		ASubCentroid(2, i) = A[i].z() - centroidA[2];

		BSubCentroid(0, i) = B[i].x() - centroidB[0];
		BSubCentroid(1, i) = B[i].y() - centroidB[1];
		BSubCentroid(2, i) = B[i].z() - centroidB[2];
	}

	//# 计算旋转矩阵
	auto matS = ASubCentroid * BSubCentroid.transpose();

	Eigen::JacobiSVD<Eigen::MatrixXd> svd(matS, Eigen::ComputeFullU | Eigen::ComputeFullV);
	auto matU = svd.matrixU();
	auto matV = svd.matrixV().transpose();

	auto matTemp = matU * matV;
	double det = matTemp.determinant();//行列式的值

	Eigen::Matrix3d matM;
	matM << 1, 0, 0, 0, 1, 0, 0, 0, det;
	auto matR = matV.transpose() * matM * matU.transpose();

	////# 计算平移量
	double delta_X = centroidB[0] - (centroidA[0] * matR(0, 0) + centroidA[1] * matR(0, 1) + centroidA[2] * matR(0, 2));
	double delta_Y = centroidB[1] - (centroidA[0] * matR(1, 0) + centroidA[1] * matR(1, 1) + centroidA[2] * matR(1, 2));
	double delta_Z = centroidB[2] - (centroidA[0] * matR(2, 0) + centroidA[1] * matR(2, 1) + centroidA[2] * matR(2, 2));

	//# 转成4x4矩阵
	Eigen::Matrix4d RT;
	RT <<
		matR(0, 0), matR(0, 1), matR(0, 2), delta_X,
		matR(1, 0), matR(1, 1), matR(1, 2), delta_Y,
		matR(2, 0), matR(2, 1), matR(2, 2), delta_Z,
		0, 0, 0, 1;

	return RT;
}

int main()
{
	std::vector<Point3> srcPoints
	{
		{1,3,2},
		{2,3,5},
		{3,7,4},
		{4,9,3},
		{5,4,2},
		{6,4,6},
		{7,0,2}
	};
	std::vector<Point3> dstPoints = srcPoints;

	auto matrix = solveTransformMatrix(srcPoints, dstPoints);
	std::cout << matrix << std::endl;

	return 0;
}
