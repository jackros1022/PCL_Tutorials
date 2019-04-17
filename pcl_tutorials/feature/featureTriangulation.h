#pragma once
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/io/ply_io.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/features/normal_3d.h>
#include <pcl/features/normal_3d_omp.h>
#include <pcl/surface/gp3.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <boost/thread/thread.hpp>
#include "boost/shared_ptr.hpp"

#include <fstream>

#include "featureNormalEstimation.h"

/*
��̰��ͶӰ���ǻ��㷨��������ƽ������ǻ������巽�����Ƚ��������ͶӰ��ĳһ�ֲ���ά����ƽ���ڣ�
��������ƽ���ڽ���ƽ���ڵ����ǻ����ٸ���ƽ������λ����������ӹ�ϵ���һ��������������ģ�͡�

̰��ͶӰ���ǻ��㷨ԭ���Ǵ���һϵ�п���ʹ�����������󡱵ĵ㣨��Ե�㣩��
������Щ��ֱ�����з��ϼ�����ȷ�Ժ�������ȷ�Եĵ㶼�����ϡ�
���㷨���ŵ��ǿ��Դ�������һ�����߶��ɨ����ɨ��õ������ж�����Ӵ���ɢ�ҵ��ơ�
�����㷨Ҳ��һ���ľ����ԣ����������ڲ������������ڱ��������⻬�����沢�ҵ����ܶȱ仯�ȽϾ��ȵ������

���㷨�����ǻ������Ǿֲ����еģ���������һ��ķ��߽��õ�ͶӰ���ֲ���ά����ƽ���ڲ������������յ㣬
Ȼ���ڽ�����һ�㡣�������������������²�����

1������SetMaximumNearestNeighbors(unsigned)��SetMu(double)��
	�����������������ǿ������������С��ǰ�߶����˿����������������
	���߹涨�˱��������������ڽ������Զ���룬����Ϊ����Ӧ�����ܶȵı仯����
	����ֵһ����50-100��2.5-3������1.5ÿդ�񣩡�

2������SetSearchRadius(double)��
	�ú������������ǻ���õ���ÿ�������ε������ܱ߳���

3������SetMinimumAngle(double)��SetMaximumAngle(double)��
	���������������ǻ���ÿ�������ε����Ǻ���С�ǡ���������Ҫ����һ��������ֵ�ֱ���10��120�ȣ����ȣ���

4������SetMaximumSurfaceAgle(double)��SetNormalConsistency(bool)��
	������������Ϊ�˴����Ե���߽Ǻܼ����Լ�һ����������߷ǳ������������
	Ϊ�˴�����Щ�������������SetMaximumSurfaceAgle(double)�涨���ĳ�㷨�߷����ƫ��
	����ָ���Ƕȣ�ע����������淨�߹��Ʒ������Թ��Ƴ������仯�ı��淨�߷��򣬼�ʹ�ڼ���ı�Ե�����£���
	�õ�Ͳ����ӵ��������ϡ��ýǶ���ͨ�����㷨���߶Σ����Է��߷���֮��ĽǶȡ�
	����SetNormalConsistency(bool)��֤���߳���������߷���һ���Ա�ʶû���趨��
	�Ͳ��ܱ�֤���Ƴ��ķ��߶�����ʼ�ճ���һ�¡���һ����������ֵΪ45�ȣ����ȣ����ڶ�������ȱʡֵΪfalse��
*/

