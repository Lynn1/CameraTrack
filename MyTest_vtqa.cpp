/*
vtqa-Video Tracking for Quadrotor Aircraft 
This is a test program for tracking quadrotor.
by Lin Lin July 13, 2016.
*/
#include <Winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include <opencv\highgui.h>
#include <opencv\cv.h>

//#include<opencv2/objdetect/objdetect.hpp> 
#include<cxcore.h>

#include<iostream>  
#include<stdio.h>

using namespace std;  
using namespace cv; 

const int MAX_CORNERS = 500;
const int CONTOUR_MAX_AERA = 10;

#define MAX_CLUSTERS 5
CvScalar color_tab[6];
const char* TrackWindowName = "Processed";
const char* OriginWindowName = "Original";
//const string PProcessWindowName = "Thresholded Image";
int V_MIN = 79;
int V_MAX = 256;
int S_Diff = 2;//acceptable slope difference between parallel lines
int S_Diff_MAX = 20;
int D_Diff = 20;//acceptable distance difference between parallel lines
int D_Diff_MAX = 50;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS=50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 5*5;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;


IplImage **frame = 0;             //定义一个IplImage型数组
int t = 0;                             //存储某一帧的变量
const int N = 2;                       //需要采集的帧数;
int pre=0, cur=0;                      //pre前一帧,cur当前帧

bool paused = false;
int MARKASSIGNED = -1;			//-1：标记未指定、0：刚新指定了标记，还未配置、1：标记已配置好
CvPoint markPos[4];
CvRect markRec[4];
int mkflag[4];
int MARK_i = 0;
CvPoint theCenterPos;

struct TransData{
	int x;
	int y;
};
TransData transData;

SOCKET socketClient;
void initSocket();
void preprocess(Mat &srcimg, Mat&thresholdImg);
void trackObject(int &cx, int &cy, Mat&thresholdImg, Mat &markImg);
void drawObject(int x, int y,Mat &frame,int flag =0);
void createTrackbars();
void on_trackbar( int, void* )
{
	//This function gets called whenever a trackbar position is changed
}
string intToString(int number){
	std::stringstream ss;
	ss << number;
	return ss.str();
}
static void help()
{
	cout << "\nThis is a test program for tracking quadrotor.(by Lin Lin July, 2016)\n";

	cout << "\n\nHot keys: \n"
		"\tESC - quit the program\n"
		"\tspace - pause video\n\n"
		"Trackbar:\n"
		"\tV_MIN & V_MAX: filter image between Gray Value (V_MIN, V_MAX)\n"
		"\tS_Diff:  acceptable slope difference between parallel lines,\n"
		"\t\t1(0.05) ~ 2(0.1) recommended.\n"
		"\tDst_Diff: acceptable distance difference between parallel lines,\n"
		"\t\t20 recommended.\n";
}

int main( )
{ 
	help();
	//initSocket();
	createTrackbars();//create slider bars for filtering
	
	int delay = 30; 
	char *filename = "D:/linlin/video/5m-15m.mov";
	//char *filename = "D:/linlin/video/5.avi";	

	//Matrix to store each frame of the camera feed
	Mat cameraFeed;
	//matrix storage for binary threshold image
	Mat thresholdImg;
	//x and y values for the location of the object
	int x=0, y=0;

	namedWindow(TrackWindowName);

	VideoCapture capture(filename);
	while(1){
		//store image to matrix
		//capture.read(cameraFeed);
		if (!capture.read(cameraFeed))
		{	
			capture = VideoCapture(filename);
			if (!capture.read(cameraFeed))
				break; 
		}
		preprocess(cameraFeed, thresholdImg);
		trackObject(x,y,thresholdImg,cameraFeed);

		//socketClient send message:

		//show frames 
		imshow(OriginWindowName,cameraFeed);
		imshow(TrackWindowName,thresholdImg);

		//delay 30ms so that screen can refresh.
		//image will not appear without this waitKey() command
		int usr = waitKey (delay);
		if(delay>=0&&usr==32) //"Space"
			waitKey(0);
		else if (usr==27) //"ESC"
			break;
	}

	//::closesocket(socketClient);
	//清理套接字
	//::WSACleanup();

	capture.release();
	destroyAllWindows();
	return 0;
}


void createTrackbars(){
	//create window for trackbars
	namedWindow(TrackWindowName);
	createTrackbar( "V_MIN", TrackWindowName, &V_MIN, V_MAX, on_trackbar );
	createTrackbar( "V_MAX", TrackWindowName, &V_MAX, V_MAX, on_trackbar );

	createTrackbar("S_Diff", TrackWindowName, &S_Diff, S_Diff_MAX, on_trackbar );
	createTrackbar( "Dst_Diff", TrackWindowName, &D_Diff, D_Diff_MAX, on_trackbar );
}


void preprocess(Mat &srcimg, Mat&thresholdImg)
{
	//convert frame from BGR to GRAY colorspace
	cvtColor(srcimg,thresholdImg,CV_BGR2GRAY);
	//filter image between values and store filtered image to
	//threshold matrix
	inRange(thresholdImg,Scalar(V_MIN),Scalar(V_MAX),thresholdImg);
	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle
	Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));
	erode(thresholdImg,thresholdImg,erodeElement);
	erode(thresholdImg,thresholdImg,erodeElement);
	dilate(thresholdImg,thresholdImg,dilateElement);
	dilate(thresholdImg,thresholdImg,dilateElement);
}


