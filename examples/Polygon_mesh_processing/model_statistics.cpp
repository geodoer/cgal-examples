/*
 *\参考
 *  cgal\Polygon_mesh_processing\test\Polygon_mesh_processing\measures_test.cpp
 */
#include <CGAL/Simple_cartesian.h> //使用double内核，因为不涉及构造与谓词操作
typedef CGAL::Simple_cartesian<double> Kernel;

//Polygon Mesh
#include <CGAL/Surface_mesh/Surface_mesh.h>
typedef Kernel::Point_3				Point;
typedef CGAL::Surface_mesh<Point>	Mesh;

#include <CGAL/Polygon_mesh_processing/measure.h>
#include <CGAL/Polygon_mesh_processing/bbox.h>

#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
namespace PMP = CGAL::Polygon_mesh_processing;

#include <filesystem>
bool traversal_file(const std::string& dir, const std::function<bool(const std::string& filepath)>& callback)
{
    if (false == std::filesystem::exists(dir))
    {
        return false; //被强行终止
    }

    std::filesystem::path dirPath(dir);

    for (auto& item : std::filesystem::directory_iterator(dirPath))
    {
        if (std::filesystem::is_directory(item.status()))
        {
			traversal_file(item.path().string(), callback);
        }
        else
        {
			bool is_continue = callback(item.path().string());
			if (false == is_continue)
			{
				return false; //被强行终止
			}
        }
    }

    return true;
}

template<typename Mesh, typename Kernel>
struct MeshStatistics
{
protected:
    typedef typename Kernel::FT FT;

    typedef typename boost::graph_traits<Mesh>::halfedge_descriptor halfedge_descriptor;
    typedef typename boost::graph_traits<Mesh>::face_descriptor     face_descriptor;
    typedef typename boost::graph_traits<Mesh>::vertex_descriptor   vertex_descriptor;
    typedef typename boost::graph_traits<Mesh>::edge_descriptor     edge_descriptor;

public:
	bool operator()(const std::string& file)
	{
		if (!PMP::IO::read_polygon_mesh(file, m_Mesh))
		{
			std::cerr << "Invalid input." << std::endl;
			return false;
		}

        std::cout << "-----" << file << std::endl;

        _computeBaseInfo();
        _computeBorderLength();
        _computeBBox();

        std::cout << std::endl;
		return true;
	}

protected:
    void _computeBaseInfo()
    {
        std::cout << "number of vertices = " << m_Mesh.number_of_vertices() << std::endl;
        std::cout << "number of edges = " << m_Mesh.number_of_edges() << std::endl;
        std::cout << "number of halfedges = " << m_Mesh.number_of_halfedges() << std::endl;
        std::cout << "number of faces = " << m_Mesh.number_of_faces() << std::endl;

        FT mesh_area = PMP::area(m_Mesh);
        std::cout << "mesh area = " << mesh_area << std::endl;

        FT mesh_area_np = PMP::area(m_Mesh, CGAL::parameters::geom_traits(Kernel()));
        std::cout << "mesh area (NP) = " << mesh_area_np << std::endl;

        auto center = PMP::centroid(m_Mesh);
        std::cout << "mesh center of mass = " << center << std::endl;
    }
    void _computeBorderLength()
    {
        //找到第一条边界边
        bool has_border = false;
        halfedge_descriptor border_he;
        for (halfedge_descriptor h : halfedges(m_Mesh))
        {
            if (is_border(h, m_Mesh))
            {
                border_he = h;
                has_border = true;
                break;
            }
        }

        //从第一条边界边开始搜索，计算此边界的长度
        FT border_l = PMP::face_border_length(border_he, m_Mesh);
        std::cout << "length of hole border = " << border_l << std::endl;

        //最长的边界
        std::pair<halfedge_descriptor, FT> res = PMP::longest_border(m_Mesh);
        if (res.first == boost::graph_traits<Mesh>::null_halfedge())
        {
            std::cout << "mesh has no border" << std::endl;
        }
        else
        {
            std::cout << "longest border has length = " << res.second << std::endl;
        }
    }
    void _computeBBox()
    {
        CGAL::Bbox_3 bb = PMP::bbox(m_Mesh);
        std::cout << "mesh bbox = " << bb << std::endl;

        CGAL::Bbox_3 bb_v;
        for (vertex_descriptor vd : vertices(m_Mesh))
            bb_v += PMP::vertex_bbox(vd, m_Mesh);
        std::cout << "vertices bbox = " << bb << std::endl;

        CGAL::Bbox_3 bb_f;
        for (face_descriptor fd : faces(m_Mesh))
            bb_f += PMP::face_bbox(fd, m_Mesh);
        std::cout << "faces bbox = " << bb << std::endl;

        CGAL::Bbox_3 bb_e;
        for (edge_descriptor ed : edges(m_Mesh))
            bb_e += PMP::edge_bbox(ed, m_Mesh);
        std::cout << "edges bbox = " << bb << std::endl;
    }

protected:
	Mesh m_Mesh;
};

int main()
{
	const std::string in_dir = DATA_PATH "submeshs/";

	std::vector<std::string> files;
	traversal_file(in_dir, [&files](const std::string& path)->bool
		{
			files.emplace_back(path);
			return true; //继续
		});

	for (const auto& file : files)
	{
		MeshStatistics<Mesh, Kernel>()(file);
	}

	return 0;
}