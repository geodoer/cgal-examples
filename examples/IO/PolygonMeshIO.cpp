#include<iostream>
#include<string>
#include<algorithm>

//Polygon Mesh
#include <CGAL/Surface_mesh/Surface_mesh.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel       K;
typedef K::Point_3                                                Point;
typedef CGAL::Surface_mesh<Point>                                 Surface_mesh;

/*支持obj off ply stl ts(gocad) vtp(vtk)
 *注意
 *1. 数据需是一个二维流形（可能有边界）
 *2. 如果数据不是一个二维流形，可尝试使用CGAL::Polygon_mesh_processing::IO::read_polygon_mesh()，位于GAL/Polygon_mesh_processing/IO/polygon_mesh_io.h
 */
#define CGAL_USE_VTK
#include <CGAL/boost/graph/IO/polygon_mesh_io.h>

int main()
{
	std::string in_path = DATA_PATH "bunny.off";
	std::string out_path = DATA_PATH "bunny.obj";
	Surface_mesh mesh;

	if(!CGAL::IO::read_polygon_mesh(in_path, mesh))
	{
		std::cout << "Read Failed" << std::endl;
		return 0;
	}

	if(!CGAL::IO::write_polygon_mesh(out_path, mesh))
	{
		std::cout << "Save Failed" << std::endl;
		return 0;
	}

	return 0;
}