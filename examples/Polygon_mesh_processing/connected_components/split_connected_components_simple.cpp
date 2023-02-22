#include <CGAL/Simple_cartesian.h> //使用double内核，因为不涉及构造与谓词操作
typedef CGAL::Simple_cartesian<double> Kernel;

//Polygon Mesh
#include <CGAL/Surface_mesh/Surface_mesh.h>
typedef Kernel::Point_3				Point;
typedef CGAL::Surface_mesh<Point>	Mesh;

#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
namespace PMP = CGAL::Polygon_mesh_processing;

int main()
{
	const std::string filename = DATA_PATH "轨枕.obj";
	const std::string out_dir = DATA_PATH "submeshs/";

	Mesh mesh;
	if (!PMP::IO::read_polygon_mesh(filename, mesh))
	{
		std::cerr << "Invalid input." << std::endl;
		return 1;
	}

	// 切割成若干连通的网格
	std::list<Mesh> splitMeshes;
	PMP::split_connected_components(mesh, splitMeshes);

	int i{ 1 };
	for (const auto& subMesh : splitMeshes)
	{
		std::string out_path = out_dir + std::to_string(i++) + ".obj";
		if (!CGAL::IO::write_polygon_mesh(out_path, subMesh))
		{
			std::cout << "Save Failed" << std::endl;
		}
	}

	return 0;
}