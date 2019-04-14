#pragma once
#include "pcl/point_cloud.h"
#include "pcl/point_types.h"
#include "pcl/keypoints/uniform_sampling.h"

void getUniformSampling(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud, pcl::PointCloud<pcl::PointXYZ>::Ptr keypoint)
{
	
	/*
	���������
	template <typename PointT>
	class UniformSampling: public Filter<PointT>

	����
	virtual void 	setRadiusSearch (double radius)
			Set the 3D grid leaf size. More...

	*/

	pcl::UniformSampling<pcl::PointXYZ>::Ptr us(new pcl::UniformSampling<pcl::PointXYZ>);
	pcl::IndicesPtr indices;

	us->setInputCloud(cloud);
	us->setRadiusSearch(0.05);
	us->filter(*keypoint);
	//indices = us->getRemovedIndices();


	//Todo , ��λ�ȡ���� 
	//copyPointCloud(*cloud, *indices, *keypoint);
}