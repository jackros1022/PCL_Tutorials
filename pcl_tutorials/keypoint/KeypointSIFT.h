#pragma once
#include "pcl/point_cloud.h"
#include "pcl/point_types.h" 
#include "pcl/keypoints/sift_keypoint.h"


/************************************************************************/
/* 
������룬��һ�δ��룬�����׳� error C2039: ��intensity��: ���ǡ�pcl::PointXYZ���ĳ�Ա
*/
/************************************************************************/
namespace pcl
{
	template<>
	struct SIFTKeypointFieldSelector<PointXYZ>
	{
		inline float
			operator () (const PointXYZ &p) const
		{
			return p.z;
		}
	};
}

void getKeypointSIFT(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud, 
					 pcl::PointCloud<pcl::PointXYZ>::Ptr keypoint)
{
	/*
		������ȡ��
		template <typename PointInT, typename PointOutT>
		class SIFTKeypoint : public Keypoint<PointInT, PointOutT>
	*/
	pcl::SIFTKeypoint<pcl::PointXYZ, pcl::PointWithScale>::Ptr sift(new pcl::SIFTKeypoint<pcl::PointXYZ, pcl::PointWithScale>);
	pcl::PointCloud<pcl::PointWithScale>::Ptr result(new pcl::PointCloud<pcl::PointWithScale>);
	pcl::PointIndicesConstPtr indices(new pcl::PointIndices);

	sift->setInputCloud(cloud);
	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);
	sift->setSearchMethod(tree);
	/*
	min_scale	��׼�� the standard deviation of the smallest scale in the scale space
	nr_octaves	the number of octaves (i.e. doublings of scale) to compute
	nr_scales_per_octave	the number of scales to compute within each octave
	*/
	sift->setScales(0.005, 10, 4);
	/*
	min_contrast	the minimum contrast required for detection
	*/
	sift->setMinimumContrast(0.0001);
	sift->compute(*result);
	indices = sift->getKeypointsIndices();	//��ȡindices

	copyPointCloud(*result, *keypoint);
}