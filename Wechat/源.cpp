#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;
using namespace cv;
Point getCurrentPosition(Mat& frame, Mat& temp);
Rect getTargetROI_region(Mat& frame, Point currentPosition);
Point getTargetPosition(Mat& ROI, Point ROIpos);
void getContour(Mat& img, vector<vector<Point> >& contours, vector<Vec4i>& hierarchy);
void drawContour(Mat& dst, vector<vector<Point> >& contours, vector<Vec4i>& hierarchy);
double calcDistance(Point p1, Point p2);
void getScreenShot();
void jump(double distance);
int getDelay();
int main()
{
	Mat frame, grayFrame, temp, matchImg, targetROI, dst;
	Point currentPosition, targetPosition;
	Rect targetROI_region;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	double distance;
	temp = imread("D:\\IMAGE\\main2.png", 0);
	VideoCapture capture("D:\\IMAGE\\test.mp4");
	namedWindow("Result", CV_WINDOW_FREERATIO);
	srand(time(0));
	while (1)
	{
		getScreenShot();
		frame = imread("autojump.png");
		//capture >> frame;
		imshow("Result", frame);
		if (frame.empty())
		{
			break;
		}
		int result_rows = frame.rows - temp.rows + 1;
		int result_cols = frame.cols - temp.cols + 1;
		if (matchImg.empty())
		{
			matchImg.create(result_rows, result_cols, CV_32FC1);
		}
		currentPosition = getCurrentPosition(frame, temp);
		//circle(frame, currentPosition, 5, Scalar(255, 0, 0), 10);
		targetROI_region = getTargetROI_region(frame, currentPosition);
		targetROI = frame(targetROI_region);
		dst = Mat::zeros(targetROI.rows, targetROI.cols, frame.type());
		//getContour(targetROI, contours, hierarchy);
		//drawContour(dst, contours, hierarchy);
		imshow("Result", dst);
		targetPosition = getTargetPosition(targetROI, Point(targetROI_region.x, targetROI_region.y));
		//circle(frame, target, 5, Scalar(0, 255, 0), 5);
		distance = calcDistance(currentPosition, targetPosition);
		jump(distance);
		int delay = getDelay();
		waitKey(delay);
		char k = waitKey(33);
		if (k == 27)
			break;
	}
	return 0;
}

Point getCurrentPosition(Mat& frame, Mat& temp)
{
	Mat matchImg, grayImg;
	int method = TM_SQDIFF;
	matchImg.create(frame.rows - temp.rows, frame.cols - temp.cols, CV_32FC1);
	cvtColor(frame, grayImg, COLOR_BGR2GRAY);
	matchTemplate(grayImg, temp, matchImg, method);
	normalize(matchImg, matchImg, 1, 0, NORM_MINMAX, -1, Mat());
	double minValue, maxValue;
	Point minLocation, maxLocation, matchLocation;
	minMaxLoc(matchImg, &minValue, &maxValue, &minLocation, &maxLocation);
	if ((method == TM_SQDIFF) or (method == TM_SQDIFF_NORMED))
	{
		matchLocation = minLocation;
	}
	else
	{
		matchLocation = maxLocation;
	}
	return Point(matchLocation.x + temp.cols / 2, matchLocation.y + temp.rows);
}

Rect getTargetROI_region(Mat& frame, Point currentPosition)
{
	int half_rows = frame.rows / 2;
	int half_cols = frame.cols / 2;
	Rect ROI_region;
	if (currentPosition.x < frame.cols / 2)
	{
		ROI_region.x = half_cols;
		ROI_region.y = 500;
		ROI_region.width = half_cols;
		ROI_region.height = currentPosition.y  -500;
	}
	else if(currentPosition.x >= frame.cols / 2)
	{
		ROI_region.x = 0;
		ROI_region.y = 500;
		ROI_region.width = half_cols;
		ROI_region.height = currentPosition.y - 500;
	}
	//rectangle(frame, ROI_region, Scalar(0, 0, 255), 4);
	return ROI_region;
}

Point getTargetPosition(Mat& ROI, Point ROIpos)
{
	Mat grayROI, temp, result;
	Point target;
	ROI.copyTo(temp);
	namedWindow("ROI", CV_WINDOW_FREERATIO);
	cvtColor(temp, grayROI, COLOR_BGR2GRAY);
	GaussianBlur(grayROI, result, Size(3, 3), 0);
	Canny(result, result, 0, 8, 3);
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(result, result, element, Point(-1,-1), 5);
	imshow("ROI", result);
	for (int i = 0; i < ROI.rows; i++)
	{
		uchar* data = result.ptr<uchar>(i);
		for (int j = 0; j < ROI.cols; j++)
		{
			if (data[j] >= 100)
			{
				target.x = ROIpos.x + j;
				target.y = ROIpos.y + i + 17;
				return target;
				for (int k = i; k < ROI.rows; k++)
				{
					data = result.ptr<uchar>(k);

					if (data[j] >= 100)
					{
						if ((k - i >= 150) and (k - i < 220))
						{
							target.x = ROIpos.x + j;
							target.y = ROIpos.y + i + (k - i) / 2;
							return target;
						}
						else if (k - i >= 220)
						{
							target.x = ROIpos.x + j;
							target.y = ROIpos.y + i + 40;
							return target;
						}
					}
				}
			}
		}
	}
}

double calcDistance(Point p1, Point p2)
{
	return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

void getScreenShot()
{
	system("adb shell screencap -p /sdcard/autojump.png");
	system("adb pull /sdcard/autojump.png");
}

void jump(double distance)
{
	char buffer[50];
	int dis = static_cast<int>(distance * 1.35);
	int touch_x = rand() % 50 + 320;
	int touch_y = rand() % 50 + 410;
	sprintf_s(buffer, "adb shell input swipe %d %d %d %d %d", touch_x, touch_y, touch_x, touch_y, dis);
	cout << buffer << endl;
	system(buffer);
}

int getDelay()
{
	int delay;
	delay = rand() % 3000 + 1000;
	return delay;
}

void getContour(Mat& img, vector<vector<Point> >& contours, vector<Vec4i>& hierarchy)
{
	Mat edge = Mat::zeros(img.rows, img.cols, CV_8UC1);
	GaussianBlur(img, img, Size(3, 3), 0);
	Canny(img, edge, 3, 50);
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(edge, edge, element, Point(-1, -1), 5);
	findContours(edge, contours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE);
	namedWindow("edge", WINDOW_FREERATIO);
	imshow("edge", edge);
}

void drawContour(Mat& dst, vector<vector<Point> >& contours, vector<Vec4i>& hierarchy)
{
	for (int index = 0; index >= 0; index = hierarchy[index][0])
	{
		Scalar color(rand() & 255, rand() & 255, rand() & 255);
		drawContours(dst, contours, index, color, 2, 8, hierarchy);
	}
}