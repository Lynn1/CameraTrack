/*
vtqa-Video Tracking for Quadrotor Aircraft 
This is a test program for tracking quadrotor.
by Lin Lin July 13, 2016.
*/

#include<opencv2/objdetect/objdetect.hpp>  
#include<opencv2/highgui/highgui.hpp>  
#include<opencv/cv.h>
#include<cxcore.h>
#include<iostream>  
#include<stdio.h>  

using namespace std;  
using namespace cv; 

const int MAX_CORNERS = 500;
const int CONTOUR_MAX_AERA = 10;

#define MAX_CLUSTERS 5
//CvScalar color_tab[MAX_CLUSTERS];

CvScalar color_tab[6];


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

//CvPoint ObjS[4] ;
//CvPoint ObjE[4] ;

//Rect selection;
//Point origin;
//bool selectObject = false;
//int trackObject = 0;

IplImage* g_image=NULL;

void detect_lk(IplImage*);
IplImage* preprocess(IplImage*);

static void onMouse( int event, int x, int y, int, void* )
{
	switch( event )
	{
	case CV_EVENT_LBUTTONDOWN:
		if (paused)
		{
			markPos[MARK_i%4] = cvPoint(x,y);
			MARK_i++;
			cout<<"FindMark: ID_"<<MARK_i<<" "<<x<<" "<<y<<endl;
			if (4 == MARK_i)   //指定了4个标记
			{
				MARK_i = 0;
				MARKASSIGNED = 0; 
				for (int k=0;k<4;k++)
					mkflag[k] = 1;
			}

		}
		break;
	case CV_EVENT_LBUTTONUP:
		break;
	}
}

static void help()
{
	cout << "\nThis is a test program for tracking quadrotor.(by Lin Lin July 13, 2016)\n";

	cout << "\n\nHot keys: \n"
		"\tESC - quit the program\n"
		"\tspace - pause video\n\n"
		"\tTo initialize tracking, You should :\n"
		"1. Press 'space' to pause the video, \n"
		"2. Use LeftButton to hit the four spots of the quadrotor with mouse.\n\n";
}

int main( )
{ 
	color_tab[0] = CV_RGB(255,255,255);
	color_tab[1] = CV_RGB(255,0,0);
	color_tab[2] = CV_RGB(0,255,0);
	color_tab[3] = CV_RGB(100,100,255);
	color_tab[4] = CV_RGB(255,0,255);
	color_tab[5] = CV_RGB(255,255,0);

	for (int k=0;k<4;k++)
		mkflag[k] = 1;

	CvCapture* capture=NULL;  
	int delay = 20; 
	char *filename = "D:/linlin/video/5.avi";
	if (!(capture = cvCaptureFromAVI(filename)))	
	{   
		fprintf(stderr, "Can not open file %s.\n", filename);
		return -2;
	}
	namedWindow("Track");
	setMouseCallback( "Track", onMouse );

	help();

	while (1)
	{
		//capture=cvCaptureFromCAM(0); 
		IplImage* fra=cvQueryFrame(capture);
		if (!fra)
		{	
			if (!(capture = cvCaptureFromAVI(filename)))	
			{   
				fprintf(stderr, "Can not open file %s.\n", filename);
				return -2;
			}
			if (!(fra=cvQueryFrame(capture)))
				break; 
		}
		cvShowImage("f",fra);

		//预处理 
		g_image= preprocess(fra);
		
		//金字塔LK光流法动捕
		detect_lk(g_image);
		
		cvReleaseImage( &g_image );

		int usr = waitKey (delay);

		if(delay>=0&&usr==32) //"Space" 
		{	
			paused = true;
			waitKey(0);
		}
		else if (usr==27) //"ESC"
			break;
		paused =false;
	}
	cvReleaseCapture(&capture);
	cvDestroyWindow("Track");
	cvDestroyWindow("f");

	return 0;
}

