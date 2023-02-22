#include <CGAL/Simple_cartesian.h> //使用double内核，因为不涉及构造与谓词操作
typedef CGAL::Simple_cartesian<double> Kernel;

//Polygon Mesh
#include <CGAL/Surface_mesh/Surface_mesh.h>
typedef Kernel::Point_3				Point;
typedef CGAL::Surface_mesh<Point>	Mesh;

#include <CGAL/Polygon_mesh_processing/distance.h>
#include <CGAL/Polygon_mesh_processing/transform.h>
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

// 平移到模型box中心
void transform_center(Mesh& mesh)
{
    CGAL::Bbox_3 box = PMP::bbox(mesh);
    CGAL::Aff_transformation_3<Kernel> matrix(
        CGAL::TRANSLATION, 
        -Kernel::Vector_3(
            (box.xmax() - box.xmin()) / 2,
            (box.ymax() - box.ymin()) / 2,
            (box.zmax() - box.zmin()) / 2
        )
    );
    PMP::transform(matrix, mesh);
}

int main()
{
    const std::string in_dir = DATA_PATH "submeshs/";

    std::vector<std::string> files;
    traversal_file(in_dir, [&files](const std::string& path)->bool
        {
            files.emplace_back(path);
            return true; //继续
        });

    Mesh firstMesh;
    auto it = files.begin();

    //get first
    for (; it != files.end(); ++it)
    {
        if (PMP::IO::read_polygon_mesh(*it, firstMesh))
        {
            std::cout << "first mesh: " << * it << std::endl;
            ++it;
            break;
        }
    }

    //transform_center(firstMesh);

    //first and other
    for (; it != files.end(); ++it)
    {
        Mesh otherMesh;

        if (!PMP::IO::read_polygon_mesh(*it, otherMesh))
        {
            continue;
        }

        //transform_center(otherMesh);

        auto dist = PMP::approximate_symmetric_Hausdorff_distance<CGAL::Parallel_if_available_tag>(
            firstMesh, otherMesh
        );
        std::cout << *it << " " << dist << std::endl;
    }

    return 0;
}