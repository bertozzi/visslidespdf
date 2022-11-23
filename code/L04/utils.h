// opencv
#include <opencv2/core/core.hpp>
#include <opencv2/calib3d.hpp>

#define USE_OPENCVVIZ

#ifdef USE_OPENCVVIZ
#include <opencv2/viz.hpp>
#endif

struct CameraParams
{
    // size
    int w, h;

    // intrinsics
    float ku, kv;
    float u0, v0;

    // estrinsics
    cv::Affine3f RT;
};

void LoadPoints(const std::string& filename, std::vector< cv::Point3f >& points)
{
    std::ifstream file;
    file.open(filename.c_str());
    
    int size;
    file >> size;
    
    for (unsigned int i = 0; i < size; ++i) 
    {
        cv::Point3f point, point_out;
        file >> point.x >> point.y >> point.z;

        //from "VisLab-body-like" to typical "camera-like" reference
        point_out.z = point.x;
        point_out.y = -point.z;
        point_out.x = -point.y;
        points.push_back(point_out);
    }
    
    file.close();
}

void PoseToAffine(float rx, float ry, float rz, float tx, float ty, float tz, cv::Affine3f& affine)
{
    cv::Mat world_RvecX_cam = cv::Mat(1,3,CV_32F);
    world_RvecX_cam.at<float>(0,0) = rx;
    world_RvecX_cam.at<float>(0,1) = 0.0;
    world_RvecX_cam.at<float>(0,2) = 0.0;
    cv::Mat world_Rx_cam;
    cv::Rodrigues(world_RvecX_cam, world_Rx_cam);
    
    cv::Mat world_RvecY_cam = cv::Mat(1,3,CV_32F);
    world_RvecY_cam.at<float>(0,0) = 0.0;
    world_RvecY_cam.at<float>(0,1) = ry;
    world_RvecY_cam.at<float>(0,2) = 0.0;
    cv::Mat world_Ry_cam;
    cv::Rodrigues(world_RvecY_cam, world_Ry_cam);
    
    cv::Mat world_RvecZ_cam = cv::Mat(1,3,CV_32F);
    world_RvecZ_cam.at<float>(0,0) = 0.0;
    world_RvecZ_cam.at<float>(0,1) = 0.0;
    world_RvecZ_cam.at<float>(0,2) = rz;
    cv::Mat world_Rz_cam;
    cv::Rodrigues(world_RvecZ_cam, world_Rz_cam);
    
    cv::Mat world_R_cam = world_Rx_cam*world_Ry_cam*world_Rz_cam;
    
    cv::Mat world_t_cam = cv::Mat(1,3,CV_32F);
    world_t_cam.at<float>(0,0) = tx;
    world_t_cam.at<float>(0,1) = ty;
    world_t_cam.at<float>(0,2) = tz;
    
    affine = cv::Affine3f(world_R_cam, world_t_cam);
}

void LoadCameraParams(const std::string& filename, CameraParams& params)
{
    std::ifstream file;
    file.open(filename.c_str());
    
    file >> params.w >> params.h;
    
    file >> params.ku >> params.kv;
    file >> params.u0 >> params.v0;
    
    float rx, ry, rz, tx, ty, tz;
    file >> rx >> ry >> rz;
    file >> tx >> ty >> tz;
    
    PoseToAffine(rx, ry, rz, tx, ty, tz, params.RT);
}

void PointsToMat(const std::vector< cv::Point3f >& points, cv::Mat& mat)
{
    mat = cv::Mat(1, 3*points.size(), CV_32FC3);
    for (unsigned int i,j = 0; i < points.size(); ++i, j+=3)
    {
        mat.at<float>(j) = points[i].x;
        mat.at<float>(j+1) = points[i].y;
        mat.at<float>(j+2) = points[i].z;
    }
}

void DrawPixels(const std::vector< cv::Point2f >& uv_points, cv::Mat& image)
{
    for (unsigned int i = 0; i < uv_points.size(); ++i)
    {
        float u = uv_points[i].x;
        float v = uv_points[i].y;

        if (u > 0 && u < image.cols && v > 0 && v < image.rows)
        {
            image.at<float>(v,u) = 1.0f;
        }
    }
}

#ifdef USE_OPENCVVIZ
cv::viz::Viz3d Viz3D(const CameraParams& params) 
{
    // for visualization
    cv::viz::Viz3d win("3D view");
    win.setWindowSize(cv::Size(800, 600));
    
    // camera frame
    cv::viz::WCameraPosition cpw(0.5);
    win.showWidget("cam", cpw, params.RT);
    cv::viz::WCameraPosition cpw_frustum(cv::Vec2f(atan2(params.w/2, params.ku), atan2(params.h/2, params.kv))); // Camera frustum
    win.showWidget("cam_frustum", cpw_frustum, params.RT);
    
    // world frame
    win.showWidget("Coordinate Widget", cv::viz::WCoordinateSystem());
     
    // specify window camera pose
    cv::Affine3f world_T_win_cam;
    PoseToAffine(0.05, 0.0, 0, 0, -5, -30, world_T_win_cam);
    win.setViewerPose(world_T_win_cam);
        
    return win;
}
#endif
