/**
 * Using random projection forest codebook, Coding each image into vector.
 * author: guoyilin1987@gmail.com
 */
#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <random>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include "string"
#include "stdlib.h"
#include "stdio.h"
#include <dirent.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <exception>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <numeric>
#include <time.h>
#include <map>
#include "Spatialtree.h"

using namespace std;
using boost::property_tree::ptree;

class Spatialtree_Vectorization {
private:
	map<string, int> label_map;
	bool image_vectorization(const string& des_dir, const string& save_file,
			vector<SpatialTree *>& forest, const string& imageListFile);
	vector<SpatialTree *> load_forest(const string& codebook_file);
	void load_imagelabel(const string& imagelabel_file);
	vector<float> stringTovector(string& w);
public:
	void exit_with_help();
	void execute(int argc, char *argv[]);
};
vector<float> Spatialtree_Vectorization::stringTovector(string& w) {
	vector<float> result;
	vector<string> strs;
	boost::split(strs, w, boost::is_any_of(","));
	for (int i = 0; i < strs.size(); i++) {
		float value = atof(strs[i].c_str());
		result.push_back(value);
	}
	return result;
}
void Spatialtree_Vectorization::load_imagelabel(const string& imagelabel_file) {
	ifstream fin(imagelabel_file.c_str());
	string line;
	int count = 0;
	while (getline(fin, line)) {
//		vector<string> strs;
//		boost::split(strs, line, boost::is_any_of("\t"));
//		string image_id = strs[0];
//		string label_id = strs[1];
		label_map.insert(std::pair<string, int>(line, count));
		count++;
	}
}
bool Spatialtree_Vectorization::image_vectorization(const string& des_dir,
		const string &save_file, vector<SpatialTree *> &forest,
		const string& imageListFile) {
	cout << "****************rp-forest image vectorization********" << endl;
	ofstream outFile(save_file.c_str(), ofstream::out);
	if (!outFile.is_open()) {
		cout << "open" + save_file + " failed" << endl;
		return false;
	}
	ifstream fin(imageListFile.c_str());
	string line;
	int count = 0;
	while (getline(fin, line)) {
		if(count ++ % 100 == 0)
			cout << "process:" << count << " encoding" << endl;
		string des_file = des_dir + "/" + line + ".jpg.yml";
		fstream _des_file;
		_des_file.open(des_file.c_str(), ios::in);
		if (!_des_file)
			continue;
		vector<string> strs;
		boost::split(strs, line, boost::is_any_of("/"));
		int image_label = label_map.find(strs[0])->second;
		vector<float> image_histogram(forest.size() * forest[0]->leaf_count, 0);
		const int LINE_LENGTH = 10000;
		char str[LINE_LENGTH];
		while (_des_file.getline(str, LINE_LENGTH)) {
			const char *d = " ";
			char *p = strtok(str, d);
			vector<float> feature;
			while (p) {
				float value = atof(p);
				feature.push_back(value);
				p = strtok(NULL, " ");
			}
			for (int i = 0; i < forest.size(); i++) {
				vector<int> leafs = forest[i]->retrievalLeaf(feature);
				for (int j = 0; j < leafs.size(); j++) {
					image_histogram[i * forest[0]->leaf_count + leafs[j]] += 1;
				}
			}
		}
		_des_file.close();
		float division = std::accumulate(image_histogram.begin(),
				image_histogram.end(), 0.0);
		if (division == 0.0) {
		//	cout << filename << " " << division << endl;
			continue;
		}
		outFile << image_label << " ";
		float factor = 1.0
				/ std::accumulate(image_histogram.begin(),
						image_histogram.end(), 0.0);
		for (int i = 0; i < image_histogram.size() - 1; i++) {
			image_histogram[i] *= factor;
			outFile << i + 1 << ":" << image_histogram[i] << " ";
		}
		image_histogram[image_histogram.size() - 1] *= factor;
		outFile << image_histogram.size() << ":"
				<< image_histogram[image_histogram.size() - 1] << "\n";
	}

	outFile.close();

}
vector<SpatialTree *> Spatialtree_Vectorization::load_forest(
		const string& codebook_file) {
	vector<SpatialTree *> forest;
	ptree pt;
	read_xml(codebook_file, pt);
	BOOST_FOREACH(const ptree::value_type &v, pt.get_child("root")) {
		if (v.first == "xmlattr")
			continue;
		ptree child_tree = v.second;
		SpatialTree * tree = new SpatialTree();
		tree->root = new TreeNode();
		tree->height = child_tree.get<int>("xmlattr.height");
		tree->min_items = child_tree.get<int>("xmlattr.min_items");
		tree->samples_rp = child_tree.get<int>("xmlattr.samples_rp");
		tree->dimension = child_tree.get<int>("xmlattr.dimension");
		tree->leaf_count = child_tree.get<int>("xmlattr.count");
		queue<ptree> q;
		queue<TreeNode *> q2;
		q.push(child_tree);
		q2.push(tree->root);
		while (!q.empty()) {
			ptree node = q.front();
			TreeNode *current = q2.front();
			int index = node.get<int>("xmlattr.index", -1);
			if (index != -1) {
				current->index = node.get<int>("xmlattr.index", -1);
			} else {
				current->index = -1;
				current->thresholds[0] = node.get<float>("xmlattr.t0");
				current->thresholds[1] = node.get<float>("xmlattr.t1");
				string w = node.get<string>("xmlattr.w");
				current->w = stringTovector(w);

				TreeNode *leftNode = new TreeNode();
				current->left = leftNode;
				TreeNode *rightNode = new TreeNode();
				current->right = rightNode;

				leftNode->height = node.get<int>("left.xmlattr.height");
				rightNode->height = node.get<int>("right.xmlattr.height");

				ptree left = node.get_child("left");
				ptree right = node.get_child("right");

				q.push(left);
				q.push(right);
				q2.push(current->left);
				q2.push(current->right);
			}
			q.pop();
			q2.pop();
		}
		forest.push_back(tree);
	}
	return forest;

}
void Spatialtree_Vectorization::exit_with_help() {
	printf(
			"Usage: codebook_file, descriptors_dir, features_file, labelFile, imageListFile\n"
					"options:\n"
					"descriptors_dir: the descriptors dir(format: label_name/descriptorFile)\n"
					"codebook_file: read the codebook\n"
					"features_file: save the features file\n"
					"labelsFile: the file contain labelname\n"
					"imageListFile: image list\n");
	exit(1);
}
void Spatialtree_Vectorization::execute(int argc, char *argv[]) {
	long start = clock();
	string codebook_file;
	string des_dir;
	string features_file;
	string label_file;
	string imageListFile;
	if (argc == 6) {
		codebook_file = argv[1];
		des_dir = argv[2];
		features_file = argv[3];
		label_file = argv[4];
		imageListFile = argv[5];
	} else {
		exit_with_help();
	}
	Spatialtree_Vectorization *instance = new Spatialtree_Vectorization();
	instance->load_imagelabel(label_file);
	vector<SpatialTree *> forest = instance->load_forest(codebook_file);
	instance->image_vectorization(des_dir, features_file, forest,
			imageListFile);
	long end = clock();
	cout << "image vectorization time:" << (end - start) / 1000000 << "s"
			<< endl;
}

