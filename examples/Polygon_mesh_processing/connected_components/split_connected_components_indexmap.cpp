/*
 *\brief 分割连通组件，保持原始的下标映射关系
 *\references:
 *	1. polygon_soup_to_mesh使用可选项：https://github.com/sloriot/cgal/blob/835ab5a0fbfd91729daeea69594f3ee9dfe59f0e/Polygon_mesh_processing/test/Polygon_mesh_processing/test_is_polygon_soup_a_polygon_mesh.cpp
 *	2. 分割连通组件参考：https://doc.cgal.org/latest/Polygon_mesh_processing/Polygon_mesh_processing_2connected_components_example_8cpp-example.html#a7
 *\todo: 自定义polygon_soup_to_polygon_mesh
 *		将顶点位置存到mesh.points()，将其他顶点属性做挂接
 *		根据Point的下标获取其他顶点属性
 */

//使用double内核，因为不涉及构造与谓词操作
#include <CGAL/Simple_cartesian.h> 
typedef CGAL::Simple_cartesian<double> Kernel;

//Polygon Mesh
#include <CGAL/Surface_mesh/Surface_mesh.h>
typedef Kernel::Point_3				Point;
typedef CGAL::Surface_mesh<Point>	Mesh;
typedef boost::graph_traits<Mesh>::vertex_descriptor vertex_descriptor;
typedef boost::graph_traits<Mesh>::face_descriptor face_descriptor;
typedef boost::graph_traits<Mesh>::halfedge_descriptor halfedge_descriptor;

#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
namespace PMP = CGAL::Polygon_mesh_processing;

#include <CGAL/boost/graph/Named_function_parameters.h>

struct PointInfo
{
	Point	coordiate;	//顶点坐标
	int			index;	//旧数据的下标
};

struct PointMap
{
	using self = PointMap;
	using key_type = Point; // The iterator's value type is an index
	using value_type = PointInfo;   // The object manipulated by the algorithm is a Point_3
	using reference = PointInfo;    // The object does not exist in memory, so there's no reference
	using category = boost::readable_property_map_tag; // The property map is only used for reading

	std::map<Point, PointInfo> data;

	friend value_type get(const self& map, const key_type& key)
	{
		auto it = map.data.find(key);
		if (it == map.data.end())
		{
			return {};
		}

		return it->second;
	}
};

