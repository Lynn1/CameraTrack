/*
Video Tracking for Quadrotor Aircraft 
This is a test program for tracking quadrotor.
by Lin Lin July 13, 2016.
latest update: August 6,2016
*/
#include <Winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include <opencv\highgui.h>
#include <opencv\cv.h>
#pragma comment(lib, "opencv_highgui249.lib")
#pragma comment(lib, "opencv_core249.lib")
#pragma comment(lib, "opencv_imgproc249.lib")

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;
using namespace cv; 

int CAMNUM = 3;

//camera parameter
//const int F_LEN = 16;	
const int F_LEN = 12;	//focal length = 16mm
const float ACTUAL_WIDTH = 4.8; //4.8mm*3.6mm
const float ACTUAL_HEIGHT = 3.6;
const int FRAME_WIDTH = 640;	//default capture width and height
const int FRAME_HEIGHT = 480;
//const float Cam_Light_D = 13-4;
const float Marks_D = 48+5; 
float CLD_MD[4];

//tracing parameter
const int MAX_NUM_OBJECTS=50;	//max number of objects to be detected in frame
const int MIN_OBJECT_AREA = 5*5;	//minimum and maximum object area
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;

int V_MIN = 255;
int V_MAX = 256;
//int S_Diff = 2;		//acceptable slope difference between parallel lines
//int S_Diff_MAX = 20;
int D_Diff = 1;	//acceptable distance difference between parallel lines
int D_Diff_MAX = 100;
int AREADIFF = 10;
int AREADIFF_MAX = 100;
int STEPS = 1;
int STEPS_MAX = 10;
int DELAY = 15;
int DELAY_MAX = 33;

string TrackWinName[4];
string OriginWinName[4];
string BarWinName;

//as client
int serAnser = -1;
SOCKADDR_IN addrSrv;

