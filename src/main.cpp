#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using std::cin;
using std::cout;
using std::endl;
using std::vector;

void distance_kernel(Mat &kernel, int kernel_size);
void bilateral_filter(Mat &src, Mat &dst, Mat &kernel, int kernel_size);

float SIGMA = 75;
int KERNEL_SIZE = 5;
float SMALL_NUMBER = 0.00000000001;

int main(int argc, char const *argv[])
{
    Mat image = imread("./inpimage.png", IMREAD_COLOR);
    Mat dst = image.clone();
    Mat kernel = Mat::ones(KERNEL_SIZE, KERNEL_SIZE, CV_32FC1);
    distance_kernel(kernel, KERNEL_SIZE);
    cout << kernel << endl;
    bilateral_filter(image, dst, kernel, KERNEL_SIZE);

    return 0;
}

void distance_kernel(Mat &kernel, int kernel_size)
{
    int mid = kernel_size / 2;
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
            kernel.at<float>(i, j) = gauss_distance;
        }
    }
    cout << kernel << endl;
}

void color_kernel(Mat &src, Mat &kernel, int kernel_size, Point main_point)
{
    float r, g, b;
    float m_r, m_g, m_b;
    float color_diff, gauss_distance;
    float w = 0;
    int main = kernel_size / 2;
    Vec3b real_color = src.at<Vec3b>(main_point.x, main_point.y);
    Vec3b other_color;
    m_r = real_color[0];
    m_g = real_color[1];
    m_b = real_color[2];
    for (int i = -1 * main; i <= main; i++)
    {
        for (int j = -1 * main; j <= main; j++)
        {
            if (j + main_point.y < 0 || j + main_point.y >= src.cols || i + main_point.x < 0 || i + main_point.x >= src.rows)
            {
            }
            else
            {
                other_color = src.at<Vec3b>(main_point.x + i, main_point.y + j);
                r = other_color[0];
                g = other_color[1];
                b = other_color[2];

                color_diff = (pow(r - m_r, 2) + pow(g - m_g, 2) + pow(b - m_b, 2)) / (2 * pow(SIGMA, 2));
                gauss_distance = exp(-1 * color_diff);
                kernel.at<float>(i + main, j + main) = gauss_distance;
            }
        }
    }
}

void mat_multiplier(Mat &src, Mat &other)
{
    for (int i = 0; i < src.cols; i++)
    {
        for (int j = 0; j < src.rows; j++)
        {
            src.at<float>(i, j) *= other.at<float>(i, j);
        }
    }
}

void bilateral_filter(Mat &src, Mat &dst, Mat &kernel, int kernel_size)
{

    Mat dst_kernel = kernel.clone();
    Mat clr_kernel = kernel.clone();

    cout << kernel << endl;
    int main = kernel_size / 2;
    int x, y;
    int counter = 0;
    for (int i = 0; i < src.cols; i++)
    {
        for (int j = 0; j < src.rows; j++)
        {
            dst_kernel = kernel.clone();
            color_kernel(src, clr_kernel, kernel_size, Point(i, j));
            mat_multiplier(dst_kernel, clr_kernel);
            counter++;
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

                        tot[0] += (int)(ml * ex_color[0]);
                        tot[1] += (int)(ml * ex_color[1]);
                        tot[2] += (int)(ml * ex_color[2]);
                        divider += ml;
                    }
                }
            }

            dst.at<Vec3b>(i, j) = Vec3b(tot[0] / divider, tot[1] / divider, tot[2] / divider);
        }
    }
    imshow("new_image", dst);
    imshow("ex_image", src);
    waitKey(0);
    destroyAllWindows();
}