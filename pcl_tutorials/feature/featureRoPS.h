#pragma once

#include "pcl/point_cloud.h"
#include "pcl/point_types.h"
#include "pcl/features/rops_estimation.h"
#include "pcl/features/feature.h"
#include "boost/shared_ptr.hpp"
#include <iostream>
#include <fstream>

#include <pcl/features/rops_estimation.h>
#include <pcl/io/pcd_io.h>
#include <pcl/visualization/pcl_plotter.h>// ֱ��ͼ�Ŀ��ӻ� ����2
#include <pcl/visualization/cloud_viewer.h>//���ǻ�
#include <boost/thread/thread.hpp>//boost::this_thread::sleep �����

pcl::PointCloud<pcl::Histogram <135> >::Ptr getRoPS(std::vector <pcl::Vertices> triangles,
													pcl::PointCloud<pcl::PointXYZ>::Ptr keypoint,
													pcl::PointCloud<pcl::Normal>::Ptr normal = NULL,
													pcl::PointCloud<pcl::PointXYZ>::Ptr surface = NULL)
{
	/*
  template <typename PointInT, typename PointOutT>
  class PCL_EXPORTS ROPSEstimation : public pcl::Feature <PointInT, PointOutT>

  ������
  void 	setNumberOfPartitionBins (unsigned int number_of_bins)
  Allows to set the number of partition bins that is used for distribution matrix calculation. More...

  void 	setNumberOfRotations (unsigned int number_of_rotations)
  This method sets the number of rotations. More...

  void 	setSupportRadius (float support_radius)
  Allows to set the support radius that is used to crop the local surface of the point. More...

  void 	setTriangles (const std::vector< pcl::Vertices > &triangles)
  This method sets the triangles of the mesh. More...

	*/

	//boost::shared_ptr<pcl::ROPSEstimation<pcl::PointXYZ, pcl::Histogram<135>>> rop(new pcl::ROPSEstimation<pcl::PointXYZ, pcl::Histogram<135>>);

	pcl::ROPSEstimation<pcl::PointXYZ, pcl::Histogram<135>> rops;//pcl����û�з�װ����ָ��
	
	pcl::PointCloud<pcl::Histogram <135> >::Ptr histograms(new pcl::PointCloud <pcl::Histogram <135> >());


	float support_radius = 0.0285f;				//�ֲ�����ü�֧�ֵİ뾶 (�������)��
	unsigned int number_of_partition_bins = 5;	//�Լ�������ɷֲ����������������
	unsigned int number_of_rotations = 3;		//����ת�Ĵ��������Ĳ�����Ӱ���������ĳ��ȡ�

												//��������
	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);
	tree->setInputCloud(keypoint);
	// rops �����㷨 ���� �������� �� ��ת������9 �õ�����ά��  3*5*9 =135
	rops.setSearchMethod(tree);//�����㷨
	rops.setSearchSurface(surface);//����ƽ��
	rops.setInputCloud(keypoint);//�������
	rops.setRadiusSearch(support_radius);//�����뾶
	rops.setTriangles(triangles);//������״
	rops.setNumberOfPartitionBins(number_of_partition_bins);//��������
	rops.setNumberOfRotations(number_of_rotations);//��ת����
	rops.setSupportRadius(support_radius);// �ֲ�����ü�֧�ֵİ뾶 

	rops.compute(*histograms);
	return histograms;
}

int computeROPS() 
{
	//======= points ��������===========
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_ptr(new pcl::PointCloud<pcl::PointXYZ>());

	//========�ؼ��������ļ�===============
	pcl::PointIndicesPtr indices = boost::shared_ptr <pcl::PointIndices>(new pcl::PointIndices());//�ؼ��� ����
	std::ifstream indices_file;//�ļ�������
	std::vector <pcl::Vertices> triangles;//=====triangles�����˶����===������״
	std::ifstream triangles_file;

	if (pcl::io::loadPCDFile("./rops/points.pcd", *cloud_ptr) == -1)
		return (-1);
	indices_file.open("./rops/indices.txt", std::ifstream::in);
	if (!indices_file) {
		std::cout << "not found index.txt file" << std::endl;
		return (-1);
	}

	triangles_file.open("./rops/triangles.txt", std::ifstream::in);
	if (!triangles_file) {
		std::cout << "not found triangles.txt file" << std::endl;
		return (-1);
	}


	//========�ؼ��������ļ�===============
	for (std::string line; std::getline(indices_file, line);)//ÿһ��Ϊ һ��������
	{
		std::istringstream in(line);
		unsigned int index = 0;
		in >> index;//����
		indices->indices.push_back(index - 1);
	}
	indices_file.close();

	//=====triangles�����˶����===������״
	for (std::string line; std::getline(triangles_file, line);)//ÿһ������������
	{
		pcl::Vertices triangle;
		std::istringstream in(line);
		unsigned int vertex = 0;//����
		in >> vertex;
		triangle.vertices.push_back(vertex - 1);
		in >> vertex;
		triangle.vertices.push_back(vertex - 1);
		in >> vertex;
		triangle.vertices.push_back(vertex - 1);
		triangles.push_back(triangle);
	}

	float support_radius = 0.0285f;//�ֲ�����ü�֧�ֵİ뾶 (�������)��
	unsigned int number_of_partition_bins = 5;//�Լ�������ɷֲ����������������
	unsigned int number_of_rotations = 3;//����ת�Ĵ��������Ĳ�����Ӱ���������ĳ��ȡ�
	pcl::search::KdTree<pcl::PointXYZ>::Ptr search_method(new pcl::search::KdTree<pcl::PointXYZ>);
	search_method->setInputCloud(cloud_ptr);

	// rops �����㷨 ���� �������� �� ��ת������9 �õ�����ά��  3*5*9 =135
	pcl::ROPSEstimation <pcl::PointXYZ, pcl::Histogram <135> > feature_estimator;
	feature_estimator.setSearchMethod(search_method);//�����㷨
	feature_estimator.setSearchSurface(cloud_ptr);//����ƽ��
	feature_estimator.setInputCloud(cloud_ptr);//�������
	feature_estimator.setIndices(indices);//�ؼ�������
	feature_estimator.setTriangles(triangles);//������״,�����������
	feature_estimator.setRadiusSearch(support_radius);//�����뾶
	feature_estimator.setNumberOfPartitionBins(number_of_partition_bins);//��������
	feature_estimator.setNumberOfRotations(number_of_rotations);//��ת����
	feature_estimator.setSupportRadius(support_radius);// �ֲ�����ü�֧�ֵİ뾶 

	pcl::PointCloud<pcl::Histogram <135> >::Ptr histograms(new pcl::PointCloud <pcl::Histogram <135> >());
	feature_estimator.compute(*histograms);
	cout <<"histograms->size: "<< histograms->size() << endl;

	// ���ӻ�
	pcl::visualization::PCLPlotter plotter;
	plotter.addFeatureHistogram(*histograms, 300); //���õĺ����곤�ȣ���ֵԽ������ʾ��Խϸ��
	plotter.plot();

	// ���ӻ�����
	//boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));
	//viewer->setBackgroundColor(0, 0, 0);//������ɫ
	//viewer->addCoordinateSystem(1.0);//����ϵ �ߴ�
	//viewer->initCameraParameters();//��ʼ���������
	//viewer->addPointCloud<pcl::PointXYZ>(cloud_ptr, "sample cloud");//������ƿ��ӻ�
	//while (!viewer->wasStopped())
	//{
	//	viewer->spinOnce(100);
	//	boost::this_thread::sleep(boost::posix_time::microseconds(100000));
	//}

	return (0);

}