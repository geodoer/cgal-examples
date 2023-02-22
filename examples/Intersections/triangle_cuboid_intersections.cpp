#include"CGAL/Exact_predicates_exact_constructions_kernel.h"

#include"CGAL/Point_3.h"
#include"CGAL/Segment_3.h"
#include"CGAL/Line_3.h"
#include"CGAL/Plane_3.h"
#include"CGAL/Triangle_3.h"

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;

///3D geometry
typedef Kernel::Point_3         CGPoint3;
typedef Kernel::Segment_3       CGSegment3;
typedef Kernel::Line_3          CGLine3;
typedef Kernel::Plane_3         CGPlane3;
typedef Kernel::Triangle_3      CGTriangle3;
typedef Kernel::Iso_cuboid_3    CGCuboid3;

typedef std::vector<CGPoint3> CGPolyline;
typedef std::list<CGPolyline> CGPolylines;

typedef Kernel::Intersect_3 CGIntersect3;

bool tri_box_intersection()
{
    CGTriangle3 tri(
        { -1.1857270000000000,0.48155900000000001,-1.1335550000000001 },
        { -0.77545799999999998,0.49755899999999997,-1.4025030000000001 },
        { -1.0632760000000001,0.074521000000000004,-1.0156069999999999 }
    );
    CGCuboid3 cuboid(
        { -1.1470848000000000,0.33968590000000010,-1.1337872550000001 },
        { -1.1232090000000001,0.35231284000000007,-1.1065752000000000 }
    );
    CGAL::cpp11::result_of<CGIntersect3(CGTriangle3, CGCuboid3)>::type result;

    try
    {
        if (!CGAL::do_intersect(tri, cuboid))
        {
            return false;
        }

        result = CGAL::intersection(tri, cuboid);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return false;
    }

    if (const CGPoint3* p = boost::get<CGPoint3>(result.get_ptr()))
    {
        std::cout << *p << std::endl;
        return true;
    }
    else if (const CGSegment3* s = boost::get<CGSegment3>(result.get_ptr()))
    {
        std::cout << s->start() << std::endl;
        std::cout << s->end() << std::endl;
        return true;
    }
    else if (const CGTriangle3* tri = boost::get<CGTriangle3>(result.get_ptr()))
    {
        std::cout << *tri << std::endl;
        //tri->vertex(0);
        //tri->vertex(1);
        //tri->vertex(2);
        return true;
    }
    else if (const std::vector<CGPoint3>* points = boost::get<std::vector <CGPoint3>>(result.get_ptr()))
    {
        for (int i = 0; i < points->size(); ++i)
        {
            std::cout << points->at(i) << std::endl;
        }

        return true;
    }

    return false;
}

int main()
{
    tri_box_intersection();

    return 0;
}