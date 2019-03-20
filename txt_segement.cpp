#include<opencv2\imgproc.hpp>
#include<opencv2\highgui.hpp>
#include<iostream>

using namespace std;
using namespace cv;

#define V_PROJECT 1
#define	H_PROJECT 2

//添加typedef后，char_range不再是一个变量（char_range.a)，而是一个结构体类型，必须先char_range s2;然后 s2.a ;
typedef struct
{
	int begin;
	int end;
}char_range;

//画出投影图
void draw(vector<int>& pos, int mode) {
	vector<int>::iterator max = max_element(begin(pos), end(pos));//迭代器找最大值
	if (mode == V_PROJECT) {
		int heigth = *max;
		int width = pos.size();
		Mat project(heigth, width, CV_8UC1, Scalar(0, 0, 0));
		for (int i = 0; i < width; i++)
			for (int j = heigth - 1; j > heigth - pos[i]-1; j--)
				project.at < uchar >(j,i)= 255;//注意Mat图像的坐标系与图像索引的差异
		imshow("vertiacl", project);
	}
	else if (mode == H_PROJECT) {
		int width = *max;
		int heigth = pos.size();
		Mat project = Mat::zeros(heigth, width, CV_8UC1);
		for (int i = 0; i < heigth; i++)
			for (int j = 0; j < pos[i] ; j++)
				project.at <uchar>(i, j) = 255;
		imshow("horizational", project);
	}
}

//获取文本的投影用于分割字符(垂直，水平),并画出投影图
int GetTextProject(Mat &src,vector<int>& pos,int mode){//vector pos就是用于存储垂直投影和水平投影的位置
	if (mode == V_PROJECT)
	{
		for (int i = 0; i < src.rows; i++)
		{
			//uchar* p = src.ptr<uchar>(i);
			//for (int j = 0; j < src.cols; j++)
			//	if (p[j] == 0)
			//		pos[j]=pos[j]+1;
			for (int j = 0; j < src.cols; j++)
				if (src.at<uchar>(i, j) == 0)
					pos[j]++;
		}
		draw(pos, mode);
	}
	else if (mode == H_PROJECT)
	{
		for (int i = 0; i < src.cols; i++)
			for (int j = 0; j < src.rows; j++)
				if (src.at<uchar>(j, i) == 0)
					pos[j]++;
		draw(pos, mode);
	}
	return 0;
}

//获取分割字符每行的范围，min_thresh：波峰的最小幅度，min_range：两个波峰的最小间隔
int GetPeekRange(vector<int>& vertical_pos,vector<char_range>& peek_range,int min_thresh=2,int min_range=10) {
	int begin = 0; int end = 0;     //字符像素对应的行数
	for (int i = 0; i < vertical_pos.size(); i++) {
		if (vertical_pos[i] >= min_thresh && begin == 0)
			begin = i;
		else if (vertical_pos[i] >= min_thresh && begin != 0)
			continue;
		else if (vertical_pos[i] < min_thresh && begin != 0)
		{
			end = i;
			if (end - begin >= min_range) {//该判断一行字符应具有的最小heigth
				char_range range;
				range.begin = begin;
				range.end = end;
				peek_range.push_back(range);
				begin = 0;
				end = 0;
			}
		}
		if (vertical_pos[i] < min_thresh || begin == 0)
			continue;
		else
			printf("error");//print只是输出，没有格式控制，而printf.可以根据需要，输出你需要的格式
	}
	return 0;
} 

//切割每一行文本的字符
int CutChar(Mat& raw, const vector<char_range>v_peek_range, vector<char_range> h_peek_range, vector<Mat>&chars_set) {
	int count = 0;
	int char_width = raw.rows;//假定字符是个正方形
	Mat show_img = raw.clone();
	cvtColor(show_img, show_img, CV_GRAY2BGR);//为了显示彩色框
	for (int i = 0; i < v_peek_range.size(); i++) {
		int char_gap = v_peek_range[i].end - v_peek_range[i].begin;//识别出的一个字符的宽度
		//if (char_gap <= (int)(char_width*1.2) && char_gap >= (int)(char_width*0.8))
		{
			int x = v_peek_range[i].begin - 2>0 ? v_peek_range[i].begin - 1 : 0;
			int width = char_gap + 4 <= raw.rows ? char_gap : raw.rows;
			Rect r(x, 0, width, raw.rows);                //x对应col
			rectangle(show_img, r, Scalar(0, 0, 255), 1);
			Mat single_char = raw(r).clone();
			chars_set.push_back(single_char);
			//save_cut(single_char, count);
			count++;
		}
	}
	namedWindow("cut", WINDOW_NORMAL);
	imshow("cut", show_img);
	return 0;
}


