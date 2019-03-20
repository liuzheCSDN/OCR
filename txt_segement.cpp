#include<opencv2\imgproc.hpp>
#include<opencv2\highgui.hpp>
#include<iostream>

using namespace std;
using namespace cv;

#define V_PROJECT 1
#define	H_PROJECT 2

//���typedef��char_range������һ��������char_range.a)������һ���ṹ�����ͣ�������char_range s2;Ȼ�� s2.a ;
typedef struct
{
	int begin;
	int end;
}char_range;

//����ͶӰͼ
void draw(vector<int>& pos, int mode) {
	vector<int>::iterator max = max_element(begin(pos), end(pos));//�����������ֵ
	if (mode == V_PROJECT) {
		int heigth = *max;
		int width = pos.size();
		Mat project(heigth, width, CV_8UC1, Scalar(0, 0, 0));
		for (int i = 0; i < width; i++)
			for (int j = heigth - 1; j > heigth - pos[i]-1; j--)
				project.at < uchar >(j,i)= 255;//ע��Matͼ�������ϵ��ͼ�������Ĳ���
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

//��ȡ�ı���ͶӰ���ڷָ��ַ�(��ֱ��ˮƽ),������ͶӰͼ
int GetTextProject(Mat &src,vector<int>& pos,int mode){//vector pos�������ڴ洢��ֱͶӰ��ˮƽͶӰ��λ��
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

//��ȡ�ָ��ַ�ÿ�еķ�Χ��min_thresh���������С���ȣ�min_range�������������С���
int GetPeekRange(vector<int>& vertical_pos,vector<char_range>& peek_range,int min_thresh=2,int min_range=10) {
	int begin = 0; int end = 0;     //�ַ����ض�Ӧ������
	for (int i = 0; i < vertical_pos.size(); i++) {
		if (vertical_pos[i] >= min_thresh && begin == 0)
			begin = i;
		else if (vertical_pos[i] >= min_thresh && begin != 0)
			continue;
		else if (vertical_pos[i] < min_thresh && begin != 0)
		{
			end = i;
			if (end - begin >= min_range) {//���ж�һ���ַ�Ӧ���е���Сheigth
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
			printf("error");//printֻ�������û�и�ʽ���ƣ���printf.���Ը�����Ҫ���������Ҫ�ĸ�ʽ
	}
	return 0;
} 

//�и�ÿһ���ı����ַ�
int CutChar(Mat& raw, const vector<char_range>v_peek_range, vector<char_range> h_peek_range, vector<Mat>&chars_set) {
	int count = 0;
	int char_width = raw.rows;//�ٶ��ַ��Ǹ�������
	Mat show_img = raw.clone();
	cvtColor(show_img, show_img, CV_GRAY2BGR);//Ϊ����ʾ��ɫ��
	for (int i = 0; i < v_peek_range.size(); i++) {
		int char_gap = v_peek_range[i].end - v_peek_range[i].begin;//ʶ�����һ���ַ��Ŀ��
		//if (char_gap <= (int)(char_width*1.2) && char_gap >= (int)(char_width*0.8))
		{
			int x = v_peek_range[i].begin - 2>0 ? v_peek_range[i].begin - 1 : 0;
			int width = char_gap + 4 <= raw.rows ? char_gap : raw.rows;
			Rect r(x, 0, width, raw.rows);                //x��Ӧcol
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
	vector<char_range> h_peek_range;//�еķ�Χ
	GetTextProject(src, horizion_pos, H_PROJECT);//��ͶӰ
	GetPeekRange(horizion_pos, h_peek_range, 2, 10);//��ÿ���ı��ķ�Χ
#if 1
	vector<Mat> lines_set;
	for (int i = 0; i < h_peek_range.size(); i++) {
		Mat line = src(Rect(0,h_peek_range[i].begin, src.cols, h_peek_range[i].end - h_peek_range[i].begin)).clone();//�Ƿ�����
		lines_set.push_back(line);
	}
	vector<Mat> chars_set;   //����ÿ���ַ���ͼƬ
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


//�ı�Ԥ��������
Mat txt_correction(Mat image) {                        //Mat image,�����ͼ����image�ĸı����
	if (image.cols>1000 || image.rows>800) {//ͼƬ���󣬽��н�����
		pyrDown(image, image);
		pyrDown(image, image);
		pyrDown(image, image);
	}
	Mat grayImage, binaryImage;
	cvtColor(image, grayImage, CV_BGR2GRAY);//ת���Ҷ�ͼ
	adaptiveThreshold(grayImage, binaryImage, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 7, 0);//����Ӧ�˲�
	vector<vector<Point> > contours;
	//RETR_EXTERNAL:��ʾֻ������������
	//CHAIN_APPROX_NONE����ȡÿ��������ÿ�����أ����ڵ������������λ�ò����1 
	findContours(binaryImage, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	//��þ��ΰ�Χ��,֮��������boundRect����Ϊ��ʹ������area�������������RotatedRect�಻�߱��÷���
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
	Rect maxRect = boundingRect(contours[index]);//�ҳ������Ǹ����ο򣨼����������
	Mat ROI = binaryImage(maxRect);
	//imshow("maxROI", ROI);

	RotatedRect rect = minAreaRect(contours[index]);//��ȡ��Ӧ����С���ο��������������б��
	Point2f rectPoint[4];
	rect.points(rectPoint);//��ȡ�ĸ��������꣬����RotatedRect�ඨ��ķ���
	double angle = rect.angle;
	//angle += 90;
	Point2f center = rect.center;


	drawContours(binaryImage, contours, -1, Scalar(255), CV_FILLED);
	// image.copyTo(RoiSrcImg,binaryImage);
	Mat RoiSrcImg = Mat::zeros(image.size(), image.type());
	image.copyTo(RoiSrcImg);

	Mat Matrix = getRotationMatrix2D(center, angle, 0.8);//�õ���ת�������ӣ�0.8��������
	warpAffine(RoiSrcImg, RoiSrcImg, Matrix, RoiSrcImg.size(), 1, 0, Scalar(255, 255, 255));//�߽��ð�ɫ���
	return RoiSrcImg;
}

int main()
{
	//Mat srcImage = imread("D:\\Program Files\\OpenCV\\opencv\\sources\\samples\\data\\imageTextR.png");
	//Mat RoiSrcImg=txt_correction(srcImage);
	//imshow("recorrected", RoiSrcImg);//��ת���ԭͼ
	Mat RoiSrcImg = imread("imageTextN.png");
	RoiSrcImg = imread("3.jpg");
	vector<Mat> chars_set = CutSingleChar(RoiSrcImg);

	while (waitKey() != 'q') {}
	return 0;
}