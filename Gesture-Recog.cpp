// Special Topic Testing.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <math.h>

using namespace std;

class hand_gesture
{
private:
	cv::Mat img_frame;
	cv::Mat hsv_image;
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;

public:
	cv::Mat read_frame(cv::VideoCapture camera);
	cv::Mat preprocess_frame(cv::Mat frame);
	void convex_rec(cv::Mat input_frame);
	void initiate_rec();
};

//Function to read each frame from the webcam
cv::Mat hand_gesture::read_frame(cv::VideoCapture camera)
{

	// Defining contours and hierarchy variables
	cv::Mat track_image;
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;

	// Reading frame from webcam
	bool read_frame = camera.read(img_frame);
	if (!read_frame)
	{
		cout << "Frame not read";
		return img_frame;
	}

	// Changing from BGR to HSV colorspace
	cv::cvtColor(img_frame, hsv_image, CV_BGR2HSV);
	cv::Mat drawing = cv::Mat::zeros(img_frame.size(), CV_8UC3);

	// Setting lower range of HSV values for orange
	int hue_low = 10;
	int sat_low = 100;
	int val_low = 150;

	// Setting upper range of HSV values for orange
	int hue_high = 20;
	int sat_high = 255;
	int val_high = 255;

	//Track image stores the masked image extracted from the video frame
	cv::inRange(hsv_image, cv::Scalar(hue_low, sat_low, val_low), cv::Scalar(hue_high, sat_high, val_high), track_image);

	// Defining structuring element for erosion and dilation
	int erosion_size = 4;
	cv::Mat element = getStructuringElement(cv::MORPH_ELLIPSE,
		cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
		cv::Point(2 * erosion_size, 2 * erosion_size));

	// Performing dilation and thresholding
	cv::dilate(track_image, track_image, element);
	cv::threshold(track_image, track_image, 10, 255, CV_THRESH_BINARY + CV_THRESH_OTSU);
	return track_image;
}

// Function to preprocess each read frame
cv::Mat hand_gesture::preprocess_frame(cv::Mat frame)
{

	// Setting the structural element for erosion and dilation
	int erosion_size = 4;
	cv::Mat element = getStructuringElement(cv::MORPH_ELLIPSE,
		cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
		cv::Point(2 * erosion_size, 2 * erosion_size));

	cv::RNG rng(12345);

	// Defining a new image to store only the hand contour
	cv::Mat final_hand = cv::Mat::zeros(frame.size(), CV_8UC3);
	cv::findContours(frame, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point());
	if (contours.size() > 0)
	{
		for (int i = 0; i < contours.size(); i++)
		{
			if (contours[i].size() > 150)
			{

				// Drawing the contour on the final_hand image
				cv::Scalar color = cv::Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
				drawContours(final_hand, contours, (int)i, color, -1, cv::LINE_8, hierarchy, 0);
			}
		}
	}

	// Changing to grayscale and overlaying on original frame
	cv::cvtColor(final_hand, final_hand, CV_BGR2GRAY);
	cv::bitwise_and(frame, final_hand, final_hand);
	cv::erode(final_hand, final_hand, element);

	// Performing blurring and thresholding on final_hand image
	cv::GaussianBlur(final_hand, final_hand, cv::Size(15, 15), 0.0, 0);
	cv::threshold(final_hand, final_hand, 10, 255, CV_THRESH_BINARY + CV_THRESH_OTSU);
	return final_hand;
}

