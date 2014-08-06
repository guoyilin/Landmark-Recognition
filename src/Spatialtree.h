/**
*Spatial tree definition.
* author: guoyilin1987@gmail.com
*/
#ifndef _SPATIAL_TREE_H
#define _SPATIAL_TREE_H

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
using namespace std;
using boost::property_tree::ptree;
struct TreeNode {
	vector<int> indices;
	int index;
	int height;
	float thresholds[2];
	vector<float> w;
	TreeNode* left;
	TreeNode* right;
};
class SpatialTree {
private:
	vector<vector<float> > *data;
	void splitF(TreeNode *node);
	void splitFbyFixed(TreeNode *node);
	vector<float>
	dot(const vector<vector<float> > &v1, const vector<float> &v2);
	float dot(const vector<float> &v1, const vector<float> &v2);
	float find_percentile(float percentage, const vector<float> &w);
	void save_rpTree(const string& rptree_file);
	void save_rpTree_sub(TreeNode *tree, ptree &pt);
public:
	float spill;
	string rule;
	TreeNode *root;
	int leaf_count;
	int min_items;
	int samples_rp;
	int height;
	int dimension;
	SpatialTree();
	vector<float> stringTovector(string& w);
	void print_rpTree(TreeNode *tree);
	SpatialTree(vector<vector<float> > &data, const string &rule, float spill,
			int height);
	void create_rpTree(const string& rptree_file);
	void load_rpTree(const string& rptree_file);
	vector<int> retrievalLeaf(vector<float> &feature);
};
#endif