void initSocket(SOCKET &socketSrv, SOCKET &socketClient);
void createTrackbars();
void preprocess(Mat &srcimg, Mat&thresholdImg);
int  trackObjectTest(int &cx, int &cy, Mat&thresholdImg, Mat &markImg);
int  trackObject(int &cx, int &cy, float &len, Mat&thresholdImg, Mat &markImg);
void drawObject(int x, int y,Mat &frame,int flag =0);
int  TransAngle(Point obj, Point tar, double &h_spin, double &v_spin);
void on_trackbar( int, void* )
{
	//This function gets called whenever a trackbar position is changed
}
string intToString(int number){
	stringstream ss;
	ss << number;
	return ss.str();
}
void initgloable()
{
	CLD_MD[0]= 0.169;
	CLD_MD[1]= 0.04;
	CLD_MD[2]= 0.16;
	CLD_MD[3]= 0.169;

	for (int i =0;i<CAMNUM;i++)
	{
		TrackWinName[i] = "Processed"+intToString(i+1);
		OriginWinName[i] = "Original"+intToString(i+1);
	}

	BarWinName = "Bar";
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

void initSocket(SOCKET &socketSrv, SOCKET &socketClient)
{
	//初始化套接字
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(1, 1);
	WSAStartup(wVersion, &wsaData);

	socketSrv = socket(AF_INET, SOCK_STREAM, 0);

	//as client:
	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//   192.168.1.2  
	//as server:
	//SOCKADDR_IN addrSrv;
	//addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//server

	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(6000);

	//绑定-server
	//bind(socketSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	//监听-server
	//listen(socketSrv, 3);

	cout<<"waiting for connection..."<<endl;

	//as client:
	socketClient = socket(AF_INET, SOCK_STREAM, 0);
	cout<<"connecting...\n";
	serAnser = connect(socketClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//连接
	if(!serAnser)
		cout<<"connected!\n";

	//as server:
	//SOCKADDR_IN addrClient;
	//int nLen = sizeof(SOCKADDR);
	//socketClient = accept(socketSrv, (SOCKADDR*)&addrClient, &nLen);
}


int main()
{ 
	help();
	initgloable();
	createTrackbars();//create slider bars for filtering

	SOCKET socketSrv ;
	SOCKET socketClient;
	initSocket(socketSrv,socketClient);
		
	int delaySteps =0;

	int delay = 30; 
	//char *filename = "D:/linlin/video/5m-15m.mov";
	//char *filename = "D:/linlin/video/4.avi";	

	//Matrix to store each frame of the camera feed
	Mat camFeed[4];
	//matrix storage for binary threshold image
	Mat threshImg[4];
	//x and y values for the location of the object
	
	int tarcount=0;
	
	VideoCapture cap[4];
	for (int i =0;i<CAMNUM;i++)
	{
		cap[i].open(i+1);
		if(!cap[i].isOpened())  
		{  
			cout<<"Can't open camera"<<i<<endl;
			return -1;  
		} 
		namedWindow(TrackWinName[i]);
		namedWindow(OriginWinName[i]);
	}

	while(1){
		delaySteps++;
		if (DELAY<=0)
			DELAY=1;
		
		for (int i =0;i<CAMNUM;i++)
		{
			if (!cap[i].read(camFeed[i]))//store image to matrix
			{	
				cout<<"Can't read camFeed!"<<i<<endl;
				break;
			}

			preprocess(camFeed[i], threshImg[i]);

			//initialize
			Point obj = Point(0,0);
			float len =0;
			Point spot;
			int tc = trackObject(obj.x, obj.y, len,threshImg[i],camFeed[i]);
			//int tc = trackObjectTest(obj.x, obj.y, threshImg[i],camFeed[i]);

			double h_spin=0,v_spin=0;
			char chs[10];
			char cvs[10];
			int dospin = 0;
			char sSend[128];
			if(1==tc)
			{
				//float dy = len * Cam_Light_D / Marks_D ;
				float dy = len * CLD_MD[i];
				spot.x = FRAME_WIDTH/2;
				//spot.y = FRAME_HEIGHT/2;
				spot.y = FRAME_HEIGHT/2 + (int)dy;
				drawObject(spot.x,spot.y,camFeed[i],3);
				dospin = TransAngle(obj,spot,h_spin,v_spin);
				float hs = (float)h_spin/STEPS; // half step
				float vs = (float)v_spin/STEPS;
				
				sprintf(sSend,"%d,%f,%f\n",i+1,hs,vs);//id = i+1
			}

			/********Socket*********/
			//as client:
			if(0!=serAnser)
			{
				cout<<"connecting...\n";
				serAnser = connect(socketClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//连接
				if(!serAnser)
					cout<<"connected!\n";
			}
			if(0==serAnser && 1==dospin && delaySteps%DELAY==0)
			{
				send(socketClient, sSend, strlen(sSend), 0);
				//send(socketClient, chs, strlen(chs), 0);
				//send(socketClient, cvs, strlen(cvs), 0);
				cout<<tarcount++<<":"<<sSend;
				delaySteps = 0;
			}
			/********Socket*********/

			//show frames 
			imshow(OriginWinName[i],camFeed[i]);
			imshow(TrackWinName[i],threshImg[i]);
		}

		//delay 30ms so that screen can refresh.
		//image will not appear without this waitKey() command
		int usr = waitKey (delay);
		if(delay>=0&&usr==32) //"Space"
			waitKey(0);
		else if (usr==27) //"ESC"
			break;
	}

	//清理套接字
	send(socketClient, "end\n", strlen("end\n"), 0);
	closesocket(socketClient);
	WSACleanup();

	for (int i =0;i<CAMNUM;i++)
		cap[i].release();

	destroyAllWindows();
	return 0;
}


void createTrackbars(){
	//create window for trackbars
	namedWindow(BarWinName);
	createTrackbar( "V_MIN", BarWinName, &V_MIN, V_MAX, on_trackbar );
	createTrackbar( "V_MAX", BarWinName, &V_MAX, V_MAX, on_trackbar );
	//createTrackbar("S_Diff", BarWinName, &S_Diff, S_Diff_MAX, on_trackbar );
	createTrackbar( "ParalErr", BarWinName, &D_Diff, D_Diff_MAX, on_trackbar );
	createTrackbar( "AreaErr", BarWinName, &AREADIFF, AREADIFF_MAX, on_trackbar );
	createTrackbar( "Delay", BarWinName, &DELAY, DELAY_MAX, on_trackbar );
	createTrackbar( "Steps", BarWinName, &STEPS, STEPS_MAX, on_trackbar );
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

	Mat erodeElement = getStructuringElement( MORPH_ELLIPSE,Size(3,3));//MORPH_RECT
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement( MORPH_ELLIPSE,Size(15,15));

	erode(thresholdImg,thresholdImg,erodeElement);
	//erode(thresholdImg,thresholdImg,erodeElement);
	//erode(thresholdImg,thresholdImg,erodeElement);
	dilate(thresholdImg,thresholdImg,dilateElement);
	//dilate(thresholdImg,thresholdImg,dilateElement);
	//dilate(thresholdImg,thresholdImg,dilateElement);
}


int trackObjectTest(int &cx, int &cy, Mat&thresholdImg, Mat &markImg)
{
	int tCount = 0;
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

	if (hierarchy.size() > 0) 
	{
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if(numObjects<MAX_NUM_OBJECTS)
		{
			for (int index = 0; index >= 0; index = hierarchy[index][0]) 
			{
				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;
				//if the area is less than MIN_OBJECT_AREA(5 px x 5px) then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				if(area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA){
					int x = moment.m10/area;
					int y = moment.m01/area;
					objectFound = true;
					if (refArea<=area) //largest
					{
						refArea = area;
						cx = x;
						cy = y;
					}
					//mpoints.push_back(Point(x,y));
					//draw object location on screen
					else
						drawObject(x,y,markImg);
				}else objectFound = false;
			}
			if (objectFound)
			{
				drawObject(cx,cy,markImg,1);
				tCount = 1;
			}
		}
		else 
			putText(markImg,"TOO MUCH NOISE! ADJUST FILTER",Point(0,50),1,2,Scalar(0,0,255),2);
	}
	return tCount;
}


int trackObject(int &cx, int &cy, float &len, Mat&thresholdImg, Mat &markImg)
{
	int tCount = 0;
	//vector<Point> mpoints;
	struct CorPoint{
		Point pos;
		double area;
	};
	struct Lines{
		CorPoint sp;
		CorPoint ep;
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
	Vector<CorPoint> mpoints;
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
	//double refArea = 0;
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
					//refArea = area;
					CorPoint cp;
					cp.area = area;
					cp.pos = Point(x,y);
					mpoints.push_back(cp);
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
						//find same area points pair
						if ( abs(mpoints[i].area - mpoints[j].area) < AREADIFF*AREADIFF )
						{
							Lines ij;
							ij.sp = mpoints[i];
							ij.ep = mpoints[j];

							ij.dx = (ij.ep.pos - ij.sp.pos).x;
							ij.dy = (ij.ep.pos - ij.sp.pos).y;
							ij.slope = abs(ij.dx)>=abs(ij.dy) ? (float)ij.dy/(float)ij.dx : (float)ij.dx/(float)ij.dy;//防止分母为0
							ij.dis = ij.dx*ij.dx +ij.dy*ij.dy;
							ij.i = i;
							ij.j = j;
							mlines.push_back(ij);
							line(markImg,ij.sp.pos,ij.ep.pos,CV_RGB(70,70,70),1);
						}
					}
				}
				//float ACCP_s = (float)S_Diff/S_Diff_MAX;
				//int ACCP_d2 = D_Diff*D_Diff;
				//找出所有连线中的平行线
				for (int k =0; k<mlines.size();k++)
				{
					for (int m=0;m<k;m++)
					{
						//if((abs(mlines[k].dx) - abs(mlines[k].dy))*(abs(mlines[m].dx) - abs(mlines[m].dy))>=0 
						//	&& abs(mlines[k].slope - mlines[m].slope) < ACCP_s 
						//	&& abs(mlines[k].dis - mlines[m].dis) < ACCP_d2)

						// find parallel lines
						if( abs(abs(mlines[k].dx) - abs(mlines[m].dx))<D_Diff 
							&& abs(abs(mlines[k].dy)-abs(mlines[m].dy))<D_Diff 
							&& mlines[k].slope * mlines[m].slope >= 0 )
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
							line(markImg,pl.a.sp.pos,pl.a.ep.pos,CV_RGB(255,0,0),1);
							line(markImg,pl.b.sp.pos,pl.b.ep.pos,CV_RGB(255,0,0),1);
						}
					}
				}
				//draw object location on screen
				for (int i = 0;i<plines.size();i++)
				{
					if (plines[i].aj_idx>=0)
					{
						Point sum = plines[i].a.sp.pos + plines[i].a.ep.pos + plines[i].b.sp.pos+ plines[i].b.ep.pos ;
						cx = sum.x / 4;
						cy = sum.y / 4;
						len = sqrt((float)plines[i].b.dis);
						drawObject(cx, cy, markImg,1);
						tCount++;
					}
				}
				if (plines.size()==1)
				{
					Point sum = plines[0].a.sp.pos + plines[0].a.ep.pos + plines[0].b.sp.pos + plines[0].b.ep.pos ;
					cx = sum.x / 4;
					cy = sum.y / 4;
					len = sqrt((float)plines[0].a.dis);
					drawObject(cx, cy, markImg,2);//weak center
					tCount++;
				}
			}
		}
		else 
			putText(markImg,"TOO MUCH NOISE! ADJUST FILTER",Point(0,50),1,2,Scalar(0,0,255),2);
	}
	mpoints.clear();
	mlines.clear();
	plines.clear();

	return tCount;
}


