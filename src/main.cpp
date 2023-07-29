#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using std::cin;
using std::cout;
using std::endl;
using std::vector;

void distance_kernel(Mat &kernel, int kernel_size);
float kernel_summer(Mat &kernel, int kernel_size);
void kernel_fixer(Mat &src, Mat &kernel, Mat &dst_kernel, int kernel_size, int i, int j);
void bilateral_filter(Mat &src, Mat &dst, Mat &kernel, int kernel_size);
Vec3b subtractor(Vec3b &f, Vec3b &s);

float MAX_COLOR_SUM = 765;
float SIGMA = 75;
int DIFF_EFFECT = 5;
int COLOR_EFFECT = 0;
int DELIMITIER = DIFF_EFFECT + COLOR_EFFECT;
int KERNEL_SIZE = 11;
float SMALL_NUMBER = 0.00000000001;

int main(int argc, char const *argv[])
{
    Mat image = imread("./inpimage.png", IMREAD_COLOR);
    Mat dst = image.clone();
    Mat kernel = Mat::ones(KERNEL_SIZE, KERNEL_SIZE, CV_32F);
    distance_kernel(kernel, KERNEL_SIZE);
    // cout << kernel << endl;
    bilateral_filter(image, dst, kernel, KERNEL_SIZE);
    // filter2D(image, dst, -1, kernel);
    // imshow("new_image", dst);
    // imshow("ex_image", image);
    // waitKey(0);
    // destroyAllWindows();

    return 0;
}

void distance_kernel(Mat &kernel, int kernel_size)
{
    int mid = kernel_size / 2;
    // float multiplier = DIFF_EFFECT / sqrt(2 * mid * mid); // this is for maximum value of anyoklidian distance we will scale this 0 to 5
    float gauss_distance;

    float power_g;

    float w = 0;
    for (int i = 0; i < kernel.cols; i++)
    {
        for (int j = 0; j < kernel.rows; j++)
        {
            // gauss : e^((x2-x1)^2+...)/sigma)
            power_g = (pow(i - mid, 2) + pow(j - mid, 2)) / (2 * pow(SIGMA, 2));

            gauss_distance = exp(-1 * power_g);
            // float oklidian_distance = sqrt(pow(i - mid, 2) + pow(j - mid, 2));
            kernel.at<float>(i, j) = gauss_distance;
        }
    }
}

// void color_kernel(Mat &src, Mat &kernel, int kernel_size, Point main_point)
// {
//     Vec3b main_color = src.at<Vec3b>(main_point);
//     int half_kernel = kernel_size / 2;

//     for (int i = -1 * half_kernel; i <= half_kernel; i++)
//     {
//         for (int j = 0; j < kernel.rows; j++)
//         {
//             // gauss : e^((x2-x1)^2+...)/sigma)
//             power_g = (pow(i - mid, 2) + pow(j - mid, 2)) / (2 * pow(SIGMA, 2));

//             gauss_distance = exp(-1 * power_g);
//             // float oklidian_distance = sqrt(pow(i - mid, 2) + pow(j - mid, 2));
//             kernel.at<float>(i, j) = gauss_distance * DIFF_EFFECT;
//             w += gauss_distance;
//         }
//     }
// }

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
            dst_kernel.at<float>(x, y) = kernel.at<float>(x, y) * (COLOR_EFFECT - COLOR_EFFECT * (total / MAX_COLOR_SUM)) / DELIMITIER;
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
    int main = kernel_size / 2;
    int x, y;
    int counter = 0;
    for (int i = 0; i < src.cols; i++)
    {
        for (int j = 0; j < src.rows; j++)
        {
            // kernel_fixer(src, kernel, dst_kernel, kernel_size, i, j);
            // cout << kernel << endl;
            vector<int> tot = vector({0, 0, 0});
            float divider = 0;
            for (int z = -1 * main; z <= main; z++)
            {
                for (int w = -1 * main; w <= main; w++)
                {
                    if (j + w < 0 || j + w >= src.cols || i + z < 0 || i + z >= src.rows)
                    {
                    }
                    else
                    {
                        Vec3b ex_color = src.at<Vec3b>(i + z, j + w);
                        float ml = dst_kernel.at<float>(z + main, w + main);
                        // cout << z + main << " " << w + main << " " << main << " " << z << " " << w << " " << ml << endl;
                        // cout << "ex tot:  " << tot[0] << " " << tot[1] << " " << tot[2] << endl;
                        // cout << "ex color:" << ex_color << ml * ex_color[0] << endl;
                        tot[0] += (int)(ml * ex_color[0]);
                        tot[1] += (int)(ml * ex_color[1]);
                        tot[2] += (int)(ml * ex_color[2]);
                        divider += ml;
                        // cout << "tot:  " << tot[0] << " " << tot[1] << " " << tot[2] << endl;
                        //      << src.at<Vec3b>(i + z, j + w) << endl;
                    }
                }
            }
            // cout << divider << endl;
            // cout << tot << " " << tot / divider << endl;

            dst.at<Vec3b>(i, j) = Vec3b(tot[0] / divider, tot[1] / divider, tot[2] / divider);
        }
    }
    imshow("new_image", dst);
    imshow("ex_image", src);
    waitKey(0);
    destroyAllWindows();
}