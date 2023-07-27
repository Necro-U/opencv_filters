#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using std::cin;
using std::cout;
using std::endl;

void distance_kernel(Mat &kernel, int kernel_size);
float kernel_summer(Mat kernel, int kernel_size);
void kernel_fixer(Mat &src, Mat &kernel, Mat dst_kernel, int kernel_size, int i, int j);
void bilateral_filter(Mat src, Mat dst, Mat kernel, int kernel_size);
int MAX_COLOR_SUM = 765;

int main(int argc, char const *argv[])
{
    int kernel_size = 5;
    Mat image = imread("./inpimage", IMREAD_COLOR);
    Mat dst = Mat(image);
    Mat kernel = Mat::ones(kernel_size, kernel_size, CV_32F);
    distance_kernel(kernel, kernel_size);
    bilateral_filter(image, dst, kernel, kernel_size);

    return 0;
}

void distance_kernel(Mat &kernel, int kernel_size)
{
    int mid = kernel_size / 2;
    float multiplier = 4 / sqrt(2 * mid * mid); // this is for maximum value of anyoklidian distance we will scale this 0 to 5
    for (int i = 0; i < kernel.cols; i++)
    {
        for (int j = 0; j < kernel.rows; j++)
        {
            float oklidian_distance = sqrt(pow(i - mid, 2) + pow(j - mid, 2));
            kernel.at<float>(i, j) = 5 - oklidian_distance * multiplier;
        }
    }
    kernel = kernel / 10;
    cout << kernel << endl;
}

float kernel_summer(Mat kernel, int kernel_size)
{
    double sum = 0.;
    for (int x = 0; x < kernel_size; x++)
    {
        for (int y = 0; y < kernel_size; y++)
        {
            sum += kernel.at<float>(x, y);
        }
    }
    return sum;
}

void kernel_fixer(Mat &src, Mat &kernel, Mat dst_kernel, int kernel_size, int i, int j)
{
    Vec3b main_point = src.at<int>(i, j);
    Vec3b iterator;
    float total;
    for (int x = -1 * kernel_size + 1; x < kernel_size; x++)
    {
        for (int y = -1 * kernel_size + 1; y < kernel_size; y++)
        {
            if (j + y < 0 || j + y >= src.cols || i + x < 0 || i + x >= src.rows)
            {
                continue;
            }
            iterator = src.at<Vec3b>(x + i, y + j) - main_point;
            total = iterator[0] + iterator[1] + iterator[2];
            dst_kernel.at<float>(x, y) = kernel.at<float>(x, y) + (5 - 4 * (total / MAX_COLOR_SUM)) / 10;
        }
    }
}

void bilateral_filter(Mat src, Mat dst, Mat kernel, int kernel_size)
{
    // 5- 4* (sums/765)
    Mat dst_kernel = Mat(kernel);
    int x, y;
    for (int i = 0; i < src.cols; i++)
    {
        for (int j = 0; j < src.rows; j++)
        {
            kernel_fixer(src, kernel, dst_kernel, kernel_size, i, j);
            Vec3b tot = Vec3b(0, 0, 0);
            float divider = 9; // Burda diveder kernel toplamı olabilir.

            for (int z = -1 * kernel_size + 1; i < kernel_size; i++)
            {
                for (int w = -1 * kernel_size + 1; i < kernel_size; i++)
                {
                    if (j + w < 0 || j + w >= src.cols || i + z < 0 || i + z >= src.rows)
                    {
                        continue;
                    }
                    else
                    {
                        tot += dst_kernel.at<float>(z, w) * src.at<Vec3b>(i + z, j + w);
                    }
                }
            }
            tot /= divider;
            dst.at<Vec3b>(i, j) = tot;
        }
    }

    imshow("name", dst);
    waitKey(0);
    destroyAllWindows();
}