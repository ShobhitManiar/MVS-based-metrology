#include<opencv2/opencv.hpp>
#include<stdio.h>
#include<iostream>
#include<vector>
using namespace cv;
using namespace std;


double r,num_pixels;
static Point origin;
bool leftDown = false, leftup = false;
Mat img;
Mat crop;
Point cor1, cor2; 
Rect box;
Point point1 = Point(-1, -1); // initialize to invalid point
Point point2 = Point(-1, -1); // initialize to invalid point


void onMouse (int event, int x, int y, int flags, void*) // Function for cropping
{
	if (event == EVENT_RBUTTONDOWN) //called when the left mouse button is pressed
	{
		leftDown = true;
		cor1.x = x;
		cor1.y = y;

	}
	if (event == EVENT_RBUTTONUP) //called when the left mouse  button is lifted
	{
		leftup = true;
		cor2.x = x;
		cor2.y = y;
		cout << "Press any key to continue"<<endl;
		
	}
	if (leftDown == true && leftup == false) //when the left button is down
	{
		Point pt;
		pt.x = x;
		pt.y = y;
		Mat temp_img = crop.clone(); //creating clone image to draw rectangle on
		rectangle(temp_img, cor1, pt, Scalar(0, 0, 255),3); //drawing a rectangle continuously
		imshow("Measure", temp_img);
	}
	if (leftDown == true && leftup == true) //when the selection is done
	{

		box.width = abs(cor1.x - cor2.x);
		box.height = abs(cor1.y - cor2.y);
		box.x = min(cor1.x, cor2.x);
		box.y = min(cor1.y, cor2.y);
		Mat img2 (crop, box); //Selecting a ROI(region of interest) from the original pic
		img = img2.clone();
		imshow("Measure", img); //showing the cropped image
		leftDown = false;	//exit loop 
		leftup = false;		//conditions

	}
}




void mouseCallback(int event, int x, int y, int flags, void* ) //Function for selecting manual points
{

	if (event == EVENT_LBUTTONDOWN) // left mouse button clicked
	{ 
		
			if (point1.x == -1) // first point not yet set
			{
			point1 = Point(x, y);
			drawMarker(img, point1, Scalar(0, 255, 0), MARKER_CROSS, 20, 3, 4);	//draws crosshiars at the point selected 	
			}
			else if (point2.x == -1) // second point not yet set
			{
			point2 = Point(x, y);
			drawMarker(img, point2, Scalar(0, 255, 0), MARKER_CROSS, 20, 3, 4); //draws crosshiars at the point selected 
			}

		imshow("Measure", img);	
		
	}
	

}

Mat Imagepreprocessing() //This function returns processed image for metrology purposes
{
	Mat grayImage, contours, image, thresh;
	GaussianBlur(img, image, Size(3, 3), 0); //gaussian blur to de-noise
	cvtColor(image, grayImage, COLOR_BGR2GRAY); //changing color to grayscale
	//thresholding for image processing using gaussian mean method and binary thresholdtype
	adaptiveThreshold(grayImage, thresh, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 153, 12); 
	Canny(thresh, contours, 0, 1); //edge detection using canny operator
	return contours;

}

void Reference()
{

	vector<Vec3f> circles;
	//Using Hough Transform to detect reference circle
	HoughCircles(Imagepreprocessing(), circles, HOUGH_GRADIENT, 1, Imagepreprocessing().rows / 1, 100, 30, 100, 1000); //  last two parameters (min_radius & max_radius) 																						
	if (circles.size() == 0) 
	{
		cout << "Cropping error or Reference could not be identified!" << endl; exit(0);

	}

	//to draw Reference Circle
	for (int i = 0; i < circles.size(); i++)
	{

		Point center(cvRound(circles[i][0]), cvRound(circles[i][1])); //stores x and y coordinates of the center
		int radius = cvRound(circles[i][2]);
		circle(img, center, radius, Scalar(0, 0, 255), 2, 8, 0);// circle outline
		num_pixels = 2 * radius; //count pixel along reference circle diameter

	}

}
	
