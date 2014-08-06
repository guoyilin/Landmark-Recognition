/**
 * spatial tree: rp-tree
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

void SpatialTree::save_rpTree_sub(TreeNode *tree, ptree &pt) {
	if (tree->index > -1)//// leaf
		pt.put("xmlattr.index", tree->index);
	else {
		////not leaf
		string w_string;
		for (std::vector<float>::const_iterator iter = tree->w.begin(); iter
				!= tree->w.end(); ++iter)
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
		save_rpTree_sub(tree->left, left_child);
		pt.put_child("left", left_child);
		right_child.put("xmlattr.height", tree->right->height);
		save_rpTree_sub(tree->right, right_child);
		pt.put_child("right", right_child);
	}

}
void SpatialTree::save_rpTree(const string& rptree_file) {
	ptree pt;
	ptree root_tree;
	root_tree.put("xmlattr.height", root->height);
	root_tree.put("xmlattr.count", this->leaf_count);
	root_tree.put("xmlattr.dimension", this->dimension);
	root_tree.put("xmlattr.min_items", this->min_items);
	root_tree.put("xmlattr.samples_rp", this->samples_rp);
	root_tree.put("xmlattr.rule", this->rule);
	root_tree.put("xmlattr.spill", this->spill);
	if (root->height != 0) {
		save_rpTree_sub(root, root_tree);
	}
	pt.put_child("root", root_tree);
	boost::property_tree::write_xml(rptree_file.c_str(), pt);

}
SpatialTree::SpatialTree() {
	this->leaf_count = 0;
	this->min_items = 64;
	this->samples_rp = 10;
}
float SpatialTree::find_percentile(float percentage, const vector<float> &w) {
	vector<float> wx_sort(w);
	std::sort(wx_sort.begin(), wx_sort.end());
	int n = wx_sort.size();
	float i = (n + 1) * percentage;
	int j = (int) floor(i);
	float g = fmod(i, 1);
	if (g == 0)
		return wx_sort[j];
	else
		return (1 - g) * wx_sort[j - 1] + g * wx_sort[j];

}
vector<int> SpatialTree::retrievalLeaf(vector<float> &feature) {
	vector<int> leafs;
	queue<TreeNode *> queue;
	queue.push(root);
	while (!queue.empty()) {
		TreeNode *item = queue.front();
		if (item->index != -1)
			leafs.push_back(item->index);
		else {
			float wx = this->dot(item->w, feature);
			if (wx >= item->thresholds[0])
				queue.push(item->right);
			if (wx < item->thresholds[1])
				queue.push(item->left);
		}
		queue.pop();
	}
	return leafs;
}
void SpatialTree::print_rpTree(TreeNode *tree) {
	if (tree->index != -1) {
		cout << "leaf:" << tree->height << endl;
		return;
	}
	for (int i = 0; i < tree->w.size(); i++) {
		cout << tree->w[i] << " ";
	}
	if (tree->index == -1) {
		cout << "height:" << tree->height << "\t";
		cout << "w size:" << tree->w.size() << endl;
		print_rpTree(tree->left);
		print_rpTree(tree->right);
	}

}
vector<float> SpatialTree::stringTovector(string& w) {
	vector<float> result;
	vector<string> strs;
	boost::split(strs, w, boost::is_any_of(","));
	for (int i = 0; i < strs.size(); i++) {
		float value = atof(strs[i].c_str());
		result.push_back(value);
	}

	return result;
}
void SpatialTree::load_rpTree(const string& rptree_file) {
	ptree pt;
	read_xml(rptree_file, pt);
	this->root = new TreeNode();
	root->height = pt.get<int> ("root.xmlattr.height");
	this->min_items = pt.get<int> ("root.xmlattr.min_items");
	this->samples_rp = pt.get<int> ("root.xmlattr.samples_rp");
	this->dimension = pt.get<int> ("root.xmlattr.dimension");
	this->leaf_count = pt.get<int> ("root.xmlattr.count");
	queue<ptree> q;
	queue<TreeNode *> q2;
	ptree root_pt;
	root_pt = pt.get_child("root");
	q.push(root_pt);
	q2.push(root);
	while (!q.empty()) {
		ptree node = q.front();
		TreeNode *current = q2.front();
		int index = node.get<int> ("xmlattr.index", -1);
		if (index != -1) {
			current->index = node.get<int> ("xmlattr.index", -1);
		} else {
			current->index = -1;
			current->thresholds[0] = node.get<float> ("xmlattr.t0");
			current->thresholds[1] = node.get<float> ("xmlattr.t1");
			string w = node.get<string> ("xmlattr.w");
			current->w = stringTovector(w);

			TreeNode *leftNode = new TreeNode();
			current->left = leftNode;
			TreeNode *rightNode = new TreeNode();
			current->right = rightNode;

			leftNode->height = node.get<int> ("left.xmlattr.height");
			rightNode->height = node.get<int> ("right.xmlattr.height");

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
}

vector<float> SpatialTree::dot(const vector<vector<float> > &v1,
		const vector<float> &v2) {
	if (v2.size() == 0)
		cout << "error in compute dot!" << endl;
	vector<float> result(v1.size());
	for (int j = 0; j < v1.size(); j++)
		for (int i = 0; i < v2.size(); i++) {
			result[j] += v1[j][i] * v2[i];
		}
	return result;
}
float SpatialTree::dot(const vector<float> &v1, const vector<float> &v2) {
	if (v1.size() != v2.size() || v1.size() == 0) {
		cout << "error in compute dot!" << endl;
	}

	float result = 0;
	for (int i = 0; i < v1.size(); i++) {
		result += v1[i] * v2[i];
	}

	return result;
}
void SpatialTree::splitF(TreeNode *node) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::normal_distribution<float> distribution(0, 1);
	vector<vector<float> > W(this->samples_rp);
	for (int i = 0; i < W.size(); i++) {
		vector<float> v(this->dimension);
		W[i] = v;
	}
	vector<float> sum(this->samples_rp);

	for (int i = 0; i < this->samples_rp; i++) {
		for (int j = 0; j < this->dimension; j++) {
			W[i][j] = (float) distribution(gen);
			sum[i] += W[i][j];
		}
	}
	for (int i = 0; i < this->samples_rp; i++) {
		for (int j = 0; j < this->dimension; j++) {
			W[i][j] = W[i][j] / sum[i];
		}
	}
	vector<float> min_val(this->samples_rp, INFINITY);
	vector<float> max_val(this->samples_rp, -INFINITY);
	for (int i = 0; i < node->indices.size(); i++) {
		vector<float> point = (*this->data)[i];
		vector<float> Wx = this->dot(W, point);
		for (int j = 0; j < min_val.size(); j++) {
			if (min_val[j] > Wx[j])
				min_val[j] = Wx[j];
		}
		for (int j = 0; j < max_val.size(); j++) {
			if (max_val[j] < Wx[j])
				max_val[j] = Wx[j];
		}
	}
	int max_index = -1;
	float max_value = -INFINITY;
	for (int i = 0; i < max_val.size(); i++) {
		max_val[i] = max_val[i] - min_val[i];
		if (max_value < max_val[i]) {
			max_value = max_val[i];
			max_index = i;
		}
	}
	node->w = W[max_index];
}

SpatialTree::SpatialTree(vector<vector<float> > &data, const string &rule,
		float spill, int height) {
	this->data = &data;
	this->rule = rule;
	this->spill = spill;
	min_items = 64;
	samples_rp = 10;
	this->height = height;
	this->leaf_count = 0;
}
void SpatialTree::create_rpTree(const string& rptree_file) {
	vector<int> indices(data->size());
	for (vector<int>::iterator iter = indices.begin(); iter != indices.end(); ++iter) {
		*iter = leaf_count;
		leaf_count++;
	}
	root = new TreeNode();
	root->indices = indices;
	root->height = this->height - 1;
	root->index = -1;
	leaf_count = 0;
	this->dimension = (*data)[0].size();
	queue<TreeNode *> q;
	q.push(this->root);
	while (!q.empty()) {
		TreeNode *node = q.front();
		if (node->height == 0 || node->indices.size() < this->min_items) {
			node->index = leaf_count;
			leaf_count++;
		} else {
			this->splitF(node);
			vector<float> wx(node->indices.size());
			for (int i = 0; i < node->indices.size(); i++) {
				wx[i] = this->dot((*this->data)[node->indices[i]], node->w);
			}
			float low_percent = 0.5 - this->spill / 2;
			float high_percent = 0.5 + this->spill / 2;
			node->thresholds[0] = this->find_percentile(low_percent, wx);
			node->thresholds[1] = this->find_percentile(high_percent, wx);
			TreeNode *left = new TreeNode();
			TreeNode *right = new TreeNode();
			node->left = left;
			node->right = right;
			left->height = node->height - 1;
			right->height = node->height - 1;
			left->index = -1;
			right->index = -1;
			for (int i = 0; i < node->indices.size(); i++) {
				if (wx[i] <= node->thresholds[1])
					left->indices.push_back(node->indices[i]);
				if (wx[i] >= node->thresholds[0])
					right->indices.push_back(node->indices[i]);
			}
			q.push(left);
			q.push(right);
		}
		q.pop();
	}
	//	this->save_rpTree(rptree_file);
}
