#include"CGAL/Exact_predicates_exact_constructions_kernel.h"

#include"CGAL/Point_3.h"
#include"CGAL/Segment_3.h"
#include"CGAL/Line_3.h"
#include"CGAL/Plane_3.h"
#include"CGAL/Triangle_3.h"

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;

///3D geometry
typedef Kernel::Point_3     CGPoint3;
typedef Kernel::Segment_3   CGSegment3;
typedef Kernel::Line_3      CGLine3;
typedef Kernel::Plane_3     CGPlane3;
typedef Kernel::Triangle_3  CGTriangle3;

typedef std::vector<CGPoint3> CGPolyline;
typedef std::list<CGPolyline> CGPolylines;

typedef Kernel::Intersect_3 CGIntersect3;

bool test()
{
    CGPoint3 tri1Pnts[3] =
    {
        {1,2,3},
        {4,5,6},
        {7,8,9}
    };
    CGPoint3 tri2Pnts[3] =
    {
        {1,2,3},
        {4,5,6},
        {7,8,9}
    };

    CGTriangle3 TriangleA(tri1Pnts[0], tri1Pnts[1], tri1Pnts[2]);
    CGTriangle3 TriangleB(tri2Pnts[0], tri2Pnts[1], tri2Pnts[2]);
    CGAL::cpp11::result_of<CGIntersect3(CGTriangle3, CGTriangle3)>::type result;

    try
    {
        if (!CGAL::do_intersect(TriangleA, TriangleB))
        {
            return false;
        }

        result = CGAL::intersection(TriangleA, TriangleB);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return false;
    }

    // 从官网的表格中可以查看到结果的所有可能
    //Point_3, or Segment_3, or Triangle_3, or std::vector<Point_3>
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
    test();

    return 0;
}