void getTriangulation(pcl::PointCloud<pcl::PointXYZ>::Ptr &cloud, 
							pcl::PointCloud<pcl::Normal>::Ptr &normals,
							std::vector <pcl::Vertices> &triangles) 
{
	// Concatenate the XYZ and normal fields*
	pcl::PointCloud<pcl::PointNormal>::Ptr cloud_with_normals(new pcl::PointCloud<pcl::PointNormal>);
	pcl::concatenateFields(*cloud, *normals, *cloud_with_normals);


	// Initialize objects
	pcl::GreedyProjectionTriangulation<pcl::PointNormal> gp3;
	pcl::PolygonMesh polygonMesh;

	// Set the maximum distance between connected points (maximum edge length)
	gp3.setSearchRadius(0.025);
	gp3.setMu(1.5);
	gp3.setMaximumNearestNeighbors(10);
	gp3.setMaximumSurfaceAngle(M_PI / 4); // 45 degrees
	gp3.setMinimumAngle(M_PI / 18); // 10 degrees
	gp3.setMaximumAngle(2 * M_PI / 3); // 120 degrees
	gp3.setNormalConsistency(false);

	// Get result
	gp3.setInputCloud(cloud_with_normals);
	pcl::search::KdTree<pcl::PointNormal>::Ptr tree(new pcl::search::KdTree<pcl::PointNormal>);
	tree->setInputCloud(cloud_with_normals);

	gp3.setSearchMethod(tree);
	gp3.reconstruct(polygonMesh);

	//��������ͼ
	//pcl::io::savePLYFile("result.ply", polygonMesh);
	
	pcl::PointCloud<pcl::PointXYZ>::Ptr tcloud(new pcl::PointCloud<pcl::PointXYZ>);
	fromPCLPointCloud2(polygonMesh.cloud, *tcloud);

	// ��polygonMesh����Ϊtxt�ļ�
	//ofstream f1("result.txt");
	//for (int i = 0; i < tcloud->size(); i++)
	//{
	//	f1 << tcloud->points[i].x << " " << tcloud->points[i].y << " " << tcloud->points[i].z << endl;
	//}
	//f1.close();

	// ��polygonMesh����Ϊvector
	for (int i = 0; i < tcloud->size(); i++)
	{
		pcl::Vertices triangle;
		triangle.vertices.push_back(tcloud->points[i].x);
		triangle.vertices.push_back(tcloud->points[i].y);
		triangle.vertices.push_back(tcloud->points[i].z);
		triangles.push_back(triangle);
	}
	std::cout << "triangles.size: " << triangles.size() << std::endl;
}

void getGP3(pcl::PointCloud<pcl::PointXYZ>::Ptr &cloud)
{

	// Normal estimation*
	pcl::NormalEstimationOMP<pcl::PointXYZ, pcl::Normal> n;
	pcl::PointCloud<pcl::Normal>::Ptr normals(new pcl::PointCloud<pcl::Normal>);
	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);
	tree->setInputCloud(cloud);
	n.setNumberOfThreads(10);
	n.setInputCloud(cloud);
	n.setSearchMethod(tree);
	n.setRadiusSearch(0.02);
	n.compute(*normals);

	//* normals should not contain the point normals + surface curvatures

	// Concatenate the XYZ and normal fields*
	pcl::PointCloud<pcl::PointNormal>::Ptr cloud_with_normals(new pcl::PointCloud<pcl::PointNormal>);
	pcl::concatenateFields(*cloud, *normals, *cloud_with_normals);
	//* cloud_with_normals = cloud + normals

	// Create search tree*
	pcl::search::KdTree<pcl::PointNormal>::Ptr tree2(new pcl::search::KdTree<pcl::PointNormal>);
	tree2->setInputCloud(cloud_with_normals);

	// Initialize objects
	pcl::GreedyProjectionTriangulation<pcl::PointNormal> gp3;
	pcl::PolygonMesh triangles;

	// Set the maximum distance between connected points (maximum edge length)
	gp3.setSearchRadius(0.025);

	// Set typical values for the parameters
	gp3.setMu(1.5);
	gp3.setMaximumNearestNeighbors(10);
	gp3.setMaximumSurfaceAngle(M_PI / 4); // 45 degrees
	gp3.setMinimumAngle(M_PI / 18); // 10 degrees
	gp3.setMaximumAngle(2 * M_PI / 3); // 120 degrees
	gp3.setNormalConsistency(false);

	// Get result
	gp3.setInputCloud(cloud_with_normals);
	gp3.setSearchMethod(tree2);
	gp3.reconstruct(triangles);

	//��������ͼ
	pcl::io::savePLYFile("result.ply", triangles);


	// ��triangles����Ϊtxt�ļ�
	pcl::PointCloud<pcl::PointXYZ>::Ptr tcloud(new pcl::PointCloud<pcl::PointXYZ>);
	fromPCLPointCloud2(triangles.cloud, *tcloud);
	ofstream f1("result.txt");
	for (int i=0;i< tcloud->size();i++)
	{
		f1 << tcloud->points[i].x << " " << tcloud->points[i].y << " " << tcloud->points[i].z << endl;
	}
	f1.close();

	//std::cout << triangles;
	// Additional vertex information
	std::vector<int> parts = gp3.getPartIDs();
	std::vector<int> states = gp3.getPointStates();

	// ��ʾ
	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));
	viewer->setBackgroundColor(0, 0, 0);
	viewer->addPolygonMesh(triangles, "my");

	viewer->addCoordinateSystem(1.0);
	viewer->initCameraParameters();
	// ��ѭ��
	while (!viewer->wasStopped())
	{
		viewer->spinOnce(100);
		boost::this_thread::sleep(boost::posix_time::microseconds(100000));
	}
}
