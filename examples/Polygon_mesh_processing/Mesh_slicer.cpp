/*@Author: geodoer
 *@Time: 2022/05/08
 *@Desc: 基于CGAL对模型进行切片
 *	CGAL切片的结果为一个线集，并没有闭合成Polygon
 *	正是因为Mesh可能出现质量问题，比如非流形。
 *	如果是非流形，Polylines结果不能够直接闭合成Polygon，需要预处理。
 */
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_slicer.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/AABB_halfedge_graph_segment_primitive.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/Timer.h>

#include"objioplugin.hpp"

using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using Mesh = CGAL::Surface_mesh<K::Point_3>;
using Polyline_type = std::vector<K::Point_3>;
using Polylines = std::list<Polyline_type>;
using HGSP = CGAL::AABB_halfedge_graph_segment_primitive<Mesh>;
using AABB_traits = CGAL::AABB_traits<K, HGSP>;
using AABB_tree = CGAL::AABB_tree<AABB_traits>;
namespace PMP = CGAL::Polygon_mesh_processing;

//后处理
class PostProcessing
{
public:
	//遍历Polylines
	static void Traverse(const Polylines& lines)
	{
		for (auto& line : lines)
		{
			for (const auto& cgpnt : line)
			{
				double x = cgpnt.x();
				double y = cgpnt.y();
				double z = cgpnt.z();
			}
		}
	}

	//TODO：将lines合并成Polygon（Polygon可能内部会有岛）
	static void UnionPolylins(const Polylines& lines)
	{
	}
};

//官方示例
void Sample(const std::string& filename)
{
	//将filename读取成Mesh
	Mesh mesh;
	if (!PMP::IO::read_polygon_mesh(filename, mesh) || is_empty(mesh) || !is_triangle_mesh(mesh))
	{
		std::cerr << "Invalid input." << std::endl;
		return;
	}

	//
	//用Mesh构造出切片器
	//
	CGAL::Polygon_mesh_slicer<Mesh, K> slicer(mesh);
	Polylines polylines; //切片结果是一个线集

	//用一个平面进行切片，获得线集
	slicer(K::Plane_3(0, 0, 1, -0.4), std::back_inserter(polylines));
	std::cout << "At z = 0.4, the slicer intersects " << polylines.size() << " polylines" << std::endl;
	polylines.clear();

	//用一个平面进行切片，获得线集
	slicer(K::Plane_3(0, 0, 1, 0.2), std::back_inserter(polylines));
	std::cout << "At z = -0.2, the slicer intersects " << polylines.size() << " polylines" << std::endl;
	polylines.clear();

	//
	//使用AABB_tree加速
	//
	AABB_tree tree(edges(mesh).first, edges(mesh).second, mesh);
	CGAL::Polygon_mesh_slicer<Mesh, K> slicer_aabb(mesh, tree);
	slicer_aabb(K::Plane_3(0, 0, 1, -0.4), std::back_inserter(polylines));
	std::cout << "At z = 0.4, the slicer intersects " << polylines.size() << " polylines" << std::endl;
	polylines.clear();
}

//性能测试
//按interval间隔多次切片
void PerformanceTest(const std::string& filename, double interval, const std::string& out_dir)
{
	Mesh mesh;
	if (!PMP::IO::read_polygon_mesh(filename, mesh) || is_empty(mesh) || !is_triangle_mesh(mesh))
	{
		std::cerr << "Invalid input." << std::endl;
		return;
	}

	//构造切片器
	AABB_tree tree(edges(mesh).first, edges(mesh).second, mesh);
	CGAL::Polygon_mesh_slicer<Mesh, K> slicer_aabb(mesh, tree);
	Polylines polylines; //切片结果是一个线集

	CGAL::Bbox_3 bbox = CGAL::bbox_3(mesh.points().begin(), mesh.points().end());
	const double z_range = bbox.max(2) - bbox.min(2);
	int size = static_cast<int>(z_range / interval + 0.5);

	CGAL::Timer t;
	t.start();

	//切片
	for (int i{0}; i < size; ++i)
	{
		double z = bbox.min(2) + i * interval;

		slicer_aabb(K::Plane_3(0, 0, 1, z), std::back_inserter(polylines));
		std::cout << "At z = " << z << ", the slicer intersects " << polylines.size() << " polylines" << std::endl;

		ObjIOPlugin::PolylinesToObj(polylines, out_dir + "polylines" + std::to_string(i));

		polylines.clear();
	}

	std::cout << "z的间隔范围" << interval << std::endl;
	std::cout << "切片次数：" << size << std::endl;
	std::cout << t.time() << " 秒." << std::endl;
}

int main(int argc, char* argv[])
{
	const std::string filename = DATA_PATH "rock.obj";
	const std::string out_dir = DATA_PATH;

	//Sample(filename);
	PerformanceTest(filename, 0.5, out_dir);
	return 0;
}