int main()
{
	const std::string filename = DATA_PATH "轨枕.obj";
	const std::string out_dir = DATA_PATH "submeshs/";

	//# 从文件中读取原样的数据
	std::vector<Point> points;
	std::vector<std::vector<std::size_t> > polygons;
	if (!CGAL::IO::read_polygon_soup(filename, points, polygons))
	{
		return 1;
	}

	//# 创建 Point属性映射，将points的下标存入
	PointMap pointMap;
	for (int i{ 0 }, size = points.size(); i < size; ++i)
	{
		const auto& p = points[i];
		pointMap.data[p] = PointInfo{ p, i };
	}

	//# repair and other
	{
		if (true) //do_repair
		{
			PMP::repair_polygon_soup(points, polygons);
		}

		if (!PMP::orient_polygon_soup(points, polygons))
		{
			std::cerr << "Some duplication happened during polygon soup orientation" << std::endl;
		}

		if (!PMP::is_polygon_soup_a_polygon_mesh(polygons))
		{
			return 1;
		}
	}

	//# PolygonSoup -> PolygonMesh，在转的途中，保持原始下标
	Mesh mesh;

	typedef CGAL::dynamic_vertex_property_t<PointInfo> Point_property;
	typedef typename boost::property_map<Mesh, Point_property>::type CustomVPM;

	//Mesh(Vertex索引+三角网) Vertex索引->Point的映射
	CustomVPM vpm = get(Point_property(), mesh);

	PMP::polygon_soup_to_polygon_mesh(points, polygons, mesh,
		CGAL::parameters::point_map(pointMap),
		CGAL::parameters::vertex_point_map(vpm));

#if 0
	for(const auto& v_idx : CGAL::make_range(mesh.vertices_begin(), mesh.vertices_end()))
	{
		auto pt = get(vpm, v_idx);
		std::cout << pt.coordiate.x() << " " << pt.coordiate.y() << " " << pt.coordiate.z() << " " << pt.index << std::endl;

		//请注意，此时不能用mesh.point()，因为它是空的！
		//	如果你自定义了vertex_point_map（上文使用CustomVPM作为vertex_point_map）
		//		并且将它（vpm）传入了函数polygon_soup_to_polygon_mesh中
		//	那么在polygon_soup_to_polygon_mesh中，它就会用vpm替换掉mesh自带的vertex_point_map
		//		而mesh.point()就是Mesh自带的vertex_point_map
		//	因此，你自定义了vertex_point_map之后，就不能在使用mesh.point()了！
		//		虽然它开辟了空间，但是它依旧是空的，polygon_soup_to_mesh函数并没有对它赋值
		auto pt2 = mesh.point(v_idx);
		std::cout << pt2.x() << " " << pt2.y() << " " << pt2.z() << std::endl;
	}
	/*问题
		1. mesh.point()被开辟了空间，却没有使用，浪费内存
		2. 如果后续还要使用CGAL的API，那就糟了。因为它们中的很多算法都需要用到mesh自带的vertex_point_map
			而mesh.point()是空的，因此，我们不得不把它在塞回去
		3. PointMap中key是使用Point，会降低效率
	  改善：自己编写polygon_soup_to_polygon_mesh
		1. PointMap中，使用points的index来获取顶点属性
		2. 将顶点属性分成 顶点位置、与其他属性。顶点位置存在mesh.point()中，其他属性通过指定的vertex_point_map来挂接
	 */
#endif
	{ //后续要使用到mesh.points()，这里添加进去
		for (const auto& v_idx : CGAL::make_range(mesh.vertices_begin(), mesh.vertices_end()))
		{
			auto pt = get(vpm, v_idx);
			mesh.points()[v_idx] = pt.coordiate;
		}
	}

	//# 切割成若干连通的网格
	Mesh::Property_map<face_descriptor, std::size_t> fccmap =
		mesh.add_property_map<face_descriptor, std::size_t>("f:CC").first; //连接组件标记
	std::size_t cc_num = PMP::connected_components(mesh, fccmap); //分割了num个连通组件

	using MyFace = std::vector<std::size_t>; //旧数据的下标
	using MyMesh = std::vector<MyFace>;
	std::vector<MyMesh> subMeshes(cc_num); //连通组件的网格

	for (face_descriptor fd : faces(mesh))
	{
		halfedge_descriptor hd = halfedge(fd, mesh);

		//这里假定Face全是三角形，直接硬编码取了三角形
		vertex_descriptor tri_vd[3] =
		{
			source(hd, mesh),
			target(hd, mesh),
			target(next(hd, mesh), mesh)
		};

		MyFace face;
		for (auto vd : tri_vd)
		{
			auto p_info = get(vpm, vd);
			face.emplace_back(p_info.index);
		}

		std::size_t subMeshIdx = fccmap[fd]; //此Face属于哪个连通组件
		subMeshes[subMeshIdx].emplace_back(face);
	}

	//# 输出校验
	int i{ 1 };
	for (const auto& subMesh : subMeshes)
	{
		std::string out_path = out_dir + std::to_string(i++) + ".obj";

		std::fstream out(out_path, std::ios_base::out);
		if (!out.is_open())
		{
			return 1;
		}

		//先输出顶点
		for (const auto& p : points)
		{
			out << "v " << p.x() << " " << p.y() << " " << p.z() << std::endl;
		}

		//输出面，obj的下标从1开始
		for (const auto& face : subMesh)
		{
			out << "f";

			for (const auto& idx : face)
			{
				out << " " << idx + 1;
			}

			out << std::endl;
		}
		
		out.close();
	}

	return 0;
}