void drawObject(int x, int y,Mat &frame, int flag)
{
	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!
	//added 'if' and 'else' statements to prevent
	//memory errors from writing off the screen (ie. (-25,-25) is not within the window!)
	Scalar color = Scalar(0,255,0);

	if (!flag)
	{
		circle(frame,Point(x,y),15,color,1);
		if(y-25>0)
			line(frame,Point(x,y),Point(x,y-25),color,1);
		else line(frame,Point(x,y),Point(x,0),color,1);
		if(y+25<FRAME_HEIGHT)
			line(frame,Point(x,y),Point(x,y+25),color,1);
		else line(frame,Point(x,y),Point(x,FRAME_HEIGHT),color,1);
		if(x-25>0)
			line(frame,Point(x,y),Point(x-25,y),color,1);
		else line(frame,Point(x,y),Point(0,y),color,1);
		if(x+25<FRAME_WIDTH)
			line(frame,Point(x,y),Point(x+25,y),color,1);
		else line(frame,Point(x,y),Point(FRAME_WIDTH,y),color,1);
	}
	else
	{
		if (flag==1)//Draw Center
			color = CV_RGB(80,80,255);
		else if (flag==2)//Draw Weak Center
			color = CV_RGB(160,160,255);
		else if (flag==3)
			color = CV_RGB(240,160,160);
		circle(frame,Point(x,y),6,color,CV_FILLED);
	}
	putText(frame,intToString(x)+","+intToString(y),Point(x,y+30),1,1,color,1);
}

int  TransAngle(Point obj, Point tar, double &h_spin, double &v_spin)
{
	if (obj.x<=0||obj.y<=0||obj.x>=FRAME_WIDTH||obj.y>=FRAME_HEIGHT)
		return 0;

	double horizontal_move = (double) ACTUAL_WIDTH * (obj.x - tar.x)/FRAME_WIDTH;
	double vertical_move = (double) ACTUAL_HEIGHT * (obj.y - tar.y)/FRAME_HEIGHT;
	double h_spinR = atan ( horizontal_move / F_LEN );
	double v_spinR = atan ( vertical_move / F_LEN );

	//double h_spinR = atan ( 2.4 / F_LEN ); // used for test
	//double v_spinR = atan ( 1.8 / F_LEN );

	//Radian to Angle Conversion
	h_spin = 90 * h_spinR / M_PI_2;
	v_spin = 90 * v_spinR / M_PI_2;

	return 1;
}