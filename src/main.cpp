#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using std::cin;
using std::cout;
using std::endl;

void distance_kernel(Mat &kernel, int kernel_size);
float kernel_summer(Mat kernel, int kernel_size);
void kernel_fixer(Mat &src, Mat &kernel, Mat &dst_kernel, int kernel_size, int i, int j);
void bilateral_filter(Mat &src, Mat &dst, Mat &kernel, int kernel_size);
Vec3b subtractor(Vec3b &f, Vec3b &s);

float MAX_COLOR_SUM = 765;
int DIFF_EFFECT = 2;
int COLOR_EFFECT = 8;
int DELIMITIER = DIFF_EFFECT + COLOR_EFFECT;

int main(int argc, char const *argv[])
{
    int kernel_size = 5;
    Mat image = imread("./inpimage.png", IMREAD_COLOR);
    Mat dst = image.clone();
    Mat kernel = Mat::ones(kernel_size, kernel_size, CV_32F);
    distance_kernel(kernel, kernel_size);
    // cout << kernel << endl;
    bilateral_filter(image, dst, kernel, kernel_size);

    return 0;
}

void distance_kernel(Mat &kernel, int kernel_size)
{
    int mid = kernel_size / 2;
    float multiplier = DIFF_EFFECT / sqrt(2 * mid * mid); // this is for maximum value of anyoklidian distance we will scale this 0 to 5
    for (int i = 0; i < kernel.cols; i++)
    {
        for (int j = 0; j < kernel.rows; j++)
        {
            float oklidian_distance = sqrt(pow(i - mid, 2) + pow(j - mid, 2));
            kernel.at<float>(i, j) = DIFF_EFFECT - oklidian_distance * multiplier;
        }
    }
    kernel = kernel / DELIMITIER;
    cout << kernel << endl;
}

float kernel_summer(Mat &kernel, int kernel_size)
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

Vec3b subtractor(Vec3b &f, Vec3b &s)
{
    return Vec3b(abs(f[0] - s[0]), abs(f[1] - s[1]), abs(f[2] - s[2]));
}

void kernel_fixer(Mat &src, Mat &kernel, Mat &dst_kernel, int kernel_size, int i, int j)
{
    Vec3b main_point = src.at<Vec3b>(i, j);
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
            iterator = subtractor(src.at<Vec3b>(x + i, y + j), main_point);
            // cout << src.at<Vec3b>(x + i, y + j) << "and" << main_point << " " << iterator << endl;
            total = iterator[0] + iterator[1] + iterator[2];
            dst_kernel.at<float>(x, y) = kernel.at<float>(x, y) + (COLOR_EFFECT - COLOR_EFFECT * (total / MAX_COLOR_SUM)) / DELIMITIER;
            // cout << kernel.at<float>(x, y) << " and right side : " << (5 - 4 * (total / MAX_COLOR_SUM)) / 10 << " total : " << total << endl;
        }
    }
    // cout << dst_kernel << endl;
}

void bilateral_filter(Mat &src, Mat &dst, Mat &kernel, int kernel_size)
{
    // 5- 4* (sums/765)
    Mat dst_kernel = kernel.clone();
    cout << kernel << endl;

    int x, y;
    int counter = 0;
    for (int i = 0; i < src.cols; i++)
    {
        for (int j = 0; j < src.rows; j++)
        {
            kernel_fixer(src, kernel, dst_kernel, kernel_size, i, j);
            // cout << kernel << endl;
            Vec3b tot = Vec3b(0, 0, 0);
            double divider = 9; // Burda diveder kernel toplamı olabilir.
            for (int z = -1 * kernel_size + 1; z < kernel_size; z++)
            {
                for (int w = -1 * kernel_size + 1; w < kernel_size; w++)
                {
                    if (j + w < 0 || j + w >= src.cols || i + z < 0 || i + z >= src.rows)
                    {
                        continue;
                    }
                    else
                    {
                        Vec3b ex_color = src.at<Vec3b>(i + z, j + w);
                        float ml = dst_kernel.at<float>(z, w);
                        if (z != 0 && w != 0)
                        {
                            ml /= 100;
                        }
                        tot[0] += ml * ex_color[0];
                        tot[1] += ml * ex_color[1];
                        tot[2] += ml * ex_color[2];
                        // cout << "tot:  " << tot << "\ndst " << dst_kernel.at<float>(z, w) << "\n"
                        //      << src.at<Vec3b>(i + z, j + w) << endl;
                    }
                }
            }
            // divider = sum(dst_kernel)[0];
            // cout << src.at<Vec3b>(i, j) << "\ntot : " << tot << endl;
            // cout << dst_kernel << endl;
            dst.at<Vec3b>(i, j) = tot;
        }
    }
    imshow("name", dst);
    waitKey(0);
    destroyAllWindows();
}