void hand_gesture::convex_rec(cv::Mat input_frame)
{

	char a[40];
	char s[40];
	const char* arrayNames[3] = { "Rock", "Paper", "Scissors" };
	int RandIndex = 0;
	srand(time(NULL));
	RandIndex = rand % 3;

	// Finding contours of the hand image
	cv::findContours(input_frame, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point());
	if (contours.size() > 0)
	{

		size_t max_index = -1;
		size_t max_size = 0;

		// Getting the size and index of the largest contour
		for (size_t i = 0; i < contours.size(); i++)
		{
			if (contours[i].size() > max_size)
			{
				max_size = contours[i].size();
				max_index = i;
			}
		}
		int count;

		//Defining Vectors for storing convex hull details
		vector<vector<int>> hull(contours.size());
		vector<vector<cv::Point>> hull_point(contours.size());
		vector<vector<cv::Vec4i>> defects(contours.size());

		for (size_t i = 0; i < contours.size(); i++)
		{

			// If contour is sufficiently large
			if (contourArea(contours[i]) > 5000)
			{

				// Constructing convex hull and detecting convexity defects
				convexHull(contours[i], hull[i], true);
				convexityDefects(contours[i], hull[i], defects[i]);

				// If the largest contour is encountered in iteration
				if (max_index == i)
				{
					for (int k = 0; k < hull[i].size(); k++)
					{
						int ind = hull[i][k];
						hull_point[i].push_back(contours[i][ind]);
					}
					count = 0;

					// Iterating over defects of the largest contour
					for (int k = 0; k < defects[i].size(); k++)
					{

						// Checking if defect in convex hull is sufficiently large
						if (defects[i][k][3] > max_size * 15)
						{

							// Storing the end portion of the particular defect
							int point_end = defects[i][k][1];
							cv::circle(img_frame, contours[i][point_end], 3, cv::Scalar(255, 0, 0), 2);
							count++;
						}
					}
					if (count == 0)
						strcpy_s(a, "Rock");
					else if (count == 1)
						strcpy_s(a, "Rock");
					else if (count == 2)
						strcpy_s(a, "Rock");
					else if (count == 3)
						strcpy_s(a, "Scissors");
					else if (count == 4)
						strcpy_s(a, "Paper");
					else if (count == 5)
						strcpy_s(a, "Paper");

					int x = strlen(a);
					int y = strlen(arrayNames[RandIndex]);

					// Printing text and drawing all the contours (convex hull, hand )
					// string text = to_string(count);
					if ((y == 8 && x == 8) || (y == 4 && x == 4) || (y == 5 && x == 5))
					{
						strcpy_s(s, "DRAW");
						putText(img, s, Point(70, 70), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 0, 0), 2, 8, false);
						putText(img, a, Point(400, 400), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 0, 0), 2, 8, false);
						putText(img, arrayNames[RandIndex], Point(70, 400), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 0, 0), 2, 8, false);
						cv::drawContours(img_frame, contours, i, cv::Scalar(255, 255, 0), 2, 8, vector<cv::Vec4i>(), 0, cv::Point());
						cv::drawContours(img_frame, hull_point, i, cv::Scalar(255, 255, 0), 1, 8, vector<cv::Vec4i>(), 0, cv::Point());
						cv::drawContours(img_frame, hull_point, i, cv::Scalar(0, 0, 255), 2, 8, vector<cv::Vec4i>(), 0, cv::Point());
					}
					else if ((x == 8 && y == 5) || (x == 5 && y == 4) || (x == 4 && y == 8))
					{
						strcpy_s(s, "WIN");
						putText(img, s, Point(70, 70), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 0, 0), 2, 8, false);
						putText(img, a, Point(400, 400), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 0, 0), 2, 8, false);
						putText(img, arrayNames[RandIndex], Point(70, 400), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 0, 0), 2, 8, false);
						cv::drawContours(img_frame, contours, i, cv::Scalar(255, 255, 0), 2, 8, vector<cv::Vec4i>(), 0, cv::Point());
						cv::drawContours(img_frame, hull_point, i, cv::Scalar(255, 255, 0), 1, 8, vector<cv::Vec4i>(), 0, cv::Point());
						cv::drawContours(img_frame, hull_point, i, cv::Scalar(0, 0, 255), 2, 8, vector<cv::Vec4i>(), 0, cv::Point());
					}
					else if ((x == 5 && y == 8) || (x == 4 && y == 5) || (x == 8 && y == 4))
					{
						strcpy_s(s, "LOSE");
						putText(img, s, Point(70, 70), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 0, 0), 2, 8, false);
						putText(img, a, Point(400, 400), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 0, 0), 2, 8, false);
						putText(img, arrayNames[RandIndex], Point(70, 400), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 0, 0), 2, 8, false);
						cv::drawContours(img_frame, contours, i, cv::Scalar(255, 255, 0), 2, 8, vector<cv::Vec4i>(), 0, cv::Point());
						cv::drawContours(img_frame, hull_point, i, cv::Scalar(255, 255, 0), 1, 8, vector<cv::Vec4i>(), 0, cv::Point());
						cv::drawContours(img_frame, hull_point, i, cv::Scalar(0, 0, 255), 2, 8, vector<cv::Vec4i>(), 0, cv::Point());
					}
				}
			}
		}
		imshow("Original_image", img_frame);
		if (cv::waitKey(30) == 27)
		{
			return;
		}
	}
}

void hand_gesture::initiate_rec()
{

	// Creating object and checking if it is open
	cv::VideoCapture camera(0);
	if (!camera.isOpened())
	{
		cout << "Camera not opened";
		return;
	}
	while (1)
	{
		// Reading input frame and converting to HSV
		cv::Mat input_frame = read_frame(camera);

		// Preprocessing the HSV frame and extracting the hand
		cv::Mat process_frame = preprocess_frame(input_frame);

		// Constructing convex hull and displaying result
		convex_rec(process_frame);
	}
}
int main(int argc, const char** argv)
{

	hand_gesture obj;
	obj.initiate_rec();
}