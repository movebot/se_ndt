#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <boost/program_options.hpp>
#include <se_ndt/ndt_fuser_hmt_se.h>
#include <pcl/common/io.h>

using namespace std;
namespace po = boost::program_options;
pcl::PointCloud<pcl::PointXYZI>::Ptr  getCloud2(string filename,bool skip=false)
{
	ifstream infile(filename); // for example
	string line = "";
	pcl::PointCloud<pcl::PointXYZI>::Ptr laserCloud(new pcl::PointCloud<pcl::PointXYZI>);
	if(skip)getline(infile, line);
	while (getline(infile, line)){
		stringstream strstr(line);
		string word = "";
		pcl::PointXYZI point;
		getline(strstr,word, ',');
		point.x=stof(word);
		getline(strstr,word, ',');
		point.y=stof(word);
		getline(strstr,word, ',');
		point.z=stof(word);
		getline(strstr,word);
		point.intensity=stof(word);
		(*laserCloud).points.push_back(point);
	}
    return laserCloud;
}
vector<double>  getMeasure(string filename)
{
	ifstream infile(filename); // for example
	string line = "";
	vector<double> measure;
	while (getline(infile, line)){
		measure.push_back(stod(line));
	}
    return measure;
}


int main(int argc, char** argv)
{
	po::options_description desc("Allowed options");
    desc.add_options()
	("help", "produce help message")
	("skip", "skip point cloud first line")
	 ("pointclouds", po::value<std::vector<string> >()->multitoken(), "Point cloud files")
	 ("sem1", po::value<std::vector<string> >()->multitoken(), "First semantic input files")
	 ("sem2", po::value<std::vector<string> >()->multitoken(), "Second semantic input files");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
	vector<string> smoothness_files;
	vector<string> pole_files;
	vector<string> pointcloud_files;
	bool skip=false;
	if(vm.count("skip"))
			skip=true;
	if(vm.count("help"))
	{
		cout<<desc;
		return 0;
	}
	if (!vm["pointclouds"].empty() && (pointcloud_files= vm["pointclouds"].as<vector<string> >()).size() >= 2) {
		///cout<<"success pointcloud read";
	}else {cout<<"pointclouds read failure";};
	if (!vm["sem1"].empty() && (smoothness_files= vm["sem1"].as<vector<string> >()).size() >= 2) {
		///cout<<"success smoothness read";
	}else {cout<<"sem1 read failure";};
	if (!vm["sem2"].empty() && (pole_files= vm["sem2"].as<vector<string> >()).size() >= 2) {
		///cout<<"success poles read";
	}else {cout<<"sem2 read failure";};
	int num_files=min(pole_files.size(),min(smoothness_files.size(),pointcloud_files.size()));


	Eigen::Affine3d T;
	T.setIdentity();
	lslgeneric::NDTFuserHMT_SE matcher (T,{0.5,1},{0,1,0},{50,50,20},{3,3},{-1,-1},0.60,25);
	//lslgeneric::NDTFuserHMT_SE matcher (the_initial_pose,{the_resolutions},{the_order_with which_the_resolutions_are_used},{the_size_of_the_map},{the_tail_segments},{ignore_values},reject_percentage,number_of_iterations);
	for(int i=0;i<num_files;i++)
	{
		pcl::PointCloud<pcl::PointXYZI>::Ptr cloud3=getCloud2(pointcloud_files[i],skip);
		pcl::PointCloud<pcl::PointXYZ>::Ptr cloud1(new pcl::PointCloud<pcl::PointXYZ>);
		pcl::copyPointCloud(*cloud3,*cloud1);
		std::vector<double> smoothness1=getMeasure(smoothness_files[i]);
		std::vector<double> poles1=getMeasure(pole_files[i]);
		Eigen::Affine3d T_pred;
		T_pred.setIdentity();
		T=matcher.update(T_pred,cloud1,{smoothness1,poles1});
		//T=matcher.match(T_pred,cloud1,{smoothness1,poles1});
//		cout<<getHes(matcher.matcher.HessianF,matcher.matcher.score_gradientF)<<endl;
		//matcher.updateMap();
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				cout<<T(i,j)<<", ";
		cout<<endl;
	}

	return 0;
}

