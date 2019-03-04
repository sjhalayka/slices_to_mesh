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

	if (0 == slices.size())
	{
		cout << "No slices found" << endl;
		return 2;
	}
	else if(1 == slices.size())
	{
		cout << "Not enough slices found" << endl;
		return 3;		
	}

	const float threshold = 0.5f;

	const size_t x_res = slices[0].cols;
	const size_t y_res = slices[0].rows;
	const size_t z_res = slices.size();
	const float x_grid_min = -(x_res / 2.0f);
	const float x_grid_max =   x_res / 2.0f;
	const float y_grid_min = -(y_res / 2.0f);
	const float y_grid_max =   y_res / 2.0f;
	const float z_grid_min = -(z_res / 2.0f);
	const float z_grid_max =   z_res / 2.0f;

	vector<triangle> triangles;
	vector<float> xyplane0(x_res*y_res, 0);
	vector<float> xyplane1(x_res*y_res, 0);

	size_t z = 0;

	// Calculate 0th xy plane.
	for (size_t x = 0; x < x_res; x++)
		for (size_t y = 0; y < y_res; y++)
			xyplane0[x*y_res + y] = slices[z].at<unsigned char>(y, x) / 255.0f;

	// Prepare for 1st xy plane.
	z++;

	// Calculate 1st and subsequent xy planes.
	for (; z < z_res; z++)
	{
		cout << "Calculating triangles from xy-plane pair " << z << " of " << z_res - 1 << endl;

		for (size_t x = 0; x < x_res; x++)
			for (size_t y = 0; y < y_res; y++)
				xyplane1[x*y_res + y] = slices[z].at<unsigned char>(y, x) / 255.0f;

		// Calculate triangles for the xy-planes corresponding to z - 1 and z by marching cubes.
		tesselate_adjacent_xy_plane_pair(
			xyplane0, xyplane1,
			z - 1,
			triangles,
			threshold, // Use threshold as isovalue.
			x_grid_min, x_grid_max, x_res,
			y_grid_min, y_grid_max, y_res,
			z_grid_min, z_grid_max, z_res);

		// Swap memory pointers (fast) instead of performing a memory copy (slow).
		xyplane1.swap(xyplane0);
	}

	if (0 < triangles.size())
		write_triangles_to_binary_stereo_lithography_file(triangles, "out.stl");

	return 0;
}