vector<Mat> CutSingleChar(Mat & img) {
	Mat src;
	cvtColor(img, src, CV_BGR2GRAY);
	threshold(src, src, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	vector<int> horizion_pos(src.rows, 0);
	vector<char_range> h_peek_range;//行的范围
	GetTextProject(src, horizion_pos, H_PROJECT);//行投影
	GetPeekRange(horizion_pos, h_peek_range, 2, 10);//求每行文本的范围
#if 1
	vector<Mat> lines_set;
	for (int i = 0; i < h_peek_range.size(); i++) {
		Mat line = src(Rect(0,h_peek_range[i].begin, src.cols, h_peek_range[i].end - h_peek_range[i].begin)).clone();//是否有误
		lines_set.push_back(line);
	}
	vector<Mat> chars_set;   //保存每个字符的图片
	for (int i = 0; i < lines_set.size(); i++) {
		Mat line = lines_set[i];
		imshow("line", line);
		vector<int> vertical_pos(line.cols,0);
		vector<char_range> v_peek_range;
		GetTextProject(line, vertical_pos, V_PROJECT);
		GetPeekRange(vertical_pos, v_peek_range,2,3);
		CutChar(line, v_peek_range, h_peek_range, chars_set);
	}
#endif
	return chars_set;
}


//文本预处理，矫正
Mat txt_correction(Mat image) {                        //Mat image,传入的图像不随image的改变而变
	if (image.cols>1000 || image.rows>800) {//图片过大，进行降采样
		pyrDown(image, image);
		pyrDown(image, image);
		pyrDown(image, image);
	}
	Mat grayImage, binaryImage;
	cvtColor(image, grayImage, CV_BGR2GRAY);//转化灰度图
	adaptiveThreshold(grayImage, binaryImage, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 7, 0);//自适应滤波
	vector<vector<Point> > contours;
	//RETR_EXTERNAL:表示只检测最外层轮廓
	//CHAIN_APPROX_NONE：获取每个轮廓的每个像素，相邻的两个点的像素位置差不超过1 
	findContours(binaryImage, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	//获得矩形包围框,之所以先用boundRect，是为了使用它的area方法求面积，而RotatedRect类不具备该方法
	float area = boundingRect(contours[0]).area();
	int index = 0;
	for (int i = 1; i<contours.size(); i++)
	{

		if (boundingRect(contours[i]).area()>area)
		{
			area = boundingRect(contours[i]).area();
			index = i;
		}
	}
	Rect maxRect = boundingRect(contours[index]);//找出最大的那个矩形框（即最大轮廓）
	Mat ROI = binaryImage(maxRect);
	//imshow("maxROI", ROI);

	RotatedRect rect = minAreaRect(contours[index]);//获取对应的最小矩形框，这个长方形是倾斜的
	Point2f rectPoint[4];
	rect.points(rectPoint);//获取四个顶点坐标，这是RotatedRect类定义的方法
	double angle = rect.angle;
	//angle += 90;
	Point2f center = rect.center;


	drawContours(binaryImage, contours, -1, Scalar(255), CV_FILLED);
	// image.copyTo(RoiSrcImg,binaryImage);
	Mat RoiSrcImg = Mat::zeros(image.size(), image.type());
	image.copyTo(RoiSrcImg);

	Mat Matrix = getRotationMatrix2D(center, angle, 0.8);//得到旋转矩阵算子，0.8缩放因子
	warpAffine(RoiSrcImg, RoiSrcImg, Matrix, RoiSrcImg.size(), 1, 0, Scalar(255, 255, 255));//边界用白色填充
	return RoiSrcImg;
}

int main()
{
	//Mat srcImage = imread("D:\\Program Files\\OpenCV\\opencv\\sources\\samples\\data\\imageTextR.png");
	//Mat RoiSrcImg=txt_correction(srcImage);
	//imshow("recorrected", RoiSrcImg);//旋转后的原图
	Mat RoiSrcImg = imread("imageTextN.png");
	RoiSrcImg = imread("3.jpg");
	vector<Mat> chars_set = CutSingleChar(RoiSrcImg);

	while (waitKey() != 'q') {}
	return 0;
}