#include "main.h"

#include <opencv2/opencv.hpp>
using namespace cv;
#pragma comment(lib, "opencv_world340.lib")


int main(void)
{
	vector<Mat> slices;
	ifstream file_list("file_list.txt");
	string line;
		
	while (getline(file_list, line))
	{
		if ("" == line)
			continue;

		Mat slice = imread(line, CV_LOAD_IMAGE_GRAYSCALE);

		if (slice.empty())
		{
			cout << "Could not read input file:" << line << endl;
			return 1;
		}

		slices.push_back(slice);
	}

	size_t res = slices.size();

	vector<triangle> triangles;
	vector<float> xyplane0(res*res, 0);
	vector<float> xyplane1(res*res, 0);

	size_t z = 0;
	float threshold = 0.5f;
	float grid_min = -1.0f;
	float grid_max = 1.0f;

	// Calculate 0th xy plane.
	for (size_t x = 0; x < res; x++)
		for (size_t y = 0; y < res; y++)
			xyplane0[x*res + y] = static_cast<float>(slices[z].at<unsigned char>(y, x) / 255.0f);

	// Prepare for 1st xy plane.
	z++;

	// Calculate 1st and subsequent xy planes.
	for (; z < res; z++)
	{
		cout << "Calculating triangles from xy-plane pair " << z << " of " << res - 1 << endl;

		for (size_t x = 0; x < res; x++)
			for (size_t y = 0; y < res; y++)
				xyplane1[x*res + y] = static_cast<float>(slices[z].at<unsigned char>(y, x) / 255.0f);

		// Calculate triangles for the xy-planes corresponding to z - 1 and z by marching cubes.
		tesselate_adjacent_xy_plane_pair(
			xyplane0, xyplane1,
			z - 1,
			triangles,
			threshold, // Use threshold as isovalue.
			grid_min, grid_max, res,
			grid_min, grid_max, res,
			grid_min, grid_max, res);

		// Swap memory pointers (fast) instead of performing a memory copy (slow).
		xyplane1.swap(xyplane0);
	}

	if (0 < triangles.size())
		write_triangles_to_binary_stereo_lithography_file(triangles, "out.stl");

	return 0;
}