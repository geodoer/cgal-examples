#pragma once
#include<string>
#include<fstream>

class ObjIOPlugin
{
public:
	/**
	 * \brief 将Polylines导出成Obj
	 * Polylines类型定义例子：
		using K = CGAL::Exact_predicates_inexact_constructions_kernel;
		using Polyline_type = std::vector<K::Point_3>;
		using Polylines = std::list<Polyline_type>;
	 */
	template<typename Polylines>
	static void PolylinesToObj(const Polylines& lines, const std::string& filename = "./lines");
};

template <typename Polylines>
inline void ObjIOPlugin::PolylinesToObj(const Polylines& lines, const std::string& filename)
{
	if (lines.empty())
	{
		return;
	}

	//将Polyline打散成Segment存储
	//因为有些查看器不支持Polyline，只支持Segment形式的obj
	std::ofstream out(filename + ".obj");
	int pntCnt = 0;

	for (auto& line : lines)
	{
		auto size = line.size();

		for (decltype(size) i{ 1 }; i < size; ++i)
		{
			auto A = line[i - 1];
			auto B = line[i];

			out << "v " << A.x() << " " << A.y() << " " << A.z() << std::endl;
			pntCnt++; //1

			out << "v " << B.x() << " " << B.y() << " " << B.z() << std::endl;
			pntCnt++; //2

			out << "l " << pntCnt - 1 << " " << pntCnt << std::endl;
		}
	}

	out.close();
}