void Measure() //This is the actual metrology function
{	
		
		int selection;
		cout << "Please Select option 1 to detect circles or 2 to select two points manually : " << endl;
		cin>>selection;// selection either 1 or 2;

			switch (selection)
			{ 
					case 1: // case 1 for automatic circle detection and measuring
					{
						Reference();
						vector<Vec3f> circle; //3-dimensional array containing x,y and radius
						HoughCircles(Imagepreprocessing(), circle, HOUGH_GRADIENT, 1, Imagepreprocessing().cols /15, 200, 20, 1, 60);
						if (circle.size() != 0)
						{
							for (int i = 0; i < circle.size(); i++)
							{
								Point center(cvRound(circle[i][0]), cvRound(circle[i][1]));
								int radius = cvRound(circle[i][2]);
								cv::circle(img, center, radius, Scalar(0, 0, 255), 2, 4, 0);
								cv::Point pta(center.x - radius, center.y); // 
								cv::Point ptb(center.x + radius, center.y);
								arrowedLine(img, pta, ptb, Scalar(0, 0, 255), 2, 4);// draws arrowed line 
								arrowedLine(img, ptb, pta, Scalar(0, 0, 255), 2, 4);// in both the direction
								double d = (r * 2 * radius) / num_pixels; // Scalling applied to get dimension 
								cout << "The dimension of the circle "<<i+1<< " is " << d << " +/- 1 mm" << endl;
								stringstream ss;
								ss << fixed <<setprecision(3) << d; //converting floating point to string to display value upto 3 decimal points
								string mystring = ss.str();
								putText(img, //target image
									mystring + " mm", //text
									Point(center.x- 50, center.y - 50), // position
									cv::FONT_HERSHEY_SIMPLEX,
									0.5, //scale 
									CV_RGB(0,0,255), //font color
									1,4); //thickness and line type
								imshow("Measure", img);
								
							}
							cout << "Do you want to select point manually as well type y or n" << endl;
								string answer;
								cin >> answer;
								if (answer == "n") //if typed n the program exits from here, else it goes to next case
								{	cout << "Please press any key to exit" << endl;
									break;
								}

								
							
						}
						else 
						{
							cout << "No circle found please retry" << endl;
							break;
						}
						
						
						
						
					}
				


					case 2:  // case 2 to manually select two points and measuring
					{	Reference();
						cout << "Please click two points with the left mouse button " << endl;
						cout << "After selecting two points press any key to measure dimesnion" << endl;
						setMouseCallback("Measure", mouseCallback, NULL);
						waitKey(0);
						
							if (point1.x != -1 && point2.x != -1)
							{
									arrowedLine(img, point1, point2, Scalar(0, 0, 255), 3, 4); // draws arrowed line from point 1 to 2
									arrowedLine(img, point2, point1, Scalar(0, 0, 255), 3, 4);  //draws arrowed line from point 2 to 1
									double dist = norm(point1 - point2);// Calculate the distance between the two points (in pixels)
									double a = (r * dist) / num_pixels; // Scalling to get dimension 
									cout << "The dimension between points clicked is " << a << "+/- 1 mm" << endl;
									stringstream ss;
									ss << fixed << setprecision(3) << a; //converting floating point to string for text display
									string mystring = ss.str();
									cv::putText(img, //target image
										"Length : " + mystring + " mm", //text to display
										Point(((point1.x + point2.x)/2) , ((point1.y + point2.y)/ 2)), //position of text
										cv::FONT_HERSHEY_SIMPLEX,
										0.5, //font scale
										CV_RGB(0, 0, 255), //font color
										1,4);
									
											
							}
							imshow("Measure", img);
							cout << "Please press any key while on image to exit" << endl;
							break;
					}
							

					default: 
					{
						cout << "Wrong input make appropriate selection. Retry!" << endl;
						Measure();
					}
			}
	

}

int main()
{
	Mat img1 = imread("Test.jpg"); if (img1.empty()) { cout << "The image could not load or is empty" << endl; return -1; }
	cout << "Enter the reference circle diameter in mm" << endl;
	cin >> r; // take user input for real dimension of reference circle drawn
	resize(img1, crop, Size(), 0.5, 0.5); // resizing to half the image size
	namedWindow("Measure");
	imshow("Measure", crop);
	cout << "The program contains two methods of finding dimension: (i) autonomous radii measurement and (ii) manual points selection " << endl;	
	cout << "Crop image by draging right mouse button  from left ro right" << endl;
	setMouseCallback("Measure", onMouse);
	waitKey(0);
	Measure();
	waitKey(0);
	destroyAllWindows();
	return 0;
}
	

	
	
	