void trackObject(int &cx, int &cy, Mat&thresholdImg, Mat &markImg)
{
	vector<Point> mpoints;
	struct Lines{
		Point sp;
		Point ep;
		int dx;
		int dy;
		float slope;
		float dis;
		int i;
		int j;
	};
	struct ParallelLines{
		Lines a;
		Lines b;
		int aj_idx;//adjoinIdx
	};
	vector<Lines> mlines;//mark points lines
	vector<ParallelLines> plines;//parallel lines

	Mat temp;
	thresholdImg.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if(numObjects<MAX_NUM_OBJECTS){
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;
				//if the area is less than MIN_OBJECT_AREA(5 px x 5px) then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				if(area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA){
					int x = moment.m10/area;
					int y = moment.m01/area;
					objectFound = true;
					refArea = area;
					mpoints.push_back(Point(x,y));
					//draw object location on screen
					drawObject(x,y,markImg);
				}else objectFound = false;
			}
			//let user know you found an object
			if(objectFound == true)
			{
				putText(markImg,"Tracking",Point(0,30),2,1,Scalar(0,255,0),2);
				//找出所有mark点连线
				for (int i=0;i<mpoints.size();i++)
				{
					for(int j=0;j<i;j++)
					{
						Lines ij;
						ij.sp = mpoints[i];
						ij.ep = mpoints[j];

						ij.dx = (ij.ep - ij.sp).x;
						ij.dy = (ij.ep - ij.sp).y;
						ij.slope = abs(ij.dx)>=abs(ij.dy) ? (float)ij.dy/ij.dx : (float)ij.dx/ij.dy;//防止分母为0
						ij.dis = ij.dx*ij.dx +ij.dy*ij.dy;
						ij.i = i;
						ij.j = j;
						mlines.push_back(ij);
					}
				}
				float ACCP_s = (float)S_Diff/S_Diff_MAX;
				int ACCP_d2 = D_Diff*D_Diff;
				//找出所有连线中的平行线
				for (int k =0; k<mlines.size();k++)
				{
					for (int m=0;m<k;m++)
					{
						if((abs(mlines[k].dx) - abs(mlines[k].dy))*(abs(mlines[m].dx) - abs(mlines[m].dy))>=0 
							&& abs(mlines[k].slope - mlines[m].slope) < ACCP_s 
							&& abs(mlines[k].dis - mlines[m].dis) < ACCP_d2)
						{
							ParallelLines pl;
							pl.a = mlines[k];
							pl.b = mlines[m];
							pl.aj_idx = -1;

							//标记共享顶点的平行线段-即平行四边形的一对边
							for (int q = 0;q<plines.size();q++)
							{
								if (pl.a.i == plines[q].a.i||
									pl.a.i == plines[q].a.j||
									pl.a.j == plines[q].a.i||
									pl.a.j == plines[q].a.j
									)
									pl.aj_idx = q;
							}
							plines.push_back(pl);
							//draw parallel lines
							line(markImg,pl.a.sp,pl.a.ep,Scalar(0,0,255),1);
							line(markImg,pl.b.sp,pl.b.ep,Scalar(0,0,255),1);
						}
					}
				}
				//draw object location on screen
				for (int i = 0;i<plines.size();i++)
				{
					if (plines[i].aj_idx>=0)
					{
						Point sum = plines[i].a.sp + plines[i].a.ep + plines[i].b.sp+ plines[i].b.ep ;
						cx = sum.x / 4;
						cy = sum.y / 4;
						drawObject(cx, cy, markImg,1);
					}
				}
				if (plines.size()==1)
				{
					Point sum = plines[0].a.sp + plines[0].a.ep + plines[0].b.sp+ plines[0].b.ep ;
					cx = sum.x / 4;
					cy = sum.y / 4;
					drawObject(cx, cy, markImg,1);//weak center
				}
			}
		}
		else 
			putText(markImg,"TOO MUCH NOISE! ADJUST FILTER",Point(0,50),1,2,Scalar(0,0,255),2);
	}
	mpoints.clear();
	mlines.clear();
	plines.clear();
}


void drawObject(int x, int y,Mat &frame, int flag)
{
	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!
	//added 'if' and 'else' statements to prevent
	//memory errors from writing off the screen (ie. (-25,-25) is not within the window!)
	if (flag==1)//Draw Center
	{
		circle(frame,Point(x,y),6,CV_RGB(255,80,80),CV_FILLED);
		return;
	}
	else if (flag==2)//Draw Weak Center
	{
		circle(frame,Point(x,y),6,CV_RGB(80,180,255),CV_FILLED);
		return;
	}

	circle(frame,Point(x,y),15,Scalar(0,255,0),1);
	if(y-25>0)
		line(frame,Point(x,y),Point(x,y-25),Scalar(0,255,0),1);
	else line(frame,Point(x,y),Point(x,0),Scalar(0,255,0),1);
	if(y+25<FRAME_HEIGHT)
		line(frame,Point(x,y),Point(x,y+25),Scalar(0,255,0),1);
	else line(frame,Point(x,y),Point(x,FRAME_HEIGHT),Scalar(0,255,0),1);
	if(x-25>0)
		line(frame,Point(x,y),Point(x-25,y),Scalar(0,255,0),1);
	else line(frame,Point(x,y),Point(0,y),Scalar(0,255,0),1);
	if(x+25<FRAME_WIDTH)
		line(frame,Point(x,y),Point(x+25,y),Scalar(0,255,0),1);
	else line(frame,Point(x,y),Point(FRAME_WIDTH,y),Scalar(0,255,0),1);

	putText(frame,intToString(x)+","+intToString(y),Point(x,y+30),1,1,Scalar(0,255,0),1);

}


void initSocket() 
{
	//初始化套接字
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(1, 1);
	::WSAStartup(wVersion, &wsaData);
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = ::inet_addr("127.0.0.1");
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = ::htons(6000);

	socketClient = ::socket(AF_INET, SOCK_STREAM, 0);
	::connect(socketClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//连接

	//cout<<"初始完毕！准备发送数据！！"<<endl;
}
