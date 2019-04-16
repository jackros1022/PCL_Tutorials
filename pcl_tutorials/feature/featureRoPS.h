#pragma once

#include "pcl/point_cloud.h"
#include "pcl/point_types.h"
#include "pcl/features/rops_estimation.h"
#include "pcl/features/feature.h"

void getRoPS(pcl::PointCloud<pcl::PointXYZ>::Ptr keypoint,
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

	pcl::ROPSEstimation<pcl::PointXYZ, pcl::Histogram<135>> rops;
	
	pcl::PointCloud<pcl::Histogram <135> >::Ptr histograms(new 
		pcl::PointCloud <pcl::Histogram <135> >());


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
}