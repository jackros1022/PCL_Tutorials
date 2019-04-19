#pragma once
#include "pcl/point_cloud.h"
#include "pcl/point_types.h"
#include "pcl/registration/icp.h"

#include <iostream>
#include <string>

#include <pcl/io/ply_io.h>
#include <pcl/point_types.h>
#include <pcl/registration/icp.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/console/time.h>   // TicToc

void computeICP()
{
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_in(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_out(new pcl::PointCloud<pcl::PointXYZ>);
	// �����������
	cloud_in->width = 5;
	cloud_in->height = 1;
	cloud_in->is_dense = false;
	cloud_in->points.resize(cloud_in->width * cloud_in->height);
	for (size_t i = 0; i < cloud_in->points.size(); ++i)
	{
		cloud_in->points[i].x = 1024 * rand() / (RAND_MAX + 1.0f);
		cloud_in->points[i].y = 1024 * rand() / (RAND_MAX + 1.0f);
		cloud_in->points[i].z = 1024 * rand() / (RAND_MAX + 1.0f);
	}

	std::cout << "Saved " << cloud_in->points.size() << " data points to input:"<< std::endl;
	for (size_t i = 0; i < cloud_in->points.size(); ++i)
	{
		std::cout << "    " << cloud_in->points[i].x << " " << cloud_in->points[i].y << " " 
			<< cloud_in->points[i].z << std::endl;
	}

	*cloud_out = *cloud_in;
	std::cout << "size:" << cloud_out->points.size() << std::endl;
	for (size_t i = 0; i < cloud_in->points.size(); ++i)
	{
		cloud_out->points[i].x = cloud_in->points[i].x + 0.7f;
	}
	std::cout << "Transformed " << cloud_in->points.size() << " data points:" << std::endl;
	for (size_t i = 0; i < cloud_out->points.size(); ++i)
	{
		std::cout << "    " << cloud_out->points[i].x << " " <<
			cloud_out->points[i].y << " " << cloud_out->points[i].z << std::endl;
	}

	/*
	template <typename PointSource, typename PointTarget, typename Scalar = float>
	class IterativeClosestPoint : public Registration<PointSource, PointTarget, Scalar>
	
	*/

	pcl::IterativeClosestPoint<pcl::PointXYZ, pcl::PointXYZ> icp;
	icp.setInputCloud(cloud_in);
	icp.setInputTarget(cloud_out);
	pcl::PointCloud<pcl::PointXYZ> Final;
	// 	Call the registration algorithm which estimates the transformation and returns the transformed source (input) as output.
	icp.align(Final);
	std::cout << "has converged:" << icp.hasConverged() << " score: " <<
		icp.getFitnessScore() << std::endl;
	std::cout << icp.getFinalTransformation() << std::endl;

}
/************************************************************************/
/* Interactive_icp                                                      */
/************************************************************************/

typedef pcl::PointXYZ PointT;
typedef pcl::PointCloud<PointT> PointCloudT;

bool next_iteration = false;

void
print4x4Matrix(const Eigen::Matrix4d & matrix)
{
	printf("Rotation matrix :\n");
	printf("    | %6.3f %6.3f %6.3f | \n", matrix(0, 0), matrix(0, 1), matrix(0, 2));
	printf("R = | %6.3f %6.3f %6.3f | \n", matrix(1, 0), matrix(1, 1), matrix(1, 2));
	printf("    | %6.3f %6.3f %6.3f | \n", matrix(2, 0), matrix(2, 1), matrix(2, 2));
	printf("Translation vector :\n");
	printf("t = < %6.3f, %6.3f, %6.3f >\n\n", matrix(0, 3), matrix(1, 3), matrix(2, 3));
}

void
keyboardEventOccurred(const pcl::visualization::KeyboardEvent& event, void* nothing)
{
	if (event.getKeySym() == "space" && event.keyDown())
		next_iteration = true;
}

int getInteractive_icp()
{
	// ����Ҫʹ�õĵ���
	PointCloudT::Ptr cloud_in(new PointCloudT);  // ��ʼ����
	PointCloudT::Ptr cloud_tr(new PointCloudT);  // ת������
	PointCloudT::Ptr cloud_icp(new PointCloudT);  // �������

	int iterations = 1;  // Ĭ��ICP��׼�ĵ�������

	pcl::console::TicToc time;
	time.tic();
	if (pcl::io::loadPLYFile("monkey.ply", *cloud_in) < 0)
	{
		PCL_ERROR("Error loading cloud");
		return (-1);
	}
	std::cout << "\nLoaded file " << " (" << cloud_in->size() << " points) in " << time.toc() << " ms\n" << std::endl;

	// ������ת�����ƽ�ƾ���
	Eigen::Matrix4d transformation_matrix = Eigen::Matrix4d::Identity();

	// ��ת����ľ��嶨��
	double theta = M_PI / 20;  // ������ת���ȵĽǶ�
	transformation_matrix(0, 0) = cos(theta);
	transformation_matrix(0, 1) = -sin(theta);
	transformation_matrix(1, 0) = sin(theta);
	transformation_matrix(1, 1) = cos(theta);

	// ����ƽ�ƾ���
	transformation_matrix(0, 3) = 0.0;
	transformation_matrix(1, 3) = 0.0;
	transformation_matrix(2, 3) = 0.0;
	std::cout << "Applying this rigid transformation to: cloud_in -> cloud_icp" << std::endl;
	print4x4Matrix(transformation_matrix);

	// ִ�г�ʼ�任
	pcl::transformPointCloud(*cloud_in, *cloud_icp, transformation_matrix);
	*cloud_tr = *cloud_icp;  // ��cloud_icp��������

							 // ����ICP��׼�㷨�������
	time.tic();
	pcl::IterativeClosestPoint<PointT, PointT> icp;
	icp.setMaximumIterations(iterations);//���õ�������
	icp.setInputSource(cloud_icp);
	icp.setInputTarget(cloud_in);
	icp.align(*cloud_icp);
	icp.setMaximumIterations(5);  //���ٴε���.align ()����ʱ���������øñ���Ϊ1��
	std::cout << "Applied " << iterations << " ICP iteration(s) in " << time.toc() << " ms" << std::endl;

	if (icp.hasConverged())
	{
		std::cout << "\nICP has converged, score is " << icp.getFitnessScore() << std::endl;
		std::cout << "\nICP transformation " << iterations << " : cloud_icp -> cloud_in" << std::endl;
		transformation_matrix = icp.getFinalTransformation().cast<double>();
		print4x4Matrix(transformation_matrix);
	}
	else
	{
		PCL_ERROR("\nICP has not converged.\n");
		return (-1);
	}

	// ���ӻ�����
	pcl::visualization::PCLVisualizer viewer("ICP demo");
	// ���������������ӿ�
	int v1(0);
	int v2(1);
	viewer.createViewPort(0.0, 0.0, 0.5, 1.0, v1);
	viewer.createViewPort(0.5, 0.0, 1.0, 1.0, v2);

	// ���ǽ�Ҫʹ�õ���ɫ
	float bckgr_gray_level = 0.0;  // ��ɫ
	float txt_gray_lvl = 1.0 - bckgr_gray_level;

	// ���ó�ʼ����Ϊ��ɫ
	pcl::visualization::PointCloudColorHandlerCustom<PointT> cloud_in_color_h(cloud_in, (int)255 * txt_gray_lvl, (int)255 * txt_gray_lvl, (int)255 * txt_gray_lvl);//������ʾ���Ƶ���ɫ
	viewer.addPointCloud(cloud_in, cloud_in_color_h, "cloud_in_v1", v1);
	viewer.addPointCloud(cloud_in, cloud_in_color_h, "cloud_in_v2", v2);

	// ���ó�ʼת����ĵ���Ϊ��ɫ
	pcl::visualization::PointCloudColorHandlerCustom<PointT> cloud_tr_color_h(cloud_tr, 20, 180, 20);
	viewer.addPointCloud(cloud_tr, cloud_tr_color_h, "cloud_tr_v1", v1);

	//  ����ICP��׼��ĵ���Ϊ��ɫ
	pcl::visualization::PointCloudColorHandlerCustom<PointT> cloud_icp_color_h(cloud_icp, 180, 20, 20);
	viewer.addPointCloud(cloud_icp, cloud_icp_color_h, "cloud_icp_v2", v2);

	//  �������ӿڣ��ֱ������������
	viewer.addText("White: Original point cloud\nGreen: Matrix transformed point cloud", 10, 15, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "icp_info_1", v1);
	viewer.addText("White: Original point cloud\nRed: ICP aligned point cloud", 10, 15, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "icp_info_2", v2);

	std::stringstream ss;
	ss << iterations;
	std::string iterations_cnt = "ICP iterations = " + ss.str();
	viewer.addText(iterations_cnt, 10, 60, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "iterations_cnt", v2);

	// ���ñ�����ɫ
	viewer.setBackgroundColor(bckgr_gray_level, bckgr_gray_level, bckgr_gray_level, v1);
	viewer.setBackgroundColor(bckgr_gray_level, bckgr_gray_level, bckgr_gray_level, v2);

	// �������λ�úͷ���
	viewer.setCameraPosition(-3.68332, 2.94092, 5.71266, 0.289847, 0.921947, -0.256907, 0);
	viewer.setSize(1280, 1024);  // ���ÿ��ӻ����ڵĳߴ�

								 // ͨ�����̣����ûص�����
	viewer.registerKeyboardCallback(&keyboardEventOccurred, (void*)NULL);

	// ������ʾ��
	while (!viewer.wasStopped())
	{
		viewer.spinOnce();

		// �û����¿ո��
		if (next_iteration)
		{
			// ��׼�㷨��ʼ����
			time.tic();
			icp.align(*cloud_icp);
			std::cout << "Applied 1 ICP iteration in " << time.toc() << " ms" << std::endl;

			if (icp.hasConverged())
			{
				printf("\033[11A");
				printf("\nICP has converged, score is %+.0e\n", icp.getFitnessScore());
				std::cout << "\nICP transformation " << ++iterations << " : cloud_icp -> cloud_in" << std::endl;
				// ��õ�ֵ���������
				transformation_matrix *= icp.getFinalTransformation().cast<double>();  
				print4x4Matrix(transformation_matrix);  // �����ʼ����͵�ǰ�����ת������

				ss.str("");
				ss << iterations;
				std::string iterations_cnt = "ICP iterations = " + ss.str();
				viewer.updateText(iterations_cnt, 10, 60, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "iterations_cnt");
				viewer.updatePointCloud(cloud_icp, cloud_icp_color_h, "cloud_icp_v2");
			}
			else
			{
				PCL_ERROR("\nICP has not converged.\n");
				return (-1);
			}
		}

		next_iteration = false;
	}

	return 0;
}