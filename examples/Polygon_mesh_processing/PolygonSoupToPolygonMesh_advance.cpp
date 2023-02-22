/*
 *\brief PolygonSoup转PolygonMesh 高级编 使用可选参数
 *\reference
 *  https://github.com/sloriot/cgal/blob/835ab5a0fbfd91729daeea69594f3ee9dfe59f0e/Polygon_mesh_processing/test/Polygon_mesh_processing/test_is_polygon_soup_a_polygon_mesh.cpp
 */
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

#include <CGAL/Polyhedron_3.h>
#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>
#include <CGAL/boost/graph/properties_Polyhedron_3.h>

#include <CGAL/boost/graph/helpers.h>
#include <CGAL/boost/graph/property_maps.h>
#include <CGAL/Dynamic_property_map.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>
#include <CGAL/Polygon_mesh_processing/polygon_mesh_to_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/orient_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>

#include <string>
#include <fstream>
#include <iostream>

typedef CGAL::Simple_cartesian<double> SC;
typedef CGAL::Exact_predicates_exact_constructions_kernel Epec;


template <typename K>
void test_polygon_soup(std::string fname, bool expected)
{
    typedef typename K::Point_3                                                 Point;

    typedef CGAL::Polyhedron_3<K>                                               Polyhedron;
    typedef typename boost::graph_traits<Polyhedron>::vertex_descriptor         vertex_descriptor;

    std::vector<Point> points;
    std::vector< std::vector<std::size_t> > polygons;
    std::ifstream input(fname.c_str());

    if (!input)
    {
        std::cerr << "Error opening file " << fname << "\n";
        exit(EXIT_FAILURE);
    }

    if(!CGAL::IO::read_polygon_soup(fname, points, polygons))
    {
        std::cerr << "Error parsing the model file " << fname << "\n";
        exit(EXIT_FAILURE);
    }

    bool is_mesh = CGAL::Polygon_mesh_processing::is_polygon_soup_a_polygon_mesh(polygons);
    std::cout << "is_polygon_soup_a_polygon_mesh(" << fname << ") == "
        << std::boolalpha << is_mesh << ";" << std::endl;
    assert(is_mesh == expected);

    if (is_mesh)
    {
        Polyhedron mesh;

        // just to test the named paramers
        typedef std::pair<Point, bool>                                            Point_with_Boolean;
        std::vector<Point_with_Boolean> points_with_pairs;
        for (const Point& pt : points)
        {
            points_with_pairs.emplace_back(pt, false);
        }

        typedef CGAL::dynamic_vertex_property_t<Point>                            Point_property;
        typedef typename boost::property_map<Polyhedron, Point_property>::type    Custom_VPM;

        Custom_VPM vpm = get(Point_property(), mesh);
        
        /* polygon_soup_to_polygon_mesh(m_points, polygons, mesh, m_pm, vpm)
			m_pm        可自定义                            点属性映射（point_map）
                用处：因为m_points[i]可能不仅仅包含了位置信息
                      因此需要从m_points[i]中拿到位置信息，然后构造Mesh中的Point

			vpm         Property_map<Vertex_index, Point>   顶点-点属性映射（vertex_point_map）
				用处：根据Mesh的顶点索引，获取到Point(顶点信息，主要是坐标)
				获得方法，例：auto vpm = get(CGAL::vertex_point, mesh);

			PM_Point    vpm类型的value_type，默认就是Point

			vertices    vector<vertex_descriptor>   mesh中的顶点集（索引）

		 *  1. 遍历points
			for i in point.size():
				PM_Point pi = convert_to_pm_point<PM_Point>(get(m_pm, m_points[i]));
				put(vpm, vertices[i], pi);

			2. 构造PM_Point
				get(m_pm, m_points[i])
					从m_pm中，根据m_points[i]获取Point的属性值------
																   |
				PM_Point pi = convert_to_pm_point<PM_Point>( 获得到的 值);

			3. 放入vertex_point_map中
				put(vpm, vertices[i], pi)
		 */
        CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(
            points_with_pairs, polygons, mesh,
            CGAL::parameters::point_map(CGAL::First_of_pair_property_map<Point_with_Boolean>()),
            CGAL::parameters::vertex_point_map(vpm)
        );
         /* 
            First_of_pair_property_map 取Point_with_Boolean的first作为Point的属性
          */

        std::cout << num_vertices(mesh) << " nv and " << num_faces(mesh) << " nf" << std::endl;
        assert(!CGAL::is_empty(mesh) && CGAL::is_valid_polygon_mesh(mesh));

        std::set<Point> ppts;
        for (const vertex_descriptor v : vertices(mesh))
        {
            ppts.insert(get(vpm, v));
        }

        assert(ppts.size() == num_vertices(mesh));

        // twice to check if adds correctly
        std::deque<Point> soup_points;
        std::vector<std::deque<std::size_t> > soup_polygons;

        CGAL::Polygon_mesh_processing::polygon_mesh_to_polygon_soup(mesh, soup_points, soup_polygons);
        CGAL::Polygon_mesh_processing::polygon_mesh_to_polygon_soup(mesh, soup_points, soup_polygons);

        std::size_t nv = static_cast<std::size_t>(num_vertices(mesh));
        std::size_t nf = static_cast<std::size_t>(num_faces(mesh));

        assert(soup_points.size() == 2 * nv);
        assert(soup_polygons.size() == 2 * nf);

        // check sanity of the polygons
        for (std::size_t fi = 0; fi < nf; ++fi)
        {
            for (const std::size_t pi : soup_polygons[fi]) {
                assert(pi < nv);
            }
        }

        for (std::size_t fi = nf; fi < 2 * nf; ++fi)
        {
            for (const std::size_t pi : soup_polygons[fi]) {
                assert(nv <= pi && pi < 2 * nv);
            }
        }
    }

    if (!expected)
    {
        CGAL::Polygon_mesh_processing::orient_polygon_soup(points, polygons);
        bool is_mesh = CGAL::Polygon_mesh_processing::is_polygon_soup_a_polygon_mesh(polygons);
        std::cout << "After orientation: is_polygon_soup_a_polygon_mesh(" << fname << ") == "
            << std::boolalpha << is_mesh << ";" << std::endl;
        if (is_mesh)
        {
            Polyhedron p;
            CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(points, polygons, p);
            std::cout << num_vertices(p) << " nv and " << num_faces(p) << " nf" << std::endl;
            assert(!CGAL::is_empty(p) && CGAL::is_valid_polygon_mesh(p));
        }
    }

    std::cout << fname << " OK\n\n\n";
}

int main()
{
    test_polygon_soup<SC>(DATA_PATH "轨枕.obj", true);
    test_polygon_soup<Epec>(DATA_PATH "轨枕.obj", true);

    return EXIT_SUCCESS;
}