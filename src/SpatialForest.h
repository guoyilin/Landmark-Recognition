/**
 * Ensemble of random projection tree
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
#include <sstream>
#include "Spatialtree.h"
using namespace std;
using boost::property_tree::ptree;


class SpatialForest {
public:
	void excute(int argc, char *argv[]);
private:
	void save_forest_sub(TreeNode *tree, ptree &pt);
	void save_forest(const string& codebook_file,
			const vector<SpatialTree *> &forest);
	void exit_with_help();
};

void SpatialForest::save_forest_sub(TreeNode *tree, ptree &pt) {
	if (tree->index > -1)
		pt.put("xmlattr.index", tree->index);
	else {
		string w_string;
		for (std::vector<float>::const_iterator iter = tree->w.begin();
				iter != tree->w.end(); ++iter)
			if (w_string.empty()) {
				ostringstream ostr;
				ostr << *iter;
				w_string += ostr.str();
				w_string += ",";
			} else {
				ostringstream ostr;
				ostr << *iter;
				w_string += ostr.str();
				w_string += ",";
			}
		w_string = w_string.substr(0, w_string.size() - 1);
		pt.put("xmlattr.w", w_string);
		pt.put("xmlattr.t0", tree->thresholds[0]);
		pt.put("xmlattr.t1", tree->thresholds[1]);
		ptree left_child;
		ptree right_child;
		left_child.put("xmlattr.height", tree->left->height);
		save_forest_sub(tree->left, left_child);
		pt.put_child("left", left_child);
		right_child.put("xmlattr.height", tree->right->height);
		save_forest_sub(tree->right, right_child);
		pt.put_child("right", right_child);
	}

}
void SpatialForest::save_forest(const string& codebook_file,
		const vector<SpatialTree *> &forest) {
	ptree pt;
	ptree rp_tree;
	rp_tree.put("xmlattr.number", forest.size());
	for (int i = 0; i < forest.size(); i++) {
		SpatialTree * tree = forest[i];
		TreeNode * root = tree->root;
		ptree root_tree;
		root_tree.put("xmlattr.height", tree->height);
		root_tree.put("xmlattr.count", tree->leaf_count);
		root_tree.put("xmlattr.dimension", tree->dimension);
		root_tree.put("xmlattr.min_items", tree->min_items);
		root_tree.put("xmlattr.samples_rp", tree->samples_rp);
		root_tree.put("xmlattr.rule", tree->rule);
		root_tree.put("xmlattr.spill", tree->spill);
		if (root->height != 0) {
			save_forest_sub(root, root_tree);
		}
		ostringstream ostr;
		ostr << "tree" << i;
		rp_tree.put_child(ostr.str(), root_tree);
	}
	pt.put_child("root", rp_tree);
	boost::property_tree::write_xml(codebook_file.c_str(), pt);

}

void SpatialForest::exit_with_help() {
	printf(
			"Usage: Spatialtree.o output_codebook_file, descriptors_dir, spill_value, tree_num, height, percentage\n"
					"options:\n"
					"output_codebook_file: generate the codebook\n"
					"descriptors_dir: the descriptors dir use for spatial-tree training\n"
					"spill_value: rptree's threshold(0.0, 0.01, 0.05, or 0.1), default=0.1\n"
					"tree_num: num of tree(1-10), default=5\n"
					"height: height of tree, 2^(height-1) = codebook_size, default = 10\n"
					"percentage: percentage of data use as training(0.1-0.5), default = 0.1\n");
	exit(1);
}

void SpatialForest::excute(int argc, char *argv[]) {
	float spill = 0.1;
	string descriptors_dir;
	string codebook_file;
	int tree_num = 5;
	int height = 10;
	float percentage = 0.1;
	int feature_count = 0;
	if (argc == 7) {
		codebook_file = argv[1];
		descriptors_dir = argv[2];
		spill = atof(argv[3]);
		tree_num = atoi(argv[4]);
		height = atoi(argv[5]);
		percentage = atof(argv[6]);
	} else if (argc == 3) {
		codebook_file = argv[1];
		descriptors_dir = argv[2];
	} else {
		cout << "please input correct argument!!!" << endl;
		exit_with_help();
	}
	vector<vector<float> > data;
	//ifstream rptree_fin(codebook_file.c_str());
	//if (!rptree_fin) {
	long start = clock();
	DIR * dir;
	struct dirent* pDir = NULL;
	dir = opendir(descriptors_dir.c_str());
	if (dir == NULL)
		cout << "Error, can't open " << descriptors_dir << endl;
	else {
		while ((pDir = readdir(dir)) != NULL) {
			if (pDir->d_name[0] == '.')
				continue;
			string categoryPath = descriptors_dir + "/" + pDir->d_name;
			DIR * subdir = opendir(categoryPath.c_str());
			//iterate all category dir.
			if (subdir == NULL) {
				cout << "Error, can't open file:" << categoryPath << endl;
			} else {
				struct dirent* imageDir = NULL;
				while ((imageDir = readdir(subdir)) != NULL) {
					if (imageDir->d_name[0] == '.') {
						continue;
					}
					string imageName = imageDir->d_name;
					string filename = categoryPath + "/" + imageName;
					ifstream fin(filename.c_str());
					const int LINE_LENGTH = 10000;
					char str[LINE_LENGTH];
					while (fin.getline(str, LINE_LENGTH)) {
						int random = rand() % 1000;
						if (random < percentage * 1000) {
							feature_count++;
							const char *d = " ";
							char *p = strtok(str, d);
							vector<float> feature;
							while (p) {
								float value = atof(p);
								feature.push_back(value);
								p = strtok(NULL, " ");
							}
							data.push_back(feature);
						}

					}
					fin.close();

				}
			}

		}
	}
	cout << "how many descriptors use for codebook training:" << feature_count
			<< endl;
	vector<SpatialTree *> forest;
	for (int i = 0; i < tree_num; i++) {
		SpatialTree *tree = new SpatialTree(data, "rp-tree", spill, height);
		tree->create_rpTree(codebook_file.c_str());
		forest.push_back(tree);
	}
	save_forest(codebook_file, forest);
	vector<vector<float> >().swap(data);
	long end = clock();
	long rptree_time = (end - start) / 1000000;
	cout << "create rptree time:" << rptree_time << "s" << endl;
}

