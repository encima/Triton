#ifndef TRITON_H
#define TRITON_H

#include "/usr/local/include/opencv2/opencv.hpp"
#include "/usr/local/include/cvblob.h"
#include <dirent.h>
#include <vector>

using namespace std;
using namespace cv;
using namespace cvb;

class Triton {
	public:
		Triton();
		~Triton();

		vector<string> split(string s, char delim);
		string splitPath(string file, bool pathOrFile);
		void identifyCvBlobs(Mat *fore, string path);
		void createBGMod(vector<string> *images);
		vector<vector<string> > sortFiles(vector<string> *files);
		bool replace(std::string& str, const std::string& from, const std::string& to);
	protected:
	private:
};
#endif //TRITON_H