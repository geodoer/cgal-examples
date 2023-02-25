/*
 *\brief 分割连通组件 简单示例
 * 1. 分割组件
 * 2. 根据一些统计值进行分组
 * 3. 分文件夹输出各个组
 */
#include <filesystem>

#include <CGAL/Simple_cartesian.h> //使用double内核，因为不涉及构造与谓词操作
typedef CGAL::Simple_cartesian<double> Kernel;

//Polygon Mesh
#include <CGAL/Surface_mesh/Surface_mesh.h>
typedef Kernel::Point_3				Point;
typedef Kernel::Vector_3            Vector;
typedef CGAL::Surface_mesh<Point>	Mesh;
typedef boost::graph_traits<Mesh>::vertex_descriptor vertex_descriptor;
typedef Mesh::template Property_map<vertex_descriptor, Vector> VNMap;

#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/Polygon_mesh_processing/measure.h>
namespace PMP = CGAL::Polygon_mesh_processing;

struct MeshInfo
{
	unsigned int number_of_vertices;
	unsigned int number_of_edges;
	unsigned int number_of_faces;
	unsigned int area; //实际面积 * 100 （后面位数截断）

	MeshInfo(const Mesh& mesh)
		: number_of_vertices(mesh.number_of_vertices())
		, number_of_edges(mesh.number_of_edges())
		, number_of_faces(mesh.number_of_faces())
		, area( PMP::area(mesh) * 100 )
	{
	}

	bool operator<(const MeshInfo& rhs) const
	{
		if (number_of_vertices != rhs.number_of_vertices)
		{
			return number_of_vertices < rhs.number_of_vertices;
		}
		else if (number_of_edges != rhs.number_of_edges)
		{
			return number_of_edges < rhs.number_of_edges;
		}
		else if (number_of_faces != rhs.number_of_faces)
		{
			return number_of_faces < rhs.number_of_faces;
		}
		
		return area < rhs.area;
	}
};
using MeshGroupMap = std::map<MeshInfo, std::vector<int>>;

int main()
{
	const std::string filename = R"(M:\01code\03geometry\opencv-examples\data\scene_obj\SHTL_SNB_CT_11.obj)";
	const std::string out_dir = R"(M:\01code\03geometry\opencv-examples\data\models)";
	const std::string save_ext = "obj";

	Mesh mesh;
	if (!PMP::IO::read_polygon_mesh(filename, mesh))
	{
		std::cerr << "Invalid input." << std::endl;
		return 1;
	}

	//# 切割成若干连通的网格
	std::vector<Mesh> splitMeshes;
	PMP::split_connected_components(mesh, splitMeshes);

	//# 根据统计值简单分组
	MeshGroupMap groups;

	int idx{ 0 };
	for (auto& subMesh : splitMeshes)
	{
		//计算法向量
		VNMap vnormals = subMesh.template add_property_map<vertex_descriptor, Vector>("v:normals", CGAL::NULL_VECTOR).first;
		CGAL::Polygon_mesh_processing::compute_vertex_normals(subMesh, vnormals);

		MeshInfo info(subMesh);
		groups[info].emplace_back(idx);

		++idx;
	}

	//# 分组保存
	int groupCnt = 0;
	for (const auto& group : groups)
	{
		std::string dir = out_dir + "/" + std::to_string(groupCnt);

		if (false == std::filesystem::exists(dir))
		{
			std::filesystem::create_directories(dir);
		}

		int meshCnt = 0;
		for (const auto& meshIdx : group.second)
		{
			std::string out_path = dir + "/" + std::to_string(meshCnt) + "." + save_ext;
			auto subMesh = splitMeshes[meshIdx];

			if (!CGAL::IO::write_polygon_mesh(out_path, subMesh))
			{
				std::cout << "Save Failed" << std::endl;
			}
			++meshCnt;
		}

		++groupCnt;
	}
	return 0;
}