//预处理 
IplImage* preprocess(IplImage* img)
{
	IplImage* image=cvCreateImage(cvGetSize(img),img->depth,1);//fra-原图,image-复制灰度图
	cvCvtColor( img, image, CV_BGR2GRAY );

	// 二值化
	cvThreshold( image, image, 200, 255, CV_THRESH_BINARY );
	// 中值滤波，消除小的噪声 
	cvSmooth( image, image, CV_MEDIAN, 3, 0, 0, 0 );
	// 向下采样，去掉噪声，图像是原图像的四分之一 
	
	IplImage* pyr = cvCreateImage( cvSize((image->width & -2)/2, (image->height & -2)/2), image->depth,image->nChannels);
	cvPyrDown( image, pyr, CV_GAUSSIAN_5x5 );
	// 做膨胀操作，消除目标的不连续空洞 
	cvDilate( pyr, pyr, 0, 1 ); 
	// 向上采样，恢复图像
	cvPyrUp( pyr, image, CV_GAUSSIAN_5x5 ); 
	//cvThreshold( image, image, 100, 255, CV_THRESH_BINARY );	
	cvReleaseImage( &pyr );// free memory 
	
	//cvShowImage("processed",image);
	return image;
}

void detect_lk(IplImage* fra) //fra是实参 、frame、img 是空的 :IplImage   
{
	////对输入的图像压缩scale倍			
	//float scale=0.9;
	//CvSize dst_sz;//dst_sz 图片大小
	//dst_sz.height= fra->height  * scale; //给定图片高度与宽度（源图像的scale倍）
	//dst_sz.width = fra->width * scale;
	////IplImage *img=cvCreateImage(dst_sz,fra->depth,fra->nChannels);//img图片，和fra的深度和通道都相同
	//IplImage *img=cvCreateImage(dst_sz,fra->depth,1);
	//cvResize(fra,img, CV_INTER_LINEAR);
	//CvSize img_sz = cvGetSize(img); //img_sz是 img的大小

	CvSize img_sz = cvGetSize(fra); 
	//分配内存
	if( 0 == frame) 
	{
		frame = (IplImage**) malloc (N*sizeof(frame[0]));
		memset( frame, 0, N*sizeof(frame[0]) );
		//创建头并分配数据
		for( int i = 0; i < N; i++ ) 
		{
			cvReleaseImage( &frame[i] );
			frame[i] = cvCreateImage( img_sz, IPL_DEPTH_8U, 1 );
			cvZero( frame[i] );
		}
	}

	//转换为灰度图像并且存储到frame[i]结构中
	//cvCvtColor( img, frame[t], CV_BGR2GRAY );
	//cvCopy(img,frame[t]);

	cvCopy(fra,frame[t]);
	pre = t;
	cur = (t + 1) % N; 
	t = cur;


	//光流检测
	int corner_count = MAX_CORNERS;                                                      //最大角点数
	int win_size = 10;
	CvPoint2D32f * cornersA = new CvPoint2D32f[ MAX_CORNERS ];
	IplImage * eig_image = cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );// eig_ tmp 是两个 IplImage ，新建的
	IplImage * tmp_image = cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );
	cvGoodFeaturesToTrack(  frame[pre] ,												 //确定图像的强角点  用于角点检测
		eig_image ,													 //两个临时图片
		tmp_image ,
		cornersA ,                                                   //输出角点的位置 cornersA
		&corner_count ,                                              //检测到角点的数目 corner_count
		0.01 ,
		5.0 ,
		0 ,
		3 ,
		0 ,
		0.04 
		);
	cvFindCornerSubPix(     frame[pre],													//角点检测的第二个函数，用来精确前一个函数检测到的角点，需在cvGoodFeaturesToTrack基础之上
		cornersA, 
		corner_count, 
		cvSize(win_size, win_size), 
		cvSize(- 1, - 1),
		cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS , 20 , 0.03) 
		);

	char  features_found[ MAX_CORNERS ];
	float feature_errors[ MAX_CORNERS ];
	CvSize pyr_sz = cvSize( frame[pre]->width+8 ,frame[cur]->height/3 );
	IplImage * pyrA = cvCreateImage( pyr_sz ,IPL_DEPTH_32F ,1 );
	IplImage * pyrB = cvCreateImage( pyr_sz ,IPL_DEPTH_32F ,1 );
	CvPoint2D32f * cornersB = new CvPoint2D32f[ MAX_CORNERS ];
	cvCalcOpticalFlowPyrLK(frame[pre],									//归一化相关求出特征点在下一帧的位移，即光流计算
		frame[cur],
		pyrA,
		pyrB,
		cornersA,
		cornersB,
		corner_count,
		cvSize(win_size, win_size),
		5,
		features_found,
		feature_errors,
		cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03),
		0
		); 

	//****//
	// 找所有轮廓  
	IplImage* tempfra=cvCreateImage(cvGetSize( frame[cur]), frame[cur]->depth,1);//fra-原图,image-复制灰度图
	cvCopy(frame[cur], tempfra);

	CvMemStorage *stor; 
	CvSeq *cont;
	//Create dynamic structure and sequence.
	stor = cvCreateMemStorage(0); 
	cont = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint) , stor);
	cvFindContours( tempfra, stor, &cont, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0)); 

	//更新markRec
	if (-1 < MARKASSIGNED) //markRec未配置
	{
		for (int k=0;k<4;k++)
		{
			if ( 0>mkflag[k] ) //k点在上一帧丢失了
				continue;
			else
				mkflag[k] = -1;//初始化此帧k点的状态
			for(CvSeq* pr=cont; pr ; pr = pr->h_next) 
			{ 
				CvRect r = ((CvContour*)pr)->rect;
				CvPoint mrcenter = cvPoint(markRec[k].x+markRec[k].width/2, markRec[k].y+markRec[k].height/2);

				if (1==MARKASSIGNED)//更新markRec
				{
					if (r.x<mrcenter.x && mrcenter.x<r.x+r.width && r.y<mrcenter.y && mrcenter.y<r.y+r.height ) //旧markRec的中心点在r内
					{
						markRec[k]=r;
						mkflag[k] =0;//标记k点已更新
						break;
					}
				}
				else if (0==MARKASSIGNED)//初始化markRec
				{
					if (r.x<markPos[k].x && markPos[k].x<r.x+r.width && r.y<markPos[k].y && markPos[k].y<r.y+r.height) //mark点在r内
					{
						markRec[k]=r;
						mkflag[k] =0;//标记k点已更新
						break;
					}
				}
			}
		}
		MARKASSIGNED = 1; //markRec已配置
	}

	//画标记
	const double PI = 3.1415926; 
	IplImage *markImg = cvCreateImage(cvGetSize(frame[cur]),8,3);
	cvCvtColor(frame[cur],markImg,CV_GRAY2RGB);
	theCenterPos = cvPoint(0,0);

	for(;cont;cont = cont->h_next) 
	{ 
		CvRect r = ((CvContour*)cont)->rect; 
		int pnum=0;

		CvPoint mStart = cvPoint( 0 , 0 );
		CvPoint mEnd = cvPoint( 0 , 0 ) ;
		
		for (int i = 0; i < corner_count; i++)
		{
			if (features_found[i] == 0 || feature_errors[i] > 550)
			{
				// cout<<"Error is"<<feature_errors[i]<<endl; 
				continue;
			}
			CvPoint p1 = cvPoint( cornersA[i].x , cornersA[i].y );
			CvPoint p0 = cvPoint( cornersB[i].x , cornersB[i].y );
				
			//p0在rect内部
			if (r.x<p0.x && p0.x<r.x+r.width && r.y<p0.y && p0.y<r.y+r.height) 
			{
				//统计p0->p1运动向量
				double dis = abs(cornersA[i].x-cornersB[i].x)+abs(cornersA[i].y-cornersB[i].y);
				//if (dis>1&&dis<50)//判断单帧移动步长，排除静止和过大的
				//{   
					pnum++;
					mStart.x += cornersB[i].x;
					mStart.y += cornersB[i].y;
					mEnd.x += cornersA[i].x;
					mEnd.y += cornersA[i].y;
				//}
			}else{
				//xy在rect边上或外部
			}
		}

		// 画框框
		CvScalar color = color_tab[0];
		for (int k=0;k<4;k++)
		{
			if (markRec[k].x == r.x && markRec[k].y == r.y && -1<mkflag[k]) //&& markRec[k].width == r.width && markRec[k].height == r.height
			{
				color = color_tab[k+1];

				CvPoint mrcenter = cvPoint(markRec[k].x+markRec[k].width/2, markRec[k].y+markRec[k].height/2);
				theCenterPos.x += mrcenter.x;
				theCenterPos.y += mrcenter.y;

				cvCircle(markImg, mrcenter,4,color,CV_FILLED);
			}
		}
		cvRectangle( markImg, cvPoint(r.x,r.y), cvPoint(r.x + r.width, r.y + r.height), color, 1, CV_AA,0);

		if (pnum>0)//如果轮廓内有光流角点
		{
			//框框内运动均值
			mStart.x = mStart.x / pnum;
			mStart.y = mStart.y / pnum;
			mEnd.x = mEnd.x / pnum;
			mEnd.y = mEnd.y / pnum;

			//motions->data.fl[m*2] = mStart.x-mEnd.x;
			//motions->data.fl[m*2+1] = mStart.y-mEnd.y;
			//mpos->data.i[m*2] = mStart.x;
			//mpos->data.i[m*2+1] = mStart.y;
			//m++;

			//画箭头
			int len = 2;
			CvPoint p2 = cvPoint( mEnd.x+len*(mEnd.x-mStart.x), mEnd.y+len*(mEnd.y-mStart.y));
			cvLine( markImg , p2, mStart, color, 1 );

			//draw an arrow
			CvPoint arrow;
			double angle = atan2((double)(mStart.y - mEnd.y), (double)(mStart.x - mEnd.x)); 
			len = 10;
			int alpha = 30;
			arrow.x = p2.x + len * cos(angle + PI * alpha / 180);     
			arrow.y = p2.y + len * sin(angle + PI * alpha / 180);  
			cvLine(markImg, p2, arrow, color, 1);   
			arrow.x = p2.x + len * cos(angle - PI * alpha / 180);     
			arrow.y = p2.y + len * sin(angle - PI * alpha / 180);    
			cvLine(markImg, p2, arrow, color, 1);

		}
	} 

	//中心点
	if (1 == MARKASSIGNED)
	{
		int mkcount=4;
		for (int k = 0; k< 4;k++)
		{
			if (0 > mkflag[k])
				mkcount--;
		}
		if (mkcount>0)
		{
			theCenterPos.x = theCenterPos.x/mkcount;
			theCenterPos.y = theCenterPos.y/mkcount;
			cvCircle(markImg, theCenterPos, 6, CV_RGB(50,180,255),CV_FILLED);
		}
	}



	//聚类方法变量
	//CvMat* clusters = cvCreateMat(corner_count, 1, CV_32SC1);
	//CvMat* motions = cvCreateMat(corner_count, 1, CV_32FC2);
	//CvMat* mpos = cvCreateMat(corner_count, 1, CV_32FC2);
	//cvKMeans2( motions, 2, clusters, cvTermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0 ));

	//画聚类点
	//for (int i = 0; i < m; i++)
	//{
	//	int clustidx = clusters->data.i[i];
	//	/*cvCircle(clustimg,cvPoint(motions->data.i[i*2],motions->data.i[i*2+1]),6,color_tab[clustidx],CV_FILLED);*/
	//	cvCircle(clustimg,cvPoint(mpos->data.i[i*2],mpos->data.i[i*2+1]),6,color_tab[clustidx],CV_FILLED);
	//}

	//****//
  	cvShowImage("Track",markImg);

	// free memory 
	delete[] cornersA;
	delete[] cornersB;
	//cvReleaseMat(&mpos);
	//cvReleaseMat(&motions);
	//cvReleaseMat(&clusters);
	cvReleaseImage(&markImg);

	cvReleaseImage(&tempfra);
	cvReleaseMemStorage(&stor); 

	cvReleaseImage( &eig_image );
	cvReleaseImage( &tmp_image );
	cvReleaseImage( &pyrA );
	cvReleaseImage( &pyrB );
	//cvReleaseImage( &img );

}
